#include "alarm.h"
#include "main.h"
#include "system_state.h"
#include "stm32f1xx_hal_gpio.h"

/* 报警阈值 */
#define TILT_THRESHOLD    30      /* 倾斜角度阈值（度） */
static uint16_t temp_threshold_x10 = 350;  /* 温度阈值 ×10（35.0°C） */

static AlarmType_t current_alarm = ALARM_OFF;
static uint32_t beep_toggle_tick = 0;

void Alarm_Init(void) {
  /* 蜂鸣器初始关（active-high，SET=关） */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
  current_alarm = ALARM_OFF;
}

void Alarm_Check(void) {
  uint8_t temp_high = 0;
  uint8_t tilt = 0;

  /* 检查温度 */
  if ((uint16_t)(g_state.temp * 10) >= temp_threshold_x10) {
    temp_high = 1;
  }

  /* 检查倾斜 */
  int16_t abs_pitch = g_state.pitch < 0 ? -g_state.pitch : g_state.pitch;
  int16_t abs_roll = g_state.roll < 0 ? -g_state.roll : g_state.roll;
  if (abs_pitch > TILT_THRESHOLD || abs_roll > TILT_THRESHOLD) {
    tilt = 1;
  }

  /* 确定报警类型 */
  if (temp_high && tilt) {
    current_alarm = ALARM_BOTH;
  } else if (temp_high) {
    current_alarm = ALARM_TEMP_HIGH;
  } else if (tilt) {
    current_alarm = ALARM_TILT;
  } else {
    current_alarm = ALARM_OFF;
  }

  /* 更新全局状态（active-high，SET=关，RESET=响） */
  if (current_alarm == ALARM_OFF) {
    g_state.alarm = ALARM_NONE;
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);  /* 关蜂鸣器 */
  } else {
    g_state.alarm = (current_alarm == ALARM_BOTH) ? ALARM_CRITICAL : ALARM_WARN;
    /* 蜂鸣器脉冲控制：500ms 切换一次 */
    uint32_t now = HAL_GetTick();
    if (now - beep_toggle_tick >= 500) {
      HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
      beep_toggle_tick = now;
    }
  }
}

AlarmType_t Alarm_GetType(void) {
  return current_alarm;
}

void Alarm_SetTempThreshold(uint16_t temp_x10) {
  temp_threshold_x10 = temp_x10;
}

uint16_t Alarm_GetTempThreshold(void) {
  return temp_threshold_x10;
}
