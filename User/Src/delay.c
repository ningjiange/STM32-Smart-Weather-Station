//非阻塞延时函数，使用HAL_GetTick()获取系统时间，适用于需要在主循环中执行其他任务的情况
#include "main.h"

uint8_t NonBlocking_Delay(uint32_t *last_time, uint32_t delay_ms) {
  uint32_t now = HAL_GetTick();

  if (now - *last_time >= delay_ms) {
    *last_time = now;
    return 1; // 时间到了
  }

  return 0; // 时间还没到
}
#include "main.h"

    uint8_t NonBlocking_Delay(uint32_t *last_time, uint32_t delay_ms) {
  uint32_t now = HAL_GetTick();

  if (now - *last_time >= delay_ms) {
    *last_time = now;
    return 1; // 时间到了
  }

  return 0; // 时间还没到
}