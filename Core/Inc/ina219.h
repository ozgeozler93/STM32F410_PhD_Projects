#ifndef INA219_H
#define INA219_H

#include "stdint.h"
#include "i2c.h"


/* Hata kodları */
typedef enum {
    INA219_OK = 0,
    INA219_ERR_PARAM,
    INA219_ERR_I2C,
    INA219_ERR_NOT_OPEN,
    INA219_ERR_UNKNOWN
} INA219_Status_t;

/* IOCTL komutları */
typedef enum {
    INA219_IOCTL_GET_VOLTAGE,   /* param: float* (Bus Voltage in Volts) */
    INA219_IOCTL_GET_CURRENT,   /* param: float* (Current in mA) */
    INA219_IOCTL_GET_POWER,     /* param: float* (Power in Watts) */
    INA219_IOCTL_SET_CALIBRATION /* param: uint16_t* (calibration value) */
} INA219_IoctlCmd_t;

// Açık kalacak cihaz sayısı
#define INA219_INSTANCE_COUNT 1

/* Public API */
INA219_Status_t INA219_Open(void* vpParam);              /* vpParam = I2C_HandleTypeDef* */
INA219_Status_t INA219_Ioctl(INA219_IoctlCmd_t eCommand, void* vpParam);
INA219_Status_t INA219_Read(void* pvBuffer, uint32_t xBytes);    /* raw register read (advanced) */
INA219_Status_t INA219_Write(const void* pvBuffer, uint32_t xBytes);  /* raw register write */
INA219_Status_t INA219_Close(void* vpParam);

#endif
