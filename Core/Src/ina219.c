#include "ina219.h"
#include "cmsis_os.h"
#include <string.h>

/* INA219 Register Map */
#define INA219_ADDR                       0x40
#define INA219_REG_CONFIG                 0x00
#define INA219_REG_SHUNTVOLTAGE           0x01
#define INA219_REG_BUSVOLTAGE             0x02
#define INA219_REG_POWER                  0x03
#define INA219_REG_CURRENT                0x04
#define INA219_REG_CALIBRATION            0x05


/* Default configuration (32V, 2A, 12-bit) */
#define INA219_DEFAULT_CONFIG             0x27A7


/* Statik değişkenler */
static I2C_HandleTypeDef* pI2cHandle = NULL;
static uint8_t bIsOpen = 0;
static uint16_t u16Calibration = 0;


/* Dışarıdaki I2C mutex'i (freertos.c içinde tanımlı) */
extern osMutexId I2C_MutexHandle;




/* Yardımcı: register yazma (thread-safe) */
static INA219_Status_t INA219_WriteRegister(uint8_t reg, uint16_t value) {
    uint8_t data[2];
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;

    if (osMutexWait(I2C_MutexHandle, osWaitForever) != osOK)
        return INA219_ERR_I2C;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(pI2cHandle, INA219_ADDR << 1,
                                                 reg, I2C_MEMADD_SIZE_8BIT,
                                                 data, 2, HAL_MAX_DELAY);
    osMutexRelease(I2C_MutexHandle);

    return (status == HAL_OK) ? INA219_OK : INA219_ERR_I2C;
}

/* Yardımcı: register okuma (thread-safe) */
static INA219_Status_t INA219_ReadRegister(uint8_t reg, uint16_t* pValue) {
    uint8_t data[2] = {0};

    if (osMutexWait(I2C_MutexHandle, osWaitForever) != osOK)
        return INA219_ERR_I2C;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(pI2cHandle, INA219_ADDR << 1,
                                                reg, I2C_MEMADD_SIZE_8BIT,
                                                data, 2, HAL_MAX_DELAY);
    osMutexRelease(I2C_MutexHandle);

    if (status == HAL_OK) {
        *pValue = ((uint16_t)data[0] << 8) | data[1];
        return INA219_OK;
    }
    return INA219_ERR_I2C;
}

/* Open: I2C handle'ı sakla ve sensörü başlat */
INA219_Status_t INA219_Open(void* vpParam) {
    if (vpParam == NULL) return INA219_ERR_PARAM;
    pI2cHandle = (I2C_HandleTypeDef*)vpParam;

    /* Varsayılan yapılandırmayı yaz */
    INA219_Status_t ret = INA219_WriteRegister(INA219_REG_CONFIG, INA219_DEFAULT_CONFIG);
    if (ret != INA219_OK) return ret;

    /* Varsayılan kalibrasyon (0x27A0 = 0.1 ohm shunt, 2A aralık) */
    u16Calibration = 0x27A0;
    ret = INA219_WriteRegister(INA219_REG_CALIBRATION, u16Calibration);
    if (ret != INA219_OK) return ret;

    bIsOpen = 1;
    return INA219_OK;
}

/* Ioctl: yüksek seviyeli komutlar */
INA219_Status_t INA219_Ioctl(INA219_IoctlCmd_t eCommand, void* vpParam) {
    if (!bIsOpen) return INA219_ERR_NOT_OPEN;
    if (vpParam == NULL) return INA219_ERR_PARAM;

    uint16_t raw;
    float* pOut = (float*)vpParam;
    INA219_Status_t ret;

    switch (eCommand) {
        case INA219_IOCTL_GET_VOLTAGE:
            ret = INA219_ReadRegister(INA219_REG_BUSVOLTAGE, &raw);
            if (ret != INA219_OK) return ret;
            /* LSB = 4mV, sağa 3 kaydır */
            *pOut = (float)((raw >> 3) * 4) / 1000.0f;
            break;

        case INA219_IOCTL_GET_CURRENT:
            ret = INA219_ReadRegister(INA219_REG_CURRENT, &raw);
            if (ret != INA219_OK) return ret;
            /* İşaretli 16-bit, LSB = Current_LSB (0.1mA) */
            *pOut = (float)((int16_t)raw) / 10.0f;
            break;

        case INA219_IOCTL_GET_POWER:
            ret = INA219_ReadRegister(INA219_REG_POWER, &raw);
            if (ret != INA219_OK) return ret;
            /* LSB = 20 * Current_LSB = 2mW (typical) */
            *pOut = (float)raw * 0.002f;
            break;

        case INA219_IOCTL_SET_CALIBRATION:
            u16Calibration = *(uint16_t*)vpParam;
            ret = INA219_WriteRegister(INA219_REG_CALIBRATION, u16Calibration);
            break;

        default:
            return INA219_ERR_PARAM;
    }
    return INA219_OK;
}

/* Raw read – ileri kullanım için */
INA219_Status_t INA219_Read(void* pvBuffer, uint32_t xBytes) {
    if (!bIsOpen) return INA219_ERR_NOT_OPEN;
    if (pvBuffer == NULL || xBytes == 0) return INA219_ERR_PARAM;

    uint8_t* pBuf = (uint8_t*)pvBuffer;
    INA219_Status_t ret;
    for (uint32_t i = 0; i < xBytes; i++) {
        /* Örnek: her byte bir register adresi olarak yorumlanır */
        uint16_t regVal;
        ret = INA219_ReadRegister(pBuf[i], &regVal);
        if (ret != INA219_OK) return ret;
        pBuf[i] = (uint8_t)regVal;
    }
    return INA219_OK;
}

/* Raw write */
INA219_Status_t INA219_Write(const void* pvBuffer, uint32_t xBytes) {
    if (!bIsOpen) return INA219_ERR_NOT_OPEN;
    if (pvBuffer == NULL || xBytes < 2) return INA219_ERR_PARAM;

    uint8_t reg = ((uint8_t*)pvBuffer)[0];
    uint16_t value = ((uint8_t*)pvBuffer)[1] | (((uint8_t*)pvBuffer)[2] << 8);
    return INA219_WriteRegister(reg, value);
}

/* Close: sürücüyü kapat */
INA219_Status_t INA219_Close(void* vpParam) {
    (void)vpParam;
    bIsOpen = 0;
    pI2cHandle = NULL;
    return INA219_OK;
}
