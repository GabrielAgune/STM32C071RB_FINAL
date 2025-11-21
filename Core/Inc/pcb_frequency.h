/*
 * Nome do Arquivo: pcb_frequency.h
 * Descrição: Interface para o driver de contagem de frequência (Capacímetro)
 * Autor: Gabriel Agune
 */

#ifndef INC_PCB_FREQUENCY_H_
#define INC_PCB_FREQUENCY_H_

// ============================================================
// Includes
// ============================================================

#include "main.h"

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o periférico de contagem de pulsos (Timer)
void Frequency_Init(void);

// Zera o contador de pulsos do timer
void Frequency_Reset(void);

// Lê o valor atual do contador de pulsos
uint32_t Frequency_Get_Pulse_Count(void);

#endif /* INC_PCB_FREQUENCY_H_ */