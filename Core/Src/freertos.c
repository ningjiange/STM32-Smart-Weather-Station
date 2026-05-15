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
#include "cmsis_os2.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "string.h"
#include "usart.h"
#include <stdint.h>
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
osMessageQueueId_t myQueue;
/* USER CODE END Variables */

osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
    .name = "OLEDTask",
    .stack_size = 128 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};
 osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
    .name = "ButtonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
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
  uint8_t receivedState = 0;
  while (1) {
    osMessageQueueGet(myQueue, &receivedState, NULL, osWaitForever);
    OLED_ShowNum(1, 1, receivedState, 1);
    if (receivedState) {
      OLED_ShowString(2, 1, "Key: ON ");
    } else {
      OLED_ShowString(2, 1, "Key: OFF");
    }
  }
}
void StartDefaultTask(void *argument);
void ButtonTask(void *argument) {
  uint8_t buttonState = 0; // 0: 未按下，1: 已按下
  while (1) {
    if(HAL_GPIO_ReadPin(KEY_GPIO_Port,  KEY_Pin) == GPIO_PIN_RESET) {
      osDelay(20); // 防抖延时
      if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
        while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) ==
               GPIO_PIN_RESET) // 例如，发送一个信号量或设置一个标志位
        {
          osDelay(10);
        }
        buttonState = !buttonState; // 切换状态
        osMessageQueuePut(myQueue, &buttonState, 0, osWaitForever); // 将状态发送到队列
      }
    } else {
      osDelay(10); // 空闲时稍微延时，减少CPU占用
    }
  }
} 
  
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
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    
    myQueue = osMessageQueueNew(10, sizeof(uint8_t), NULL);
  
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  OLEDTaskHandle = osThreadNew(OLEDTask, NULL, &OLEDTask_attributes);
  ButtonTaskHandle=osThreadNew(ButtonTask, NULL, &ButtonTask_attributes);
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
  uint32_t startTick = osKernelGetTickCount();
  for (;;) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

