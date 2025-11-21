/*
 * Nome do Arquivo: battery_handler.h
 * Descrição: Gerenciador de aplicação para bateria e interface com Display
 * Autor: Gabriel Agune
 */

#ifndef BATTERY_HANDLER_H
#define BATTERY_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include "i2c.h"

// ============================================================
// Defines de Configuração
// ============================================================

// Define a capacidade da bateria para o algoritmo de Coulomb Counting
#define BATTERY_CAPACITY_MAH    2600

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o handler, drivers do BQ25622 e módulo de SoC
void Battery_Handler_Init(I2C_HandleTypeDef *hi2c);

// Processa a lógica de SoC e atualiza a UI (chamar no loop principal)
void Battery_Handler_Process(void);

#endif // BATTERY_HANDLER_H