#include "ina219.h"
#include "cmsis_os.h" // FreeRTOS kütüphanesi

extern I2C_HandleTypeDef hi2c1; // CubeMX'in oluşturduğu I2C değişkeni
extern osMutexId I2C_MutexHandle; // CubeMX'teki "I2C_Mutex" için oluşturulan handle ismi

static I2C_HandleTypeDef *ina219_i2c;

// Sensörü başlatan fonksiyon
void INA219_Init(I2C_HandleTypeDef *hi2c) {
    ina219_i2c = hi2c;
    // Kalibrasyon register'ına başlangıç değeri yazma işlemleri buraya eklenecek
}

// Güvenli (Thread-Safe) Register Okuma Fonksiyonu
uint16_t INA219_ReadRegister(uint8_t reg) {
    uint8_t value[2];
    uint16_t regValue = 0;

    // 1. I2C hattını kilitle (Başka task giremez)
    if (osMutexWait(I2C_MutexHandle, osWaitForever) == osOK) {

        // 2. I2C üzerinden okuma yap
        HAL_I2C_Mem_Read(ina219_i2c, INA219_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, value, 2, HAL_MAX_DELAY);

        // 3. I2C hattının kilidini aç (Diğer tasklar kullanabilsin)
        osMutexRelease(I2C_MutexHandle);

        regValue = ((uint16_t)value[0] << 8) | value[1];
    }
    return regValue;
}

float INA219_ReadBusVoltage(void) {
    uint16_t value = INA219_ReadRegister(INA219_REG_BUSVOLTAGE);
    // LSB'yi kaydırıp mV cinsinden hesaplama
    return (float)((value >> 3) * 4) / 1000.0f; // Volt cinsinden döndür
}

// Güvenli Akım Okuma Fonksiyonu
float INA219_ReadCurrent_mA(void) {
    // Current register'ından (0x04) okuma yap
    uint16_t value = INA219_ReadRegister(INA219_REG_CURRENT);

    // Değeri işaretli tamsayıya (int16_t) çevir (çünkü akım negatif de olabilir)
    int16_t signed_value = (int16_t)value;

    // Kalibrasyona göre uygun çarpana böl (Örnek: 10.0f)
    return (float)signed_value / 10.0f;
}
