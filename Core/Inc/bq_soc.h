/*
 * Nome do Arquivo: bq_soc.h
 * Descrição: Interface do módulo de estimativa de SoC (State of Charge)
 * Autor: Gabriel Agune
 */

#ifndef INC_BQ_SOC_H_
#define INC_BQ_SOC_H_

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include "i2c.h"
#include "bq25622_driver.h"

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o contador de Coulomb e estima SoC inicial via tensão
void    bq_soc_coulomb_init(I2C_HandleTypeDef *hi2c, uint16_t battery_capacity_mah);

// Função de atualização periódica (chamar no loop principal)
void    bq_soc_coulomb_update(I2C_HandleTypeDef *hi2c);

// Callback do SysTick (deve ser chamado a cada 1ms)
void    bq_soc_systick_callback(void);

// Retorna a porcentagem de bateria calculada (0.0% a 100.0%)
float   bq_soc_get_percentage(void);

// Retorna a última tensão VBAT lida (Volts)
float   bq_soc_get_last_vbat(void);

// Retorna a última tensão VBUS lida (Volts)
float   bq_soc_get_last_vbus(void);

// Retorna a última corrente IBAT lida (Amperes)
float   bq_soc_get_last_ibat(void);

// Retorna a última temperatura do chip lida (°C)
float   bq_soc_get_last_tdie(void);

// Retorna o último status de carga lido
BQ25622_ChargeStatus_t bq_soc_get_last_chg_status(void);

#endif /* INC_BQ_SOC_H_ */