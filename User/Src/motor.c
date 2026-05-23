#include "cmsis_os2.h"
#include "freertos.h"
#include "main.h"
#include "stm32f1xx_hal_gpio.h"
#include "system_state.h"
#include "task.h"
#include <stdint.h>
#include "tim.h"

void TB6612_Init() {
  HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
}

void fan_control() {
  if (g_state.mode != MODE_AUTO)
    return;
  if (g_state.temp < g_state.temp_threshold) {
    g_state.fan_speed = 0;                                   // 温度低于26°C时关闭风扇
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, g_state.fan_speed); // 设置PWM占空比
  } else if (g_state.temp < (g_state.temp_threshold + 4.0) && g_state.temp >= g_state.temp_threshold) {
    g_state.fan_speed = 30; // 温度在26°C到30°C之间时设置为中速
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, g_state.fan_speed*9/100); // 设置PWM占空比
  } else {
    g_state.fan_speed = 60; // 温度高于预制+4°C时设置为高速
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, g_state.fan_speed*9/100); // 设置PWM占空比
  }
}
//光敏ADC原始数据越大，光照越弱
void SG90_Control() {
  if (g_state.mode != MODE_AUTO)
    return;
  if(g_state.light_raw < 1000) { // 光敏 ADC 原始值小于1000时
    g_state.servo_angle = 180; // 设置舵机角度为180度
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 500+(g_state.servo_angle*2000)/180); // 设置PWM占空比
  } else if(g_state.light_raw < 3000 && g_state.light_raw >= 1000) { // 光敏 ADC 原始值在1000到3000之间时
    g_state.servo_angle = 90; // 设置舵机角度为90度
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 500+(g_state.servo_angle*2000)/180); // 设置PWM占空比
  } else {
    g_state.servo_angle = 0; // 光敏 ADC 原始值大于3000时设置为0度
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 500+(g_state.servo_angle*2000)/180); // 设置PWM占空比
  }
}