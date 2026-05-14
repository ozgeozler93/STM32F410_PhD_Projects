// esp8266.h

#ifndef ESP8266_H
#define ESP8266_H

#include <stdint.h>
#include <stdbool.h>
#include "usart.h"


/* Hata kodları */
typedef enum {
    ESP_OK = 0,
    ESP_ERR_PARAM,
    ESP_ERR_UART_TX,
    ESP_ERR_UART_RX,
    ESP_ERR_NOT_OPEN,
    ESP_ERR_TIMEOUT,
    ESP_ERR_NO_RESPONSE
} ESP_Status_t;


/* IOCTL komutları */
typedef enum {
    ESP_IOCTL_CONNECT_WIFI,      /* param: struct {char* ssid; char* pwd;} */
    ESP_IOCTL_SEND_HTTP_GET,     /* param: char* url (ör: "http://api.thingspeak.com/update?api_key=...&field1=123") */
    ESP_IOCTL_DISCONNECT,
    ESP_IOCTL_GET_RESPONSE       /* param: char* buffer, max uzunluk */
} ESP_IoctlCmd_t;


/* Ring buffer yapısı (kullanıcı tarafından görülebilir) */
#define UART_BUFFER_SIZE 256
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} RingBuffer_t;

/* Public API */
ESP_Status_t ESP8266_Open(void* vpParam);               /* vpParam = UART_HandleTypeDef* */
ESP_Status_t ESP8266_Ioctl(ESP_IoctlCmd_t eCommand, void* vpParam);
ESP_Status_t ESP8266_Write(const void* pvBuffer, uint32_t xBytes);   /* raw AT command */
ESP_Status_t ESP8266_Read(void* pvBuffer, uint32_t xBytes);          /* read from ring buffer */
ESP_Status_t ESP8266_Close(void* vpParam);

/* Interrupt'tan çağrılacak (UART RX callback) */
void ESP8266_RxCallback(uint8_t data);

#endif /* ESP8266_H */
