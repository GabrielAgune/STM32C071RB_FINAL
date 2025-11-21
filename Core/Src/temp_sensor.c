/*
 * Nome do Arquivo: temp_sensor.c
 * Descrição: Implementação da leitura e cálculo da temperatura interna do MCU
 * Autor: Gabriel Agune
 */

// ============================================================
// Includes
// ============================================================

#include "temp_sensor.h"
#include "adc.h"
#include "stm32c0xx_ll_adc.h"

// ============================================================
// Variáveis Externas
// ============================================================

extern ADC_HandleTypeDef hadc1;

// ============================================================
// Constantes de Calibração (Específicas do STM32C0)
// ============================================================

// Temperatura de referência para o ponto de calibração 1
#define TEMP_CAL_P1_TEMP          15.0f

// Slope (inclinação) da curva do sensor (Volts/°C)
#define AVG_SLOPE_TYP             0.00161f

// Tensão de referência usada durante a calibração de fábrica
#define VDDA_CALIBRATION_VOLTAGE  3.0f

// Resolução máxima do ADC de 12 bits
#define ADC_MAX_VALUE             4095.0f

// ============================================================
// Funções Públicas
// ============================================================

// Lê a temperatura do sensor interno do MCU usando calibração de fábrica
float TempSensor_GetTemperature(void) {
    uint32_t raw_temp_sensor = 0;
    uint16_t temp_cal1_raw;

    // 1. Configura e inicia o ADC para o canal do sensor de temperatura
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        return -273.0f;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return -273.0f;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        raw_temp_sensor = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    if (raw_temp_sensor == 0) {
        return -273.0f;
    }

    // 2. Obtém o valor de calibração de fábrica (TS_CAL1)
    temp_cal1_raw = *TEMPSENSOR_CAL1_ADDR;

    // 3. Converte a contagem de calibração para tensão (V30)
    float v30_calibrated = VDDA_CALIBRATION_VOLTAGE * (float)temp_cal1_raw / ADC_MAX_VALUE;

    // 4. Converte a leitura atual do sensor para tensão (Vsense)
    float vsense_voltage = VDDA_CALIBRATION_VOLTAGE * (float)raw_temp_sensor / ADC_MAX_VALUE;

    // 5. Aplica a fórmula de ponto único
    float temperature_celsius = ((vsense_voltage - v30_calibrated) / AVG_SLOPE_TYP) + TEMP_CAL_P1_TEMP;

    return temperature_celsius;
}