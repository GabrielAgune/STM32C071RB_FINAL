/*
 * Nome do Arquivo: display_handler.h
 * Descrição: Interface do Handler de Display (DWIN) e FSMs de UI
 * Autor: Gabriel Agune
 */

#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Inicializa o handler de display
void DisplayHandler_Init(void);

// Processa as lógicas de atualização e FSMs no super-loop
void DisplayHandler_Process(void);

/* --- Handlers de Eventos chamados pelo Controller --- */

// Liga/Desliga display e ajusta o backlight
void Display_OFF(uint16_t received_value);

// Trata evento de impressão ou apenas exibição de resultado final
void Display_ProcessPrintEvent(uint16_t received_value);

// Ajusta o número de repetições de medição (UI + config)
void Display_SetRepeticoes(uint16_t received_value);

// Ajusta o número de casas decimais da umidade (UI + config)
void Display_SetDecimals(uint16_t received_value);

// Atualiza o usuário configurado, a partir de um payload do DWIN
void Display_SetUser(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

// Atualiza a empresa configurada, a partir de um payload do DWIN
void Display_SetCompany(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

// Entra na tela de ajuste do capacímetro
void Display_Adj_Capa(uint16_t received_value);

// Exibe a tela "Sobre o sistema"
void Display_ShowAbout(void);

// Exibe a tela de modelo/OEM
void Display_ShowModel(void);

// Lida com preset de configuração de fábrica
void Display_Preset(uint16_t received_value);

// Configura o número de série do equipamento
void Display_Set_Serial(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

// Inicia a sequência de telas para o processo de medição (não-bloqueante)
void Display_StartMeasurementSequence(void);

// Habilita ou desabilita a impressão de relatório físico
void Display_SetPrintingEnabled(bool is_enabled);

// Retorna se a impressão está habilitada
bool Display_IsPrintingEnabled(void);

#endif // DISPLAY_HANDLER_H