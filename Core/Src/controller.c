/*
 * Nome do Arquivo: controller.c
 * Descrição: Implementação do Controlador (Roteamento de Eventos e VPs)
 * Autor: Gabriel Agune
 */

#include "controller.h"
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Includes dos módulos do sistema
#include "dwin_driver.h"
#include "rtc.h"
#include "rtc_driver.h"
#include "gerenciador_configuracoes.h"
#include "autenticacao_handler.h"
#include "battery_handler.h"
#include "rtc_handler.h"
#include "display_handler.h"
#include "graos_handler.h"
#include "app_manager.h"
#include "relato.h"

// ============================================================
// Variáveis Privadas
// ============================================================

static int16_t  s_received_value    = 0;
static uint16_t s_current_screen_id = PRINCIPAL;

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static void Handle_Escape_Navigation(uint16_t received_value);

// ============================================================
// Funções Públicas
// ============================================================

// Retorna a tela atual armazenada na variável de estado
uint16_t Controller_GetCurrentScreen(void) {
    return s_current_screen_id;
}

// Define a nova tela no display e atualiza a variável de estado
void Controller_SetScreen(uint16_t screen_id) {
    s_current_screen_id = screen_id;
    DWIN_Driver_SetScreen(screen_id);
}

// Callback principal: processa pacotes UART e despacha para os Handlers
void Controller_DwinCallback(const uint8_t* data, uint16_t len) {
    // Validação básica do cabeçalho DWIN (0x5A 0xA5)
    if (len < 6 || data[0] != 0x5A || data[1] != 0xA5) {
        return;
    }

    // Verifica comando de escrita em VP (0x83)
    if (data[3] == 0x83) {
        uint16_t vp_address = (data[4] << 8) | data[5];

        if (len >= 8) {
            // Filtra VPs que enviam dados complexos (arrays) em vez de int16
            if (vp_address != SENHA_CONFIG && vp_address != SET_SENHA && vp_address != SET_TIME) {
                uint8_t payload_len = data[2];
                if (len >= (3 + payload_len)) {
                    s_received_value = (data[3 + payload_len - 2] << 8) | data[3 + payload_len - 1];
                }
            } else {
                s_received_value = 0;
            }
        }

        // Despachante de Comandos (Switch Central)
        switch (vp_address) {
            // ----------------------------------------------------
            // Tela Inicial e Operação
            // ----------------------------------------------------
            case DESCARTA_AMOSTRA:      Display_StartMeasurementSequence();             						break;
            case SELECT_GRAIN:          Graos_Handle_Entrada_Tela();                    						break;
            case PRINT:                 Display_ProcessPrintEvent(s_received_value);    						break;
            case OFF:                   Display_OFF(s_received_value);                 	 						break;
            case SHOW_MEDIDA:           Relatorio_QRCode_WhoAmI();                      						break;

            // ----------------------------------------------------
            // Menu Configurar
            // ----------------------------------------------------
            case SENHA_CONFIG:          Auth_ProcessLoginEvent(data, len);              						break;
            case ENTER_SET_TIME:        Controller_SetScreen(TELA_SET_JUST_TIME);       						break;
            case SET_TIME:              RTC_Handle_Set_Time(data, len, s_received_value); 					break;
            case NR_REPETICOES:         Display_SetRepeticoes(s_received_value);        						break;
            case DECIMALS:              Display_SetDecimals(s_received_value);          						break;
            case DES_HAB_PRINT:         Display_SetPrintingEnabled(s_received_value == 0x01); 			break;
            case SET_SENHA:             Auth_ProcessSetPasswordEvent(data, len);        						break;
            case DIAGNOSTIC:            App_Manager_Run_Self_Diagnostics(TELA_AUTO_DIAGNOSIS); 			break;
            case USER:                  Display_SetUser(data, len, s_received_value);  						  break;
            case COMPANY:               Display_SetCompany(data, len, s_received_value);						break;
            case ABOUT_SYS:             Display_ShowAbout();                            						break;

            // ----------------------------------------------------
            // Menu Serviço
            // ----------------------------------------------------
            case PRESET_PRODUCT:        Display_Preset(s_received_value);               						break;
            case SET_DATE_TIME:         RTC_Handle_Set_Date_And_Time(data, len, s_received_value); 	break;
            case MODEL_OEM:             Display_ShowModel();                            						break;
            case ADJUST_SCALE:          /* Handler não implementado */                  						break;
            case ADJUST_TERMO:          /* Handler não implementado */                  						break;
            case ADJUST_CAPA:           Display_Adj_Capa(s_received_value);             						break;
            case SET_SERIAL:            Display_Set_Serial(data, len, s_received_value); 						break;
            case SET_UNITS:             /* Handler não implementado */                  						break;
            case MONITOR:               Controller_SetScreen(TELA_MONITOR_SYSTEM);      						break;
            case SERVICE_REPORT:        /* Handler não implementado */                  						break;
            case SYSTEM_BURNIN:         /* Handler não implementado */                  						break;
            case BATTERY_INFORMATION:   Controller_SetScreen(TELA_BATERIA);            							break;

            // ----------------------------------------------------
            // Navegação e Pesquisa
            // ----------------------------------------------------	
            case TECLAS:                Graos_Handle_Navegacao(s_received_value);      							break;
            case ESCAPE:                Handle_Escape_Navigation(s_received_value);     						break;
            case VP_SEARCH_INPUT:       Graos_Handle_Pesquisa_Texto(data, len);        						  break;
            case VP_RESULT_SELECT:      Graos_Confirmar_Selecao_Pesquisa(s_received_value); 				break;
            case VP_PAGE_INDICATOR:     Graos_Handle_Page_Change();                     						break;

            default:
                break;
        }
    }
}

// ============================================================
// Funções Privadas
// ============================================================

// Gerencia a lógica do botão ESC dependendo do contexto
static void Handle_Escape_Navigation(uint16_t received_value) {
    if (received_value == 0x0051) {
        Controller_SetScreen(TELA_SERVICO);
        printf("CONTROLLER: Tela de Servico.\r\n");
    } else {
        Controller_SetScreen(PRINCIPAL);
        printf("CONTROLLER: Tela Principal.\r\n");
    }
}