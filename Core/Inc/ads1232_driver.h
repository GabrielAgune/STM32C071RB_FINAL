/*
 * Nome do Arquivo: ads1232_driver.h
 * Descrição: Driver ADS1232 Não-Bloqueante com Power Management
 */

#ifndef __ADS1232_DRIVER_H
#define __ADS1232_DRIVER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// Configuração de Calibração
#define NUM_CAL_POINTS 4

typedef struct {
    float   grams;
    int32_t adc_value;
} CalPoint_t;

extern CalPoint_t cal_points[NUM_CAL_POINTS];

// Estados do Driver
typedef enum {
    ADS1232_STATE_POWER_DOWN, // Chip desligado (PDWN low)
    ADS1232_STATE_STARTUP,    // Acabou de ligar, esperando estabilizar
    ADS1232_STATE_IDLE,       // Ligado, mas esperando comando ou DRDY
    ADS1232_STATE_READING     // Processando leitura
} ADS1232_State_t;

// ============================================================
// Funções Públicas
// ============================================================

void    ADS1232_Init(void);

// Máquina de estados principal (chamar no loop infinito)
void    ADS1232_Process(void);

// Controle de Energia
void    ADS1232_PowerUp(void);
void    ADS1232_PowerDown(void);

// Verifica se há um novo valor de peso consolidado (filtrado)
bool    ADS1232_IsDataAvailable(void);
float   ADS1232_GetGrams(void);

// Funções de Tara e Calibração (agora não-bloqueantes ou semi-bloqueantes dependendo da estratégia)
void    ADS1232_SetTareCurrent(void);
int32_t ADS1232_GetOffset(void);
void    ADS1232_SetOffset(int32_t new_offset);

// Callback da interrupção externa (DRDY)
void    Drv_ADS1232_DRDY_Callback(void);

#endif // __ADS1232_DRIVER_H