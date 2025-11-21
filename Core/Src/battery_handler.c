/*
 * Nome do Arquivo: battery_handler.c
 * Descrição: Implementação da lógica de gerenciamento de bateria e ícones
 * Autor: Gabriel Agune
 */

#include "battery_handler.h"
#include "bq25622_driver.h"
#include "bq_soc.h"
#include "dwin_driver.h"
#include "controller.h"
#include "cli_driver.h"
#include <stdio.h>

// ============================================================
// Variáveis Privadas
// ============================================================

static I2C_HandleTypeDef *s_hi2c             = NULL;
static uint32_t           s_last_update_tick = 0;
static int16_t            s_last_icon_id     = -1;

// Constante de intervalo de atualização da tela (1 segundo)
static const uint32_t SCREEN_UPDATE_INTERVAL_MS = 1000;

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static void    update_battery_screen_data(void);
static int16_t get_icon_id_from_status(void);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa todo o subsistema da bateria
void Battery_Handler_Init(I2C_HandleTypeDef *hi2c) {
    s_hi2c = hi2c;
    uint8_t device_id = 0;

    // 1. Validação de comunicação
    if (bq25622_validate_comm(s_hi2c, &device_id) != HAL_OK || device_id != 0x0A) {
        CLI_Printf("BATERIA: FALHA na comunicacao com BQ25622!\r\n");
        s_hi2c = NULL;
        return;
    }
    CLI_Printf("BATERIA: BQ25622 detectado. ID: 0x%02X\r\n", device_id);

    // 2. Configuração do Charger
    if (bq25622_init(s_hi2c, BATTERY_CAPACITY_MAH) != HAL_OK) {
        CLI_Printf("BATERIA: FALHA ao configurar parametros do BQ25622.\r\n");
        return;
    }

    // 3. Habilitação do ADC
    if (bq25622_adc_init(s_hi2c) != HAL_OK) {
        CLI_Printf("BATERIA: FALHA ao habilitar ADC do BQ25622.\r\n");
        return;
    }

    // 4. Inicialização do SoC (Coulomb Counter)
    bq_soc_coulomb_init(s_hi2c, BATTERY_CAPACITY_MAH);
    CLI_Printf("BATERIA: Handler inicializado para %dmAh. SoC inicial: %.1f%%\r\n",
               BATTERY_CAPACITY_MAH, bq_soc_get_percentage());
}

// Função de processamento cíclico
void Battery_Handler_Process(void) {
    if (s_hi2c == NULL) {
        return;
    }

    // Atualiza o cálculo do SoC a cada ciclo (controlado internamente pelo bq_soc)
    bq_soc_coulomb_update(s_hi2c);

    // Atualiza a interface gráfica a cada intervalo definido
    if (HAL_GetTick() - s_last_update_tick >= SCREEN_UPDATE_INTERVAL_MS) {
        s_last_update_tick = HAL_GetTick();

        // Determina o ícone atual baseado no estado
        int16_t current_icon_id = get_icon_id_from_status();

        // Atualiza o display apenas se o ícone mudou (otimização de bus)
        if (current_icon_id != s_last_icon_id) {
            DWIN_Driver_WriteInt(VP_ICON_BAT, current_icon_id);
            s_last_icon_id = current_icon_id;
        }

        // Se estiver na tela de diagnóstico de bateria, atualiza dados em tempo real
        if (Controller_GetCurrentScreen() == TELA_BATERIA) {
            update_battery_screen_data();
        }
    }
}

// ============================================================
// Funções Privadas
// ============================================================

// Mapeia o estado da bateria para um ID de ícone DWIN
static int16_t get_icon_id_from_status(void) {
    float vbus = bq_soc_get_last_vbus();

    // Se VBUS > 4.5V, carregador conectado -> Ícone de Raio/Plug
    if (vbus > 4.5f) {
        return 4;
    }

    // Mapeamento de SoC para barras de bateria
    float soc = bq_soc_get_percentage();

    if (soc > 85.0f) {
        return 3; // Cheia (4 barras)
    } else if (soc > 50.0f) {
        return 2; // 3/4
    } else if (soc > 30.0f) {
        return 1; // 1/2
    } else if (soc > 15.0f) {
        return 0; // 1/4 (Critica)
    } else {
        return -1; // Vazia/Desligando
    }
}

// Envia dados detalhados para a tela de diagnóstico
static void update_battery_screen_data(void) {
    // Leitura dos dados em cache
    float vbus = bq_soc_get_last_vbus();
    float vbat = bq_soc_get_last_vbat();
    float ibat = bq_soc_get_last_ibat();
    float tdie = bq_soc_get_last_tdie();
    float perc = bq_soc_get_percentage();

    // Conversão para formato de ponto fixo (inteiros) para o DWIN
    int32_t vbus_dwin = (int32_t)(vbus * 1000.0f);
    int32_t vbat_dwin = (int32_t)(vbat * 1000.0f);
    int32_t ibat_dwin = (int32_t)(ibat * 10000.0f); // Escala maior para precisão de mA
    int32_t tdie_dwin = (int32_t)(tdie * 10.0f);
    int32_t perc_dwin = (int32_t)(perc * 10.0f);

    // Envio para os VPs
    DWIN_Driver_WriteInt32(VP_VBUS, vbus_dwin);
    DWIN_Driver_WriteInt32(VP_VBAT, vbat_dwin);
    DWIN_Driver_WriteInt32(VP_IBAT, ibat_dwin);
    DWIN_Driver_WriteInt32(VP_TEMP, tdie_dwin);
    DWIN_Driver_WriteInt32(VP_PERC, perc_dwin);
}