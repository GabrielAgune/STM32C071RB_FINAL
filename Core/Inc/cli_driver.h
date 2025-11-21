/*
 * Nome do Arquivo: cli_driver.h
 * Descrição: Driver de baixo nível para CLI (Transporte e Buffering)
 * Autor: Gabriel Agune
 */

#ifndef CLI_DRIVER_H
#define CLI_DRIVER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Typedefs
// ============================================================

// Callback chamado quando uma linha completa (terminada em \r ou \n) é recebida
typedef void (*cli_line_callback_t)(const char* line);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa o driver e registra o callback de processamento de linha
void CLI_Init(cli_line_callback_t line_cb);

// Enfileira uma string simples para envio (não bloqueante)
void CLI_Puts(const char* str);

// Envia uma string formatada para o terminal (estilo printf)
void CLI_Printf(const char* format, ...);

// Processa o envio de dados do FIFO para a interface USB (chamar no loop)
void CLI_TX_Pump(void);

// Processa um byte recebido da interface de hardware
void CLI_Receive_Char(uint8_t received_char);

// Verifica se o host USB está conectado e a classe CDC ativa
bool CLI_Is_USB_Connected(void);

#endif // CLI_DRIVER_H