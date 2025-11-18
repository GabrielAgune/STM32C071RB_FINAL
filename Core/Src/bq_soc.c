/*
 * Nome do Arquivo: bq_soc.c
 * Descrição: Implementação do algoritmo de Coulomb Counting e Filtros
 * Autor: Gabriel Agune
 */

#include "bq_soc.h"
#include "bq25622_driver.h"
#include <stddef.h>
#include <math.h>

// ============================================================
// Defines e Constantes
// ============================================================

#define UPDATE_INTERVAL_MS      1000
static const float UPDATE_INTERVAL_HOURS = (float)UPDATE_INTERVAL_MS / 3600000.0f;
static const float CURRENT_DEADBAND_A    = 0.008f;

// ============================================================
// Typedefs e Estruturas
// ============================================================

typedef struct {
    float voltage;
    float percentage;
} SocPoint;

// ============================================================
// Variáveis Privadas
// ============================================================

// Tabela de estimativa inicial (Ordem Decrescente de Tensão)
static const SocPoint soc_table[] = {
    { 4.20f, 100.0f },
    { 4.10f,  90.0f },
    { 4.00f,  80.0f },
    { 3.90f,  70.0f },
    { 3.80f,  60.0f },
    { 3.70f,  40.0f },
    { 3.60f,  20.0f },
    { 3.50f,  10.0f },
    { 3.30f,   5.0f },
    { 3.00f,   0.0f }
};

static const size_t soc_table_size = sizeof(soc_table) / sizeof(SocPoint);

// Variáveis de Estado do Algoritmo
static float             g_total_capacity_mAh    = 210.0f;
static float             g_capacidade_atual_mAh  = 0.0f;
static volatile uint32_t g_systick_counter       = 0;
static volatile uint8_t  g_update_soc_flag       = 0;

// Cache de Leituras Recentes
static float            g_last_vbat             = 0.0f;
static float            g_last_vbus             = 0.0f;
static float            g_last_ibat             = 0.0f;
static float            g_last_tdie             = 0.0f;
static BQ25622_ChargeStatus_t g_last_chg_status = CHG_STAT_NOT_CHARGING;

// ============================================================
// Funções Privadas
// ============================================================

// Realiza interpolação linear entre dois pontos
static float linear_interpolate(float x, float x0, float y0, float x1, float y1) {
    if (x1 == x0) {
        return y0;
    }
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

// Estima a porcentagem inicial baseada apenas na tensão (Lookup Table)
static float bq_soc_estimate_percentage_from_voltage(float vbat_V) {
    if (vbat_V >= soc_table[0].voltage) {
        return 100.0f;
    }
    if (vbat_V <= soc_table[soc_table_size - 1].voltage) {
        return 0.0f;
    }

    for (size_t i = 0; i < soc_table_size - 1; i++) {
        const SocPoint *p_upper = &soc_table[i];
        const SocPoint *p_lower = &soc_table[i + 1];

        if (vbat_V >= p_lower->voltage) {
            return linear_interpolate(vbat_V,
                                      p_lower->voltage, p_lower->percentage,
                                      p_upper->voltage, p_upper->percentage);
        }
    }
    return 0.0f;
}

// ============================================================
// Funções Públicas
// ============================================================

// Callback do timer do sistema para agendar atualizações
void bq_soc_systick_callback(void) {
    g_systick_counter++;
    if (g_systick_counter >= UPDATE_INTERVAL_MS) {
        g_systick_counter = 0;
        g_update_soc_flag = 1;
    }
}

// Inicializa o módulo, definindo capacidade total e estimativa inicial
void bq_soc_coulomb_init(I2C_HandleTypeDef *hi2c, uint16_t battery_capacity_mah) {
    g_total_capacity_mAh = (float)battery_capacity_mah;
    float vbat_inicial = 0.0f;

    if (bq25622_read_vbat(hi2c, &vbat_inicial) == HAL_OK) {
        g_last_vbat = vbat_inicial;
        float perc_inicial = bq_soc_estimate_percentage_from_voltage(vbat_inicial);
        g_capacidade_atual_mAh = (perc_inicial / 100.0f) * g_total_capacity_mAh;
    } else {
        // Falha na leitura: assume 50%
        g_capacidade_atual_mAh = g_total_capacity_mAh / 2.0f;
    }

    // Leituras iniciais de cache
    bq25622_read_vbus(hi2c, &g_last_vbus);
    bq25622_read_charge_status(hi2c, &g_last_chg_status);
    bq25622_read_die_temp(hi2c, &g_last_tdie);

    g_systick_counter = 0;
    g_update_soc_flag = 0;
}

// Executa a integração de corrente (Coulomb Counting)
void bq_soc_coulomb_update(I2C_HandleTypeDef *hi2c) {
    if (!g_update_soc_flag) {
        return;
    }
    g_update_soc_flag = 0;

    float vbus_now, vbat_now, ibat_now_raw, tdie_now;
    BQ25622_ChargeStatus_t status_now;

    // 1. Coleta de Dados
    bq25622_read_vbus(hi2c, &vbus_now);
    bq25622_read_vbat(hi2c, &vbat_now);
    bq25622_read_ibat(hi2c, &ibat_now_raw);
    bq25622_read_charge_status(hi2c, &status_now);
    bq25622_read_die_temp(hi2c, &tdie_now);

    // 2. Filtragem de Corrente (Deadband)
    float ibat_now = ibat_now_raw;
    if (fabs(ibat_now) < CURRENT_DEADBAND_A) {
        ibat_now = 0.0f;
    }

    // 3. Lógica de Integração
    // Se estiver conectado ao carregador, carga cheia e tensão alta: força 100%
    if (vbus_now > 4.5f && status_now == CHG_STAT_NOT_CHARGING && vbat_now > 4.15f) {
        g_capacidade_atual_mAh = g_total_capacity_mAh;
        ibat_now = 0.0f;
    } else {
        float ibat_mA = ibat_now * 1000.0f;
        float delta_mAh = ibat_mA * UPDATE_INTERVAL_HOURS;
        g_capacidade_atual_mAh += delta_mAh;
    }

    // 4. Clamp (Limites de Segurança)
    if (g_capacidade_atual_mAh > g_total_capacity_mAh) {
        g_capacidade_atual_mAh = g_total_capacity_mAh;
    }
    if (g_capacidade_atual_mAh < 0.0f) {
        g_capacidade_atual_mAh = 0.0f;
    }

    // 5. Atualização do Cache Global
    g_last_vbus = vbus_now;
    g_last_vbat = vbat_now;
    g_last_ibat = ibat_now;
    g_last_chg_status = status_now;
    g_last_tdie = tdie_now;
}

// Retorna a porcentagem calculada
float bq_soc_get_percentage(void) {
    if (g_total_capacity_mAh <= 0.0f) {
        return 0.0f;
    }
    return (g_capacidade_atual_mAh / g_total_capacity_mAh) * 100.0f;
}

// Retorna última leitura de Vbat
float bq_soc_get_last_vbat(void) {
    return g_last_vbat;
}

// Retorna última leitura de Vbus
float bq_soc_get_last_vbus(void) {
    return g_last_vbus;
}

// Retorna última leitura de Ibat
float bq_soc_get_last_ibat(void) {
    return g_last_ibat;
}

// Retorna último status de carga
BQ25622_ChargeStatus_t bq_soc_get_last_chg_status(void) {
    return g_last_chg_status;
}

// Retorna última leitura de temperatura
float bq_soc_get_last_tdie(void) {
    return g_last_tdie;
}