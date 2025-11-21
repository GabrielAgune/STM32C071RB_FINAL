/*
 * Nome do Arquivo: relato.h
 * Descrição: Interface do módulo de formatação de relatórios (Impressora e QR Code)
 * Autor: Gabriel Agune
 */

#ifndef RELATO_H
#define RELATO_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include "stm32c0xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// ============================================================
// Protótipos de Funções Externas
// ============================================================

// Imprime informações de identificação do dispositivo (CLI/Impressora)
extern void Who_am_i(void);

// Imprime o rodapé com data e responsável (Impressora)
extern void Assinatura(void);

// Imprime o cabeçalho do relatório (Impressora)
extern void Cabecalho(void);

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Gera e envia o relatório formatado para a impressora
extern void Relatorio_Printer(void);

// Gera a string formatada para o QR Code e a envia ao display
extern void Relatorio_QRCode_WhoAmI(void);

#endif /* RELATO_H */