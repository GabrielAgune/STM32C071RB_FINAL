/*
 * Nome do Arquivo: rtc_handler.h
 * Descrição: Interface para o módulo de tratamento de eventos de ajuste de RTC
 * Autor: Gabriel Agune
 */

#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Trata o recebimento de uma string de hora vinda do DWIN para ajustar o RTC
void RTC_Handle_Set_Time(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

// Trata o recebimento de uma string de data e hora vinda do DWIN para ajustar o RTC
void RTC_Handle_Set_Date_And_Time(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

#endif // RTC_HANDLER_H