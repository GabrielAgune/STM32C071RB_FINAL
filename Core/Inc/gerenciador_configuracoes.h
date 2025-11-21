/*
 * Nome do Arquivo: gerenciador_configuracoes.h
 * Descrição: API para gerenciar as configurações da aplicação (Cache, EEPROM, CRC)
 * Autor: Gabriel Agune, Refatorado por Copilot (Senior Embedded Arch)
 */

#ifndef GERENCIADOR_CONFIGURACOES_H
#define GERENCIADOR_CONFIGURACOES_H

// ============================================================
// Includes
// ============================================================

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Definições de Configuração
// ============================================================

#define MAX_GRAOS               135
#define MAX_NOME_GRAO_LEN       16
#define MAX_SENHA_LEN           10
#define MAX_VALIDADE_LEN        10
#define MAX_USUARIOS            10

#define HARDWARE                "1.00"
#define FIRMWARE                "0.00.001"
#define FIRM_IHM                "0.00.02"

// ============================================================
// Estruturas de Dados
// ============================================================

typedef struct {
    char        nome[MAX_NOME_GRAO_LEN + 1];
    char        validade[MAX_VALIDADE_LEN + 2];
    uint32_t    id_curva;
    int16_t     umidade_min;
    int16_t     umidade_max;
} Config_Grao_t;

typedef struct {
	char        Nome[20];
	char        Empresa[20];
} Config_Usuario_t;

typedef struct {
    uint32_t          versao_struct;
    uint8_t           indice_idioma_selecionado;
    uint8_t           indice_grao_ativo;
    uint8_t           preenchimento[2];
    char              senha_sistema[MAX_SENHA_LEN + 2];
    float             fat_cal_a_gain;
    float             fat_cal_a_zero;
	uint16_t          nr_repetition;
	uint16_t          nr_decimals;
	Config_Grao_t     graos[MAX_GRAOS];
	Config_Usuario_t  usuarios[MAX_USUARIOS];
	char              nr_serial[16];
    uint32_t          crc; // IMPORTANTE: Deve ser o último membro
} Config_Aplicacao_t;

// ============================================================
// Mapeamento de Memória (EEPROM)
// ============================================================

#define EEPROM_TOTAL_SIZE_BYTES 65536

#define CONFIG_BLOCK_SIZE       sizeof(Config_Aplicacao_t)
#define ADDR_CONFIG_PRIMARY     0x0000
#define ADDR_CONFIG_BACKUP1     (ADDR_CONFIG_PRIMARY + CONFIG_BLOCK_SIZE)
#define ADDR_CONFIG_BACKUP2     (ADDR_CONFIG_BACKUP1 + CONFIG_BLOCK_SIZE)

#define END_OF_CONFIG_DATA      (ADDR_CONFIG_BACKUP2 + CONFIG_BLOCK_SIZE)

// ============================================================
// API Pública do Módulo
// ============================================================

// Inicializa o gerenciador de configurações
void Gerenciador_Config_Init(CRC_HandleTypeDef* hcrc);

// Valida as cópias na EEPROM e carrega a configuração válida para o cache
bool Gerenciador_Config_Validar_e_Restaurar(void);

// Carrega os valores padrão de fábrica para o cache RAM e marca para salvar
void Carregar_Configuracao_Padrao(void);

// Máquina de estados do gerenciador (orquestra a escrita não-bloqueante)
void Gerenciador_Config_Run_FSM(void);


// Obtém o erro do último salvamento e limpa a flag
bool Gerenciador_Config_GetAndClearErrorFlag(void);

// Sinaliza que a configuração em cache foi modificada e precisa ser salva
void Gerenciador_Config_Marcar_Como_Pendente(void);

// Verifica se há configurações pendentes para salvar
bool Gerenciador_Config_Ha_Pendencias(void);

// --- Funções "Get" e "Set" ---

bool Gerenciador_Config_Set_Indice_Idioma(uint8_t novo_indice);
bool Gerenciador_Config_Get_Indice_Idioma(uint8_t* indice);

bool Gerenciador_Config_Set_Senha(const char* nova_senha);
bool Gerenciador_Config_Get_Senha(char* buffer, uint8_t tamanho_buffer);

bool Gerenciador_Config_Set_Grao_Ativo(uint8_t novo_indice);
bool Gerenciador_Config_Get_Grao_Ativo(uint8_t* indice_ativo);

bool Gerenciador_Config_Get_Dados_Grao(uint8_t indice, Config_Grao_t* dados_grao);
uint8_t Gerenciador_Config_Get_Num_Graos(void);

bool Gerenciador_Config_Set_Cal_A(float gain, float zero);
bool Gerenciador_Config_Get_Cal_A(float* gain, float* zero);

bool Gerenciador_Config_Set_NR_Repetitions(uint16_t nr_repetitions);
uint16_t Gerenciador_Config_Get_NR_Repetition(void);

bool Gerenciador_Config_Set_NR_Decimals(uint16_t nr_decimals);
uint16_t Gerenciador_Config_Get_NR_Decimals(void);

bool Gerenciador_Config_Set_Usuario(const char* novo_usuario);
bool Gerenciador_Config_Get_Usuario(char* usuario, uint8_t tamanho_usuario);

bool Gerenciador_Config_Set_Company(const char* nova_empresa);
bool Gerenciador_Config_Get_Company(char* empresa, uint8_t tamanho_empresa);

bool Gerenciador_Config_Set_Serial(const char* novo_serial);
bool Gerenciador_Config_Get_Serial(char* serial, uint8_t tamanho_buffer);

#endif // GERENCIADOR_CONFIGURACOES_H