#include "esp8266.h"
#include "cmsis_os.h"
#include <string.h>

extern UART_HandleTypeDef huart1; // Kullandığınız UART portuna göre değişebilir
extern osMutexId UART_MutexHandle; // V1 formatı: osMutexId (t'siz)

RingBuffer_t rxBuffer = {0};
uint8_t rxByte;

// Buffer'a veri ekleme (Interrupt içinden çağrılacak)
void RingBuffer_Write(uint8_t data) {
    uint16_t next_head = (rxBuffer.head + 1) % UART_BUFFER_SIZE;
    if (next_head != rxBuffer.tail) { // Buffer dolu değilse
        rxBuffer.buffer[rxBuffer.head] = data;
        rxBuffer.head = next_head;
    }
}

// Buffer'dan veri okuma (Task içinden çağrılacak)
bool ESP8266_ReadBuffer(uint8_t *data) {
    if (rxBuffer.head == rxBuffer.tail) {
        return false; // Buffer boş
    }
    *data = rxBuffer.buffer[rxBuffer.tail];
    rxBuffer.tail = (rxBuffer.tail + 1) % UART_BUFFER_SIZE;
    return true;
}

// AT Komutu Gönderme (Thread-Safe - V1 Formatı)
void ESP8266_SendCommand(const char *cmd) {
    if (osMutexWait(UART_MutexHandle, osWaitForever) == osOK) {
        HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
        osMutexRelease(UART_MutexHandle);
    }
}
