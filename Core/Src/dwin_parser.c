/*
 * Nome do Arquivo: dwin_parser.c
 * Descrição: Implementação da lógica de extração e sanitização de strings
 * Autor: Gabriel Agune
 */

#include "dwin_parser.h"
#include <string.h>
#include <stddef.h>

// ============================================================
// Funções Públicas
// ============================================================

// Sanitiza o payload recebido, removendo o byte de tamanho e terminadores 0xFF
bool DWIN_Parse_String_Payload_Robust(const uint8_t* payload, uint16_t payload_len, char* out_buffer, uint8_t max_len) {
    if (payload == NULL || out_buffer == NULL || payload_len <= 1 || max_len == 0) {
        if (out_buffer != NULL && max_len > 0) {
            out_buffer[0] = '\0';
        }
        return false;
    }

    memset(out_buffer, 0, max_len);
    uint8_t write_idx = 0;

    // Itera sobre o payload pulando o primeiro byte (assumido como tamanho/comando)
    for (int i = 0; (write_idx < (max_len - 1)) && ((1 + i) < payload_len); i++) {
        char c = (char)payload[1 + i];

        if (c == (char)0xFF) {
            break;
        }

        // Ignora caracteres não imprimíveis (controle)
        if (c < ' ') {
            continue;
        }

        out_buffer[write_idx] = c;
        write_idx++;
    }

    out_buffer[write_idx] = '\0';
    return true;
}