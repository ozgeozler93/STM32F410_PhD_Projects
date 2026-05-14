/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "filter.h"
#include "ina219.h"
#include "esp8266.h"
#include "usart.h"
#include <string.h>
extern IWDG_HandleTypeDef hiwdg;
#include <stdio.h> // sprintf için
#include <math.h> // fabs() mutlak değer fonksiyonu için eklendi
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// LDR DMA Okuma Tamponu (CubeMX'te Data Width: Half Word seçtiğimiz için uint16_t)
extern uint16_t ldr_values[4];


// Tasklar arası paylaşılacak filtrelenmiş veriler
float ldr_filtered[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


// Güç tüketimi verileri
float system_voltage = 0.0f;
float system_current = 0.0f;

// Watchdog timer (IWDG) referansı
extern IWDG_HandleTypeDef hiwdg;

// Servo Motor Timer referansı (Örn: TIM2. Kendi konfigürasyonunuza göre değiştirin)
extern TIM_HandleTypeDef htim3;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId DataAcqTaskHandle;
osThreadId ControlTaskHandle;
osThreadId CommTaskHandle;
osMutexId I2C_MutexHandle;
osMutexId UART_MutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of I2C_Mutex */
  osMutexDef(I2C_Mutex);
  I2C_MutexHandle = osMutexCreate(osMutex(I2C_Mutex));

  /* definition and creation of UART_Mutex */
  osMutexDef(UART_Mutex);
  UART_MutexHandle = osMutexCreate(osMutex(UART_Mutex));

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of DataAcqTask */
  osThreadDef(DataAcqTask, StartTask02, osPriorityNormal, 0, 128);
  DataAcqTaskHandle = osThreadCreate(osThread(DataAcqTask), NULL);

  /* definition and creation of ControlTask */
  osThreadDef(ControlTask, StartTask03, osPriorityNormal, 0, 128);
  ControlTaskHandle = osThreadCreate(osThread(ControlTask), NULL);

  /* definition and creation of CommTask */
  osThreadDef(CommTask, StartTask04, osPriorityNormal, 0, 128);
  CommTaskHandle = osThreadCreate(osThread(CommTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the DataAcqTask thread.
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */


	static uint32_t counter = 0;
	if (counter++ % 100 == 0) {
	    char msg[] = "Task02 alive\r\n";
	    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
	}

    /* INA219 sürücüsünü bir kere aç */
    INA219_Status_t status = INA219_Open(&hi2c1);
    if (status != INA219_OK) {
        /* Hata durumunda burada kal, reset beklenir (watchdog yapar) */
        while(1) {
            HAL_IWDG_Refresh(&hiwdg);
            osDelay(100);
        }
    }


	/* Infinite loop */
	for (;;) {
		// 1. Dört LDR için adaptif filtreyi uygula
        for (int i = 0; i < 4; i++) {
            ldr_filtered[i] = apply_adaptive_ema_filter(
                    (float) ldr_values[i], ldr_filtered[i]);
        }

        // 2. Güç okumaları (Ioctl ile)
//        INA219_Ioctl(INA219_IOCTL_GET_VOLTAGE, &system_voltage);
//        INA219_Ioctl(INA219_IOCTL_GET_CURRENT, &system_current);

        // 3. Watchdog'u besle
        HAL_IWDG_Refresh(&hiwdg);
        osDelay(50);
	}

	char msg[50];
	sprintf(msg, "LDRs: %d %d %d %d\r\n", ldr_values[0], ldr_values[1], ldr_values[2], ldr_values[3]);
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the ControlTask thread.
 */
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */

	// PWM Sinyallerini Başlat
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	// Servoların başlangıç açıları
	uint32_t servo_pan = 1500;
	uint32_t servo_tilt = 1500;

	// DİKKAT: Motorlar ışığa çok daha kolay tepki versin diye eşik 20'ye düşürüldü!
	const float ERROR_THRESHOLD = 20.0f;

	/* Infinite loop */
	for (;;) {
		float top_avg = (ldr_filtered[0] + ldr_filtered[1]) / 2.0f;
		float bot_avg = (ldr_filtered[2] + ldr_filtered[3]) / 2.0f;
		float left_avg = (ldr_filtered[0] + ldr_filtered[2]) / 2.0f;
		float right_avg = (ldr_filtered[1] + ldr_filtered[3]) / 2.0f;

		float err_tilt = top_avg - bot_avg;
		float err_pan = left_avg - right_avg;

		if (fabs(err_tilt) > ERROR_THRESHOLD) {
			if (err_tilt > 0)
				servo_tilt -= 5;
			else
				servo_tilt += 5;
		}

		if (fabs(err_pan) > ERROR_THRESHOLD) {
			if (err_pan > 0)
				servo_pan -= 5;
			else
				servo_pan += 5;
		}

		if (servo_pan > 2500)
			servo_pan = 2500;
		if (servo_pan < 500)
			servo_pan = 500;
		if (servo_tilt > 2500)
			servo_tilt = 2500;
		if (servo_tilt < 500)
			servo_tilt = 500;

		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo_pan);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo_tilt);

		HAL_IWDG_Refresh(&hiwdg);
		osDelay(50);
	}
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
 * @brief Function implementing the CommTask thread.
 */
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
    // ESP8266 sürücüsünü aç
//    ESP8266_Open(&huart1);   // huart1 sizin UART handle'ınız

    // --- GEÇİCİ OLARAK YORUM SATIRI ---
    /*
    // Wi-Fi'ya bağlan (SSID ve şifrenizi girin)
    struct {
        char* ssid;
        char* pwd;
    } wifi = {"makbule_ozge", "MiyaMiya"};
    ESP8266_Ioctl(ESP_IOCTL_CONNECT_WIFI, &wifi);

    char url[200];
    */

	/* Infinite loop */
	for (;;) {
        /* GEÇİCİ: Sadece watchdog besle, haberleşme yapma */
        /*
        // ThingSpeak'e veri göndermek için URL oluştur (sprintf'siz daha güvenli, ama idarelik)
        snprintf(url, sizeof(url),
                 "AT+HTTPGET=\"http://api.thingspeak.com/update?api_key=YOUR_API_KEY_DENEME&field1=%.2f&field2=%.2f\"",
                 system_voltage, system_current);

        ESP8266_Ioctl(ESP_IOCTL_SEND_HTTP_GET, url);
        */

        // Watchdog'u besle
        HAL_IWDG_Refresh(&hiwdg);

        // 15 saniye bekle (ThingSpeak limiti)
        osDelay(15000);
	}
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
