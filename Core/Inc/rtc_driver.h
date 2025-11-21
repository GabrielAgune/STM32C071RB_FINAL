/*
 * Nome do Arquivo: rtc_driver.h
 * Descrição: Interface para o driver do Real-Time Clock (RTC)
 * Autor: Gabriel Agune
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

// ============================================================
// Includes
// ============================================================

#include "rtc.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o driver do RTC
void RTC_Driver_Init(RTC_HandleTypeDef* hrtc);

// Define a data (dia, mês, ano) no RTC
bool RTC_Driver_SetDate(uint8_t day, uint8_t month, uint8_t year);

// Define a hora (hora, minuto, segundo) no RTC
bool RTC_Driver_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

// Obtém a data atual do RTC
bool RTC_Driver_GetDate(uint8_t* day, uint8_t* month, uint8_t* year, char* weekday_str);

// Obtém a hora atual do RTC
bool RTC_Driver_GetTime(uint8_t* hours, uint8_t* minutes, uint8_t* seconds);

#endif // RTC_DRIVER_H