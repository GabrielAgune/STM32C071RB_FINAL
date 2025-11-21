/*
 * Nome do Arquivo: medicao_handler.c
 * Descrição: Implementação do Handler de Medições (Balança e Frequência)
 * Autor: Gabriel Agune
 */

#include "medicao_handler.h"
#include "ads1232_driver.h"
#include "pcb_frequency.h"
#include "gerenciador_configuracoes.h"
#include "main.h"
#include <string.h>
#include <math.h>


// ============================================================
// Variáveis Estáticas e Constantes
// ============================================================

// Armazena o estado interno das medições
static DadosMedicao_t s_dados_medicao_atuais;
static bool           s_balanca_ativa = false;

// Controle de tempo para atualização de frequência
static uint32_t       s_freq_last_tick          = 0;
static const uint32_t FREQ_UPDATE_INTERVAL_MS   = 1000;

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static void  UpdateScaleData(void);
static void  UpdateFrequencyData(void);
static float CalculateEscalaA(uint32_t frequencia_hz);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa o estado interno das medições com zero
void Medicao_Init(void) {
    memset(&s_dados_medicao_atuais, 0, sizeof(DadosMedicao_t));
		s_balanca_ativa = false;
}

// Função para ligar a balança apenas quando necessário
void Medicao_Start_Balanca(void) {
    if (!s_balanca_ativa) {
        ADS1232_PowerUp();
        s_balanca_ativa = true;
    }
}

void Medicao_Stop_Balanca(void) {
    if (s_balanca_ativa) {
        ADS1232_PowerDown();
        s_balanca_ativa = false;
    }
}

// Executa a lógica de aquisição de dados
void Medicao_Process(void) {
		ADS1232_Process();
    UpdateScaleData();
    UpdateFrequencyData();
}

void Medicao_Tare_Balanca(void) {
    // Tare não bloqueante: apenas seta o offset atual
    if (s_balanca_ativa) {
        ADS1232_SetTareCurrent();
    }
}

// Obtém uma cópia da última medição consolidada
void Medicao_Get_UltimaMedicao(DadosMedicao_t* dados_out) {
    if (dados_out != NULL) {
        memcpy(dados_out, &s_dados_medicao_atuais, sizeof(DadosMedicao_t));
    }
}

// Define a temperatura do instrumento
void Medicao_Set_Temp_Instru(float temp_instru) {
    s_dados_medicao_atuais.Temp_Instru = temp_instru;
}

// Define a densidade
void Medicao_Set_Densidade(float densidade) {
    s_dados_medicao_atuais.Densidade = densidade;
}

// Define a umidade
void Medicao_Set_Umidade(float umidade) {
    s_dados_medicao_atuais.Umidade = umidade;
}

// ============================================================
// Funções Privadas
// ============================================================

// Verifica se um novo dado da balança está pronto e o processa
static void UpdateScaleData(void) {
    // Apenas atualiza a struct se a balança estiver ativa e tiver dado novo
    if (s_balanca_ativa && ADS1232_IsDataAvailable()) {
        s_dados_medicao_atuais.Peso = ADS1232_GetGrams();
    }
}

// Atualiza a leitura de frequência e o cálculo da Escala A
static void UpdateFrequencyData(void) {
    if (HAL_GetTick() - s_freq_last_tick >= FREQ_UPDATE_INTERVAL_MS) {
        s_freq_last_tick = HAL_GetTick();

        uint32_t pulsos = Frequency_Get_Pulse_Count();
        Frequency_Reset();

        s_dados_medicao_atuais.Frequencia = (float)pulsos;
        s_dados_medicao_atuais.Escala_A = CalculateEscalaA(pulsos);
    }
}

// Calcula o valor da Escala A com base na frequência e nos fatores de calibração
static float CalculateEscalaA(uint32_t frequencia_hz) {
    // Equação linear base
    float escala_a = (-0.00014955f * (float)frequencia_hz) + 396.85f;

    float gain = 1.0f;
    float zero = 0.0f;
    
    // Aplica correção de calibração
    Gerenciador_Config_Get_Cal_A(&gain, &zero);
    escala_a = (escala_a * gain) + zero;

    return escala_a;
}