/*
 * Nome do Arquivo: servo_controle.c
 * Descrição: Implementação da FSM de alto nível para sequência de servos
 * Autor: Gabriel Agune
 */

#include "servo_controle.h"
#include "pwm_servo_driver.h"
#include <stdbool.h>

// ============================================================
// Defines e Constantes
// ============================================================

#define ESTADO_OCIOSO       0xFF

#define ANGULO_FECHADO      0.0f
#define ANGULO_FUNIL_ABRE   75.0f
#define ANGULO_SCRAP_ABRE   90.0f

// ============================================================
// Typedefs e Estruturas
// ============================================================

typedef void (*Funcao_Acao_t)(void);

typedef struct {
    ServoStep_t     id_passo;
    Funcao_Acao_t   acao;
    uint32_t        duracao_ms;
    uint8_t         indice_proximo_estado;
} Passo_Processo_t;

// ============================================================
// Variáveis Externas
// ============================================================

extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

// ============================================================
// Variáveis de Estado do Módulo
// ============================================================

static volatile uint32_t s_timer_funil           = 0;
static volatile uint32_t s_timer_scrap           = 0;
static volatile uint8_t  s_indice_estado_atual   = ESTADO_OCIOSO;
static volatile uint32_t s_timer_estado_ms       = 0;

// Configuração dos Servos
static Servo_t s_servo_funil = {
    .htim         = &htim17,
    .channel      = TIM_CHANNEL_1,
    .min_pulse_us = 700,
    .max_pulse_us = 2300
};

static Servo_t s_servo_scrap = {
    .htim         = &htim16,
    .channel      = TIM_CHANNEL_1,
    .min_pulse_us = 650,
    .max_pulse_us = 2400
};

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static void Acao_Abrir_Funil(void);
static void Acao_Varrer_Scrap(void);
static void Acao_Finalizar(void);
static void Entrar_No_Estado(uint8_t indice_estado);

// ============================================================
// Tabela FSM (Fluxo do Processo)
// ============================================================

static const Passo_Processo_t s_fluxo_processo[] = {
    // ID Passo,           Ação,               Duração (ms), Próximo Índice
    { SERVO_STEP_FUNNEL,   Acao_Abrir_Funil,  2000,         1 },
    { SERVO_STEP_FUNNEL,   NULL,              500,          2 },
    { SERVO_STEP_SCRAPER,  Acao_Varrer_Scrap, 2000,         3 },
    { SERVO_STEP_SCRAPER,  NULL,              500,          4 },
    { SERVO_STEP_FINISHED, Acao_Finalizar,    1,            ESTADO_OCIOSO },
};

#define NUM_PASSOS_PROCESSO (sizeof(s_fluxo_processo) / sizeof(s_fluxo_processo[0]))

// ============================================================
// Funções Públicas
// ============================================================

// Decrementa os temporizadores internos de controle dos servos (Chamar a cada 1ms)
void Servos_Tick_ms(void) {
    if (s_timer_estado_ms > 0) s_timer_estado_ms--;
    if (s_timer_funil > 0)     s_timer_funil--;
    if (s_timer_scrap > 0)     s_timer_scrap--;
}

// Inicializa o módulo de controle dos servos
void Servos_Init(void) {
    PWM_Servo_Init(&s_servo_scrap);
    PWM_Servo_Init(&s_servo_funil);
    s_indice_estado_atual = ESTADO_OCIOSO;
}

// Processa a máquina de estados e atualiza a posição dos servos
void Servos_Process(void) {
    uint32_t timer_snapshot;

    __disable_irq();
    timer_snapshot = s_timer_estado_ms;
    __enable_irq();

    // Transição de Estado: Se não está ocioso e o timer zerou
    if (s_indice_estado_atual != ESTADO_OCIOSO && timer_snapshot == 0) {
        uint8_t proximo_indice = s_fluxo_processo[s_indice_estado_atual].indice_proximo_estado;
        Entrar_No_Estado(proximo_indice);
    }

    // Atualiza a posição dos servos (sempre no Process)
    PWM_Servo_SetAngle(&s_servo_funil, (s_timer_funil > 0) ? ANGULO_FUNIL_ABRE : ANGULO_FECHADO);
    PWM_Servo_SetAngle(&s_servo_scrap, (s_timer_scrap > 0) ? ANGULO_SCRAP_ABRE : ANGULO_FECHADO);
}

// Inicia a sequência de movimento dos servos
void Servos_Start_Sequence(void) {
    if (s_indice_estado_atual == ESTADO_OCIOSO) {
        Entrar_No_Estado(0);
    }
}

// ============================================================
// Funções Privadas
// ============================================================

// Gerencia a transição para um novo estado da FSM
static void Entrar_No_Estado(uint8_t indice_estado) {
    if (indice_estado >= NUM_PASSOS_PROCESSO) {
        s_indice_estado_atual = ESTADO_OCIOSO;
        return;
    }

    s_indice_estado_atual = indice_estado;
    const Passo_Processo_t* passo = &s_fluxo_processo[indice_estado];

    if (passo->acao != NULL) {
        passo->acao();
    }

    s_timer_estado_ms = passo->duracao_ms;

    if (passo->id_passo == SERVO_STEP_FINISHED) {
        // Lógica de finalização aqui
    }
}

// Ação: Inicia o timer para manter o funil aberto
static void Acao_Abrir_Funil(void) {
    s_timer_funil = 2000;
}

// Ação: Inicia o timer para manter o scrap (raspador) varrendo
static void Acao_Varrer_Scrap(void) {
    s_timer_scrap = 2000;
}

// Ação: Nenhuma (Finalização)
static void Acao_Finalizar(void) {
}