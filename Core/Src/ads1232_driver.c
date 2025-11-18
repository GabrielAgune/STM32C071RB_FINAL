/*
 * Nome do Arquivo: ads1232_driver.c
 * Descrição: Implementação do driver ADS1232 com suporte a simulação
 * Autor: Gabriel Agune
 */

#include "ads1232_driver.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ============================================================
// Defines de Configuração
// ============================================================

/* * 1 = Modo Simulação (Não trava sem hardware)
 * 0 = Modo Normal (Hardware real necessário)
 */
#define ADS1232_SIMULATION_MODE 1

// ============================================================
// Variáveis Privadas
// ============================================================

static int32_t       s_cal_zero_adc     = 0;
static int32_t       s_adc_offset       = 0;
volatile bool        g_ads_data_ready   = false;

// ============================================================
// Variáveis Públicas (Globais)
// ============================================================

CalPoint_t cal_points[NUM_CAL_POINTS] = {
    {0.0f,   235469},
    {50.0f,  546061},
    {100.0f, 856428},
    {200.0f, 1477409}
};

// ============================================================
// Funções Privadas
// ============================================================

// Ordena três valores inteiros (auxiliar para filtro de mediana)
static void sort_three(int32_t *a, int32_t *b, int32_t *c) {
    int32_t temp;
    if (*a > *b) { temp = *a; *a = *b; *b = temp; }
    if (*b > *c) { temp = *b; *b = *c; *c = temp; }
    if (*a > *b) { temp = *a; *a = *b; *b = temp; }
}

// ============================================================
// Funções Públicas
// ============================================================

// Callback chamado pela ISR externa quando o pino DRDY é acionado
void Drv_ADS1232_DRDY_Callback(void) {
    g_ads_data_ready = true;
}

// Inicializa o ADS1232 (pulso de Power Down) e carrega calibração zero
void ADS1232_Init(void) {
    #if ADS1232_SIMULATION_MODE == 0
    HAL_GPIO_WritePin(AD_PDWN_BAL_GPIO_Port, AD_PDWN_BAL_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(AD_PDWN_BAL_GPIO_Port, AD_PDWN_BAL_Pin, GPIO_PIN_SET);
    #endif

    s_cal_zero_adc = cal_points[0].adc_value;
}

// Realiza a leitura via bit-banging do protocolo SPI do ADS1232
int32_t ADS1232_Read(void) {
    #if ADS1232_SIMULATION_MODE == 1
    return 235000; 
    #else
    uint32_t data = 0;

    // Gera 24 pulsos de clock para leitura
    for(int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_RESET);
    }
    
    // Pulso extra para forçar DRDY high
    HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AD_SCLK_BAL_GPIO_Port, AD_SCLK_BAL_Pin, GPIO_PIN_RESET);

    // Lê os bits (MSB first)
    for(int i = 0; i < 24; i++) {
        data = data << 1;
        if(HAL_GPIO_ReadPin(AD_DOUT_BAL_GPIO_Port, AD_DOUT_BAL_Pin) == GPIO_PIN_SET) {
            data |= 1;
        }
    }

    // Extensão de sinal para 32 bits se for negativo (24-bit signed)
    if (data & 0x800000) {
        data |= 0xFF000000;
    }
    
    return (int32_t)data;
    #endif
}

// Lê 3 amostras e retorna a mediana (filtro de ruído impulsivo)
int32_t ADS1232_Read_Median_of_3(void) {
    #if ADS1232_SIMULATION_MODE == 1
    HAL_Delay(50);
    return 235000;
    #else
    int32_t s1, s2, s3;

    while(!s_ads_data_ready) {}; s1 = ADS1232_Read(); s_ads_data_ready = false;
    while(!s_ads_data_ready) {}; s2 = ADS1232_Read(); s_ads_data_ready = false;
    while(!s_ads_data_ready) {}; s3 = ADS1232_Read(); s_ads_data_ready = false;

    sort_three(&s1, &s2, &s3);
    return s2;
    #endif
}

// Realiza a tara da balança buscando estabilidade nas leituras
int32_t ADS1232_Tare(void) {
    #if ADS1232_SIMULATION_MODE == 1
    printf("ADS1232: Tare em modo de simulacao.\r\n");
    s_adc_offset = 235469;
    return s_adc_offset;
    #else
    const int num_samples = 32;
    const int32_t stability_threshold = 300;
    int max_retries = 10;

    for (int retry = 0; retry < max_retries; retry++) {
        int64_t sum = 0;
        int32_t min_val = 0x7FFFFFFF;
        int32_t max_val = 0x80000000;

        for (int i = 0; i < num_samples; i++) {
            int32_t sample = ADS1232_Read_Median_of_3();
            sum += sample;
            
            if (sample < min_val) min_val = sample;
            if (sample > max_val) max_val = sample;
            
            HAL_Delay(10);
        }

        if ((max_val - min_val) < stability_threshold) {
            s_adc_offset = (int32_t)(sum / num_samples);
            return s_adc_offset;
        }
    }
    return s_adc_offset;
    #endif
}

// Converte ADCs brutos para gramas usando interpolação linear multiponto
float ADS1232_ConvertToGrams(int32_t raw_value) {
    int32_t eff_adc = (raw_value - s_adc_offset) + s_cal_zero_adc;

    // Verifica intervalos definidos
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

    // Extrapolação (fora dos limites definidos)
    if (NUM_CAL_POINTS >= 2) {
        if (eff_adc < cal_points[0].adc_value) {
            int32_t x1 = cal_points[0].adc_value;
            int32_t x2 = cal_points[1].adc_value;
            float   y1 = cal_points[0].grams;
            float   y2 = cal_points[1].grams;
            float   m  = (y2 - y1) / (float)(x2 - x1);
            
            return y1 + m * (eff_adc - x1);
        }
        
        int32_t x1 = cal_points[NUM_CAL_POINTS - 2].adc_value;
        int32_t x2 = cal_points[NUM_CAL_POINTS - 1].adc_value;
        float   y1 = cal_points[NUM_CAL_POINTS - 2].grams;
        float   y2 = cal_points[NUM_CAL_POINTS - 1].grams;
        float   m  = (y2 - y1) / (float)(x2 - x1);
        
        return y2 + m * (eff_adc - x2);
    }

    return 0.0f;
}

// Retorna o offset atual configurado
int32_t ADS1232_GetOffset(void) {
    return s_adc_offset;
}

// Permite definir manualmente o offset
void ADS1232_SetOffset(int32_t new_offset) {
    s_adc_offset = new_offset;
}

// Função stub para compatibilidade (implementação futura)
void ADS1232_SetCalibrationFactor(float factor) {
    (void)factor;
}

// Função stub para compatibilidade (implementação futura)
float ADS1232_GetCalibrationFactor(void) {
    return 1.0f;
}