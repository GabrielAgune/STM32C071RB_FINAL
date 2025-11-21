/*
 * Nome do Arquivo: retarget.h
 * Descrição: Interface para o módulo de redirecionamento de I/O (printf)
 * Autor: Gabriel Agune
 */

#ifndef RETARGET_H
#define RETARGET_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include <stdio.h>

// ============================================================
// Tipos de Dados e Variáveis Globais
// ============================================================

// Enumeração para os possíveis destinos de saída do printf
typedef enum {
    TARGET_DEBUG, // UART/USB para o PC/CLI (padrão)
    TARGET_DWIN   // UART para o Display DWIN (reservado)
} RetargetDestination_t;

// Variável global que controla para onde o printf envia os dados
extern RetargetDestination_t g_retarget_dest;

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o módulo de retarget com os handles das UARTs (mantido por compatibilidade)
void Retarget_Init(UART_HandleTypeDef* debug_huart, UART_HandleTypeDef* dwin_huart);

// ============================================================
// Protótipos de Sys-calls
// ============================================================

// Implementação de fputc (para algumas libs C)
int fputc(int ch, FILE *f);

// Implementação de _write (base do printf)
int _write(int file, char *ptr, int len);

#endif // RETARGET_H