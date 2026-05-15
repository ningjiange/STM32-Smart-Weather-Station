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
osSemaphoreId_t binarySem;
extern volatile uint32_t sem_tick;
extern volatile uint32_t notif_tick;
extern volatile uint32_t sem_response;
extern volatile uint32_t notif_response;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
    .name = "OLEDTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
osThreadId_t PA1TaskHandle;
const osThreadAttr_t PA1Task_attributes = {
    .name = "PA1Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
    .name = "ButtonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
}; */
    /* Private function prototypes
       -----------------------------------------------*/
    /* USER CODE BEGIN FunctionPrototypes */
void OLEDTask(void *argument) {
  uint8_t receivedpc13Counter;
  uint8_t receivePA1Counter;
  while (1) {
    osMessageQueueGet(myQueue, &receivedpc13Counter, NULL, osWaitForever);
    OLED_ShowNum(1, 1, receivedpc13Counter, 3);
    osMessageQueueGet(myQueue, &receivePA1Counter, NULL, osWaitForever);
    OLED_ShowNum(1, 5, receivePA1Counter, 3);
    OLED_ShowNum(2, 1, sem_response, 5);
    OLED_ShowNum(3, 1, notif_response, 5);
  }
}
void PA1Task(void *argument) {
  uint8_t PA1Counter = 0;
  while (1) {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
    notif_response = (DWT->CYCCNT - notif_tick) / 72;  // 转换为微秒
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
    PA1Counter++;
    osMessageQueuePut(myQueue, &PA1Counter, 0, osWaitForever);
    osDelay(1000);
  }
}
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
}  */
  
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
  binarySem = osSemaphoreNew(1, 0, NULL);
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
  PA1TaskHandle = osThreadNew(PA1Task, NULL, &PA1Task_attributes);
  /* ButtonTaskHandle=osThreadNew(ButtonTask, NULL, &ButtonTask_attributes); */
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
  uint8_t pc13Counter = 0;
  for (;;) {
    osSemaphoreAcquire(binarySem, osWaitForever);
    sem_response = (DWT->CYCCNT - sem_tick) / 72;  // 转换为微秒
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    pc13Counter++;
    osMessageQueuePut(myQueue, &pc13Counter, 0, osWaitForever);
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

