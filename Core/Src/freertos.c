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
/* volatile uint8_t defaultTaskAlive = 1; // 初始状态：存在 */
volatile uint32_t PA1LEDTaskCounter = 0;
volatile uint32_t PA1LEDTaskelapsed = 0;
volatile uint32_t PC13TaskCounter = 0;
volatile uint32_t PC13Taskelapsed = 0;
/* USER CODE END Variables */

osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
    .name = "OLEDTask",
    .stack_size = 128 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};
osThreadId_t PA1LEDTaskHandle;
const osThreadAttr_t PA1LEDTask_attributes = {
    .name = "PA1LEDTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
/* osThreadId_t UartTaskHandle;
const osThreadAttr_t UartTask_attributes = {
    .name = "UartTask",
    .stack_size = 128 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};
 osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
    .name = "ButtonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
}; */
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
    OLED_ShowNum(1, 1, PA1LEDTaskCounter, 3);
    OLED_ShowNum(1, 5, PC13TaskCounter, 3);
    OLED_ShowNum(2, 1, PA1LEDTaskelapsed, 6);
    OLED_ShowNum(3, 1, PC13Taskelapsed, 6);
    taskYIELD();
    osDelay(500); // 每0.5秒更新一次显示
  }
}
void PA1LEDTask(void *argument) {
  uint32_t startTick = osKernelGetTickCount(); // 任务开始时刻
  uint32_t lastWakeTime = osKernelGetTickCount(); // 上次唤醒时刻
  uint16_t period = 1000; // 任务周期，单位为毫秒
  while (1) {
    HAL_GPIO_TogglePin(GPIOA, LED_Pin);
    lastWakeTime += period;
    osDelayUntil(lastWakeTime); // 每1秒切换一次LED状态
    PA1LEDTaskCounter++;
    PA1LEDTaskelapsed = osKernelGetTickCount() - startTick;
    if (PA1LEDTaskCounter >= 100) {
      osThreadSuspend(PA1LEDTaskHandle);
    }
  }
}
/* void UartTask(void *argument) {

  char buffer[480];
  while (1) {
    vTaskList(buffer);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer),
                      HAL_MAX_DELAY);
    osDelay(2000); // 每2秒更新一次显示
  }
} */
/* void StartDefaultTask(void *argument);
void ButtonTask(void *argument) {
  while (1) {
    if(HAL_GPIO_ReadPin(KEY_GPIO_Port,  KEY_Pin) == GPIO_PIN_RESET) {
      osDelay(20); // 防抖延时
      if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
        while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) ==
               GPIO_PIN_RESET) // 例如，发送一个信号量或设置一个标志位
        {
          osDelay(10);
        }
        HAL_UART_Transmit(&huart1, (uint8_t *)"Button pressed!\r\n", 17, HAL_MAX_DELAY);
        if (defaultTaskAlive) {
          osThreadTerminate(defaultTaskHandle); // 删除
          defaultTaskAlive = 0;
        }
        else {
          defaultTaskHandle =
              osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
          defaultTaskAlive = 1; // 重新创建
        }
      osDelay(200); // 防止连续触发
      }
    } else {
      osDelay(10); // 空闲时稍微延时，减少CPU占用
    }
  }
} */
  
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
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  OLEDTaskHandle=osThreadNew(OLEDTask, NULL, &OLEDTask_attributes);
  PA1LEDTaskHandle = osThreadNew(PA1LEDTask, NULL, &PA1LEDTask_attributes);
/*   UartTaskHandle = osThreadNew(UartTask, NULL, &UartTask_attributes);
  ButtonTaskHandle = osThreadNew(ButtonTask, NULL, &ButtonTask_attributes); */
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
    PC13TaskCounter++;
    PC13Taskelapsed = osKernelGetTickCount() - startTick;
    if (PC13TaskCounter >= 100) {
      osThreadSuspend(defaultTaskHandle);
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */

