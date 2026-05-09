#ifndef ESP8266_H
#define ESP8266_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// ESP8266'dan gelecek maksimum mesaj uzunluğuna göre tampon boyutu
#define UART_BUFFER_SIZE 256

// Circular (Halka) Buffer Yapısı
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer_t;

// --- Fonksiyon Prototipleri ---

// Donanım Başlatma
void ESP8266_Init(UART_HandleTypeDef *huart);

// Interrupt (Kesme) İçinden Çağrılacak Fonksiyon (Gelen veriyi kaydeder)
void RingBuffer_Write(uint8_t data);

// Task İçinden Çağrılacak Fonksiyon (Kaydedilen veriyi okur)
bool ESP8266_ReadBuffer(uint8_t *data);

// AT Komutu Gönderme Fonksiyonu (Mutex Korumalı)
void ESP8266_SendCommand(const char *cmd);

#endif // ESP8266_H
