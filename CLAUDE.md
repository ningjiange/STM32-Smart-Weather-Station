# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

基于 STM32F103C8T6 + FreeRTOS 的智能桌面气象站。集成传感器采集、OLED 显示、风扇/舵机控制、WiFi/蓝牙通信、Flash 日志、报警系统。

## 构建命令

```bash
cmake --preset Debug
cmake --build build/Debug
```

VS Code 中可用 CMake 扩展：`Ctrl+Shift+P → CMake: Build`。

## 硬件外设分配

| 引脚 | 功能 | 接口 |
|------|------|------|
| PA0 | DHT11 温湿度 | 单总线 (软件) |
| PA1 | 光敏传感器 | ADC1 |
| PA2/PA3 | ESP8266 TX/RX | USART2 |
| PA6 | TB6612 风扇 PWM | TIM3 CH1 |
| PA7/PA8 | TB6612 AIN1/AIN2 | GPIO |
| PA9/PA10 | HC-06 蓝牙 TX/RX | USART1 |
| PA11 | SG-90 舵机 | TIM1 CH4 |
| PA15 | W25Q64 CS | SPI1 片选 |
| PB0/PB1 | 编码器 A/B | EXTI |
| PB2 | 编码器 SW | GPIO |
| PB3/PB4/PB5 | W25Q64 SCK/MISO/MOSI | SPI1 |
| PB6/PB7 | OLED SCL/SDA | 软件 I2C (地址 0x78) |
| PB8/PB9/PB13 | 绿/黄/红 LED | GPIO (active-high) |
| PB10/PB11 | MPU6050 SCL/SDA | I2C2 |
| PB12/PB13 | KEY1/KEY2 | GPIO |
| PB14 | 蜂鸣器 | GPIO (active-high) |

OLED 正负极直接接面包板电源轨，**不通过 GPIO 供电**。

## FreeRTOS 任务架构

| 任务 | 优先级 | 栈大小 | 周期 | 职责 |
|------|--------|--------|------|------|
| SensorTask | High | 1536B | 1s | DHT11/MPU6050/光敏采集 + 风扇/舵机控制 |
| OLEDTask | AboveNormal | 1280B | 200ms | 4页OLED显示切换 (实时/阈值/日志/系统) |
| defaultTask | AboveNormal | 2048B | 100ms | LED状态灯 (绿=正常/黄=警告/红=严重) |
| BT_Task | AboveNormal | 1024B | 阻塞 | 蓝牙指令接收与解析 |
| WiFi_Task | AboveNormal | 2048B | 30s | ESP8266 WiFi上报传感器数据到TCP服务器 |
| AlarmTask | AboveNormal | 512B | 1s | 温度/倾斜报警检测 |
| ButtonTask | AboveNormal | 512B | 10ms | 编码器旋转 + 按键消抖 |
| LoggerTask | Normal | 768B | 60s | W25Q64 Flash日志写入 |

所有任务通过全局结构体 `g_state` (SystemState_t) 共享数据，定义在 `User/Inc/system_state.h`。

## 用户模块

```
User/Src + User/Inc/
├── sensor.c/h      — DHT11 + 光敏ADC + MPU6050驱动
├── motor.c/h       — TB6612 风扇 PWM 调速 + SG-90 舵机角度控制
├── BT_WIFI.c/h     — HC-06 蓝牙指令解析 + ESP8266 AT指令驱动 + DebugPrint
├── alarm.c/h       — 多级报警 (温度/倾斜) + 蜂鸣器脉冲控制
├── logger.c/h      — W25Q64 Flash 日志 (12字节/条, 可存~5400条)
├── Encoder.c/h     — 旋转编码器读取 + 按键消抖 + buttonTask
├── oled.c/h        — SSD1306 OLED 软件I2C驱动
├── system_state.c/h — 全局状态结构体 g_state
├── w25q64.c/h      — SPI Flash 驱动
└── delay.c/h       — 微秒/毫秒延时
```

## 关键配置注意事项

- **FreeRTOS 堆大小**: `FreeRTOSConfig.h` 中 `configTOTAL_HEAP_SIZE` 必须设为 **12288** (12KB)。8KB 不够 9 个任务的栈 + TCB，会导致最后创建的任务 (WiFi_Task) 创建失败。
- **JTAG 引脚释放**: `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 必须在 `MX_SPI1_Init()` **之前**调用，否则 PB3/PB4/PB5 被 JTAG 占用，SPI1/W25Q64 初始化失败。
- **LED/蜂鸣器极性**: 硬件为 **active-high** (HIGH=亮/响，LOW=灭/静)。代码中 `HAL_GPIO_WritePin(..., GPIO_PIN_SET)` = 灭/关。
- **UART1 共享**: 蓝牙 (BT_Task) 和调试输出 (DebugPrint) 共用 USART1，通过 `UART1_MutexHandle` 互斥。DebugPrint 使用 50ms 短超时避免阻塞。
- **OLED 软件 I2C**: 使用 PB6/PB7 GPIO 位带模拟，**不是**硬件 I2C 外设。地址 `0x78`。

## CubeMX 注意

`.ioc` 中的 `configTOTAL_HEAP_SIZE` 仍为 8192（CubeMX 默认值），但实际已改为 12288。**不要**重新生成代码覆盖 `FreeRTOSConfig.h`。如果必须重新生成，需手动恢复堆大小。修改 `Core/Src` 和 `Core/Inc` 中的代码时，优先放在 `USER CODE BEGIN/END` 区域。
