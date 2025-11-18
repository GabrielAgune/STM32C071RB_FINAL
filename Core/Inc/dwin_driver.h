/*
 * Nome do Arquivo: dwin_driver.h
 * Descrição: Interface pública do driver não-bloqueante para o Display DWIN
 * Autor: Gabriel Agune
 */

#ifndef __DRIVER_DWIN_H
#define __DRIVER_DWIN_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Defines
// ============================================================

#define DWIN_RX_BUFFER_SIZE     64u
#define DWIN_UART_TIMEOUT_MS    100u

// ============================================================
// Constantes de Comandos
// ============================================================

static const uint8_t CMD_AJUSTAR_BACKLIGHT_10[]  = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x82, 0x0A, 0x00};
static const uint8_t CMD_AJUSTAR_BACKLIGHT_100[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x82, 0x64, 0x00};

// ============================================================
// Endereços VP (Variáveis)
// ============================================================

enum {
    // Variáveis Globais
    VP_DATA_HORA            = 0x0010,
    VP_FIRMWARE             = 0x1000,
    VP_HARDWARE             = 0x1010,
    VP_FIRM_IHM             = 0x1020,
    VP_SERIAL               = 0x1030,
    VP_ICON_BAT             = 0x1100,
    VP_REGRESSIVA           = 0x1500,

    // Data e Hora
    HORA_SISTEMA            = 0x2000,
    DATA_SISTEMA            = 0x2010,

    // Relatório de Medidas
    GRAO_A_MEDIR            = 0x2070,
    UMIDADE_1_CASA          = 0x2100,
    UMIDADE_2_CASAS         = 0x2100,
    TEMP_SAMPLE             = 0x2110,
    DENSIDADE               = 0x2120,
    CURVA                   = 0x2130,
    AMOSTRAS                = 0x2140,
    UMI_MIN                 = 0x2150,
    UMI_MAX                 = 0x2160,
    DATA_VAL                = 0x2170,
    RESULTADO_MEDIDA        = 0x2180,

    // Variáveis de Sistema
    PESO                    = 0x2190,
    AD_BALANCA              = 0x2200,
    FAT_CAL_BAL             = 0x2210,
    AD_TEMP_SAMPLE          = 0x2220,
    TEMP_INSTRU             = 0x2230,
    AD_TEMP_INSTRU          = 0x2240,
    FREQUENCIA              = 0x2250,
    ESCALA_A                = 0x2260,
    PHOTDIODE               = 0x2270,
    GAVETA                  = 0x2280,

    // Monitoramento Elétrico
    VP_VBUS                 = 0x2290,
    VP_VBAT                 = 0x2300,
    VP_IBAT                 = 0x2310,
    VP_TEMP                 = 0x2320,
    VP_PERC                 = 0x2330,

    // Mensagens e Pesquisa
    VP_MESSAGES             = 0x4096,
    VP_SEARCH_INPUT         = 0x8100,

    // Resultados de Pesquisa
    VP_RESULT_NAME_1        = 0x8200,
    VP_RESULT_NAME_2        = 0x8220,
    VP_RESULT_NAME_3        = 0x8240,
    VP_RESULT_NAME_4        = 0x8260,
    VP_RESULT_NAME_5        = 0x8280,
    VP_RESULT_NAME_6        = 0x8300,
    VP_RESULT_NAME_7        = 0x8320,
    VP_RESULT_NAME_8        = 0x8340,
    VP_RESULT_NAME_9        = 0x8360,
    VP_RESULT_NAME_10       = 0x8380,

    VP_RESULT_SELECT        = 0x8400,
    VP_PAGE_INDICATOR       = 0x8500,
};

// ============================================================
// Endereços de Controle (Botões)
// ============================================================

enum {
    // Tela Principal
    WAKEUP_CONFIRM_BTN      = 0x1900,
    OFF                     = 0x2020,
    SENHA_CONFIG            = 0x2030,
    SELECT_GRAIN            = 0x2040,
    PRINT                   = 0x2050,
    DESCARTA_AMOSTRA        = 0x2060,
    SHOW_MEDIDA             = 0x2090,

    // Menu Configuração
    SET_TIME                = 0x300F,
    ENTER_SET_TIME          = 0x3010,
    NR_REPETICOES           = 0x3020,
    DECIMALS                = 0x3030,
    DES_HAB_PRINT           = 0x3040,
    SET_SENHA               = 0x3060,
    DIAGNOSTIC              = 0x3070,
    USER                    = 0x3080,
    COMPANY                 = 0x3090,
    ABOUT_SYS               = 0x3100,

    // Menu Serviço
    TECLAS                  = 0x4080,
    ESCAPE                  = 0x5000,
    PRESET_PRODUCT          = 0x7010,
    SET_DATE_TIME           = 0x7020,
    MODEL_OEM               = 0x7030,
    ADJUST_SCALE            = 0x7040,
    ADJUST_TERMO            = 0x7050,
    ADJUST_CAPA             = 0x7060,
    SET_SERIAL              = 0x7070,
    SET_UNITS               = 0x7080,
    MONITOR                 = 0x7090,
    SERVICE_REPORT          = 0x7100,
    SYSTEM_BURNIN           = 0x7110,
    BATTERY_INFORMATION     = 0x7120,
};

// ============================================================
// IDs de Tela (PIC)
// ============================================================

enum {
    // Boot
    LOGO                    = 0,
    BOOT_CHECK_SERVOS       = 1,
    BOOT_CHECK_CAPACI       = 2,
    BOOT_BALANCE            = 3,
    BOOT_THERMOMETER        = 4,
    BOOT_MEMORY             = 5,
    BOOT_CLOCK              = 6,
    BOOT_CRIPTO             = 7,

    // Operação Normal
    PRINCIPAL               = 8,
    SYSTEM_STANDBY          = 11,
    MEDE_ENCHE_CAMARA       = 13,
    MEDE_AJUSTANDO          = 14,
    MEDE_RASPA_CAMARA       = 15,
    MEDE_PESO_AMOSTRA       = 16,
    MEDE_TEMP_SAMPLE        = 17,
    MEDE_UMIDADE            = 18,
    MEDE_RESULT_01          = 19,
    MEDE_REPETICAO          = 21,
    MEDE_PRINT_REPORT       = 22,
    TELA_CONFIRM_WAKEUP     = 99,
    TELA_PESQUISA           = 101,
    SELECT_GRAO             = 102,
    TELA_BATERIA            = 104,
    MEDE_RESULT_02          = 119,

    // Configurações
    TELA_CONFIGURAR         = 23,
    TELA_SET_JUST_TIME      = 25,
    TELA_SETUP_REPETICOES   = 26,
    TELA_SET_DECIMALS       = 27,
    TELA_SET_COPIES         = 28,
    TELA_SET_BRIGHT         = 29,
    TELA_SET_PASSWORD       = 30,
    TELA_SET_PASS_AGAIN     = 31,
    TELA_AUTO_DIAGNOSIS     = 32,
    TELA_ABOUT_SYSTEM       = 33,
    TELA_USER               = 34,
    TELA_COMPANY            = 35,

    // Serviço
    TELA_SERVICO            = 46,
    TELA_PRESET_PRODUCT     = 48,
    TELA_ADJUST_TIME        = 49,
    TELA_MODEL_OEM          = 50,
    TELA_ADJUST_SCALE       = 51,
    TELA_ADJUST_TERMO       = 52,
    TELA_ADJUST_CAPA        = 53,
    TELA_SET_SERIAL         = 54,
    TELA_SET_UNITS          = 55,
    TELA_MONITOR_SYSTEM     = 56,
    TELA_REPORT_SERV        = 57,
    TELA_BURNIN             = 58,

    // Mensagens de Erro/Alerta
    MSG_ERROR               = 59,
    MSG_ALERTA              = 60,
    ERROR_GAVETA_MISS       = 61,
    SENHA_ERRADA            = 62,
    SENHA_MIN_4_CARAC       = 63,
    SENHAS_DIFERENTES       = 64,
};

// ============================================================
// Typedefs
// ============================================================

typedef void (*dwin_rx_callback_t)(const uint8_t *buffer, uint16_t len);

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

void     DWIN_Driver_Init(UART_HandleTypeDef *huart, dwin_rx_callback_t callback);
void     DWIN_Driver_Process(void);
uint32_t DWIN_Driver_GetRxPacketCounter(void);

bool     DWIN_Driver_SetScreen(uint16_t screen_id);
bool     DWIN_Driver_WriteInt(uint16_t vp_address, int16_t value);
bool     DWIN_Driver_WriteInt32(uint16_t vp_address, int32_t value);
bool     DWIN_Driver_WriteString(uint16_t vp_address, const char *text, uint16_t max_len);
bool     DWIN_Driver_Write_QR_String(uint16_t vp_address, const char *text, uint16_t max_len);
bool     DWIN_Driver_WriteRawBytes(const uint8_t *data, uint16_t size);

void     DWIN_Driver_HandleRxEvent(uint16_t size);
void     DWIN_Driver_HandleError(UART_HandleTypeDef *huart);

#endif /* __DRIVER_DWIN_H */