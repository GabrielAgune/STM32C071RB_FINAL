/*
 * Nome do Arquivo: medicao_handler.h
 * Descrição: Interface do Handler central para aquisição e cálculo de medições
 * Autor: Gabriel Agune
 */

#ifndef MEDICAO_HANDLER_H
#define MEDICAO_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Typedefs e Estruturas
// ============================================================

// Estrutura de dados que armazena a última medição completa
typedef struct {
    float Peso;
    float Frequencia;
    float Escala_A;
    float Temp_Instru;
    float Densidade;
    float Umidade;
} DadosMedicao_t;

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o handler de medição
void Medicao_Init(void);

// Processa as lógicas de medição (leitura de balança e frequência)
void Medicao_Process(void);

// Obtém uma cópia da última medição consolidada
void Medicao_Get_UltimaMedicao(DadosMedicao_t* dados);

// Atualiza a temperatura do instrumento lida pelo sensor do MCU
void Medicao_Set_Temp_Instru(float temp_instru);

// Define a densidade do grão atual (usado para cálculos futuros)
void Medicao_Set_Densidade(float densidade);

// Define a umidade do grão atual (usado para cálculos futuros)
void Medicao_Set_Umidade(float umidade);

#endif // MEDICAO_HANDLER_H