/*******************************************************************************
 * @file        medicao_handler.h
 * @brief       Interface do Handler de Medies.
 * @version     2.0 
 * @author      Gabriel Agune
 * @details     Centraliza toda a lgica de aquisio e clculo de dados de
 * medio (peso, frequncia, temperatura, etc.).
 ******************************************************************************/

#ifndef MEDICAO_HANDLER_H
#define MEDICAO_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

// Estrutura de dados que armazena a ltima medio completa.
typedef struct {
    float Peso;
    float Frequencia;
    float Escala_A;
    float Temp_Instru;
    float Densidade;
    float Umidade;
} DadosMedicao_t;

/**
 * @brief Inicializa o handler de medio.
 */
void Medicao_Init(void);

/**
 * @brief Processa as lgicas de medio que devem rodar no super-loop.
 * Isso inclui a leitura da balana e a atualizao peridica da frequncia.
 */
void Medicao_Process(void);

/**
 * @brief Obtm uma cpia da ltima medio consolidada.
 * @param[out] dados Ponteiro para a estrutura onde os dados sero copiados.
 */
void Medicao_Get_UltimaMedicao(DadosMedicao_t* dados);

// --- Funes de atualizao para valores definidos externamente ---

/**
 * @brief Atualiza a temperatura do instrumento lida pelo sensor do MCU.
 */
void Medicao_Set_Temp_Instru(float temp_instru);

/**
 * @brief Define a densidade do gro atual (usado para clculos futuros).
 */
void Medicao_Set_Densidade(float densidade);

/**
 * @brief Define a umidade do gro atual (usado para clculos futuros).
 */
void Medicao_Set_Umidade(float umidade);


#endif // MEDICAO_HANDLER_H