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
#include <stddef.h>
#include <stdint.h>
#include "sensor.h"
#include "system_state.h"
#include "Encoder.h"
#include "motor.h"
#include "BT_WIFI.h"
#include "w25q64.h"
#include "logger.h"
#include "alarm.h"
#include "stdio.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
    .name = "OLEDTask",
    .stack_size = 128 * 10,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
    .name = "SensorTask",
    .stack_size = 128 * 12,
    .priority = (osPriority_t)osPriorityHigh,
};
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
    .name = "ButtonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
osThreadId_t BT_TaskHandle;
const osThreadAttr_t BT_Task_attributes = {
    .name = "BT_Task",
    .stack_size = 128 * 8,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
osThreadId_t WiFi_TaskHandle;
const osThreadAttr_t WiFi_Task_attributes = {
    .name = "WiFi_Task",
    .stack_size = 128 * 16,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
osThreadId_t LoggerTaskHandle;
const osThreadAttr_t LoggerTask_attributes = {
    .name = "LoggerTask",
    .stack_size = 128 * 6,
    .priority = (osPriority_t)osPriorityNormal,
};
osThreadId_t AlarmTaskHandle;
const osThreadAttr_t AlarmTask_attributes = {
    .name = "AlarmTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
osMutexId_t UART1_MutexHandle;
const osMutexAttr_t UART1_Mutex_attributes = {
    .name = "UART1_Mutex"
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* ====== 任务定义 ====== */

void OLEDTask(void *argument) {
  uint8_t last_page = 0xFF;
  char line[20];  /* OLED 一行最多 16 字符 + '\0'，留余量防格式截断 */

  while (1) {
    if (g_state.display_page != last_page) {
      OLED_Clear();
      last_page = g_state.display_page;
    }

    switch (g_state.display_page) {
    case 0: /* ====== 第1页：实时传感器数据 ====== */
      snprintf(line, sizeof(line), "T:%d.%dC H:%d.%d%%",
               (int)g_state.temp, ((int)(g_state.temp * 10)) % 10,
               (int)g_state.humi, ((int)(g_state.humi * 10)) % 10);
      OLED_ShowString(1, 1, line);

      snprintf(line, sizeof(line), "Light: %d", g_state.light_raw);
      OLED_ShowString(2, 1, line);

      snprintf(line, sizeof(line), "P:%d  R:%d",
               (int)g_state.pitch, (int)g_state.roll);
      OLED_ShowString(3, 1, line);

      /* 第4行显示模式 + 报警 */
      if (g_state.mode == MODE_AUTO)      OLED_ShowString(4, 1, "AUTO  ");
      else if (g_state.mode == MODE_MANUAL) OLED_ShowString(4, 1, "MANUAL");
      else                                  OLED_ShowString(4, 1, "SLEEP ");
      if (g_state.alarm == ALARM_WARN)     OLED_ShowString(4, 8, "WARN");
      else if (g_state.alarm == ALARM_CRITICAL) OLED_ShowString(4, 8, "ALRM");
      else                                  OLED_ShowString(4, 8, "OK  ");
      break;

    case 1: /* ====== 第2页：阈值设置 + 执行器 ====== */
      snprintf(line, sizeof(line), "Thres:%d.%dC",
               (int)g_state.temp_threshold,
               ((int)(g_state.temp_threshold * 10)) % 10);
      OLED_ShowString(1, 1, line);

      snprintf(line, sizeof(line), "Fan: %d%%", g_state.fan_speed);
      OLED_ShowString(2, 1, line);

      snprintf(line, sizeof(line), "Servo: %d",
               (int)g_state.servo_angle);
      OLED_ShowString(3, 1, line);

      if (g_state.mode == MODE_AUTO)      OLED_ShowString(4, 1, "Mode: AUTO  ");
      else if (g_state.mode == MODE_MANUAL) OLED_ShowString(4, 1, "Mode: MANUAL");
      else                                  OLED_ShowString(4, 1, "Mode: SLEEP ");
      break;

    case 2: /* ====== 第3页：日志信息 ====== */
      OLED_ShowString(1, 1, "=== LOG ===");
      snprintf(line, sizeof(line), "Count: %d", g_state.log_count);
      OLED_ShowString(2, 1, line);
      snprintf(line, sizeof(line), "Max: %d", (int)LOG_MAX_RECORDS);
      OLED_ShowString(3, 1, line);
      OLED_ShowString(4, 1, "Bt:LOG to dump");
      break;

    case 3: /* ====== 第4页：系统信息 ====== */
      OLED_ShowString(1, 1, "=== SYS ===");
      snprintf(line, sizeof(line), "Up: %dmin",
               (int)(g_state.uptime_sec / 60));
      OLED_ShowString(2, 1, line);
      snprintf(line, sizeof(line), "Heap: %d",
               (int)g_state.free_heap);
      OLED_ShowString(3, 1, line);
      snprintf(line, sizeof(line), "Free: %d",
               (int)xPortGetFreeHeapSize());
      OLED_ShowString(4, 1, line);
      break;

    default:
      g_state.display_page = PAGE_DATA;
      break;
    }

    osDelay(200);
  }
}

void SensorTask(void *argument) {
  char dbg[48];
  uint32_t last_sns_dbg = 0;
  DebugPrint("SNS: SensorTask start\r\n");
  MPU6050_Init();
  DebugPrint("SNS: MPU6050 Init done\r\n");
  TB6612_Init();
  while (1) {
    LightSensor_Read();
    DHT11_Read();
    MPU6050_Read();
    fan_control();
    SG90_Control();
    g_state.uptime_sec = HAL_GetTick() / 1000;
    g_state.free_heap = xPortGetFreeHeapSize();
    /* 每10秒打印传感器值 */
    if (HAL_GetTick() - last_sns_dbg >= 10000) {
      snprintf(dbg, sizeof(dbg), "SNS: T=%d H=%d L=%u P=%d\r\n",
               (int)g_state.temp, (int)g_state.humi,
               (unsigned)g_state.light_raw, (int)g_state.pitch);
      DebugPrint(dbg);
      last_sns_dbg = HAL_GetTick();
    }
    osDelay(1000);
  }
}

void Task_BT(void *argument) {
  BT_Control();
}

void Task_WiFi(void *argument) {
  char dbg[48];

  /* 启动确认：直接写 UART1（蓝牙），不走互斥量 */
  HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: task started\r\n", 21, 100);

  osDelay(8000);  /* 等其他任务稳定 */

  HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: delay done, init...\r\n", 27, 100);

  /* WiFi 初始化，最多重试3次 */
  uint8_t wifi_ok = 1;
  for (int i = 0; i < 3; i++) {
    snprintf(dbg, sizeof(dbg), "WiFi: try %d\r\n", i);
    HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), 100);
    wifi_ok = ESP8266_Init();
    if (wifi_ok == 0) {
      HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: SUCCESS\r\n", 16, 100);
      break;
    }
    snprintf(dbg, sizeof(dbg), "WiFi: FAIL=%d\r\n", wifi_ok);
    HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), 100);
    osDelay(5000);
  }

  uint32_t last_report = HAL_GetTick();
  while(1) {
    if (wifi_ok == 0 && (HAL_GetTick() - last_report >= 30000)) {
      HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: report...\r\n", 18, 100);
      if (ESP8266_Report() != 0) {
        HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: report FAIL\r\n", 20, 100);
        wifi_ok = ESP8266_Init();
      } else {
        HAL_UART_Transmit(&huart1, (uint8_t *)"WiFi: report OK\r\n", 18, 100);
      }
      last_report = HAL_GetTick();
    }
    osDelay(100);
  }
}

/* 日志任务：每 60 秒写入一条记录 */
void Task_Logger(void *argument) {
  osDelay(8000);  /* 等系统稳定 + WiFi 初始化完成 */
  while (1) {
    Logger_Write();
    g_state.log_count = Logger_GetCount();
    osDelay(60000);  /* 60 秒 */
  }
}

/* 报警任务：等传感器稳定后再开始检查 */
void Task_Alarm(void *argument) {
  osDelay(10000);  /* 等DHT11/MPU6050稳定，避免误报警 */
  /* 启动时强制关闭蜂鸣器和报警状态 */
  g_state.alarm = ALARM_NONE;
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
  while (1) {
    Alarm_Check();
    g_state.log_count = Logger_GetCount();
    osDelay(1000);
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
  UART1_MutexHandle = osMutexNew(&UART1_Mutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  OLEDTaskHandle = osThreadNew(OLEDTask, NULL, &OLEDTask_attributes);
  SensorTaskHandle = osThreadNew(SensorTask, NULL, &SensorTask_attributes);
  ButtonTaskHandle = osThreadNew(buttonTask, NULL, &ButtonTask_attributes);
  BT_TaskHandle = osThreadNew(Task_BT, NULL, &BT_Task_attributes);
  WiFi_TaskHandle = osThreadNew(Task_WiFi, NULL, &WiFi_Task_attributes);
  LoggerTaskHandle = osThreadNew(Task_Logger, NULL, &LoggerTask_attributes);
  AlarmTaskHandle = osThreadNew(Task_Alarm, NULL, &AlarmTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
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
  /* 初始化外设 */
  DebugPrint("SYS: W25Q64_Init...\r\n");
  W25Q64_Init();
  DebugPrint("SYS: Logger_Init...\r\n");
  Logger_Init();
  DebugPrint("SYS: Alarm_Init...\r\n");
  Alarm_Init();

  /* 启动时立即设LED绿灯 + 蜂鸣器关（硬件active-high，LOW=亮/响） */
  g_state.alarm = ALARM_NONE;
  HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_RESET);    /* 红灯灭 */
  HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_SET);  /* 绿灯亮 */
  HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_RESET); /* 黄灯灭 */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);        /* 蜂鸣器关 */
  DebugPrint("SYS: LED set to GREEN\r\n");

  /* 主循环：LED 状态灯控制 */
  uint32_t last_blink = 0;
  uint32_t last_dbg = 0;
  char dbg[48];
  for (;;) {
    uint32_t now = HAL_GetTick();

    /* 每5秒打印一次报警状态 */
    if (now - last_dbg >= 5000) {
      snprintf(dbg, sizeof(dbg), "SYS: alarm=%d tick=%u\r\n",
               (int)g_state.alarm, (unsigned)now);
      DebugPrint(dbg);
      last_dbg = now;
    }

    switch (g_state.alarm) {
    case ALARM_CRITICAL:
      /* 红灯快闪（200ms），active-high */
      if (now - last_blink >= 200) {
        HAL_GPIO_TogglePin(Red_LED_GPIO_Port, Red_LED_Pin);
        HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_RESET);  /* 绿灭 */
        HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_RESET); /* 黄灭 */
        last_blink = now;
      }
      break;

    case ALARM_WARN:
      /* 黄灯常亮，active-high */
      HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_RESET);    /* 红灭 */
      HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_RESET); /* 绿灭 */
      HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_SET);  /* 黄亮 */
      break;

    default:
      /* 绿灯常亮（正常），active-high */
      HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_RESET);    /* 红灭 */
      HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_SET);   /* 绿亮 */
      HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_RESET); /* 黄灭 */
      break;
    }

    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */
