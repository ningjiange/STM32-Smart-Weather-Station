#include "cmsis_os2.h"
#include "freertos.h"
#include "main.h"
#include "stm32f1xx_hal_gpio.h"
#include "system_state.h"
#include "task.h"
#include <stdint.h>

// 编码器中断回调（HAL_GPIO_EXTI_IRQHandler 内部调用的弱函数）
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == EncoderA_Pin) {
        // A相上升沿触发，读B相判断方向
        if (HAL_GPIO_ReadPin(EncoderB_GPIO_Port, EncoderB_Pin) ==
            GPIO_PIN_SET) {
          if (g_state.temp_threshold > 15.0f) {
            g_state.temp_threshold -= 0.5f;  //顺时针减，不低于15°C
          }
        } else {
          if (g_state.temp_threshold < 35.0f) {
            g_state.temp_threshold += 0.5f;  // 逆时针加，不高于35°C
          }
        }
    }
}

static uint8_t sw_state = 0;  // 0=空闲, 1=检测到, 2=等待释放

void EncoderPress(void) {
    if (sw_state == 0) {
      if (HAL_GPIO_ReadPin(EncoderC_GPIO_Port, EncoderC_Pin) == GPIO_PIN_RESET) {
        sw_state = 1;  // 第一次检测到
      }
    } else if (sw_state == 1) {
      if (HAL_GPIO_ReadPin(EncoderC_GPIO_Port, EncoderC_Pin) == GPIO_PIN_RESET) {
        g_state.temp_threshold = 28.0f;  // 重置温度阈值
        sw_state = 2;
      } else {
        sw_state = 0;  // 抖动，忽略
      }
    } else if (sw_state == 2) {
      if (HAL_GPIO_ReadPin(EncoderC_GPIO_Port, EncoderC_Pin) == GPIO_PIN_SET) {
        sw_state = 0;  // 释放，恢复检测
      }
    }
}

void buttonTask(void *argument) {
    while (1) {
      EncoderPress();
      if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
        osDelay(20); // 按键消抖，等待按键释放
        if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
          while (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) ==
                 GPIO_PIN_RESET) // 等待按键释放
          {
            osDelay(10);
          }
          g_state.mode = (g_state.mode + 1) % 3; // 切换系统模式
        }
      } else {
        osDelay(10); // 避免CPU占用过高
      }
      if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
        osDelay(20); // 按键消抖，等待按键释放
        if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
          while (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) ==
                 GPIO_PIN_RESET) // 等待按键释放
          {
            osDelay(10);
          }
          g_state.display_page = (g_state.display_page + 1) % PAGE_COUNT; // 切换OLED页面
        }
      } else {
        osDelay(10); // 避免CPU占用过高
      }
    }
  }