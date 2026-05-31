# STM32 智能桌面气象站

基于 **STM32F103C8T6 + FreeRTOS** 的智能桌面气象站，集成多传感器采集、OLED 显示、风扇/舵机控制、WiFi/蓝牙通信、Flash 日志记录和多级报警系统。

## 功能特性

- **多传感器数据采集**：DHT11 温湿度、光敏电阻光照强度、MPU6050 六轴姿态
- **OLED 实时显示**：4 页面切换（实时数据 / 阈值设置 / 日志查看 / 系统信息）
- **智能控制**：风扇 PWM 无级调速、舵机角度控制
- **无线通信**：HC-06 蓝牙指令控制 + ESP8266 WiFi 数据上报
- **数据持久化**：W25Q64 SPI Flash 日志存储（约 5400 条记录）
- **多级报警**：温度超限 / 设备倾斜检测 + 蜂鸣器脉冲报警
- **用户交互**：旋转编码器 + 按键操作

## 硬件清单

| 模块 | 型号 | 接口 |
|------|------|------|
| 主控 | STM32F103C8T6 | — |
| 温湿度传感器 | DHT11 | 单总线 (PA0) |
| 光照传感器 | 光敏电阻 | ADC1 (PA1) |
| 六轴传感器 | MPU6050 | I2C2 (PB10/PB11) |
| OLED 显示屏 | SSD1306 0.96寸 | 软件 I2C (PB6/PB7) |
| 电机驱动 | TB6612 | PWM + GPIO (PA6/PA7/PA8) |
| 舵机 | SG-90 | PWM (PA11) |
| 蓝牙模块 | HC-06 | USART1 (PA9/PA10) |
| WiFi 模块 | ESP8266 | USART2 (PA2/PA3) |
| Flash 存储 | W25Q64 | SPI1 (PA15/PB3/PB4/PB5) |
| 旋转编码器 | — | EXTI (PB0/PB1/PB2) |
| LED 指示灯 | 绿/黄/红 | GPIO (PB8/PB9/PB13) |
| 蜂鸣器 | 有源蜂鸣器 | GPIO (PB14) |

## 项目结构

```
├── Core/                   # CubeMX 生成的核心代码
│   ├── Inc/                # 头文件 (main.h, gpio.h, freertos_config.h 等)
│   └── Src/                # 源文件 (main.c, gpio.c, freertos.c 等)
├── Drivers/                # HAL 库 + CMSIS
├── Middlewares/            # FreeRTOS 中间件
├── User/
│   ├── Inc/                # 用户头文件
│   │   ├── sensor.h        # 传感器驱动接口
│   │   ├── motor.h         # 风扇/舵机控制接口
│   │   ├── BT_WIFI.h       # 蓝牙/WiFi 通信接口
│   │   ├── alarm.h         # 报警系统接口
│   │   ├── logger.h        # Flash 日志接口
│   │   ├── Encoder.h       # 编码器接口
│   │   ├── oled.h          # OLED 驱动接口
│   │   ├── w25q64.h        # SPI Flash 驱动接口
│   │   └── system_state.h  # 全局状态结构体
│   └── Src/                # 用户源文件
│       ├── sensor.c        # DHT11 + 光敏 ADC + MPU6050
│       ├── motor.c         # TB6612 风扇 PWM + SG-90 舵机
│       ├── BT_WIFI.c       # HC-06 蓝牙 + ESP8266 WiFi + 调试输出
│       ├── alarm.c         # 多级报警 + 蜂鸣器控制
│       ├── logger.c        # W25Q64 Flash 日志
│       ├── Encoder.c       # 旋转编码器 + 按键消抖
│       ├── oled.c          # SSD1306 软件 I2C 驱动
│       ├── w25q64.c        # SPI Flash 驱动
│       └── system_state.c  # 全局状态管理
├── CMakeLists.txt          # CMake 构建配置
├── CMakePresets.json        # CMake 预设
└── freertos学习.ioc        # CubeMX 配置文件
```

## FreeRTOS 任务架构

| 任务 | 优先级 | 栈大小 | 周期 | 职责 |
|------|--------|--------|------|------|
| SensorTask | High | 1536B | 1s | 传感器采集 + 执行器控制 |
| OLEDTask | AboveNormal | 1280B | 200ms | OLED 4 页面显示 |
| defaultTask | AboveNormal | 2048B | 100ms | LED 状态指示 |
| BT_Task | AboveNormal | 1024B | 阻塞 | 蓝牙指令接收 |
| WiFi_Task | AboveNormal | 2048B | 30s | WiFi 数据上报 |
| AlarmTask | AboveNormal | 512B | 1s | 温度/倾斜报警 |
| ButtonTask | AboveNormal | 512B | 10ms | 编码器 + 按键消抖 |
| LoggerTask | Normal | 768B | 60s | Flash 日志写入 |

## 构建方式

### 环境要求

- STM32CubeMX
- VS Code + CMake 扩展
- arm-none-eabi-gcc 交叉编译器
- CMake + Ninja

### 编译

```bash
cmake --preset Debug
cmake --build build/Debug
```

或在 VS Code 中：`Ctrl+Shift+P → CMake: Build`

### 烧录

通过 ST-Link 或串口下载到 STM32F103C8T6 开发板。

## 蓝牙指令协议

通过 HC-06 蓝牙模块发送以下指令：

| 指令 | 功能 |
|------|------|
| `FAN:0-100` | 设置风扇转速 (0-100%) |
| `SERVO:0-180` | 设置舵机角度 |
| `THRESHOLD:xx` | 设置温度报警阈值 |
| `STATUS` | 查询当前状态 |
| `LOG` | 导出 Flash 日志 |

## 关键配置

- **FreeRTOS 堆大小**：必须设为 12288 (12KB)，8KB 会导致任务创建失败
- **JTAG 引脚释放**：PB3/PB4/PB5 需释放给 SPI1 使用
- **LED/蜂鸣器极性**：active-high（HIGH=亮/响）

## License

MIT
