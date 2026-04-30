#include "delay.h"
#include "main.h"


// 非阻塞延时函数，使用 HAL_GetTick() 获取系统时间
uint8_t NonBlocking_Delay(uint32_t *last_time, uint32_t delay_ms) {
  uint32_t now = HAL_GetTick();

  if (now - *last_time >= delay_ms) {
    *last_time = now;
    return 1; // 时间到了
  }

  return 0; // 时间还没到
}