#ifndef GRAOS_HANDLER_H
#define GRAOS_HANDLER_H

#include <stdint.h>
#include <stdbool.h>


/**
 * @brief Processa o evento de entrada na tela de sele??o de gr?os.
 * Orquestra a inicializa??o da l?gica e a atualiza??o da UI.
 */
void Graos_Handle_Entrada_Tela(void);

/**
 * @brief Processa um evento de navega??o (tecla) na tela de sele??o.
 * Orquestra a execu??o da l?gica de navega??o e a atualiza??o da UI.
 * @param tecla O c?digo da tecla recebida do DWIN.
 */
void Graos_Handle_Navegacao(int16_t tecla);

/**
 * @brief Verifica se a l?gica de sele??o de gr?os est? ativa.
 * @return true se a tela de sele??o estiver ativa, false caso contr?rio.
 */
bool Graos_Esta_Em_Tela_Selecao(void);

void Graos_Exibir_Resultados_Pesquisa(void);
void Graos_Executar_Pesquisa(const char* termo_pesquisa);
void Graos_Confirmar_Selecao_Pesquisa(uint8_t slot_selecionado);
void Graos_Limpar_Resultados_Pesquisa(void);;

void Graos_Handle_Pesquisa_Texto(const uint8_t* data, uint16_t len);

/**
 * @brief (NOVA FUN??O) Trata o evento de clique no bot?o de mudan?a de p?gina.
 */
void Graos_Handle_Page_Change(void);

#endif // GRAOS_HANDLER_H