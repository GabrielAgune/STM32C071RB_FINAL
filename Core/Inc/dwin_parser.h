/*
 * Nome do Arquivo: dwin_parser.h
 * Descrição: Parser de strings robusto para payloads do protocolo DWIN
 * Autor: Gabriel Agune
 */

#ifndef DWIN_PARSER_H
#define DWIN_PARSER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Extrai string de um payload, ignorando 0xFF e caracteres de controle
bool DWIN_Parse_String_Payload_Robust(const uint8_t* payload, uint16_t payload_len, char* out_buffer, uint8_t max_len);

#endif // DWIN_PARSER_H