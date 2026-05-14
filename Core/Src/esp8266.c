#include "esp8266.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* Statik değişkenler */
static UART_HandleTypeDef* pUartHandle = NULL;
static uint8_t bIsOpen = 0;
static RingBuffer_t sRxRingBuffer = {0};

/* Dışarıdaki UART mutex'i (freertos.c içinde tanımlı) */
extern osMutexId UART_MutexHandle;

/* Yardımcı: AT komutu gönder (mutex ile) */
static ESP_Status_t ESP8266_SendAT(const char* cmd, uint32_t timeoutMs) {
    if (!bIsOpen) return ESP_ERR_NOT_OPEN;
    if (cmd == NULL) return ESP_ERR_PARAM;

    if (osMutexWait(UART_MutexHandle, osWaitForever) != osOK)
        return ESP_ERR_UART_TX;

    HAL_StatusTypeDef status = HAL_UART_Transmit(pUartHandle, (uint8_t*)cmd, strlen(cmd), timeoutMs);
    osMutexRelease(UART_MutexHandle);

    return (status == HAL_OK) ? ESP_OK : ESP_ERR_UART_TX;
}

/* Public: Open */
ESP_Status_t ESP8266_Open(void* vpParam) {
    if (vpParam == NULL) return ESP_ERR_PARAM;
    pUartHandle = (UART_HandleTypeDef*)vpParam;
    bIsOpen = 1;

    /* Ring buffer sıfırla */
    sRxRingBuffer.head = 0;
    sRxRingBuffer.tail = 0;
    memset(sRxRingBuffer.buffer, 0, UART_BUFFER_SIZE);

    /* UART RX interrupt'ını başlat (zaten main'de başlatılmış olabilir) */
    HAL_UART_Receive_IT(pUartHandle, (uint8_t*)"", 0); /* dummy */
    return ESP_OK;
}

/* Ioctl: Wi-Fi bağlan, HTTP GET gönder vb. */
ESP_Status_t ESP8266_Ioctl(ESP_IoctlCmd_t eCommand, void* vpParam) {
    if (!bIsOpen) return ESP_ERR_NOT_OPEN;

    switch (eCommand) {
        case ESP_IOCTL_CONNECT_WIFI:
        {
            struct { char* ssid; char* pwd; }* pWifi = (void*)vpParam;
            if (pWifi == NULL) return ESP_ERR_PARAM;
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", pWifi->ssid, pWifi->pwd);
            return ESP8266_SendAT(cmd, 10000);  /* 10 saniye timeout */
        }

        case ESP_IOCTL_SEND_HTTP_GET:
        {
            char* url = (char*)vpParam;
            if (url == NULL) return ESP_ERR_PARAM;
            /* Önce AT+CIPSTART, sonra AT+CIPSEND ... burada basit bir örnek: */
            /* Not: Gerçek uygulamada TCP bağlantısı ve gönderme adımları yapılmalı */
            char cmd[200];
            snprintf(cmd, sizeof(cmd), "AT+HTTPGET=\"%s\"\r\n", url);
            return ESP8266_SendAT(cmd, 5000);
        }

        case ESP_IOCTL_DISCONNECT:
            return ESP8266_SendAT("AT+CWQAP\r\n", 2000);

        case ESP_IOCTL_GET_RESPONSE:
        {
            char* buf = (char*)vpParam;
            if (buf == NULL) return ESP_ERR_PARAM;
            /* Ring buffer'dan oku */
            uint16_t idx = 0;
            while (sRxRingBuffer.head != sRxRingBuffer.tail && idx < UART_BUFFER_SIZE-1) {
                buf[idx++] = sRxRingBuffer.buffer[sRxRingBuffer.tail];
                sRxRingBuffer.tail = (sRxRingBuffer.tail + 1) % UART_BUFFER_SIZE;
            }
            buf[idx] = '\0';
            return ESP_OK;
        }

        default:
            return ESP_ERR_PARAM;
    }
}

/* Raw write: direkt AT komutu gönder */
ESP_Status_t ESP8266_Write(const void* pvBuffer, uint32_t xBytes) {
    if (!bIsOpen) return ESP_ERR_NOT_OPEN;
    if (pvBuffer == NULL || xBytes == 0) return ESP_ERR_PARAM;

    char* cmd = (char*)pvBuffer;
    /* Sonunda \r\n yoksa ekle? Kullanıcı tam komut göndermeli */
    return ESP8266_SendAT(cmd, 5000);
}

/* Raw read: ring buffer'dan kopyala (isteğe bağlı) */
ESP_Status_t ESP8266_Read(void* pvBuffer, uint32_t xBytes) {
    if (!bIsOpen) return ESP_ERR_NOT_OPEN;
    if (pvBuffer == NULL || xBytes == 0) return ESP_ERR_PARAM;

    uint8_t* buf = (uint8_t*)pvBuffer;
    uint32_t copied = 0;
    while (copied < xBytes && sRxRingBuffer.head != sRxRingBuffer.tail) {
        buf[copied++] = sRxRingBuffer.buffer[sRxRingBuffer.tail];
        sRxRingBuffer.tail = (sRxRingBuffer.tail + 1) % UART_BUFFER_SIZE;
    }
    return (copied > 0) ? ESP_OK : ESP_ERR_UART_RX;
}

/* Close */
ESP_Status_t ESP8266_Close(void* vpParam) {
    (void)vpParam;
    bIsOpen = 0;
    pUartHandle = NULL;
    return ESP_OK;
}

/* UART RX kesmesinden çağrılacak fonksiyon (USART1_IRQHandler veya HAL_UART_RxCpltCallback içinde) */
void ESP8266_RxCallback(uint8_t data) {
    if (!bIsOpen) return;
    uint16_t next_head = (sRxRingBuffer.head + 1) % UART_BUFFER_SIZE;
    if (next_head != sRxRingBuffer.tail) {
        sRxRingBuffer.buffer[sRxRingBuffer.head] = data;
        sRxRingBuffer.head = next_head;
    }
    /* Buffer doluysa veri atılır (opsiyonel hata sayacı) */
}
