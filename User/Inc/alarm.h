#ifndef __ALARM_H
#define __ALARM_H

#include <stdint.h>

typedef enum {
  ALARM_OFF = 0,
  ALARM_TEMP_HIGH,      /* 温度过高 */
  ALARM_TILT,           /* 设备倾斜 */
  ALARM_BOTH            /* 两者同时 */
} AlarmType_t;

/* 初始化 */
void Alarm_Init(void);

/* 检查报警条件并控制蜂鸣器 */
void Alarm_Check(void);

/* 获取当前报警类型 */
AlarmType_t Alarm_GetType(void);

/* 设置温度报警阈值（×10，如 350 表示 35.0°C） */
void Alarm_SetTempThreshold(uint16_t temp_x10);

/* 获取温度报警阈值（×10） */
uint16_t Alarm_GetTempThreshold(void);

#endif
