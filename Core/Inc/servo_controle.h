/*
 * Nome do Arquivo: servo_controle.h
 * Descrição: Interface de alto nível para o controle sequencial de servomotores
 * Autor: Gabriel Agune
 */

#ifndef SERVO_CONTROLE_H
#define SERVO_CONTROLE_H

// ============================================================
// Includes
// ============================================================

#include "main.h"

// ============================================================
// Tipos de Dados
// ============================================================

typedef enum {
    SERVO_STEP_FUNNEL,
    SERVO_STEP_SCRAPER,
    SERVO_STEP_IDLE,
    SERVO_STEP_FINISHED
} ServoStep_t;


// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o módulo de controle dos servos
void Servos_Init(void);

// Processa a máquina de estados e atualiza a posição dos servos
void Servos_Process(void);

// Inicia a sequência de movimento dos servos
void Servos_Start_Sequence(void);

// Decrementa os temporizadores internos de controle dos servos (Chamar a cada 1ms)
void Servos_Tick_ms(void);

#endif // SERVO_CONTROLE_H