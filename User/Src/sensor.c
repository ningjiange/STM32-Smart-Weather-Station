#include "main.h"
#include <stdint.h>
#include "stm32f1xx_hal_gpio.h"
#include "system_state.h"
#include "math.h"
#include "adc.h"
#include "i2c.h"
#include "freertos.h"
#include "cmsis_os2.h"
#include "task.h"

#define MPU6050_ADDR (0x68 << 1) // HAL 要求 8bit 地址，左移 1 位

void LightSensor_Read(void) {
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
    g_state.light_raw = HAL_ADC_GetValue(&hadc1);
    // 这里可以将 adc_value 存储到全局变量或者系统状态中
  }
  HAL_ADC_Stop(&hadc1);
}

// ===== 直接寄存器读引脚，比 HAL_GPIO_ReadPin 快 =====
#define DHT11_READ() ((DHT11_GPIO_Port->IDR & DHT11_Pin) ? 1 : 0)

static void DHT11_SetOutput(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_SetInput(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

// ===== TIM2 做微秒计数器（重配置PSC=7, 1us一跳） =====
void DHT11_TIM_Init(void) {
  __HAL_RCC_TIM2_CLK_ENABLE();
  TIM2->CR1 &= ~TIM_CR1_CEN; // 先停
  // APB1=36MHz, 定时器时钟=2×APB1=72MHz
  // PSC=71 → 72MHz/(71+1)=1MHz → 1us/count
  TIM2->PSC = 71;
  TIM2->ARR = 0xFFFF;
  TIM2->EGR = TIM_EGR_UG;   // 立即加载PSC
  TIM2->CR1 |= TIM_CR1_CEN; // 启动
}

static void delay_us(uint32_t us) {
  uint32_t start = TIM2->CNT;
  while ((TIM2->CNT - start) < us)
    ;
}

// ===== DHT11 调试 =====
// debug_code: 0=成功, 1=等待从机拉低超时, 2=等待从机拉高超时,
//             3=等待数据开始超时, 4=读bit超时, 5=校验失败
uint8_t dht11_debug_code = 0;

// ===== DHT11 读取 =====
void DHT11_Read(void) {
  uint8_t data[5] = {0};
  uint32_t timeout;

  // 0. 总线空闲恢复（确保DHT11处于空闲状态）
  DHT11_SetInput();
  delay_us(50); // 等总线稳定

  // 1. 发开始信号（用TIM2硬件延时，不依赖SysTick的HAL_Delay）
  DHT11_SetOutput();
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
  delay_us(20000); // 拉低20ms，TIM2计数，不受FreeRTOS影响

  // 2. 关中断，后面时序敏感
  __disable_irq();
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
  delay_us(30);
  DHT11_SetInput(); // 切输入

  // 3. 等DHT11响应（拉低80us + 拉高80us）
  timeout = 500;
  while (DHT11_READ() && --timeout)
    ;
  if (timeout == 0) {
    dht11_debug_code = 1;
    __enable_irq();
    return;
  }

  timeout = 500;
  while (!DHT11_READ() && --timeout)
    ;
  if (timeout == 0) {
    dht11_debug_code = 2;
    __enable_irq();
    return;
  }

  timeout = 500;
  while (DHT11_READ() && --timeout)
    ;
  if (timeout == 0) {
    dht11_debug_code = 3;
    __enable_irq();
    return;
  }

  // 4. 读40bit
  for (int i = 0; i < 5; i++) {
    uint8_t byte = 0;
    for (int j = 0; j < 8; j++) {
      byte <<= 1;
      timeout = 500;
      while (!DHT11_READ() && --timeout)
        ;           // 等50us低电平结束
      delay_us(28); // ← 关键：28us，不是40
      if (DHT11_READ()) {
        byte |= 0x01;
        timeout = 500;
        while (DHT11_READ() && --timeout)
          ; // 等高电平结束
      } else {
        // 0-bit: 等高电平出现（准备下一个bit）
        timeout = 500;
        while (!DHT11_READ() && --timeout)
          ;
      }
    }
    data[i] = byte;
  }

  __enable_irq();

  // 5. 校验
  if ((data[0] + data[1] + data[2] + data[3]) == data[4]) {
    dht11_debug_code = 0;
    g_state.humi = (float)data[0];
    g_state.temp = (float)data[2];
  } else {
    dht11_debug_code = 5;
  }
}


void MPU6050_Init(void) {
  uint8_t data = 0x00;
  // 唤醒：写 0x00 到 0x6B 寄存器（PWR_MGMT_1）
  HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, 0x6B, 1, &data, 1, 100);
}

void MPU6050_Read(void) {
  uint8_t buf[14];
  HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, 0x3B, 1, buf, 14, 100);
  int16_t ax = (buf[0] << 8) | buf[1];
  int16_t ay = (buf[2] << 8) | buf[3];
  int16_t az = (buf[4] << 8) | buf[5];
  
  // 简单的姿态估计（不使用滤波器）
  g_state.pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * (180.0f / M_PI);
  g_state.roll = atan2f(ay, az) * (180.0f / M_PI);
}