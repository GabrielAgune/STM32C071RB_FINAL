/*
 * Nome do Arquivo: autenticacao_handler.h
 * Descrição: Interface para o módulo de gerenciamento de login e senha
 * Autor: Gabriel Agune
 */

#ifndef AUTENTICACAO_HANDLER_H
#define AUTENTICACAO_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Processa o evento de tentativa de login vindo do controller
void Auth_ProcessLoginEvent(const uint8_t* dwin_data, uint16_t len);

// Processa o evento de definição de nova senha vindo do controller
void Auth_ProcessSetPasswordEvent(const uint8_t* dwin_data, uint16_t len);

#endif // AUTENTICACAO_HANDLER_H