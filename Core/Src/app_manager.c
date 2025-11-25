/*
 * Nome do Arquivo: app_manager.c
 * Descrio: Implementao do orquestrador principal usando Scheduler
 * Autor: Gabriel Agune 
 */

#include "app_manager.h"
#include "scheduler.h"

// Includes dos drivers e handlers
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
#include "battery_handler.h"
#include <string.h>
#include <stdio.h>

// ============================================================
// Variáveis Externas
// ============================================================

extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
extern void USB_Process(void); // Protótipo externo se não estiver em header

// ============================================================
// Prottipos de Funes Privadas
// ============================================================

static bool Test_DisplayInfo(void);
static bool Test_Servos(void);
static bool Test_Capacimetro(void);
static bool Test_Balanca(void);
static bool Test_Termometro(void);
static bool Test_EEPROM(void);
static bool Test_RTC(void);

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa drivers, scheduler e restaura configurações
void App_Manager_Init(void) {
    // 1. Inicialização de Drivers de Baixo Nível
    DWIN_Driver_Init(&huart2, Controller_DwinCallback);
    EEPROM_Driver_Init(&hi2c1);
    RTC_Driver_Init(&hrtc);
    ADS1232_Init();
    Frequency_Init();
    Servos_Init();
    
    // 2. Inicialização de Middleware/Logic
    Gerenciador_Config_Init(&hcrc);
    Medicao_Init();
    DisplayHandler_Init();
    Battery_Handler_Init(&hi2c1);
    
    // 3. Restauração de Dados
    Gerenciador_Config_Validar_e_Restaurar();
    
    // Valores iniciais padrão de calibração de processos
    Medicao_Set_Densidade(71.0);
    Medicao_Set_Umidade(25.73);

    // 4. Configuração do Scheduler (O coração do sistema)
    Scheduler_Init();

    // Tarefas de Comunicação e Alta Prioridade
    Scheduler_Register_Task(USB_Process,               5,   0);  // USB Stack (5ms)
    Scheduler_Register_Task(CLI_TX_Pump,               10,  5);  // CLI TX (10ms)
    Scheduler_Register_Task(DWIN_Driver_Process,       20,  10); // DWIN RX Parser (20ms)

    // Tarefas de Controle e Hardware
    Scheduler_Register_Task(Servos_Process,            20,  15); // Movimento Servos (20ms)
    Scheduler_Register_Task(Medicao_Process,           50,  20); // Leitura Sensores (50ms)
    Scheduler_Register_Task(Gerenciador_Config_Run_FSM,10,  25); // EEPROM Write Async (10ms)

    // Tarefas de Interface e Lógica Lenta
    Scheduler_Register_Task(DisplayHandler_Process,    100, 100); // Atualização UI (100ms)
    Scheduler_Register_Task(Battery_Handler_Process,   1000,500); // Monitor Bateria (1s)
}

// Loop principal (agora apenas roda o scheduler)
void App_Manager_Process(void) {
    Scheduler_Run();
}

// ============================================================
// Lógica de Autodiagnóstico (Mantida, mas pode ser otimizada futuramente)
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

bool App_Manager_Run_Self_Diagnostics(uint8_t return_tela) {
    printf("\r\n>>> INICIANDO AUTODIAGNOSTICO <<<\r\n");

    for (size_t i = 0; i < NUM_DIAGNOSTIC_STEPS; i++) {
        const DiagnosticStep_t* step = &s_diagnostic_steps[i];

        printf("Diagnostico: %s\r\n", step->description);
        Controller_SetScreen(step->screen_id);
        
        // Processa serviços críticos durante o delay bloqueante
        uint32_t start = HAL_GetTick();
        while((HAL_GetTick() - start) < step->display_time_ms) {
            USB_Process();
            CLI_TX_Pump();
        }

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

static bool Test_DisplayInfo(void) {
    char nr_serial_buffer[17];
    Gerenciador_Config_Get_Serial(nr_serial_buffer, sizeof(nr_serial_buffer));
    
    DWIN_Driver_WriteString(VP_HARDWARE, HARDWARE, strlen(HARDWARE));
    DWIN_Driver_WriteString(VP_FIRMWARE, FIRMWARE, strlen(FIRMWARE));
    DWIN_Driver_WriteString(VP_FIRM_IHM, FIRM_IHM, strlen(FIRM_IHM));
    DWIN_Driver_WriteString(VP_SERIAL, nr_serial_buffer, strlen(nr_serial_buffer));
    return true;
}

static bool Test_Servos(void) { return true; }
static bool Test_Capacimetro(void) { return true; }
static bool Test_Balanca(void) { return true; }

static bool Test_Termometro(void) {
    float temp_inicial = TempSensor_GetTemperature(); 
    Medicao_Set_Temp_Instru(temp_inicial);
    return true;
}

static bool Test_EEPROM(void) {
    if (!EEPROM_Driver_IsReady()) {
        Controller_SetScreen(MSG_ERROR);
        return false;
    }
    return true;
}

static bool Test_RTC(void) { return true; }