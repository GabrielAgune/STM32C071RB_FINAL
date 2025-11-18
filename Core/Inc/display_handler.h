#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

/*******************************************************************************
 * @file        display_handler.h
 * @brief       Interface do Handler de Display.
 * @version     2.1 (Refatorado para display_map.h)
 * @author      -
 * @details     Gerencia as atualizações periódicas de dados no display DWIN e
 *              as sequências de telas, como o processo de medição, feedback de
 *              salvamento de configurações e monitor do sistema.
 ******************************************************************************/

#include "display_map.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Inicializa o handler de display.
 */
void DisplayHandler_Init(void);

/**
 * @brief Processa as lógicas de atualização de display que devem rodar no super-loop.
 *        Inclui:
 *        - FSM da medição
 *        - FSM de feedback de salvamento de configurações
 *        - Atualizações periódicas de monitor/relógio
 */
void DisplayHandler_Process(void);

/* --- Handlers de Eventos chamados pelo Controller --- */

/**
 * @brief Liga/Desliga display e ajusta backlight.
 */
void Display_OFF(uint16_t received_value);

/**
 * @brief Trata evento de impressão ou apenas exibição de resultado.
 */
void Display_ProcessPrintEvent(uint16_t received_value);

/**
 * @brief Ajusta número de repetições de medição (UI + config).
 */
void Display_SetRepeticoes(uint16_t received_value);

/**
 * @brief Ajusta número de casas decimais da umidade (UI + config).
 */
void Display_SetDecimals(uint16_t received_value);

/**
 * @brief Atualiza usuário configurado, a partir de um payload do DWIN.
 */
void Display_SetUser(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

/**
 * @brief Atualiza empresa configurada, a partir de um payload do DWIN.
 */
void Display_SetCompany(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

/**
 * @brief Entra na tela de ajuste do capacímetro.
 */
void Display_Adj_Capa(uint16_t received_value);

/**
 * @brief Exibe tela "Sobre o sistema".
 */
void Display_ShowAbout(void);

/**
 * @brief Exibe tela de modelo/OEM.
 */
void Display_ShowModel(void);

/**
 * @brief Lida com preset de configuração (fábrica).
 */
void Display_Preset(uint16_t received_value);

/**
 * @brief Configura número de série do equipamento.
 */
void Display_Set_Serial(const uint8_t* dwin_data, uint16_t len, uint16_t received_value);

/**
 * @brief Inicia a sequência de telas para o processo de medição.
 *        Esta função é NÃO-BLOQUEANTE e apenas inicia a máquina de estados.
 */
void Display_StartMeasurementSequence(void);

/**
 * @brief Inicia FSM de feedback visual de salvamento de configurações.
 *
 * @param return_screen Tela de retorno após concluir o salvamento.
 * @param success_msg   Mensagem a ser exibida na VP_MESSAGES em caso de sucesso.
 * @return true se o processo foi iniciado, false se já havia um feedback em andamento.
 */
bool DisplayHandler_StartSaveFeedback(uint16_t return_screen, const char* success_msg);

/* --- Getters/Setters para estado interno de impressão --- */

/**
 * @brief Habilita ou desabilita impressão (relatório físico).
 */
void Display_SetPrintingEnabled(bool is_enabled);

/**
 * @brief Retorna se a impressão está habilitada.
 */
bool Display_IsPrintingEnabled(void);

#endif // DISPLAY_HANDLER_H