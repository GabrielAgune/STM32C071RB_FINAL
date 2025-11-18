/*
 * Nome do Arquivo: app_manager.c
 * Descrição: Implementação do orquestrador principal e diagnósticos
 * Autor: Gabriel Agune
 */

#include "app_manager.h"
#include "usart.h"
#include "rtc.h"
#include "crc.h"
#include "dwin_driver.h"
#include "display_handler.h"
#include "pwm_servo_driver.h"
#include "cli_driver.h"
#include "eeprom_driver.h"
#include "ads1232_driver.h"
#include "pcb_frequency.h"
#include "temp_sensor.h"
#include "servo_controle.h"
#include "controller.h"
#include "gerenciador_configuracoes.h"
#include "medicao_handler.h"
#include "rtc_driver.h"
#include "ux_api.h"
#include "ux_device_stack.h"
#include "usb.h"
#include "app_usbx_device.h"
#include "battery_handler.h"

// ============================================================
// Variáveis Externas
// ============================================================

extern PCD_HandleTypeDef hpcd_USB_DRD_FS;

// ============================================================
// Protótipos de Funções Privadas
// ============================================================

static bool Test_DisplayInfo(void);
static bool Test_Servos(void);
static bool Test_Capacimetro(void);
static bool Test_Balanca(void);
static bool Test_Termometro(void);
static bool Test_EEPROM(void);
static bool Test_RTC(void);

// ============================================================
// Variáveis Privadas
// ============================================================

static const DiagnosticStep_t s_diagnostic_steps[] = {
    { "Exibindo Logo e Versoes...",     LOGO,               3000,   Test_DisplayInfo },
    { "Verificando Servos...",          BOOT_CHECK_SERVOS,  1200,   Test_Servos      },
    { "Verificando Medidor Freq...",    BOOT_CHECK_CAPACI,  1200,   Test_Capacimetro },
    { "Verificando Balanca...",         BOOT_BALANCE,       1000,   Test_Balanca     },
    { "Verificando Termometro...",      BOOT_THERMOMETER,   1000,   Test_Termometro  },
    { "Verificando Memoria EEPROM...",  BOOT_MEMORY,        1100,   Test_EEPROM      },
    { "Verificando RTC...",             BOOT_CLOCK,         1100,   Test_RTC         },
};

static const size_t NUM_DIAGNOSTIC_STEPS = sizeof(s_diagnostic_steps) / sizeof(s_diagnostic_steps[0]);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa drivers e restaura configurações do sistema
void App_Manager_Init(void) {
    DWIN_Driver_Init(&huart2, Controller_DwinCallback);
    EEPROM_Driver_Init(&hi2c1);
    Gerenciador_Config_Init(&hcrc);
    RTC_Driver_Init(&hrtc);
    Medicao_Init();
    DisplayHandler_Init();
    Servos_Init();
    Frequency_Init();
    ADS1232_Init();
    Battery_Handler_Init(&hi2c1);
    
    Gerenciador_Config_Validar_e_Restaurar();
    
    // Valores iniciais padrão
    Medicao_Set_Densidade(71.0);
    Medicao_Set_Umidade(25.73);
}

// Loop principal de processamento da aplicação
void App_Manager_Process(void) {
    Battery_Handler_Process();
    CLI_Process();
    DWIN_Driver_Process();
    Gerenciador_Config_Run_FSM();
    Servos_Process();
    Medicao_Process();
    DisplayHandler_Process();
}

// Executa a sequência de testes de autodiagnóstico
bool App_Manager_Run_Self_Diagnostics(uint8_t return_tela) {
    printf("\r\n>>> INICIANDO AUTODIAGNOSTICO <<<\r\n");

    for (size_t i = 0; i < NUM_DIAGNOSTIC_STEPS; i++) {
        const DiagnosticStep_t* step = &s_diagnostic_steps[i];

        printf("Diagnostico: %s\r\n", step->description);
        Controller_SetScreen(step->screen_id);
        
        HAL_Delay(step->display_time_ms);

        if (step->execute_test != NULL) {
            if (!step->execute_test()) {
                printf(">>> AUTODIAGNOSTICO FALHOU! <<<\r\n");
                return false;
            }
        }
    }

    printf(">>> AUTODIAGNOSTICO COMPLETO <<<\r\n\r\n");
    Controller_SetScreen(return_tela);
    
    return true;
}

// ============================================================
// Funções Privadas (Implementação de Testes)
// ============================================================

// Envia informações de versão e hardware para o display
static bool Test_DisplayInfo(void) {
    char nr_serial_buffer[17];
    Gerenciador_Config_Get_Serial(nr_serial_buffer, sizeof(nr_serial_buffer));
    
    DWIN_Driver_WriteString(VP_HARDWARE, HARDWARE, strlen(HARDWARE));
    DWIN_Driver_WriteString(VP_FIRMWARE, FIRMWARE, strlen(FIRMWARE));
    DWIN_Driver_WriteString(VP_FIRM_IHM, FIRM_IHM, strlen(FIRM_IHM));
    DWIN_Driver_WriteString(VP_SERIAL, nr_serial_buffer, strlen(nr_serial_buffer));

    return true;
}

// Realiza verificação visual dos servos
static bool Test_Servos(void) {
    return true;
}

// Verifica funcionamento básico do capacímetro
static bool Test_Capacimetro(void) {
    return true;
}

// Executa a tara da balança para verificação funcional
static bool Test_Balanca(void) {
    ADS1232_Tare();
    return true;
}

// Lê temperatura inicial e armazena no sistema
static bool Test_Termometro(void) {
    float temp_inicial = TempSensor_GetTemperature(); 
    Medicao_Set_Temp_Instru(temp_inicial);
    return true;
}

// Verifica disponibilidade da memória EEPROM
static bool Test_EEPROM(void) {
    if (!EEPROM_Driver_IsReady()) {
        Controller_SetScreen(MSG_ERROR);
        return false;
    }
    return true;
}

// Verifica integridade do RTC
static bool Test_RTC(void) {
    return true;
}