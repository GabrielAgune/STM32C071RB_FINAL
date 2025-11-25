/*
 * Nome do Arquivo: scheduler.h
 * Descrição: Interface do Escalonador Cooperativo (Time-Slicing)
 * Autor: Gabriel Agune
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Typedefs
// ============================================================

// Ponteiro de função para a tarefa
typedef void (*TaskFunction_t)(void);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa o escalonador
void Scheduler_Init(void);

// Registra uma nova tarefa no sistema
// task_func: Função a ser executada
// period_ms: Intervalo de tempo em milissegundos
// start_delay_ms: Atraso inicial antes da primeira execução
bool Scheduler_Register_Task(TaskFunction_t task_func, uint32_t period_ms, uint32_t start_delay_ms);

// Executa o ciclo do escalonador (Chamar no loop infinito do main)
void Scheduler_Run(void);


#endif // SCHEDULER_H