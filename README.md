# STM32 智能桌面气象站

基于 **STM32F103C8T6 + FreeRTOS** 的智能桌面气象站，集成多传感器采集、OLED 显示、风扇/舵机控制、WiFi/蓝牙通信、Flash 日志记录和多级报警系统。

**技术栈**：C · STM32 HAL · FreeRTOS · CMake · 传感器驱动 · PWM · SPI · I2C · UART · 蓝牙通信 · WiFi 通信

## 功能特性

- **多传感器数据采集**：DHT11 温湿度、光敏电阻光照强度、MPU6050 六轴姿态
- **OLED 实时显示**：4 页面切换（实时数据 / 阈值设置 / 日志查看 / 系统信息）
- **智能控制**：风扇 PWM 无级调速、舵机角度控制
- **无线通信**：HC-06 蓝牙指令控制 + ESP8266 WiFi 数据上报
- **数据持久化**：W25Q64 SPI Flash 日志存储（约 5400 条记录）
- **多级报警**：温度超限 / 设备倾斜检测 + 蜂鸣器脉冲报警
- **用户交互**：旋转编码器 + 按键操作

## 技术架构

### 硬件模块

| 类别 | 模块 | 说明 |
|------|------|------|
| 主控 | STM32F103C8T6 | ARM Cortex-M3, 72MHz |
| 传感器 | DHT11 / 光敏电阻 / MPU6050 | 温湿度 + 光照 + 六轴姿态 |
| 显示 | SSD1306 OLED | 软件 I2C 驱动 |
| 执行器 | TB6612 + SG-90 | 风扇 PWM 调速 + 舵机角度控制 |
| 通信 | HC-06 + ESP8266 | 蓝牙指令控制 + WiFi 数据上报 |
| 存储 | W25Q64 | 64Mbit SPI Flash, 日志持久化 |
| 交互 | 旋转编码器 + 按键 | 参数调节 + 页面切换 |

### FreeRTOS 任务架构

系统基于 FreeRTOS 实现 8 个并发任务，通过全局状态结构体共享数据：

| 任务 | 职责 | 设计要点 |
|------|------|----------|
| SensorTask | 传感器采集 + 执行器控制 | 1s 周期, 高优先级 |
| OLEDTask | 4 页面 OLED 显示 | 200ms 刷新, 页面缓冲 |
| defaultTask | LED 状态指示 | 100ms 周期, 三色分级 |
| BT_Task | 蓝牙指令接收与解析 | 阻塞等待, 互斥访问串口 |
| WiFi_Task | WiFi 数据上报 | 30s 周期, AT 指令驱动 |
| AlarmTask | 温度/倾斜报警检测 | 多级阈值, 蜂鸣器脉冲 |
| ButtonTask | 编码器 + 按键消抖 | 10ms 扫描, 软件消抖 |
| LoggerTask | Flash 日志写入 | 60s 周期, 环形存储 |

### 蓝牙指令协议

通过 HC-06 蓝牙模块支持远程控制：

| 指令 | 功能 |
|------|------|
| `FAN:0-100` | 设置风扇转速 (0-100%) |
| `SERVO:0-180` | 设置舵机角度 |
| `THRESHOLD:xx` | 设置温度报警阈值 |
| `STATUS` | 查询当前状态 |
| `LOG` | 导出 Flash 日志 |

## 项目结构

```
├── Core/
│   ├── Src/main.c                  — 系统初始化、外设配置、调度器启动
│   ├── Src/freertos.c              — FreeRTOS 任务定义与创建
│   ├── Src/gpio.c                  — GPIO 引脚初始化
│   ├── Src/tim.c                   — 定时器 PWM 配置 (风扇/舵机)
│   ├── Src/usart.c                 — UART1/UART2 串口配置
│   ├── Src/adc.c                   — ADC1 光敏采集配置
│   ├── Src/i2c.c                   — I2C2 MPU6050 通信配置
│   ├── Src/spi.c                   — SPI1 W25Q64 Flash 配置
│   └── Inc/FreeRTOSConfig.h        — FreeRTOS 配置 (堆12KB, 栈溢出检测)
├── User/
│   ├── Src/sensor.c                — DHT11 温湿度 + MPU6050 姿态 + 光敏驱动
│   ├── Src/motor.c                 — TB6612 风扇 PWM 调速 + SG90 舵机角度控制
│   ├── Src/BT_WIFI.c              — HC-06 蓝牙指令解析 + ESP8266 AT指令驱动
│   ├── Src/alarm.c                 — 多级报警 (温度/倾斜) + 蜂鸣器脉冲
│   ├── Src/logger.c                — W25Q64 Flash 日志 (10字节/条, ~409条)
│   ├── Src/Encoder.c               — 旋转编码器 + 按键消抖
│   ├── Src/oled.c                  — SSD1306 OLED 软件 I2C 驱动
│   ├── Src/w25q64.c                — SPI Flash 底层读写驱动
│   ├── Src/system_state.c          — 全局状态结构体 g_state 初始化
│   ├── Src/delay.c                 — 微秒/毫秒延时
│   └── Inc/                        — 对应头文件
├── Drivers/                        — STM32 HAL 驱动库 (自动生成, 勿改)
├── Middlewares/Third_Party/FreeRTOS — FreeRTOS 内核源码
├── CMakeLists.txt                  — CMake 构建配置
├── CMakePresets.json               — 编译预设 (Debug/Release)
├── freertos学习.ioc                — CubeMX 工程配置
├── 项目技术文档.md                  — 详细技术文档
└── CLAUDE.md                       — AI 辅助开发指引
```

## 项目难点与解决方案

| 难点 | 解决方案 |
|------|----------|
| FreeRTOS 多任务资源冲突 | 蓝牙与调试输出共用 USART1，通过互斥锁 + 短超时避免阻塞 |
| JTAG 引脚占用 SPI1 | 启动时释放 JTAG 引脚，将 PB3/PB4/PB5 重映射为 SPI1 |
| OLED 软件 I2C 时序 | GPIO 位带模拟实现 I2C 协议，兼容 SSD1306 驱动 |
| Flash 日志环形存储 | 设计环形缓冲区，自动覆盖旧记录，支持蓝牙导出 |
| FreeRTOS 堆内存不足 | 通过分析 TCB + 栈开销，精确计算堆大小为 12KB |

## License

MIT
