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
#include "oled.h"
#include "string.h"
#include "usart.h"
#include <stddef.h>
#include <stdint.h>
#include "sensor.h"
#include "system_state.h"
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
/* osMessageQueueId_t myQueue;
osSemaphoreId_t binarySem;
osTimerId_t Timer01;
osTimerId_t Timer02;
volatile uint8_t TimerOutFlag = 0; */
osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
    .name = "OLEDTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
    .name = "SensorTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void OLEDTask(void *argument) {
  while (1) {
    OLED_Clear();
    OLED_ShowString(1, 1, "Temp:");
    OLED_ShowNum(1, 6, (uint32_t)g_state.temp, 2);
    OLED_ShowString(2, 1, "Humi:");
    OLED_ShowNum(2, 6, (uint32_t)g_state.humi, 2);
    OLED_ShowString(3, 1, "Light:");
    OLED_ShowNum(3, 6, g_state.light_raw, 4);
    OLED_ShowString(4, 1, "P:");
    OLED_ShowNum(4, 3, (int16_t)g_state.pitch, 3);
    OLED_ShowString(4, 7, "R:");
    OLED_ShowNum(4, 9, (int16_t)g_state.roll, 3);

    osDelay(1000);
  }
}
void SensorTask(void *argument) {
  while (1) {
    LightSensor_Read();
    DHT11_Read();
    MPU6050_Read();
    osDelay(1000); // 每秒读取一次
  }
}
/* void Timer01_Callback(void *argument) {
   HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
}
void Timer02_Callback(void *argument) {
  TimerOutFlag = 1;
  } */
/* void ButtonTask(void *argument) {
  while (1) {
    if(HAL_GPIO_ReadPin(KEY_GPIO_Port,  KEY_Pin) == GPIO_PIN_RESET) {
      osDelay(20); //
      if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
        while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) ==
               GPIO_PIN_RESET) // 按键消抖，等待按键释放
        {
          osDelay(10);
        }
       
        
      }
    } else {
      osDelay(10); // 避免CPU占用过高
    }
  }
} 
   */
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* binarySem = osSemaphoreNew(1, 0, NULL); */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* Timer01 = osTimerNew(Timer01_Callback, osTimerPeriodic, NULL, NULL);
  osTimerStart(Timer01, 1000); // 1秒后启动定时器
  Timer02 = osTimerNew(Timer02_Callback, osTimerOnce, NULL, NULL);
  osTimerStart(Timer02, 3000); // 3秒后启动定时器 */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    
  /* myQueue = osMessageQueueNew(10, sizeof(uint8_t), NULL); */
  
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  OLEDTaskHandle = osThreadNew(OLEDTask, NULL, &OLEDTask_attributes);
  SensorTaskHandle = osThreadNew(SensorTask, NULL, &SensorTask_attributes);

 /*  ButtonTaskHandle=osThreadNew(ButtonTask, NULL, &ButtonTask_attributes); */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
  /**
   * @brief  Function implementing the defaultTask thread.
   * @param  argument: Not used
   * @retval None
   */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for (;;) {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

