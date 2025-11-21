/*
 * Nome do Arquivo: graos_handler.h
 * Descrição: Interface para o módulo de gerenciamento de seleção de grãos, pesquisa e paginação
 * Autor: Gabriel Agune
 */

#ifndef GRAOS_HANDLER_H
#define GRAOS_HANDLER_H

// ============================================================
// Includes
// ============================================================

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

// Processa o evento de entrada na tela de seleção de grãos
void Graos_Handle_Entrada_Tela(void);

// Processa um evento de navegação (tecla) na tela de seleção
void Graos_Handle_Navegacao(int16_t tecla);

// Trata o evento de recebimento de texto de pesquisa do DWIN
void Graos_Handle_Pesquisa_Texto(const uint8_t* data, uint16_t len);

// Trata o evento de clique no botão de mudança de página
void Graos_Handle_Page_Change(void);

// Verifica se a lógica de seleção de grãos está ativa
bool Graos_Esta_Em_Tela_Selecao(void);

// Exibe os resultados da pesquisa na tela atual
void Graos_Exibir_Resultados_Pesquisa(void);

// Executa a lógica de pesquisa de grãos baseado em um termo
void Graos_Executar_Pesquisa(const char* termo_pesquisa);

// Confirma a seleção de um grão a partir de um slot de resultado
void Graos_Confirmar_Selecao_Pesquisa(uint8_t slot_selecionado);

// Limpa todas as variáveis de estado de pesquisa e paginação
void Graos_Limpar_Resultados_Pesquisa(void);

#endif // GRAOS_HANDLER_H