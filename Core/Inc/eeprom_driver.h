/*
 * Nome do Arquivo: eeprom_driver.h
 * Descrição: API pública para o driver da EEPROM I2C
 * Autor: Gabriel Agune
 */

#ifndef EEPROM_DRIVER_H
#define EEPROM_DRIVER_H

// ============================================================
// Includes
// ============================================================

#include "stm32c0xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Definições de Configuração
// ============================================================

#define EEPROM_PAGE_SIZE         128  // Tamanho de página (AT24C512)
#define EEPROM_WRITE_TIME_MS       5  // Tempo de escrita interna (tWR)
#define EEPROM_I2C_TIMEOUT_BOOT  100  // Timeout padrão de inicialização

// ============================================================
// API Pública - Inicialização e Status
// ============================================================

// Inicializa o driver da EEPROM
void EEPROM_Driver_Init(I2C_HandleTypeDef *hi2c);

// Verifica se a EEPROM está presente e pronta (ACK) no barramento I2C
bool EEPROM_Driver_IsReady(void);

// ============================================================
// API Pública - Funções Bloqueantes (Testes)
// ============================================================

// Lê dados da EEPROM de forma bloqueante
bool EEPROM_Driver_Read_Blocking(uint16_t addr, uint8_t *data, uint16_t size);

// Escreve dados na EEPROM de forma bloqueante
bool EEPROM_Driver_Write_Blocking(uint16_t addr, const uint8_t *data, uint16_t size);

// ============================================================
// API Pública - Funções Assíncronas (FSM)
// ============================================================

// Verifica se a máquina de estados (FSM) de escrita está ocupada
bool EEPROM_Driver_IsBusy(void);

// Inicia uma operação de escrita assíncrona (não-bloqueante)
bool EEPROM_Driver_Write_Async_Start(uint16_t addr, const uint8_t *data, uint16_t size);

// Processa a máquina de estados (FSM) de escrita assíncrona
void EEPROM_Driver_FSM_Process(void);

// Obtém o status de erro da FSM e limpa a flag
bool EEPROM_Driver_GetAndClearErrorFlag(void);

#endif // EEPROM_DRIVER_H