/*
 * Nome do Arquivo: relato.c
 * Descrição: Implementação da formatação de relatórios Who_am_i e Medição
 * Autor: Gabriel Agune
 */

// ============================================================
// Includes
// ============================================================

#include "relato.h"
#include "gerenciador_configuracoes.h"
#include "medicao_handler.h"
#include "rtc_driver.h"
#include "dwin_driver.h"
#include "cli_driver.h"
#include <stdio.h>

// ============================================================
// Variáveis Privadas e Constantes de Formatação
// ============================================================

static char qr_buffer[400];

// Constantes de formatação para impressora
const char Ejeta[] = "================================\n\r\n\r\n\r\n\r";
const char Dupla[] = "\n\r================================\n\r";
const char Linha[] = "--------------------------------\n\r";

// ============================================================
// Protótipos de Funções Públicas (Para uso interno do módulo)
// ============================================================

void Cabecalho(void);
void Assinatura(void);

// ============================================================
// Funções Públicas
// ============================================================

// Imprime informações de identificação do dispositivo (CLI/Impressora)
void Who_am_i(void) {
    char serial[17];
    Gerenciador_Config_Get_Serial(serial, sizeof(serial));

    printf(Dupla);
    printf("         G620_Teste_Gab\n\r");
    printf("     (c) GEHAKA, 2004-2025\n\r");
    printf(Linha);
    printf("CPU      =           STM32C071RB\n\r");
    printf("Firmware = %21s\r\n", FIRMWARE);
    printf("Hardware = %21s\r\n", HARDWARE);
    printf("Serial   = %21s\r\n", serial);
    printf(Linha);
    printf("Medidas  = %21d\n\r", 22);
    printf(Ejeta);
}

// Gera e envia o relatório formatado para a impressora
void Relatorio_Printer (void) {
    uint16_t nr_decimals = Gerenciador_Config_Get_NR_Decimals();

    uint8_t indice_grao_ativo = 0;
    Gerenciador_Config_Get_Grao_Ativo(&indice_grao_ativo);

    Config_Grao_t dados_grao_ativo;
    Gerenciador_Config_Get_Dados_Grao(indice_grao_ativo, &dados_grao_ativo);

    DadosMedicao_t medicao_snapshot;
    Medicao_Get_UltimaMedicao(&medicao_snapshot);

    Cabecalho();

    printf("Produto       = %16s\n\r", dados_grao_ativo.nome);
  	printf("Versao Equacao= %10lu\n\r", (unsigned long)dados_grao_ativo.id_curva);
  	printf("Validade Curva= %13s\n\r", dados_grao_ativo.validade);
  	printf("Amostra Numero= %8i\n\r", 4);
  	printf("Temp.Amostra .= %8.1f 'C\n\r", 22.0);
  	printf("Temp.Instru ..= %8.1f 'C\n\r", medicao_snapshot.Temp_Instru);
  	printf("Peso Amostra .= %8.1f g\n\r", medicao_snapshot.Peso);
  	printf("Densidade ....= %8.1f Kg/hL\n\r", medicao_snapshot.Densidade);
    printf(Linha);
  	printf("Umidade ......= %14.*f %%\n\r", (int)nr_decimals, medicao_snapshot.Umidade);
  	printf(Linha);

  	Assinatura();
}

// Gera a string formatada para o QR Code e a envia ao display
void Relatorio_QRCode_WhoAmI(void) {
    char serial[17];
    Gerenciador_Config_Get_Serial(serial, sizeof(serial));

    uint16_t nr_decimals = Gerenciador_Config_Get_NR_Decimals();

    uint8_t indice_grao = 0;
    Gerenciador_Config_Get_Grao_Ativo(&indice_grao);

    Config_Grao_t grao;
    Gerenciador_Config_Get_Dados_Grao(indice_grao, &grao);

    DadosMedicao_t dados;
    Medicao_Get_UltimaMedicao(&dados);

    uint8_t hh=0, mm=0, ss=0, dd=0, mo=0, yy=0;
    char weekday_dummy[4];
    RTC_Driver_GetTime(&hh, &mm, &ss);
    RTC_Driver_GetDate(&dd, &mo, &yy, weekday_dummy);

    int n = snprintf(qr_buffer, sizeof(qr_buffer),
                     "G620_Teste_Gab\n"
                     "===================\n\r"
                     "Produto: %.*s\n"
										 "Umidade: %.*f %%\n"
                     "Curva: %lu\n"
                     "Amostra: %d\n"
                     "Temp. instru: %.1f C\n"
                     "Peso: %.1f g\n"
                     "Densidade: %.1f Kg/hL\n"
										 "Validade: %s\n"
										 "===================\n\r"
                     "Data: %02u/%02u/%02u\n"
                     "Hora: %02u:%02u:%02u",
                     MAX_NOME_GRAO_LEN, grao.nome, 
										 (int)nr_decimals, dados.Umidade,
                     (unsigned long)grao.id_curva,
                     4,
                     dados.Temp_Instru,
                     dados.Peso,
                     dados.Densidade,
										 grao.validade,
                     dd, mo, yy,
                     hh, mm, ss);

    DWIN_Driver_WriteString(RESULTADO_MEDIDA, qr_buffer, sizeof(qr_buffer));
}

// ============================================================
// Funções Auxiliares (Impressora)
// ============================================================

// Imprime o cabeçalho do relatório
void Cabecalho(void) {
    char serial[17] = {0};
    Gerenciador_Config_Get_Serial(serial, sizeof(serial));

    printf(Dupla);
		printf("GEHAKA            G620_Teste_Gab\n\r");
    printf(Linha);
		printf("Versao Firmware= %15s\n\r", FIRMWARE);
		printf("Numero de Serie= %15s\n\r", serial);
    printf(Linha);
}

// Imprime o rodapé com data e responsável
void Assinatura(void) {
    uint8_t hours, minutes, seconds;
    uint8_t day, month, year;
    char weekday_dummy[4];

    printf("\n\r\n\r");
    printf(Linha);
    if (RTC_Driver_GetTime(&hours, &minutes, &seconds)) {
        printf("Assinatura              %02d:%02d:%02d\n\r", hours, minutes, seconds);
    }
    if (RTC_Driver_GetDate(&day, &month, &year, weekday_dummy)) {
        printf("Responsavel             %02d/%02d/%02d\n\r", day, month, year);
    }
    printf ("\n\r\n\r\n\r\n\r");
}