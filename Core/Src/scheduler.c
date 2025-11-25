/*
 * Nome do Arquivo: scheduler.c
 * Descrio: Implementao do Escalonador Cooperativo Simples
 * Autor: Gabriel Agune
 */

#include "scheduler.h"
#include "main.h" // Para HAL_GetTick()
#include <stddef.h>

// ============================================================
// Configuraes
// ============================================================

#define MAX_TASKS   16

// ============================================================
// Estruturas de Dados
// ============================================================

typedef struct {
    TaskFunction_t func;
    uint32_t       period;
    uint32_t       last_run;
    bool           enabled;
} Task_t;

// ============================================================
// Variveis Privadas
// ============================================================

static Task_t s_tasks[MAX_TASKS];
static uint8_t s_num_tasks = 0;

// ============================================================
// Funes Pblicas
// ============================================================

void Scheduler_Init(void) {
    s_num_tasks = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        s_tasks[i].func = NULL;
        s_tasks[i].enabled = false;
    }
}

bool Scheduler_Register_Task(TaskFunction_t task_func, uint32_t period_ms, uint32_t start_delay_ms) {
    if (s_num_tasks >= MAX_TASKS || task_func == NULL) {
        return false;
    }

    // Adiciona a nova tarefa
    s_tasks[s_num_tasks].func     = task_func;
    s_tasks[s_num_tasks].period   = period_ms;
    s_tasks[s_num_tasks].enabled  = true;
    
    // Configura o tempo inicial para respeitar o start_delay
    s_tasks[s_num_tasks].last_run = HAL_GetTick() + start_delay_ms - period_ms; 
    // (Subtramos o perodo para que, ao somar o perodo na checagem, d o delay correto)

    s_num_tasks++;
    return true;
}

void Scheduler_Run(void) {
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < s_num_tasks; i++) {
        Task_t* task = &s_tasks[i];

        if (task->enabled && task->func != NULL) {
            // Verifica se passou o tempo (lidando com overflow do HAL_GetTick)
            if ((now - task->last_run) >= task->period) {
                task->last_run = now;
                task->func(); // Executa a tarefa
            }
        }
    }
}