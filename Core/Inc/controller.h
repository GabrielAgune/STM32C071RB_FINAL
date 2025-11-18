/*
 * Nome do Arquivo: controller.h
 * Descrição: Interface do Controlador do Sistema (Arquitetura MVC)
 * Autor: Gabriel Agune
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Defines
// ============================================================

/* Códigos de teclas recebidos do DWIN */
#define DWIN_TECLA_SETA_ESQ     0x03
#define DWIN_TECLA_SETA_DIR     0x02
#define DWIN_TECLA_CONFIRMA     0x01
#define DWIN_TECLA_ESCAPE       0x06

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Função de callback registrada no driver DWIN para receber dados brutos
void     Controller_DwinCallback(const uint8_t* data, uint16_t len);

// Retorna o ID da tela que o controlador assume estar ativa
uint16_t Controller_GetCurrentScreen(void);

// Altera a tela atual e atualiza o estado interno do controlador
void     Controller_SetScreen(uint16_t screen_id);

#endif /* CONTROLLER_H */