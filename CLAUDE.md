# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

OLED 显示屏驱动实验项目，使用 STM32F103xB 通过软件 I2C（GPIO 位带模拟）驱动 SSD1306 OLED 屏幕。

## 构建命令

```bash
cmake --preset Debug
cmake --build build/Debug
```

VS Code 中可用 CMake 扩展：`Ctrl+Shift+P → CMake: Build`。

## OLED 硬件连接 (软件 I2C)

| 引脚 | OLED 功能         |
| ---- | ----------------- |
| PB4  | GND (输出低)      |
| PB5  | VCC (输出高)      |
| PB6  | SCL (开漏 + 上拉) |
| PB7  | SDA (开漏 + 上拉) |

OLED 使用 GPIO 位带模拟 I2C 时序，**不是硬件 I2C 外设**。I2C 从机地址为 `0x78`。

## 关键架构

- `User/Inc/oled.h` / `User/Src/oled.c` — OLED 驱动模块，提供 `OLED_Init`、`OLED_ShowString`、`OLED_ShowNum` 等接口。使用 8x16 字体，4 行 16 列显示。
- `User/Inc/oled_font.h` — 8x16 ASCII 字库数据。
- `User/Inc/delay.h` / `User/Src/delay.c` — 微秒延时函数。
- `Core/Inc/main.h` — 定义了 `OLED_GND`、`OLED_VCC`、`OLED_SCL`、`OLED_SDA` 的引脚宏。
- `cmake/stm32cubemx/CMakeLists.txt` — **CubeMX 生成的构建文件**，用户 .c 文件需要在此文件的 `MX_Application_Src` 列表中注册。

## 注意事项

- OLED 的 GND 和 VCC 通过 GPIO 供电（PB4 输出低，PB5 输出高），代码启动后需要在 `OLED_I2C_Init()` 中先初始化。
- `main.c` 中的 `OLED_Init()` 和 `OLED_ShowString()` 调用在 `USER CODE BEGIN/END 2` 区域内。
- 工程中无 `fix_cmake_user.ps1`，需手动在 `cmake/stm32cubemx/CMakeLists.txt` 的 `MX_Application_Src` 中添加新文件。

## 学习进度记录

- FreeRTOS 初学者，正在学习任务创建和调度
- 已掌握：osThreadNew 创建任务、osDelay 延时、任务优先级、抢占
- 正在学：__已经学习完1.1，1.2并且完成了实验1，2_（随时更新）
