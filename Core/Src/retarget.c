/*******************************************************************************
 * @file        retarget.c
 * @brief       Redirecionamento (Retarget) de syscalls para I/O.
 * @author      Gabriel Agune
 *
 * Esta vers?o integra o printf com o driver de CLI (USB CDC via USBX),
 * evitando travamentos quando o host n?o est? conectado e usando FIFO
 * n?o bloqueante para transmiss?o.
 ******************************************************************************/

#include "retarget.h"
#include "cli_driver.h"
#include "ux_device_cdc_acm.h"  // Para saber se o CDC est? ativo
#include <stdio.h>
#include <stdarg.h>

/*==============================================================================
 *  CONTEXTO E CONFIGURA??O
 *============================================================================*/

RetargetDestination_t g_retarget_dest = TARGET_DEBUG;

// Inst?ncia CDC-ACM fornecida pelo USBX (definida em ux_device_cdc_acm.c)
extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

/**
 * @brief Inicializa??o opcional do m?dulo de retarget.
 *        Mantida apenas para compatibilidade. No momento, o printf ?
 *        redirecionado sempre para o CLI/USB, ent?o os ponteiros s?o
 *        ignorados.
 */
void Retarget_Init(UART_HandleTypeDef* debug_huart, UART_HandleTypeDef* dwin_huart)
{
    (void)debug_huart;
    (void)dwin_huart;
    g_retarget_dest = TARGET_DEBUG;
}

/*==============================================================================
 *  HELPERS INTERNOS
 *============================================================================*/

static inline bool usb_cdc_ready(void)
{
    // Usa o ponteiro da classe CDC-ACM para saber se o host USB est? conectado
    return (cdc_acm != NULL);
}

/**
 * @brief Envia um bloco de dados para o destino selecionado.
 *
 * Atualmente:
 *  - TARGET_DEBUG  -> CLI/USB (n?o bloqueante, via FIFO do CLI)
 *  - TARGET_DWIN   -> (reservado p/ futuro, hoje tamb?m descarta ou usa debug)
 */
static int retarget_write_block(const char *ptr, int len)
{
    if (len <= 0 || ptr == NULL) {
        return 0;
    }

    switch (g_retarget_dest)
    {
        case TARGET_DEBUG:
        default:
        {
            // Se o USB CDC n?o est? pronto, descarta rapidamente para n?o travar.
            if (!usb_cdc_ready()) {
                return len;
            }

            // Enfileira byte a byte no FIFO do CLI.
            // CLI_TX_Pump() ser? chamado no super-loop.
            for (int i = 0; i < len; i++) {
                char c[2] = { ptr[i], '\0' };
                CLI_Puts(c);
            }
            return len;
        }

        case TARGET_DWIN:
        {
            // No momento n?o h? caminho separado para DWIN aqui.
            // Poderia futuramente encaminhar para uma UART espec?fica.
            // Por seguran?a, simplesmente descarta.
            return len;
        }
    }
}

/*==============================================================================
 *  IMPLEMENTA??O DOS SYS-CALLS USADOS PELO printf
 *============================================================================*/

/**
 * @brief Implementa??o de fputc usada por algumas libs de C.
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    char c = (char)ch;
    (void)retarget_write_block(&c, 1);
    return ch;
}

/**
 * @brief Implementa??o do syscall _write, base do printf.
 *
 *  - N?o bloqueia esperando USB.
 *  - Descarta a sa?da se o host USB n?o estiver conectado.
 *  - Usa o FIFO do CLI para transmiss?o ass?ncrona.
 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    return retarget_write_block(ptr, len);
}