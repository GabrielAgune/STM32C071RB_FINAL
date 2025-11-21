/*
 * Nome do Arquivo: retarget.c
 * Descrição: Redirecionamento (Retarget) de syscalls para I/O não-bloqueante via CLI
 * Autor: Gabriel Agune
 */

// ============================================================
// Includes
// ============================================================

#include "retarget.h"
#include "cli_driver.h"
#include "ux_device_cdc_acm.h"
#include <stdio.h>

// ============================================================
// Variáveis Globais
// ============================================================

RetargetDestination_t g_retarget_dest = TARGET_DEBUG;

// Instância CDC-ACM fornecida pelo USBX
extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

// ============================================================
// Funções Públicas
// ============================================================

// Inicialização opcional do módulo de retarget (parâmetros ignorados atualmente)
void Retarget_Init(UART_HandleTypeDef* debug_huart, UART_HandleTypeDef* dwin_huart) {
    (void)debug_huart;
    (void)dwin_huart;
    g_retarget_dest = TARGET_DEBUG;
}

// ============================================================
// Funções Privadas (Helpers)
// ============================================================

// Verifica se a classe CDC-ACM (USB) está pronta
static inline bool usb_cdc_ready(void) {
    return (cdc_acm != NULL);
}

// Envia um bloco de dados para o destino selecionado (principalmente CLI/USB)
static int retarget_write_block(const char *ptr, int len) {
    if (len <= 0 || ptr == NULL) {
        return 0;
    }

    switch (g_retarget_dest) {
        case TARGET_DEBUG:
        default: {
            // Se o USB CDC não está pronto, retorna imediatamente
            if (!usb_cdc_ready()) {
                return len;
            }

            // Enfileira byte a byte no FIFO do CLI
            for (int i = 0; i < len; i++) {
                char c[2] = { ptr[i], '\0' };
                CLI_Puts(c);
            }
            return len;
        }

        case TARGET_DWIN: {
            // Destino DWIN não implementado: descarta
            return len;
        }
    }
}

// ============================================================
// Implementação dos Sys-calls (printf)
// ============================================================

// Implementação de fputc
int fputc(int ch, FILE *f) {
    (void)f;
    char c = (char)ch;
    (void)retarget_write_block(&c, 1);
    return ch;
}

// Implementação do syscall _write
int _write(int file, char *ptr, int len) {
    (void)file;
    return retarget_write_block(ptr, len);
}