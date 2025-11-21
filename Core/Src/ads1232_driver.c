/*
 * Nome do Arquivo: ads1232_driver.c
 * Descrição: Implementação do driver ADS1232 com suporte a simulação
 * Autor: Gabriel Agune
 */

#include "ads1232_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Configurações
// ============================================================
#define ADS1232_SIMULATION_MODE 1  // 0 = Hardware Real
#define MEDIAN_FILTER_SIZE      3  // Número de amostras para mediana

// ============================================================
// Variáveis Privadas
// ============================================================
static ADS1232_State_t s_state = ADS1232_STATE_POWER_DOWN;
static int32_t         s_cal_zero_adc = 0;
static int32_t         s_adc_offset   = 0;
static float           s_final_grams  = 0.0f;
static bool            s_new_data_available = false;

// Buffer para filtro de mediana
static int32_t s_sample_buffer[MEDIAN_FILTER_SIZE];
static uint8_t s_sample_count = 0;

// Flag volátil da interrupção
volatile bool g_ads_data_ready_irq = false;

// Variáveis para simulação
static uint32_t s_sim_last_tick = 0;

// ============================================================
// Variáveis Globais
// ============================================================
CalPoint_t cal_points[NUM_CAL_POINTS] = {
    {0.0f,   235469},
    {50.0f,  546061},
    {100.0f, 856428},
    {200.0f, 1477409}
};

// ============================================================
// Protótipos Privados
// ============================================================
static int32_t ReadRawSPI(void);
static void    AddSampleAndFilter(int32_t raw_val);
static int32_t CalculateMedian(void);
static float   ConvertToGrams(int32_t raw_val);
static int     CompareInt32(const void * a, const void * b);

// ============================================================
// Funções Públicas
// ============================================================

void ADS1232_Init(void) {
    // Carrega valor inicial de calibração
    s_cal_zero_adc = cal_points[0].adc_value;
    
    // Inicia desligado para economizar energia
    ADS1232_PowerDown();
}

void ADS1232_PowerUp(void) {
    if (s_state == ADS1232_STATE_POWER_DOWN) {
        #if ADS1232_SIMULATION_MODE == 0
        // Sequência de Power-Up conforme datasheet [cite: 1301]
        HAL_GPIO_WritePin(AD_PDWN_BAL_GPIO_Port, AD_PDWN_BAL_Pin, GPIO_PIN_RESET);
        HAL_Delay(1); // Aguarda estabilizar (>10us)
        HAL_GPIO_WritePin(AD_PDWN_BAL_GPIO_Port, AD_PDWN_BAL_Pin, GPIO_PIN_SET);
        // O chip leva algum tempo para acordar e dar o primeiro DRDY
        #endif
        
        s_sample_count = 0; // Reinicia filtro
        s_state = ADS1232_STATE_STARTUP;
    }
}

void ADS1232_PowerDown(void) {
    #if ADS1232_SIMULATION_MODE == 0
    // Coloca pino PDWN em Low 
    HAL_GPIO_WritePin(AD_PDWN_BAL_GPIO_Port, AD_PDWN_BAL_Pin, GPIO_PIN_RESET);
    #endif
    s_state = ADS1232_STATE_POWER_DOWN;
    s_new_data_available = false;
}

// Callback da Interrupção (Chamar no HAL_GPIO_EXTI_Callback)
void Drv_ADS1232_DRDY_Callback(void) {
    // Apenas seta a flag, não processa nada aqui para não bloquear ISR
    g_ads_data_ready_irq = true;
}

// Processamento principal - Chamar no loop do App_Manager
void ADS1232_Process(void) {
    
    // Se estiver desligado, não faz nada
    if (s_state == ADS1232_STATE_POWER_DOWN) {
        return;
    }

    // Modo Simulação
    #if ADS1232_SIMULATION_MODE == 1
        if (HAL_GetTick() - s_sim_last_tick > 100) { // Simula 10 SPS
            s_sim_last_tick = HAL_GetTick();
            g_ads_data_ready_irq = true; 
        }
    #endif

    // Verifica se há dados prontos (flag setada pela ISR)
    if (g_ads_data_ready_irq) {
        g_ads_data_ready_irq = false; // Limpa flag

        // Se estava em startup, agora passa para leitura normal
        if (s_state == ADS1232_STATE_STARTUP) {
            s_state = ADS1232_STATE_READING;
        }

        // Lê o dado bruto via SPI (Bit-bang)
        int32_t raw = ReadRawSPI();
        
        // Adiciona ao buffer e filtra
        AddSampleAndFilter(raw);
    }
}

bool ADS1232_IsDataAvailable(void) {
    if (s_new_data_available) {
        s_new_data_available = false; // Consome o aviso
        return true;
    }
    return false;
}

float ADS1232_GetGrams(void) {
    return s_final_grams;
}

void ADS1232_SetTareCurrent(void) {
    // Usa a última mediana calculada como offset
    int32_t current_median = CalculateMedian(); 
    s_adc_offset = current_median;
}

int32_t ADS1232_GetOffset(void) {
    return s_adc_offset;
}

void ADS1232_SetOffset(int32_t new_offset) {
    s_adc_offset = new_offset;
}

// ============================================================
// Funções Privadas
// ============================================================

static int32_t ReadRawSPI(void) {
    #if ADS1232_SIMULATION_MODE == 1
    return 235000 + (rand() % 100); // Valor simulado com ruído
    #else
    uint32_t data = 0;

    // O DRDY já está baixo (pois a ISR disparou), podemos clockar
    // Gera 24 pulsos de clock
    for(int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_SET);
        // Delay pequeno se o MCU for muito rápido (>50Mhz), senão o GPIO toggle é lento o suficiente
        // O ADS1232 aceita até 5MHz no SCLK [cite: 1133]
        HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_RESET);
        
        data = data << 1;
        if(HAL_GPIO_ReadPin(AD_DOUT_BAL_GPIO_Port, AD_DOUT_BAL_Pin) == GPIO_PIN_SET) {
            data |= 1;
        }
    }
    
    // 25º Pulso para forçar DRDY/DOUT para High [cite: 1130]
    HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_RESET);

    // Extensão de sinal 24-bit para 32-bit
    if (data & 0x800000) {
        data |= 0xFF000000;
    }
    
    return (int32_t)data;
    #endif
}

static void AddSampleAndFilter(int32_t raw_val) {
    // Adiciona ao buffer (lógica de preenchimento inicial)
    if (s_sample_count < MEDIAN_FILTER_SIZE) {
        s_sample_buffer[s_sample_count++] = raw_val;
    } else {
        // Shift buffer (janela deslizante) ou Ring Buffer
        // Para simplicidade de 3 amostras, shift é rápido:
        s_sample_buffer[0] = s_sample_buffer[1];
        s_sample_buffer[1] = s_sample_buffer[2];
        s_sample_buffer[2] = raw_val;
    }

    // Só calcula se tivermos amostras suficientes
    if (s_sample_count == MEDIAN_FILTER_SIZE) {
        int32_t filtered_adc = CalculateMedian();
        s_final_grams = ConvertToGrams(filtered_adc);
        s_new_data_available = true;
    }
}

// Helper para qsort
static int CompareInt32(const void * a, const void * b) {
    return ( *(int32_t*)a - *(int32_t*)b );
}

static int32_t CalculateMedian(void) {
    int32_t temp_buf[MEDIAN_FILTER_SIZE];
    memcpy(temp_buf, s_sample_buffer, sizeof(temp_buf));
    
    // Ordena para pegar a mediana
    // Para 3 itens, if/else é mais rápido que qsort, mas qsort é genérico
    qsort(temp_buf, MEDIAN_FILTER_SIZE, sizeof(int32_t), CompareInt32);
    
    return temp_buf[1]; // Retorna o elemento do meio (índice 1 de 0..2)
}

static float ConvertToGrams(int32_t raw_value) {
    int32_t eff_adc = (raw_value - s_adc_offset) + s_cal_zero_adc;
    
    // Lógica de interpolação linear (mesma do original)
    for (int i = 0; i < NUM_CAL_POINTS - 1; i++) {
        int32_t x1 = cal_points[i].adc_value;
        int32_t x2 = cal_points[i+1].adc_value;

        if (eff_adc >= x1 && eff_adc <= x2) {
            float y1 = cal_points[i].grams;
            float y2 = cal_points[i+1].grams;
            float dx = (float)(x2 - x1);
            if (dx == 0.0f) return y1;
            float m = (y2 - y1) / dx;
            return y1 + m * (eff_adc - x1);
        }
    }
    // Extrapolação simples se passar do range
    return cal_points[NUM_CAL_POINTS-1].grams; // Simplificado para exemplo
}