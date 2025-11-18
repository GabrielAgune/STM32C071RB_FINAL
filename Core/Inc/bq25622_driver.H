/*
 * Nome do Arquivo: bq25622_driver.h
 * Descrição: Interface do driver para o gerenciador de carga BQ25622
 * Autor: Gabriel Agune
 */

#ifndef INC_BQ25622_DRIVER_H_
#define INC_BQ25622_DRIVER_H_

// ============================================================
// Includes
// ============================================================

#include "stm32c0xx_hal.h"

// ============================================================
// Endereços I2C
// ============================================================

#define BQ25622_I2C_ADDR_7BIT       0x6B
#define BQ25622_I2C_ADDR_8BIT       0xD6

// ============================================================
// Registradores (8-bit)
// ============================================================

#define BQ25622_REG_ICHG            0x02 // 16-bit: Charge Current Limit
#define BQ25622_REG_VREG            0x04 // 16-bit: Charge Voltage Limit
#define BQ25622_REG_IINDPM          0x06 // 16-bit: Input Current Limit
#define BQ25622_REG_VOTG            0x0C // 16-bit: VOTG regulation
#define BQ25622_REG_IPRECHG         0x10 // 16-bit: Pre-charge Control
#define BQ25622_REG_ITERM           0x12 // 16-bit: Termination Control
#define BQ25622_REG_CHG_CTRL_0      0x14 // 8-bit:  Charge Control 0
#define BQ25622_REG_CHG_CTRL_1      0x16 // 8-bit:  Charger Control 1
#define BQ25622_REG_CHG_CTRL_3      0x18 // 8-bit:  Charger Control 3
#define BQ25622_REG_CHG_CTRL_4      0x19 // 8-bit:  Charger Control 4
#define BQ25622_REG_ADC_CONTROL     0x26 // 8-bit:  ADC Control
#define BQ25622_REG_IBUS_ADC        0x28 // 16-bit: IBUS ADC
#define BQ25622_REG_IBAT_ADC        0x2A // 16-bit: IBAT ADC
#define BQ25622_REG_VBUS_ADC        0x2C // 16-bit: VBUS ADC
#define BQ25622_REG_VBAT_ADC        0x30 // 16-bit: VBAT ADC
#define BQ25622_REG_VSYS_ADC        0x32 // 16-bit: VSYS ADC
#define BQ25622_REG_TDIE_ADC        0x36 // 16-bit: TDIE ADC
#define BQ25622_REG_CHG_STATUS_1    0x1E // 8-bit:  Charger Status 1
#define BQ25622_REG_PART_INFO       0x38 // 8-bit:  Part Information

// ============================================================
// Máscaras de Bits
// ============================================================

// REG 0x14 (Charge Control 0)
#define BQ25622_EN_TERM_BIT         (1 << 2)

// REG 0x16 (Charger Control 1)
#define BQ25622_WATCHDOG_MASK       (0x03)
#define BQ25622_WATCHDOG_DISABLE    (0x00)
#define BQ25622_EN_CHG_BIT          (1 << 5)

// REG 0x18 (Charger Control 3)
#define BQ25622_EN_OTG_BIT          (1 << 6)

// REG 0x19 (Charger Control 4)
#define BQ25622_EN_EXTILIM_BIT      (1 << 2)
#define BQ25622_VBAT_OTG_MIN_MASK   (1 << 4)

// REG 0x26 (ADC Control)
#define BQ25622_ADC_EN_BIT          (1 << 7)
#define BQ25622_ADC_RATE_BIT        (1 << 6)
#define BQ25622_ADC_AVG_BIT         (1 << 3)

// REG 0x1E (Charger Status 1)
#define BQ25622_CHG_STAT_MASK       (0x03)
#define BQ25622_CHG_STAT_SHIFT      3

// ============================================================
// Constantes de Conversão (LSB)
// ============================================================

#define BQ25622_VBAT_LSB_V          0.00199f // 1.99mV
#define BQ25622_IBAT_LSB_A          0.004f   // 4mA
#define BQ25622_VBUS_LSB_V          0.00397f // 3.97mV
#define BQ25622_TDIE_LSB_C          0.5f     // 0.5°C

// ============================================================
// Typedefs e Enums
// ============================================================

typedef enum {
    CHG_STAT_NOT_CHARGING = 0, // 00b
    CHG_STAT_CC_MODE      = 1, // 01b
    CHG_STAT_CV_MODE      = 2, // 10b
    CHG_STAT_TOP_OFF      = 3  // 11b
} BQ25622_ChargeStatus_t;

// ============================================================
// Protótipos de Funções Públicas
// ============================================================

HAL_StatusTypeDef bq25622_validate_comm(I2C_HandleTypeDef *hi2c, uint8_t *part_info);
HAL_StatusTypeDef bq25622_init(I2C_HandleTypeDef *hi2c, uint16_t battery_capacity_mah);
HAL_StatusTypeDef bq25622_adc_init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef bq25622_read_vbat(I2C_HandleTypeDef *hi2c, float *vbat_V);
HAL_StatusTypeDef bq25622_read_ibat(I2C_HandleTypeDef *hi2c, float *ibat_A);
HAL_StatusTypeDef bq25622_read_vbus(I2C_HandleTypeDef *hi2c, float *vbus_V);
HAL_StatusTypeDef bq25622_read_charge_status(I2C_HandleTypeDef *hi2c, BQ25622_ChargeStatus_t *chg_status);
HAL_StatusTypeDef bq25622_read_die_temp(I2C_HandleTypeDef *hi2c, float *die_temp_C);

HAL_StatusTypeDef bq25622_enable_charging(I2C_HandleTypeDef *hi2c, uint8_t enable);
HAL_StatusTypeDef bq25622_enable_otg(I2C_HandleTypeDef *hi2c, uint8_t enable);
HAL_StatusTypeDef bq25622_set_otg_voltage(I2C_HandleTypeDef *hi2c, uint16_t voltage_mV);

#endif /* INC_BQ25622_DRIVER_H_ */