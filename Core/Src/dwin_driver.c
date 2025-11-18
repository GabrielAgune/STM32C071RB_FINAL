/*
 * Nome do Arquivo: dwin_driver.c
 * Descrição: Implementação do driver DWIN com suporte a UART/DMA
 * Autor: Gabriel Agune
 */

#include "dwin_driver.h"
#include <string.h>

// ============================================================
// Variáveis Privadas
// ============================================================

static UART_HandleTypeDef *s_huart              = NULL;
static dwin_rx_callback_t  s_rx_callback        = NULL;
static uint8_t             s_rx_buffer[DWIN_RX_BUFFER_SIZE];

static volatile bool       s_frame_received     = false;
static volatile uint16_t   s_received_len       = 0;
static volatile uint32_t   s_rx_packet_counter  = 0;

// ============================================================
// Funções Privadas
// ============================================================

// Rearma a recepção via DMA/Idle Line
static void DWIN_Restart_Rx(void) {
    if (s_huart == NULL) {
        return;
    }

    if (HAL_UARTEx_ReceiveToIdle_IT(s_huart, s_rx_buffer, DWIN_RX_BUFFER_SIZE) != HAL_OK) {
        Error_Handler();
    }
}

// ============================================================
// Funções Públicas
// ============================================================

// Inicializa o driver e configura os callbacks
void DWIN_Driver_Init(UART_HandleTypeDef *huart, dwin_rx_callback_t callback) {
    s_huart = huart;
    s_rx_callback = callback;

    s_frame_received = false;
    s_received_len = 0;
    s_rx_packet_counter = 0;

    DWIN_Restart_Rx();
}

// Processa o buffer recebido e chama o callback do usuário
void DWIN_Driver_Process(void) {
    if (!s_frame_received) {
        return;
    }

    uint8_t  local_buffer[DWIN_RX_BUFFER_SIZE];
    uint16_t local_len;

    __disable_irq();
    local_len = s_received_len;
    s_frame_received = false;
    __enable_irq();

    if ((local_len == 0u) || (local_len > DWIN_RX_BUFFER_SIZE)) {
        return;
    }

    memcpy(local_buffer, s_rx_buffer, local_len);

    if (s_rx_callback != NULL) {
        s_rx_callback(local_buffer, local_len);
    }
}

// Retorna o contador total de pacotes recebidos
uint32_t DWIN_Driver_GetRxPacketCounter(void) {
    return s_rx_packet_counter;
}

// Altera a tela atual exibida no display
bool DWIN_Driver_SetScreen(uint16_t screen_id) {
    if (s_huart == NULL) {
        return false;
    }

    const uint16_t VP_ADDR_PIC_ID = 0x0084;

    uint8_t cmd_buffer[] = {
        0x5A, 0xA5,
        0x07,
        0x82,
        (uint8_t)(VP_ADDR_PIC_ID >> 8),
        (uint8_t)(VP_ADDR_PIC_ID & 0xFF),
        0x5A, 0x01,
        (uint8_t)(screen_id >> 8),
        (uint8_t)(screen_id & 0xFF)
    };

    return (HAL_UART_Transmit(s_huart, cmd_buffer, sizeof(cmd_buffer), DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Escreve um valor inteiro de 16 bits em um endereço VP
bool DWIN_Driver_WriteInt(uint16_t vp_address, int16_t value) {
    if (s_huart == NULL) {
        return false;
    }

    uint8_t cmd_buffer[] = {
        0x5A, 0xA5,
        0x05,
        0x82,
        (uint8_t)(vp_address >> 8),
        (uint8_t)(vp_address & 0xFF),
        (uint8_t)(value >> 8),
        (uint8_t)(value & 0xFF)
    };

    return (HAL_UART_Transmit(s_huart, cmd_buffer, sizeof(cmd_buffer), DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Escreve um valor inteiro de 32 bits em um endereço VP
bool DWIN_Driver_WriteInt32(uint16_t vp_address, int32_t value) {
    if (s_huart == NULL) {
        return false;
    }

    uint8_t cmd_buffer[] = {
        0x5A, 0xA5,
        0x07,
        0x82,
        (uint8_t)(vp_address >> 8),
        (uint8_t)(vp_address & 0xFF),
        (uint8_t)((value >> 24) & 0xFF),
        (uint8_t)((value >> 16) & 0xFF),
        (uint8_t)((value >> 8) & 0xFF),
        (uint8_t)(value & 0xFF)
    };

    return (HAL_UART_Transmit(s_huart, cmd_buffer, sizeof(cmd_buffer), DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Escreve uma string ASCII em um endereço VP
bool DWIN_Driver_WriteString(uint16_t vp_address, const char *text, uint16_t max_len) {
    if ((s_huart == NULL) || (text == NULL) || (max_len == 0u)) {
        return false;
    }

    size_t text_len = strlen(text);
    if (text_len > max_len) {
        text_len = max_len;
    }

    uint8_t payload_len = (uint8_t)(3u + text_len + 2u);
    uint16_t total_frame_size = (uint16_t)(3u + payload_len);
    uint8_t frame[3 + 3 + max_len + 2];

    frame[0] = 0x5A;
    frame[1] = 0xA5;
    frame[2] = payload_len;
    frame[3] = 0x82;
    frame[4] = (uint8_t)(vp_address >> 8);
    frame[5] = (uint8_t)(vp_address & 0xFF);

    memcpy(&frame[6], text, text_len);

    frame[6 + text_len] = 0xFF;
    frame[6 + text_len + 1] = 0xFF;

    return (HAL_UART_Transmit(s_huart, frame, total_frame_size, DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Escreve string formatada para QR Code (sem terminadores 0xFF)
bool DWIN_Driver_Write_QR_String(uint16_t vp_address, const char *text, uint16_t max_len) {
    if ((s_huart == NULL) || (text == NULL)) {
        return false;
    }

    size_t text_len = strlen(text);
    if (text_len == 0u) {
        return true;
    }
    if (text_len > max_len) {
        text_len = max_len;
    }

    uint8_t payload_len = (uint8_t)(3u + text_len);
    uint16_t total_frame_size = (uint16_t)(3u + payload_len);
    uint8_t frame[3 + 3 + max_len];

    frame[0] = 0x5A;
    frame[1] = 0xA5;
    frame[2] = payload_len;
    frame[3] = 0x82;
    frame[4] = (uint8_t)(vp_address >> 8);
    frame[5] = (uint8_t)(vp_address & 0xFF);

    memcpy(&frame[6], text, text_len);

    return (HAL_UART_Transmit(s_huart, frame, total_frame_size, DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Envia bytes brutos diretamente para o display
bool DWIN_Driver_WriteRawBytes(const uint8_t *data, uint16_t size) {
    if ((s_huart == NULL) || (data == NULL) || (size == 0u)) {
        return false;
    }

    return (HAL_UART_Transmit(s_huart, (uint8_t *)data, size, DWIN_UART_TIMEOUT_MS) == HAL_OK);
}

// Trata o evento de recepção UART (Idle Line)
void DWIN_Driver_HandleRxEvent(uint16_t size) {
    if ((size == 0u) || (size > DWIN_RX_BUFFER_SIZE)) {
        DWIN_Restart_Rx();
        return;
    }

    s_received_len = size;
    s_frame_received = true;
    s_rx_packet_counter++;

    DWIN_Restart_Rx();
}

// Trata erros de UART reiniciando a recepção
void DWIN_Driver_HandleError(UART_HandleTypeDef *huart) {
    if ((s_huart == NULL) || (huart->Instance != s_huart->Instance)) {
        return;
    }

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }

    HAL_UART_AbortReceive_IT(huart);
    DWIN_Restart_Rx();
}