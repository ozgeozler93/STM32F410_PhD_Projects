#ifndef INA219_H
#define INA219_H

#include "stm32l4xx_hal.h"

// INA219 Varsayılan I2C Adresi (A0 ve A1 pinleri GND'ye bağlıysa)
#define INA219_ADDRESS         (0x40 << 1)

// INA219 Register Adresleri
#define INA219_REG_CONFIG      0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE  0x02
#define INA219_REG_POWER       0x03
#define INA219_REG_CURRENT     0x04
#define INA219_REG_CALIBRATION 0x05

// Fonksiyon Prototipleri
void INA219_Init(I2C_HandleTypeDef *hi2c);
float INA219_ReadBusVoltage(void);
float INA219_ReadCurrent_mA(void);

#endif // INA219_H
