/*
 * Nome do Arquivo: display_handler.c
 * Descrição: Implementação das FSMs de medição e atualizações de UI
 * Autor: Gabriel Agune
 */

#include "display_handler.h"
#include "dwin_driver.h"
#include "controller.h"
#include "gerenciador_configuracoes.h"
#include "medicao_handler.h"
#include "rtc_driver.h"
#include "relato.h"
#include "temp_sensor.h"
#include "dwin_parser.h"


// ============================================================
// Defines e Constantes
// ============================================================

#define DWIN_VP_ENTRADA_TELA    0x0050
#define DWIN_VP_ENTRADA_SERVICO 0x0000

// ============================================================
// Máquina de Estados (Medição)
// ============================================================

typedef enum {
    MEDE_STATE_IDLE,
    MEDE_STATE_ENCHE_CAMARA,
    MEDE_STATE_AJUSTANDO,
    MEDE_STATE_RASPA_CAMARA,
    MEDE_STATE_PESO_AMOSTRA,
    MEDE_STATE_TEMP_SAMPLE,
    MEDE_STATE_UMIDADE,
    MEDE_STATE_MOSTRA_RESULTADO
} MedeState_t;

// ============================================================
// Variáveis Estáticas
// ============================================================

static MedeState_t s_mede_state          = MEDE_STATE_IDLE;
static uint32_t    s_mede_last_tick      = 0;
static uint32_t    s_monitor_last_tick   = 0;
static uint32_t    s_clock_last_tick     = 0;
static bool        s_printing_enabled    = true;
static uint8_t     s_temp_update_counter = 0;

// Constantes de Intervalo
static const uint32_t MEDE_INTERVAL_MS             = 1000;
static const uint32_t MONITOR_UPDATE_INTERVAL_MS   = 1000;
static const uint32_t CLOCK_UPDATE_INTERVAL_MS     = 1000;
static const uint8_t  TEMP_UPDATE_PERIOD_SECONDS   = 5;

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static void UpdateMonitorScreen(void);
static void UpdateClockOnMainScreen(void);
static void ProcessMeasurementSequenceFSM(void);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa o estado do módulo
void DisplayHandler_Init(void) {
    s_mede_state = MEDE_STATE_IDLE;
    s_printing_enabled = true;
}

// Executa as lógicas de FSM e atualizações periódicas
void DisplayHandler_Process(void) {
	ProcessMeasurementSequenceFSM();
	UpdateMonitorScreen();
	UpdateClockOnMainScreen();
}

// Inicia a sequência de telas para a medição (disparador da FSM)
void Display_StartMeasurementSequence(void) {
    if (s_mede_state == MEDE_STATE_IDLE) {
        printf("DISPLAY: Iniciando sequencia de medicao...\r\n");
        s_mede_state = MEDE_STATE_ENCHE_CAMARA;
        s_mede_last_tick = HAL_GetTick();
        Controller_SetScreen(MEDE_ENCHE_CAMARA);
    }
}

// Liga/Desliga o display e ajusta o backlight
void Display_OFF(uint16_t received_value) {
	if (received_value == 0x0010) {
		Controller_SetScreen(SYSTEM_STANDBY);
		DWIN_Driver_WriteRawBytes(CMD_AJUSTAR_BACKLIGHT_10, sizeof(CMD_AJUSTAR_BACKLIGHT_10));
	} else {
		Controller_SetScreen(PRINCIPAL);
		DWIN_Driver_WriteRawBytes(CMD_AJUSTAR_BACKLIGHT_100, sizeof(CMD_AJUSTAR_BACKLIGHT_100));
	}
}

// Trata evento de impressão ou exibição de resultado final
void Display_ProcessPrintEvent(uint16_t received_value) {
    if (!s_printing_enabled && received_value != 0x0000) {
        return;
    }

    if (received_value == 0x0000) {
        Config_Grao_t  dados_grao;
        DadosMedicao_t dados_medicao;
        uint8_t        indice_grao;

        Gerenciador_Config_Get_Grao_Ativo(&indice_grao);
        Gerenciador_Config_Get_Dados_Grao(indice_grao, &dados_grao);
        Medicao_Get_UltimaMedicao(&dados_medicao);

        const uint16_t casas_decimais = Gerenciador_Config_Get_NR_Decimals();

        DWIN_Driver_WriteString(GRAO_A_MEDIR, dados_grao.nome, MAX_NOME_GRAO_LEN);
        DWIN_Driver_WriteInt(CURVA, dados_grao.id_curva);
        DWIN_Driver_WriteInt(UMI_MIN, (int16_t)(dados_grao.umidade_min * 10));
        DWIN_Driver_WriteInt(UMI_MAX, (int16_t)(dados_grao.umidade_max * 10));
				DWIN_Driver_WriteString(DATA_VAL, dados_grao.validade, sizeof(dados_grao.validade));

        if (casas_decimais == 1) {
            DWIN_Driver_WriteInt(UMIDADE_1_CASA, (int16_t)(dados_medicao.Umidade * 10.0f));
            Controller_SetScreen(MEDE_RESULT_01);
        } else {
            DWIN_Driver_WriteInt(UMIDADE_2_CASAS, (int16_t)(dados_medicao.Umidade * 100.0f));
            Controller_SetScreen(MEDE_RESULT_02);
        }
    } else {
        Relatorio_Printer();
    }
}

// Ajusta número de repetições de medição
void Display_SetRepeticoes(uint16_t received_value) {
    char buffer[40];
    if (received_value == DWIN_VP_ENTRADA_TELA) {
        uint16_t atual = Gerenciador_Config_Get_NR_Repetition();
        sprintf(buffer, "Atual NR_Repetition: %u", atual);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer, strlen(buffer));
        Controller_SetScreen(TELA_SETUP_REPETICOES);
    } else {
        Gerenciador_Config_Set_NR_Repetitions(received_value);
        sprintf(buffer, "Repeticoes: %u", received_value);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer, strlen(buffer));
    }
}

// Ajusta número de casas decimais
void Display_SetDecimals(uint16_t received_value) {
    char buffer[40];
    if (received_value == DWIN_VP_ENTRADA_TELA) {
        uint16_t atual = Gerenciador_Config_Get_NR_Decimals();
        sprintf(buffer, "Atual NR_Decimals: %u", atual);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer, strlen(buffer));
        Controller_SetScreen(TELA_SET_DECIMALS);
    } else {
        Gerenciador_Config_Set_NR_Decimals(received_value);
        sprintf(buffer, "Casas decimais: %u", received_value);
		DWIN_Driver_WriteString(VP_MESSAGES, buffer, strlen(buffer));
    }
}

// Atualiza o nome de usuário (string)
void Display_SetUser(const uint8_t* dwin_data, uint16_t len, uint16_t received_value) {
    char buffer_display[50];
    if (received_value == DWIN_VP_ENTRADA_TELA) {
        char nome_atual[21] = {0};
        Gerenciador_Config_Get_Usuario(nome_atual, sizeof(nome_atual));
        sprintf(buffer_display, "Atual Usuario: %s", nome_atual);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
        Controller_SetScreen(TELA_USER);
    } else {
        char novo_nome[21] = {0};
        const uint8_t* payload = &dwin_data[6];
        uint16_t payload_len = len - 6;

        if (DWIN_Parse_String_Payload_Robust(payload, payload_len, novo_nome, sizeof(novo_nome)) && strlen(novo_nome) > 0) {
            Gerenciador_Config_Set_Usuario(novo_nome);
            sprintf(buffer_display, "Usuario: %s", novo_nome);
            DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
        }
    }
}

// Atualiza o nome da empresa (string)
void Display_SetCompany(const uint8_t* dwin_data, uint16_t len, uint16_t received_value) {
    char buffer_display[50];
    if (received_value == DWIN_VP_ENTRADA_TELA) {
        char empresa_atual[21] = {0};
        Gerenciador_Config_Get_Company(empresa_atual, sizeof(empresa_atual));
        sprintf(buffer_display, "Atual Empresa: %s", empresa_atual);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
        Controller_SetScreen(TELA_COMPANY);
    } else {
        char nova_empresa[21] = {0};
        const uint8_t* payload = &dwin_data[6];
        uint16_t payload_len = len - 6;

        if (DWIN_Parse_String_Payload_Robust(payload, payload_len, nova_empresa, sizeof(nova_empresa)) && strlen(nova_empresa) > 0) {
            Gerenciador_Config_Set_Company(nova_empresa);
            sprintf(buffer_display, "Empresa: %s", nova_empresa);
            DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
        }
    }
}

// Entra na tela de ajuste do capacímetro
void Display_Adj_Capa(uint16_t received_value) {
    DWIN_Driver_WriteString(VP_MESSAGES, "AdjustFrequency: 3000.0KHz+/-2.0", strlen("AdjustFrequency: 3000.0KHz+/-2.0"));
    Controller_SetScreen(TELA_ADJUST_CAPA);
}

// Exibe tela "Sobre o sistema"
void Display_ShowAbout(void) {
    DWIN_Driver_WriteString(VP_MESSAGES, "G620_Teste_Gab", strlen("G620_Teste_Gab"));
    Controller_SetScreen(TELA_ABOUT_SYSTEM);
}

// Exibe tela de modelo/OEM
void Display_ShowModel(void) {
    DWIN_Driver_WriteString(VP_MESSAGES, "G620_Teste_Gab", strlen("G620_Teste_Gab"));
    Controller_SetScreen(TELA_MODEL_OEM);
}

// Lida com preset de configuração de fábrica
void Display_Preset(uint16_t received_value) {
    if (received_value == DWIN_VP_ENTRADA_SERVICO) {
        DWIN_Driver_WriteString(VP_MESSAGES, "Preset redefine os ajustes!", strlen("Preset redefine os ajustes!"));
        Controller_SetScreen(TELA_PRESET_PRODUCT);
    } else {
        Carregar_Configuracao_Padrao();
		DWIN_Driver_WriteString(VP_MESSAGES, "Peset completo!", strlen("Peset completo!"));
    }
}

// Configura o número de série do equipamento
void Display_Set_Serial(const uint8_t* dwin_data, uint16_t len, uint16_t received_value) {
    char buffer_display[50] = {0};

    if (received_value == DWIN_VP_ENTRADA_SERVICO) {
        Controller_SetScreen(TELA_SET_SERIAL);
        char serial_atual[17] = {0};
        Gerenciador_Config_Get_Serial(serial_atual, sizeof(serial_atual));
        sprintf(buffer_display, "%s", serial_atual);
        DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
    } else {
        char novo_serial[17] = {0};
        const uint8_t* payload = &dwin_data[5];
        uint16_t payload_len = len - 5;

        if (DWIN_Parse_String_Payload_Robust(payload, payload_len, novo_serial, sizeof(novo_serial)) && strlen(novo_serial) > 0) {
            printf("Display Handler: Recebido novo serial: '%s'\n", novo_serial);
            Gerenciador_Config_Set_Serial(novo_serial);
            sprintf(buffer_display, "Serial: %s", novo_serial);
            DWIN_Driver_WriteString(VP_MESSAGES, buffer_display, strlen(buffer_display));
        }
    }
}

// Habilita ou desabilita a impressão de relatório
void Display_SetPrintingEnabled(bool is_enabled) {
    s_printing_enabled = is_enabled;
    printf("Display Handler: Impressao %s\r\n", s_printing_enabled ? "HABILITADA" : "DESABILITADA");
}

// Retorna se a impressão está habilitada
bool Display_IsPrintingEnabled(void) {
    return s_printing_enabled;
}

// ============================================================
// Funções Privadas
// ============================================================

// Máquina de estados não-bloqueante para a sequência de medição
static void ProcessMeasurementSequenceFSM(void) {
    if (s_mede_state == MEDE_STATE_IDLE) {
        return;
    }

    if (HAL_GetTick() - s_mede_last_tick < MEDE_INTERVAL_MS) {
        return;
    }
    s_mede_last_tick = HAL_GetTick();

    switch (s_mede_state) {
        case MEDE_STATE_ENCHE_CAMARA:
            s_mede_state = MEDE_STATE_AJUSTANDO;
            Controller_SetScreen(MEDE_AJUSTANDO);
            break;
        case MEDE_STATE_AJUSTANDO:
            s_mede_state = MEDE_STATE_RASPA_CAMARA;
            Controller_SetScreen(MEDE_RASPA_CAMARA);
            break;
        case MEDE_STATE_RASPA_CAMARA:
            s_mede_state = MEDE_STATE_PESO_AMOSTRA;
            Controller_SetScreen(MEDE_PESO_AMOSTRA);
            break;
        case MEDE_STATE_PESO_AMOSTRA:
            s_mede_state = MEDE_STATE_TEMP_SAMPLE;
            Controller_SetScreen(MEDE_TEMP_SAMPLE);
            break;
        case MEDE_STATE_TEMP_SAMPLE:
            s_mede_state = MEDE_STATE_UMIDADE;
            Controller_SetScreen(MEDE_UMIDADE);
            break;
        case MEDE_STATE_UMIDADE:
            s_mede_state = MEDE_STATE_MOSTRA_RESULTADO;
            Display_ProcessPrintEvent(0x0000); // Exibe resultado na tela
            break;
        case MEDE_STATE_MOSTRA_RESULTADO:
            s_mede_state = MEDE_STATE_IDLE;
            printf("DISPLAY: Sequencia de medicao finalizada.\r\n");
            break;
        default:
            s_mede_state = MEDE_STATE_IDLE;
            break;
    }
}

// Atualiza os VPs da tela de Monitor/Ajuste periodicamente
static void UpdateMonitorScreen(void) {
    if (HAL_GetTick() - s_monitor_last_tick < MONITOR_UPDATE_INTERVAL_MS) {
        return;
    }
    s_monitor_last_tick = HAL_GetTick();

    uint16_t tela_atual = Controller_GetCurrentScreen();
    if (tela_atual != TELA_MONITOR_SYSTEM && tela_atual != TELA_ADJUST_CAPA) {
        s_temp_update_counter = 0;
        return;
    }

    DadosMedicao_t dados_atuais;
    Medicao_Get_UltimaMedicao(&dados_atuais);

    // Converte e envia dados de medição
    int32_t frequencia_para_dwin = (int32_t)(dados_atuais.Frequencia * 0.01f);
    DWIN_Driver_WriteInt32(FREQUENCIA, frequencia_para_dwin);

    int32_t escala_a_para_dwin = (int32_t)(dados_atuais.Escala_A * 10.0f);
    DWIN_Driver_WriteInt32(ESCALA_A, escala_a_para_dwin);

    // Atualiza a temperatura do instrumento em um ciclo mais lento
    s_temp_update_counter++;
    if (s_temp_update_counter >= TEMP_UPDATE_PERIOD_SECONDS) {
        s_temp_update_counter = 0;
        float temp_mcu = TempSensor_GetTemperature();
        Medicao_Set_Temp_Instru(temp_mcu);

        int16_t temperatura_para_dwin = (int16_t)(temp_mcu * 10.0f);
        DWIN_Driver_WriteInt(TEMP_INSTRU, temperatura_para_dwin);
    }
}

// Atualiza o relógio na tela principal e em telas relacionadas
static void UpdateClockOnMainScreen(void) {
    if (HAL_GetTick() - s_clock_last_tick < CLOCK_UPDATE_INTERVAL_MS) {
        return;
    }
    s_clock_last_tick = HAL_GetTick();

    switch (Controller_GetCurrentScreen()) {
        case PRINCIPAL:
        case MEDE_RESULT_01:
        case MEDE_RESULT_02:
        case TELA_SET_JUST_TIME:
        case TELA_ABOUT_SYSTEM:
        case TELA_ADJUST_TIME: {
            uint8_t h, m, s, d, mo, y;
            char weekday_dummy[4];

            if (RTC_Driver_GetTime(&h, &m, &s) && RTC_Driver_GetDate(&d, &mo, &y, weekday_dummy)) {
                // Montagem do comando DWIN para escrita da data e hora
                uint8_t rtc_command[] = {
                    0x5A, 0xA5,
                    0x0B,
                    0x82,
                    (VP_DATA_HORA >> 8) & 0xFF,
                    VP_DATA_HORA & 0xFF,
                    y,
                    mo,
                    d,
                    0x03,
                    h,
                    m,
                    s,
                    0x00
                };
                DWIN_Driver_WriteRawBytes(rtc_command, sizeof(rtc_command));
            }
        }
        default:
            break;
    }
}