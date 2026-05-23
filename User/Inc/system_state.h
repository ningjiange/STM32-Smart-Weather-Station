#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#include "main.h"

typedef enum { MODE_AUTO = 0, MODE_MANUAL, MODE_SLEEP } SystemMode_t;
typedef enum { ALARM_NONE = 0, ALARM_WARN, ALARM_CRITICAL } AlarmLevel_t;
typedef enum { PAGE_DATA = 0, PAGE_SETTING, PAGE_LOG, PAGE_SYSTEM } DisplayPage_t;
#define PAGE_COUNT 4

typedef struct {
  // 传感器数据
  float temp;         // 温度
  float humi;         // 湿度
  uint16_t light_raw; // 光敏 ADC 原始值
  int16_t pitch;      // 俯仰角
  int16_t roll;       // 横滚角

  // 控制参数
  float temp_threshold; // 温度阈值
  uint8_t display_page; // OLED页面号：0=实时 1=阈值 2=日志 3=系统

  // 执行器状态
  uint8_t fan_speed;    // 风扇转速 0~100%
  uint16_t servo_angle; // 舵机角度 0~180°

  // 系统信息
  SystemMode_t mode;    // AUTO / MANUAL / SLEEP
  AlarmLevel_t alarm;   // 报警级别
  uint16_t log_count;   // 日志记录数
  uint32_t uptime_sec;  // 运行时间（秒）
  uint32_t free_heap;   // 剩余堆空间
} SystemState_t;

extern SystemState_t g_state; // 全局变量声明

#endif
