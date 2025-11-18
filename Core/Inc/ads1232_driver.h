/*
 * Nome do Arquivo: ads1232_driver.h
 * Descrição: Interface do driver para o ADC ADS1232 (Balança)
 * Autor: Gabriel Agune
 */

#ifndef __ADS1232_DRIVER_H
#define __ADS1232_DRIVER_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include <stdint.h>

// ============================================================
// Defines
// ============================================================

#define NUM_CAL_POINTS          4

// ============================================================
// Typedefs e Estruturas
// ============================================================

typedef struct {
    float   grams;
    int32_t adc_value;
} CalPoint_t;

// ============================================================
// Variáveis Externas
// ============================================================

// Tabela de pontos de calibração compartilhada
extern CalPoint_t cal_points[NUM_CAL_POINTS];

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa os pinos e o estado interno do driver
void    ADS1232_Init(void);

// Lê um valor bruto único do ADC (bloqueante ou simulado)
int32_t ADS1232_Read(void);

// Realiza três leituras e retorna a mediana para filtragem de ruído
int32_t ADS1232_Read_Median_of_3(void);

// Executa a rotina de tara (zero) da balança
int32_t ADS1232_Tare(void);

// Converte o valor bruto do ADC para gramas usando interpolação
float   ADS1232_ConvertToGrams(int32_t raw_value);

// Retorna o offset atual (valor de tara)
int32_t ADS1232_GetOffset(void);

// Define manualmente o offset da balança
void    ADS1232_SetOffset(int32_t new_offset);

// Callback de interrupção para sinal DRDY (Data Ready)
void    Drv_ADS1232_DRDY_Callback(void);

// Define o fator de calibração (stub para compatibilidade futura)
void    ADS1232_SetCalibrationFactor(float factor);

// Retorna o fator de calibração (stub para compatibilidade futura)
float   ADS1232_GetCalibrationFactor(void);

#endif // __ADS1232_DRIVER_H