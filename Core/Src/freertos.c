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
extern uint16_t ldr_raw_values[4];

// Tasklar arası paylaşılacak filtrelenmiş veriler
float ldr_filtered[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

// Güç tüketimi verileri
float system_voltage = 0.0f;
float system_current = 0.0f;

// Watchdog timer (IWDG) referansı
extern IWDG_HandleTypeDef hiwdg;

// Servo Motor Timer referansı (Örn: TIM2. Kendi konfigürasyonunuza göre değiştirin)
extern TIM_HandleTypeDef htim2;

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

void StartDefaultTask(void const *argument);
void StartTask02(void const *argument);
void StartTask03(void const *argument);
void StartTask04(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

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
void StartDefaultTask(void const *argument) {
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
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void const *argument) {
	/* USER CODE BEGIN StartTask02 */

	// INA219 Başlatma (I2C1 varsayılmıştır)
	extern I2C_HandleTypeDef hi2c1;
	INA219_Init(&hi2c1);
	/* Infinite loop */
	for (;;) {
		// 1. Dört LDR için adaptif filtreyi uygula
		for (int i = 0; i < 4; i++) {
			ldr_filtered[i] = apply_adaptive_ema_filter(
					(float) ldr_raw_values[i], ldr_filtered[i]);
		}

		// 2. Güç tüketimini Mutex korumalı sürücümüzden oku
		system_voltage = INA219_ReadBusVoltage();
		system_current = INA219_ReadCurrent_mA(); // (Bu fonksiyonu ina219.c'ye eklediğinizi varsayıyorum)

		// 3. Hayati Adım: Sistemi resetlenmekten kurtar (Watchdog'u besle)
		HAL_IWDG_Refresh(&hiwdg);

		// Saniyede 20 kez okuma yap (50ms gecikme)
		osDelay(50);
	}
	/* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the ControlTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask03 */
void StartTask03(void const *argument) {
	/* USER CODE BEGIN StartTask03 */

	// PWM Sinyallerini Başlat (Kanal 1: Yatay, Kanal 2: Dikey varsayılmıştır)
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

	// Servoların başlangıç açıları (Orta nokta pwm değerleri)
	uint32_t servo_pan = 1500;  // Yatay
	uint32_t servo_tilt = 1500; // Dikey

	const float ERROR_THRESHOLD = 150.0f; // Jitter önleyici ölü bölge sınırı

	/* Infinite loop */
	for (;;) {
		// LDR Konumları: 0:Sol-Üst, 1:Sağ-Üst, 2:Sol-Alt, 3:Sağ-Alt
		float top_avg = (ldr_filtered[0] + ldr_filtered[1]) / 2.0f;
		float bot_avg = (ldr_filtered[2] + ldr_filtered[3]) / 2.0f;
		float left_avg = (ldr_filtered[0] + ldr_filtered[2]) / 2.0f;
		float right_avg = (ldr_filtered[1] + ldr_filtered[3]) / 2.0f;

		// Hata hesaplama
		float err_tilt = top_avg - bot_avg;
		float err_pan = left_avg - right_avg;

		// Dikey (Tilt) Kontrolü
		if (fabs(err_tilt) > ERROR_THRESHOLD) {
			if (err_tilt > 0)
				servo_tilt -= 5; // Adım hassasiyeti
			else
				servo_tilt += 5;
		}

		// Yatay (Pan) Kontrolü
		if (fabs(err_pan) > ERROR_THRESHOLD) {
			if (err_pan > 0)
				servo_pan -= 5;
			else
				servo_pan += 5;
		}

		// Motorlara sınır koyma (Mekanik hasarı önlemek için)
		if (servo_pan > 2500)
			servo_pan = 2500;
		if (servo_pan < 500)
			servo_pan = 500;
		if (servo_tilt > 2500)
			servo_tilt = 2500;
		if (servo_tilt < 500)
			servo_tilt = 500;

		// PWM Duty Cycle Güncelleme
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, servo_pan);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, servo_tilt);

		// Motor kontrol döngüsü çok hızlı olmamalı (Saniyede 20 kare yeterli)
		osDelay(50);
	}
	/* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
 * @brief Function implementing the CommTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask04 */
void StartTask04(void const *argument) {
	/* USER CODE BEGIN StartTask04 */
	char at_command[128];

	/* Infinite loop */
	for (;;) {
		// ThingSpeak için GET request hazırlığı örneği
		// Field1: Voltaj, Field2: Akım
		sprintf(at_command,
				"AT+CIPSEND=...\r\nGET /update?api_key=YOUR_API_KEY&field1=%.2f&field2=%.2f\r\n",
				system_voltage, system_current);

		// Mutex ile korunan sürücümüzü kullanarak komutu gönder
		ESP8266_SendCommand(at_command);

		// ThingSpeak'in limitlerine takılmamak için 15 saniye bekle
		osDelay(15000);

	}
	/* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
