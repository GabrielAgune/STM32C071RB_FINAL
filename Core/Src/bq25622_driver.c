/*
 * Nome do Arquivo: bq25622_driver.c
 * Descrição: Implementação do driver BQ25622 (I2C, Carga e ADC)
 * Autor: Gabriel Agune
 */

#include "bq25622_driver.h"

// ============================================================
// Defines Internos
// ============================================================

#define BQ25622_I2C_TIMEOUT     100 // ms

// ============================================================
// Funções Privadas (Auxiliares I2C)
// ============================================================

// Lê um registrador de 8 bits
static HAL_StatusTypeDef bq25622_read_reg_8bit(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *pData) {
    return HAL_I2C_Mem_Read(hi2c, BQ25622_I2C_ADDR_8BIT, reg_addr, I2C_MEMADD_SIZE_8BIT, pData, 1, BQ25622_I2C_TIMEOUT);
}

// Escreve em um registrador de 8 bits
static HAL_StatusTypeDef bq25622_write_reg_8bit(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t data) {
    return HAL_I2C_Mem_Write(hi2c, BQ25622_I2C_ADDR_8BIT, reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, BQ25622_I2C_TIMEOUT);
}

// Lê um registrador de 16 bits (Little-Endian)
static HAL_StatusTypeDef bq25622_read_reg_16bit(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint16_t *pData) {
    uint8_t buffer[2];
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, BQ25622_I2C_ADDR_8BIT, reg_addr, I2C_MEMADD_SIZE_8BIT, buffer, 2, BQ25622_I2C_TIMEOUT);

    if (status == HAL_OK) {
        *pData = (uint16_t)(buffer[1] << 8) | buffer[0];
    }
    return status;
}

// Escreve em um registrador de 16 bits (Little-Endian)
static HAL_StatusTypeDef bq25622_write_reg_16bit(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = (uint8_t)(data & 0x00FF);
    buffer[1] = (uint8_t)((data >> 8) & 0x00FF);

    return HAL_I2C_Mem_Write(hi2c, BQ25622_I2C_ADDR_8BIT, reg_addr, I2C_MEMADD_SIZE_8BIT, buffer, 2, BQ25622_I2C_TIMEOUT);
}

// Modifica bits específicos (Read-Modify-Write)
static HAL_StatusTypeDef bq25622_modify_reg_8bit(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t mask, uint8_t value) {
    uint8_t reg_val;
    HAL_StatusTypeDef status = bq25622_read_reg_8bit(hi2c, reg_addr, &reg_val);

    if (status != HAL_OK) {
        return status;
    }

    reg_val &= ~mask;
    reg_val |= (value & mask);

    return bq25622_write_reg_8bit(hi2c, reg_addr, reg_val);
}

// ============================================================
// Funções Públicas
// ============================================================

// Valida a comunicação verificando o Part Information
HAL_StatusTypeDef bq25622_validate_comm(I2C_HandleTypeDef *hi2c, uint8_t *part_info) {
    return bq25622_read_reg_8bit(hi2c, BQ25622_REG_PART_INFO, part_info);
}

// Configura parâmetros de carga baseado na capacidade da bateria
HAL_StatusTypeDef bq25622_init(I2C_HandleTypeDef *hi2c, uint16_t battery_capacity_mah) {
    HAL_StatusTypeDef status;
    uint16_t calculated_val;
    uint16_t reg_val_16;

    // 1. Desabilitar Watchdog e Controle de ILIM externo
    status = bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_1, BQ25622_WATCHDOG_MASK, BQ25622_WATCHDOG_DISABLE);
    if (status != HAL_OK) return status;

    status = bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_4, BQ25622_EN_EXTILIM_BIT, 0x00);
    if (status != HAL_OK) return status;

    // 2. Configurar IINDPM (Limite de Entrada) ~500mA
    calculated_val = (uint16_t)(25 << 4);
    status = bq25622_write_reg_16bit(hi2c, BQ25622_REG_IINDPM, calculated_val);
    if (status != HAL_OK) return status;

    // 3. Configurar ICHG (Carga Rápida) ~0.8C
    calculated_val = (uint16_t)((battery_capacity_mah * 0.8f) / 80.0f);
    if (calculated_val == 0) calculated_val = 1;
    reg_val_16 = (calculated_val << 6);
    
    status = bq25622_write_reg_16bit(hi2c, BQ25622_REG_ICHG, reg_val_16);
    if (status != HAL_OK) return status;

    // 4. Configurar IPRECHG (Pré-Carga) ~0.2C
    calculated_val = (uint16_t)((battery_capacity_mah * 0.2f) / 20.0f);
    if (calculated_val == 0) calculated_val = 1;
    reg_val_16 = (calculated_val << 4);

    status = bq25622_write_reg_16bit(hi2c, BQ25622_REG_IPRECHG, reg_val_16);
    if (status != HAL_OK) return status;

    // 5. Configurar ITERM (Término) ~0.1C
    calculated_val = (uint16_t)((battery_capacity_mah * 0.1f) / 10.0f);
    if (calculated_val == 0) calculated_val = 1;
    reg_val_16 = (calculated_val << 3);

    status = bq25622_write_reg_16bit(hi2c, BQ25622_REG_ITERM, reg_val_16);
    if (status != HAL_OK) return status;

    // 6. Garantir Habilitação de Término
    return bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_0, BQ25622_EN_TERM_BIT, BQ25622_EN_TERM_BIT);
}

// Habilita o ADC em modo contínuo com média
HAL_StatusTypeDef bq25622_adc_init(I2C_HandleTypeDef *hi2c) {
    uint8_t adc_ctrl = BQ25622_ADC_EN_BIT | BQ25622_ADC_AVG_BIT;
    return bq25622_write_reg_8bit(hi2c, BQ25622_REG_ADC_CONTROL, adc_ctrl);
}

// Lê a tensão da bateria (VBAT) convertida para Volts
HAL_StatusTypeDef bq25622_read_vbat(I2C_HandleTypeDef *hi2c, float *vbat_V) {
    uint16_t raw_adc;
    HAL_StatusTypeDef status = bq25622_read_reg_16bit(hi2c, BQ25622_REG_VBAT_ADC, &raw_adc);

    if (status == HAL_OK) {
        uint16_t adc_val = (raw_adc & 0x1FFE) >> 1;
        *vbat_V = (float)adc_val * BQ25622_VBAT_LSB_V;
    }
    return status;
}

// Lê a corrente da bateria (IBAT) convertida para Amperes
HAL_StatusTypeDef bq25622_read_ibat(I2C_HandleTypeDef *hi2c, float *ibat_A) {
    uint16_t raw_adc;
    HAL_StatusTypeDef status = bq25622_read_reg_16bit(hi2c, BQ25622_REG_IBAT_ADC, &raw_adc);

    if (status == HAL_OK) {
        int16_t signed_raw = (int16_t)raw_adc;
        int16_t adc_val = signed_raw >> 2;
        *ibat_A = (float)adc_val * BQ25622_IBAT_LSB_A;
    }
    return status;
}

// Lê a tensão do barramento USB (VBUS) convertida para Volts
HAL_StatusTypeDef bq25622_read_vbus(I2C_HandleTypeDef *hi2c, float *vbus_V) {
    uint16_t raw_adc;
    HAL_StatusTypeDef status = bq25622_read_reg_16bit(hi2c, BQ25622_REG_VBUS_ADC, &raw_adc);

    if (status == HAL_OK) {
        uint16_t adc_val = (raw_adc & 0x7FFC) >> 2;
        *vbus_V = (float)adc_val * BQ25622_VBUS_LSB_V;
    }
    return status;
}

// Decodifica o status de carga atual
HAL_StatusTypeDef bq25622_read_charge_status(I2C_HandleTypeDef *hi2c, BQ25622_ChargeStatus_t *chg_status) {
    uint8_t reg_val;
    HAL_StatusTypeDef status = bq25622_read_reg_8bit(hi2c, BQ25622_REG_CHG_STATUS_1, &reg_val);

    if (status == HAL_OK) {
        uint8_t status_bits = (reg_val >> BQ25622_CHG_STAT_SHIFT) & BQ25622_CHG_STAT_MASK;
        *chg_status = (BQ25622_ChargeStatus_t)status_bits;
    }
    return status;
}

// Habilita ou desabilita a carga
HAL_StatusTypeDef bq25622_enable_charging(I2C_HandleTypeDef *hi2c, uint8_t enable) {
    uint8_t value = (enable == 1) ? BQ25622_EN_CHG_BIT : 0x00;
    return bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_1, BQ25622_EN_CHG_BIT, value);
}

// Lê a temperatura do die com extensão de sinal
HAL_StatusTypeDef bq25622_read_die_temp(I2C_HandleTypeDef *hi2c, float *die_temp_C) {
    uint16_t raw_adc;
    HAL_StatusTypeDef status = bq25622_read_reg_16bit(hi2c, BQ25622_REG_TDIE_ADC, &raw_adc);

    if (status == HAL_OK) {
        int16_t adc_val = (int16_t)(raw_adc & 0x0FFF);
        
        // Extensão de sinal do bit 11
        if (adc_val & 0x0800) {
            adc_val |= 0xF000;
        }

        *die_temp_C = (float)adc_val * BQ25622_TDIE_LSB_C;
    }
    return status;
}

// Habilita modo OTG (Boost) com proteção de tensão
HAL_StatusTypeDef bq25622_enable_otg(I2C_HandleTypeDef *hi2c, uint8_t enable) {
    float vbus_V = 0.0f;
    
    // Verificação de segurança: VBUS deve estar baixo
    bq25622_read_vbus(hi2c, &vbus_V);
    if (enable && vbus_V > 2.0f) {
        return HAL_ERROR;
    }

    // Configura VBAT_OTG_MIN para 3.0V
    HAL_StatusTypeDef status = bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_4, BQ25622_VBAT_OTG_MIN_MASK, 0x00);
    if (status != HAL_OK) return status;

    uint8_t value = (enable == 1) ? BQ25622_EN_OTG_BIT : 0x00;
    return bq25622_modify_reg_8bit(hi2c, BQ25622_REG_CHG_CTRL_3, BQ25622_EN_OTG_BIT, value);
}

// Configura a tensão alvo do Boost OTG
HAL_StatusTypeDef bq25622_set_otg_voltage(I2C_HandleTypeDef *hi2c, uint16_t voltage_mV) {
    if (voltage_mV < 3840 || voltage_mV > 9600) {
        return HAL_ERROR;
    }

    uint16_t reg_val = (uint16_t)((voltage_mV - 3840) / 80);
    uint16_t reg_val_shifted = (reg_val << 6);

    return bq25622_write_reg_16bit(hi2c, BQ25622_REG_VOTG, reg_val_shifted);
}