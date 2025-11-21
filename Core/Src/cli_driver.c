/*
 * Nome do Arquivo: cli_driver.c
 * Descrição: Implementação do driver CLI (FIFO TX e Line Buffering RX)
 * Autor: Gabriel Agune
 */

#include "cli_driver.h"
#include "ux_device_cdc_acm.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ============================================================
// Defines e Constantes
// ============================================================

#define CLI_TX_FIFO_SIZE        1536
#define CLI_BUFFER_SIZE        	 256
#define CLI_USB_MAX_PKT         	64      

// ============================================================
// Variáveis Externas
// ============================================================

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

// ============================================================
// Variáveis Privadas
// ============================================================

// FIFO de Transmissão
static uint8_t              s_cli_tx_fifo[CLI_TX_FIFO_SIZE];
static uint16_t             s_cli_tx_head       = 0;
static uint16_t             s_cli_tx_tail       = 0;

// Buffer de Recepção de Linha
static char                 s_cli_buffer[CLI_BUFFER_SIZE];
static uint16_t             s_cli_buffer_index  = 0;
static bool                 s_command_ready     = false;

// Callback registrado
static cli_line_callback_t  s_line_callback     = NULL;

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa as estruturas internas e registra o callback
void CLI_Init(cli_line_callback_t line_cb) {
    s_line_callback     = line_cb;
    s_cli_tx_head       = 0;
    s_cli_tx_tail       = 0;
    s_cli_buffer_index  = 0;
    s_command_ready     = false;
    
    memset(s_cli_buffer, 0, sizeof(s_cli_buffer));
}

// Verifica estado da conexão USB
bool CLI_Is_USB_Connected(void) {
    return (cdc_acm != NULL);
}

// Adiciona string ao FIFO de transmissão
void CLI_Puts(const char* str) {
    if (!str || !CLI_Is_USB_Connected()) {
        return;
    }

    const uint32_t len = (uint32_t)strlen(str);
    if (len == 0u) {
        return;
    }

    for (uint32_t i = 0; i < len; i++) {
        const uint16_t next_head = (uint16_t)((s_cli_tx_head + 1u) % CLI_TX_FIFO_SIZE);

        if (next_head == s_cli_tx_tail) {
            // FIFO cheio: descarta o restante
            break;
        }
        s_cli_tx_fifo[s_cli_tx_head] = (uint8_t)str[i];
        s_cli_tx_head = next_head;
    }
}

// Formata string e envia para o FIFO
void CLI_Printf(const char* format, ...) {
    if (!format) {
        return;
    }

    static char printf_buffer[256];

    va_list args;
    va_start(args, format);
    const int len = vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);
    va_end(args);

    if (len > 0) {
        CLI_Puts(printf_buffer);
    }
}

// Move dados do FIFO para o endpoint USB CDC
void CLI_TX_Pump(void) {
    if (!CLI_Is_USB_Connected() || (s_cli_tx_head == s_cli_tx_tail)) {
        return;
    }

    uint16_t bytes_to_send;
    if (s_cli_tx_head > s_cli_tx_tail) {
        bytes_to_send = (uint16_t)(s_cli_tx_head - s_cli_tx_tail);
    } else {
        bytes_to_send = (uint16_t)(CLI_TX_FIFO_SIZE - s_cli_tx_tail);
    }

    if (bytes_to_send > CLI_USB_MAX_PKT) {
        bytes_to_send = CLI_USB_MAX_PKT;
    }

    uint32_t sent_bytes = 0;
    if (USBD_CDC_ACM_Transmit(&s_cli_tx_fifo[s_cli_tx_tail], bytes_to_send, &sent_bytes) == UX_SUCCESS) {
        if (sent_bytes > 0u) {
            s_cli_tx_tail = (uint16_t)((s_cli_tx_tail + sent_bytes) % CLI_TX_FIFO_SIZE);
        }
    }
}

// Gerencia a recepção de caracteres e montagem de linhas
void CLI_Receive_Char(uint8_t received_char) {
    if (s_command_ready) {
        return;
    }

    // Tratamento de fim de linha (CR ou LF)
    if (received_char == '\r' || received_char == '\n') {
        if (s_cli_buffer_index > 0u) {
            s_cli_buffer[s_cli_buffer_index] = '\0';
            s_command_ready = true;

            if (s_line_callback) {
                s_line_callback(s_cli_buffer);
            }

            s_cli_buffer_index = 0;
            memset(s_cli_buffer, 0, sizeof(s_cli_buffer));
            s_command_ready = false;
        }
        return;
    }

    // Tratamento de Backspace
    if (received_char == '\b' || received_char == 127u) {
        if (s_cli_buffer_index > 0u) {
            s_cli_buffer_index--;
            CLI_Puts("\b \b");
        }
        return;
    }

    // Caracteres imprimíveis
    if (s_cli_buffer_index < (CLI_BUFFER_SIZE - 1u) && isprint(received_char)) {
        s_cli_buffer[s_cli_buffer_index++] = (char)received_char;

        // Eco local
        char echo[2] = { (char)received_char, '\0' };
        CLI_Puts(echo);
    }
}
