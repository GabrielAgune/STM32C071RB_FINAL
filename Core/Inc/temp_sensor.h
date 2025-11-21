/*
 * Nome do Arquivo: temp_sensor.h
 * Descrição: Interface para o driver de temperatura do sensor interno do MCU
 * Autor: Gabriel Agune
 */

#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

// ============================================================
// Includes
// ============================================================

#include "main.h"

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Lê a temperatura do sensor interno do STM32
float TempSensor_GetTemperature(void);

#endif // TEMP_SENSOR_H