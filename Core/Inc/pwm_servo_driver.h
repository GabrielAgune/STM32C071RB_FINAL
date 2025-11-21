/*
 * Nome do Arquivo: pwm_servo_driver.h
 * Descrição: Interface do driver de baixo nível para controle de servomotores
 * Autor: Gabriel Agune
 */

#ifndef PWM_SERVO_DRIVER_H
#define PWM_SERVO_DRIVER_H

// ============================================================
// Includes
// ============================================================

#include "main.h" // Necessário para o tipo TIM_HandleTypeDef

// ============================================================
// Typedefs e Estruturas
// ============================================================

// Estrutura para definir um servomotor e suas propriedades
typedef struct {
    TIM_HandleTypeDef *htim;         // Ponteiro para o timer que o controla
    uint32_t           channel;      // Canal do timer
    uint16_t           min_pulse_us; // Pulso mínimo (em us) para 0°
    uint16_t           max_pulse_us; // Pulso máximo (em us) para 180°
} Servo_t;

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa e inicia a geração de PWM para um servo específico
HAL_StatusTypeDef PWM_Servo_Init(Servo_t *servo);

// Define a posição do servo em um ângulo específico
void PWM_Servo_SetAngle(Servo_t *servo, float angle);

// Para a geração de PWM para um servo específico
HAL_StatusTypeDef PWM_Servo_DeInit(Servo_t *servo);

#endif // PWM_SERVO_DRIVER_H