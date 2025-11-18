/*
 * Nome do Arquivo: app_manager.h
 * Descrição: Gerenciador central da aplicação e orquestrador de estados
 * Autor: Gabriel Agune
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ============================================================
// Typedefs 
// ============================================================

// Ponteiro de função para um teste de diagnóstico individual
typedef bool (*DiagnosticTestFunc)(void);

// Estrutura que define uma única etapa do autodiagnóstico
typedef struct {
    const char* description;        // Descrição para log
    uint16_t            screen_id;          // ID da tela DWIN
    uint32_t            display_time_ms;    // Tempo de exibição
    DiagnosticTestFunc  execute_test;       // Função de teste
} DiagnosticStep_t;

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa todos os módulos da aplicação em sequência
void App_Manager_Init(void);

// Executa o loop de processamento principal da aplicação
void App_Manager_Process(void);

// Executa a rotina de autodiagnóstico do sistema
bool App_Manager_Run_Self_Diagnostics(uint8_t return_tela);


#endif // APP_MANAGER_H