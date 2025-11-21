/*
 * Nome do Arquivo: pcb_frequency.c
 * Descrição: Implementação do driver de contagem de frequência usando Timer HAL
 * Autor: Gabriel Agune
 */

#include "pcb_frequency.h"
#include "tim.h"

// ============================================================
// Variáveis Externas
// ============================================================

// Handle do timer configurado (gerado no arquivo tim.h)
extern TIM_HandleTypeDef htim2;

// ============================================================
// Funções Públicas
// ============================================================

// Inicia o Timer 2 no modo de contagem de pulsos
void Frequency_Init(void) {
  HAL_TIM_Base_Start(&htim2);
}

// Zera o contador de pulsos do timer
void Frequency_Reset(void) {
  __HAL_TIM_SET_COUNTER(&htim2, 0);
}

// Lê o valor atual do contador de pulsos do timer
uint32_t Frequency_Get_Pulse_Count(void) {
  return __HAL_TIM_GET_COUNTER(&htim2);
}