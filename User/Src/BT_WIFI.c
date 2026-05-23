#include "cmsis_os2.h"
#include "freertos.h"
#include "main.h"
#include "stm32f1xx_hal_uart.h"
#include "system_state.h"
#include "task.h"
#include "logger.h"
#include "alarm.h"
#include <stdint.h>
#include <usart.h>
#include "tim.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*==================== UART1 互斥量（蓝牙+诊断共用）====================*/
extern osMutexId_t UART1_MutexHandle;

/* 诊断输出：用短超时获取互斥量，避免被蓝牙任务卡死 */
void DebugPrint(const char *msg) {
  if (osMutexAcquire(UART1_MutexHandle, 50) == osOK) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 200);
    osMutexRelease(UART1_MutexHandle);
  }
}

/*==================== 蓝牙指令解析 ====================*/
static void BT_ParseCommand(char *cmd) {
  char reply[32];

  if (strncmp(cmd, "FAN:", 4) == 0) {
    int val = atoi(cmd + 4);
    g_state.fan_speed = (uint8_t)val;
    if (g_state.mode == MODE_AUTO) g_state.mode = MODE_MANUAL;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)val * 9 / 100);
    snprintf(reply, sizeof(reply), "OK FAN:%d\r\n", val);
  } else if (strncmp(cmd, "SERVO:", 6) == 0) {
    int val = atoi(cmd + 6);
    g_state.servo_angle = (uint16_t)val;
    if (g_state.mode == MODE_AUTO) g_state.mode = MODE_MANUAL;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, (uint32_t)(500 + (val * 2000) / 180));
    snprintf(reply, sizeof(reply), "OK SERVO:%d\r\n", val);
  } else if (strcmp(cmd, "STATUS") == 0) {
    snprintf(reply, sizeof(reply), "T:%d H:%d L:%d\r\n",
             (int)g_state.temp, (int)g_state.humi, (int)g_state.light_raw);
  } else if (strcmp(cmd, "STOP") == 0) {
    g_state.fan_speed = 0;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    snprintf(reply, sizeof(reply), "OK STOP\r\n");
  } else if (strcmp(cmd, "LOG:DUMP") == 0) {
    snprintf(reply, sizeof(reply), "LOG %u rec\r\n", (unsigned)Logger_GetCount());
    osMutexAcquire(UART1_MutexHandle, osWaitForever);
    HAL_UART_Transmit(&huart1, (uint8_t *)reply, strlen(reply), 100);
    osMutexRelease(UART1_MutexHandle);
    Logger_DumpToBT();
    return;  /* 已发送，不走下面的回复 */
  } else if (strcmp(cmd, "LOG:CLEAR") == 0) {
    Logger_Clear();
    g_state.log_count = 0;
    snprintf(reply, sizeof(reply), "OK LOG CLR\r\n");
  } else if (strncmp(cmd, "THRESHOLD:", 10) == 0) {
    int val = atoi(cmd + 10);
    Alarm_SetTempThreshold((uint16_t)(val * 10));
    g_state.temp_threshold = (float)val;
    snprintf(reply, sizeof(reply), "OK TH:%d\r\n", val);
  } else {
    snprintf(reply, sizeof(reply), "ERR\r\n");
  }

  /* 蓝牙回复需要保证送达，用 osWaitForever */
  osMutexAcquire(UART1_MutexHandle, osWaitForever);
  HAL_UART_Transmit(&huart1, (uint8_t *)reply, strlen(reply), 100);
  osMutexRelease(UART1_MutexHandle);
}

/* 蓝牙接收任务 */
void BT_Control(void) {
  char buffer[32];
  uint8_t idx;
  uint8_t byte;

  while (1) {
    idx = 0;
    while (1) {
      if (HAL_UART_Receive(&huart1, &byte, 1, osWaitForever) == HAL_OK) {
        if (byte == '\n' || byte == '\r') {
          if (idx > 0) break;
        } else if (idx < 31) {
          buffer[idx++] = byte;
        }
      }
    }
    buffer[idx] = '\0';
    BT_ParseCommand(buffer);
  }
}

/*==================== ESP8266 WiFi 驱动 ====================*/
static char esp_rx_buf[512];
static uint16_t esp_rx_len = 0;
static uint8_t esp_rx_byte;

#define ESP8266_TIMEOUT       5000
#define ESP8266_WIFI_TIMEOUT  15000

static void ESP8266_ClearBuf(void) {
  esp_rx_len = 0;
  memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
}

/* 发 AT 指令并等待期望响应（成功返回 0，超时返回 1） */
static uint8_t ESP8266_SendCmd(const char *cmd, const char *expect,
                               uint32_t timeout) {
  char full_cmd[128];

  ESP8266_ClearBuf();

  snprintf(full_cmd, sizeof(full_cmd), "%s\r\n", cmd);
  HAL_UART_Transmit(&huart2, (uint8_t *)full_cmd, strlen(full_cmd), 1000);

  uint32_t tick = HAL_GetTick();
  while ((HAL_GetTick() - tick) < timeout) {
    if (HAL_UART_Receive(&huart2, &esp_rx_byte, 1, 50) == HAL_OK) {
      if (esp_rx_len < sizeof(esp_rx_buf) - 1) {
        esp_rx_buf[esp_rx_len++] = esp_rx_byte;
        esp_rx_buf[esp_rx_len] = '\0';
      }
      if (strstr(esp_rx_buf, expect) != NULL) {
        return 0;
      }
    }
  }
  return 1;
}

/* 初始化 ESP8266，带逐段诊断（直接写 UART1，不走互斥量） */
#define ESP_DIAG(msg, len) HAL_UART_Transmit(&huart1, (uint8_t *)(msg), (len), 100)

uint8_t ESP8266_Init(void) {
  char dbg[64];
  uint8_t dummy;

  /* 1. 先排空 UART2 缓冲区 */
  ESP_DIAG("ESP: drain...\r\n", 14);
  while (HAL_UART_Receive(&huart2, &dummy, 1, 100) == HAL_OK);
  ESP_DIAG("ESP: drain done\r\n", 17);

  /* 2. 发 RST 重置（不等OK，直接等3秒） */
  ESP_DIAG("ESP: RST...\r\n", 13);
  HAL_UART_Transmit(&huart2, (uint8_t *)"AT+RST\r\n", 8, 100);
  HAL_Delay(3000);
  /* 排空重启杂音 */
  while (HAL_UART_Receive(&huart2, &dummy, 1, 100) == HAL_OK);
  ESP_DIAG("ESP: RST done\r\n", 15);

  /* 3. 测试 AT 通信（最多试3次） */
  ESP_DIAG("ESP: AT test\r\n", 14);
  uint8_t at_ok = 0;
  for (int i = 0; i < 3; i++) {
    while (HAL_UART_Receive(&huart2, &dummy, 1, 50) == HAL_OK);
    if (ESP8266_SendCmd("AT", "OK", 3000) == 0) {
      at_ok = 1;
      break;
    }
    snprintf(dbg, sizeof(dbg), "ESP: AT retry %d\r\n", i);
    ESP_DIAG(dbg, strlen(dbg));
  }
  if (!at_ok) {
    ESP_DIAG("ESP: AT fail\r\n", 14);
    return 1;
  }
  ESP_DIAG("ESP: AT OK\r\n", 12);

  /* 关闭回显 */
  ESP8266_SendCmd("ATE0", "OK", 2000);
  ESP_DIAG("ESP: ATE0 done\r\n", 16);

  /* 4. Station 模式 */
  ESP_DIAG("ESP: CWMODE\r\n", 13);
  if (ESP8266_SendCmd("AT+CWMODE=1", "OK", ESP8266_TIMEOUT) != 0) {
    ESP_DIAG("ESP: CWMODE fail\r\n", 18);
    return 2;
  }
  ESP_DIAG("ESP: CWMODE OK\r\n", 17);

  /* 5. 连接 WiFi */
  ESP_DIAG("ESP: CWJAP\r\n", 12);
  if (ESP8266_SendCmd("AT+CWJAP=\"MyHotspot\",\"12345678\"",
                       "OK", ESP8266_WIFI_TIMEOUT) != 0) {
    ESP_DIAG("ESP: CWJAP fail\r\n", 17);
    return 3;
  }
  ESP_DIAG("ESP: CWJAP OK\r\n", 16);

  /* 6. 查询 IP */
  ESP8266_ClearBuf();
  HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CIFSR\r\n", 10, 100);
  uint32_t tick = HAL_GetTick();
  while ((HAL_GetTick() - tick) < 3000) {
    if (HAL_UART_Receive(&huart2, &esp_rx_byte, 1, 50) == HAL_OK) {
      if (esp_rx_len < sizeof(esp_rx_buf) - 1) {
        esp_rx_buf[esp_rx_len++] = esp_rx_byte;
        esp_rx_buf[esp_rx_len] = '\0';
      }
    }
  }
  snprintf(dbg, sizeof(dbg), "ESP: CIFSR [%.30s]\r\n", esp_rx_buf);
  ESP_DIAG(dbg, strlen(dbg));

  return 0;
}

/* 上报传感器数据 */
uint8_t ESP8266_Report(void) {
  char cmd[64];
  char json[100];
  char dbg[48];
  uint32_t tick;

  sprintf(json, "{\"temp\":%d,\"humi\":%d,\"light\":%d,\"fan\":%d}",
          (int)g_state.temp, (int)g_state.humi, g_state.light_raw,
          g_state.fan_speed);

  /* CIPSTART */
  DebugPrint("ESP: CIPSTART...\r\n");
  if (ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"10.89.186.172\",8080",
                       "CONNECT", ESP8266_TIMEOUT) != 0) {
    /* 尝试接受 OK */
    if (ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"10.89.186.172\",8080",
                         "OK", ESP8266_TIMEOUT) != 0) {
      snprintf(dbg, sizeof(dbg), "ESP: CIPFAIL [%.30s]\r\n", esp_rx_buf);
      DebugPrint(dbg);
      return 1;
    }
  }
  DebugPrint("ESP: CIP OK\r\n");

  /* CIPSEND */
  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", strlen(json));
  DebugPrint("ESP: CIPSEND...\r\n");
  if (ESP8266_SendCmd(cmd, ">", ESP8266_TIMEOUT) != 0) {
    DebugPrint("ESP: CIPSEND fail\r\n");
    ESP8266_SendCmd("AT+CIPCLOSE", "OK", 2000);
    return 2;
  }

  /* 发送数据 */
  HAL_UART_Transmit(&huart2, (uint8_t *)json, strlen(json), 2000);

  /* 等 SEND OK */
  tick = HAL_GetTick();
  uint8_t send_ok = 0;
  ESP8266_ClearBuf();
  while ((HAL_GetTick() - tick) < ESP8266_TIMEOUT) {
    if (HAL_UART_Receive(&huart2, &esp_rx_byte, 1, 50) == HAL_OK) {
      if (esp_rx_len < sizeof(esp_rx_buf) - 1) {
        esp_rx_buf[esp_rx_len++] = esp_rx_byte;
        esp_rx_buf[esp_rx_len] = '\0';
      }
      if (strstr(esp_rx_buf, "SEND OK") != NULL) {
        send_ok = 1;
        break;
      }
    }
  }
  if (!send_ok) {
    snprintf(dbg, sizeof(dbg), "ESP: SENDFAIL [%.25s]\r\n", esp_rx_buf);
    DebugPrint(dbg);
    ESP8266_SendCmd("AT+CIPCLOSE", "OK", 2000);
    return 3;
  }

  DebugPrint("ESP: SEND OK\r\n");
  ESP8266_SendCmd("AT+CIPCLOSE", "OK", 2000);
  return 0;
}
