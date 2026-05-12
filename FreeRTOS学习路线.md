# FreeRTOS 学习路线 (STM32F103C8T6)

> 开发环境：STM32CubeMX + VS Code  
> 调试显示：0.96寸 OLED (软件I2C, SCL=PB6, SDA=PB7)  
> 芯片：STM32F103C8T6 (Cortex-M3, 72MHz, 20KB SRAM, 64KB Flash)

---

## 第一阶段：FreeRTOS 基础概念

### 1.1 认识 RTOS 与 FreeRTOS

**学习内容：**
- 什么是实时操作系统 (RTOS)，与裸机轮询的区别
- FreeRTOS 的内核结构：任务调度器、任务控制块 (TCB)
- CubeMX 中 FreeRTOS 的配置选项 (CMSIS_V1 / CMSIS_V2)
- FreeRTOS 的时钟节拍 (Tick) 概念，`configTICK_RATE_HZ` 的含义
- 系统启动流程：`main()` → HAL初始化 → `osKernelStart()` → 任务调度

**关键配置：**
```
CubeMX → Middleware → FREERTOS → Interface: CMSIS_V1
configTICK_RATE_HZ = 1000   (1ms 一个 tick)
configMINIMAL_STACK_SIZE = 128 (words)
configTOTAL_HEAP_SIZE = 10240 (bytes)
```

**实验 1：点亮第一个任务**
- 创建一个任务 `Task_LED`，周期性翻转 PC13 LED (500ms)
- 创建一个任务 `Task_OLED`，在 OLED 上显示 "FreeRTOS OK"
- 观察：LED 闪烁 + OLED 显示 = RTOS 已正常运行

---

### 1.2 任务创建与删除

**学习内容：**
- `osThreadNew()` / `xTaskCreate()` 函数原型与参数
- 任务函数原型：`void TaskFunc(void *argument)`
- 任务的五种状态：运行态、就绪态、阻塞态、挂起态、删除态
- 任务优先级：`osPriorityNormal`, `osPriorityAboveNormal` 等
- `osThreadTerminate()` 删除任务的注意事项
- 空闲任务 (Idle Task) 的作用，回收被删除任务的资源

**实验 2：多任务创建**
- 创建 3 个任务：
  - `Task_LED` (优先级 Low)：LED 500ms 闪烁
  - `Task_OLED_Show` (优先级 Normal)：OLED 第一行显示运行计数
  - `Task_Button` (优先级 AboveNormal)：检测按键，按下时在 OLED 第二行显示 "Pressed"
- 观察：3 个任务互不干扰同时运行
- 挑战：按键按下后删除 `Task_LED`，再按重新创建

---

### 1.3 任务延时与调度

**学习内容：**
- `osDelay()` vs `osDelayUntil()` 的区别
- `vTaskDelay()` 延时（相对）和 `vTaskDelayUntil()` 延时（绝对/周期精确）
- 阻塞延时 vs 空转等待的效率对比
- `configUSE_PREEMPTION`：抢占式 vs 协作式调度
- `configUSE_TIME_SLICING`：同优先级时间片轮转
- `portYIELD()` / `taskYIELD()` 主动让出 CPU

**实验 3：精确周期任务**
- 用 `osDelayUntil()` 实现精确 1000ms 周期
- OLED 第三行显示 `Task_LED` 的精确运行次数
- 对比 `osDelay(1000)` 与 `osDelayUntil()` 的周期误差
- 实验方法：让任务跑 100 次，OLED 显示实际耗时，观察误差累积

---

## 第二阶段：任务间通信

### 2.1 队列 (Queue)

**学习内容：**
- 队列的概念：FIFO 缓冲区，任务间传数据的主要手段
- `osMessageQueueNew()` / `xQueueCreate()`
- `osMessageQueuePut()` / `osMessageQueueGet()` 阻塞发送/接收
- 队列深度与数据宽度
- 队列的复制机制：数据是拷贝进去的，不是传指针

**实验 4：按键消息队列**
- 按键任务检测按键，将按键编号 (uint32) 发送到队列
- 显示任务从队列接收消息，OLED 上显示 "Key: 1" / "Key: 2"
- 实验体会：生产者-消费者模型

---

### 2.2 信号量 (Semaphore)

**学习内容：**
- 二值信号量：0/1，用于同步（事件通知）
- 计数信号量：计数值，用于资源管理
- `osSemaphoreNew()` / `xSemaphoreCreateBinary()` / `xSemaphoreCreateCounting()`
- `osSemaphoreAcquire()` / `osSemaphoreRelease()`
- 信号量与队列的区别：信号量只传信号，不传数据

**实验 5：中断同步**
- 配置定时器 TIM2 中断，每 500ms 触发一次
- ISR 中调用 `osSemaphoreRelease()`
- 任务中 `osSemaphoreAcquire()` 等待，收到后翻转 LED + OLED 计数+1
- 体会：中断 → 任务 的同步机制

---

### 2.3 互斥量 (Mutex)

**学习内容：**
- 互斥量 vs 信号量的区别：互斥量有优先级继承
- `osMutexNew()` / `xSemaphoreCreateMutex()`
- 临界区保护：多个任务访问共享资源 (OLED) 的安全问题
- 优先级反转问题及其解决方案
- 递归互斥量 `osMutexRecursiveNew()` 的使用场景

**实验 6：OLED 互斥访问**
- 创建一个互斥量保护 OLED
- Task_A：互斥量保护下，OLED 第 1-3 行显示 "A: count=xxx"
- Task_B：互斥量保护下，OLED 第 4-6 行显示 "B: count=xxx"
- 不加互斥量时观察花屏 → 加上后正常

---

### 2.4 事件组 (Event Group)

**学习内容：**
- 事件组：一个变量中多个 bit 表示多个事件
- `osEventFlagsNew()` / `xEventGroupCreate()`
- `osEventFlagsSet()` / `osEventFlagsWait()`
- 等待多个事件的逻辑：AND（所有位） vs OR（任一位）

**实验 7：多条件触发**
- 任务 A 检测按键1，按下后 Set bit0
- 任务 B 检测按键2，按下后 Set bit1
- 任务 C 等待 bit0 AND bit1 都置位，触发后 OLED 显示 "Both Ready!"
- 体会：多条件同步

---

## 第三阶段：高级同步与通信

### 3.1 任务通知 (Task Notification)

**学习内容：**
- 任务通知：比队列/信号量更轻量的替代方案
- `osThreadFlagsSet()` / `osThreadFlagsWait()`
- 通知的四种发送方式：NoAction, SetBits, Increment, OverwriteWith
- 任务通知的优势：更快、更省内存
- 任务通知的局限：只能一对一，不能广播

**实验 8：用任务通知替代信号量**
- 将实验 5 的信号量改为任务通知
- 对比代码量和响应速度
- OLED 显示两种方式的中断响应延迟对比

---

### 3.2 软件定时器 (Software Timer)

**学习内容：**
- 软件定时器的原理：由 Timer Service Task 管理
- `osTimerNew()` / `xTimerCreate()`
- 单次定时器 vs 周期定时器
- `osTimerStart()` / `osTimerStop()` / `osTimerDelete()`
- 定时器回调函数中只能调用 "FromISR" 安全的 API

**实验 9：定时器控制 LED**
- 创建周期定时器 (500ms)：回调中翻转 LED
- 创建单次定时器 (3s)：按键触发，3 秒后 OLED 显示 "Timeout!"
- 创建另一个周期定时器 (100ms)：回调中检测按键长按

---

## 第四阶段：内存管理

### 4.1 FreeRTOS 内存管理

**学习内容：**
- `heap_1` ~ `heap_5` 五种内存分配策略的区别
- `configTOTAL_HEAP_SIZE` 的含义与调整
- `pvPortMalloc()` / `vPortFree()` 的使用
- 内存碎片问题
- 静态分配 vs 动态分配任务
- `xPortGetFreeHeapSize()` 查看剩余堆空间

**实验 10：堆内存监控**
- OLED 实时显示 `xPortGetFreeHeapSize()`
- 创建/删除任务观察堆变化
- 故意创建内存泄漏，观察堆不断减少
- 使用 `heap_4` 策略，观察碎片整理效果

---

## 第五阶段：中断与 RTOS

### 5.1 中断优先级与 FreeRTOS

**学习内容：**
- Cortex-M3 的 NVIC 优先级机制
- `configMAX_SYSCALL_INTERRUPT_PRIORITY`：哪些优先级可以调用 FreeRTOS API
- `configKERNEL_INTERRUPT_PRIORITY`：SysTick 和 PendSV 的优先级
- 中断安全 API：后缀 `FromISR` 的函数
- 中断中不能调用阻塞函数

**关键规则：**
```
STM32F103 优先级分组 4 (4bit 抢占)
SysTick = 最低优先级 (15)
PendSV  = 最低优先级 (15)
FreeRTOS 可调用 API 的中断优先级: 5 ~ 15
用户中断优先级: 0 ~ 4 (不能调用 FreeRTOS API)
```

**实验 11：中断与任务协作**
- 配置 EXTI 外部中断 (按键)
- ISR 中用 `xQueueSendFromISR()` 发送按键事件
- 配置 UART 接收中断，ISR 中用 `xSemaphoreGiveFromISR()` 通知任务
- 任务中处理串口数据并在 OLED 显示

---

## 第六阶段：实用项目

### 6.1 多任务传感器采集系统

**实验 12：温度采集 + 显示 + 报警**
- Task_Sensor (优先级 High)：定时采集 ADC 值，通过队列发送
- Task_Display (优先级 Normal)：从队列接收，OLED 显示温度曲线
- Task_Alarm (优先级 AboveNormal)：判断温度超限，LED 快闪报警
- Task_Key (优先级 Normal)：按键切换显示模式

### 6.2 串口命令行系统

**实验 13：FreeRTOS + UART Shell**
- UART 接收中断 → 队列 → 解析任务
- 实现简单命令：`help`, `status`(显示各任务状态), `free`(显示堆剩余), `led on/off`
- 使用 `vTaskList()` 输出任务列表

---

## 第七阶段：性能分析与优化

### 7.1 运行时统计与调试

**学习内容：**
- `vTaskList()`：获取任务状态表 (Name, State, Priority, Stack, Num)
- `vTaskGetRunTimeStats()`：各任务 CPU 占用率
- `uxTaskGetStackHighWaterMark()`：栈溢出检测
- `configUSE_TRACE_FACILITY` 和 `configUSE_STATS_FORMATTING_FUNCTIONS` 配置
- FreeRTOS+Trace / Percepio Tracealyzer 工具介绍

**实验 14：任务健康监控**
- OLED 轮流显示每个任务的栈高水位线
- 按键触发时通过串口输出 `vTaskList()` 完整信息
- 故意制造栈溢出，观察 HardFault

---

### 7.2 低功耗与 Tickless Idle

**学习内容：**
- FreeRTOS 的 Tickless Idle 模式原理
- `configUSE_TICKLESS_IDLE` 配置
- `portSUPPRESS_TICKS_AND_SLEEP()` 的实现
- 空闲任务中进入 Stop/Standby 模式

**实验 15：低功耗测量**
- 配置 Tickless Idle
- 只保留一个 LED 闪烁任务 (周期 2s)
- 用万用表测量功耗变化

---

## 附录

### A. 关键 CubeMX 配置速查

| 配置项 | 推荐值 | 说明 |
|--------|--------|------|
| FREERTOS Interface | CMSIS_V1 | 接口风格 |
| configTICK_RATE_HZ | 1000 | 1ms 时钟节拍 |
| configMAX_PRIORITIES | 7 | 最大优先级数 |
| configMINIMAL_STACK_SIZE | 128 | 最小栈 (words) |
| configTOTAL_HEAP_SIZE | 10240 | 堆大小 (bytes) |
| configUSE_PREEMPTION | 1 | 抢占式调度 |
| configUSE_IDLE_HOOK | 0 | 初期不用 |
| configUSE_MUTEXES | 1 | 使能互斥量 |
| configUSE_COUNTING_SEMAPHORES | 1 | 使能计数信号量 |
| configUSE_RECURSIVE_MUTEXES | 1 | 使能递归互斥量 |

### B. OLED 调试库适配

建议封装以下函数供 FreeRTOS 任务调用：
```c
// OLED_Debug.c
void OLED_ShowTaskInfo(uint8_t row, const char *name, uint32_t count);
void OLED_ShowHeapFree(uint8_t row);
void OLED_ClearRow(uint8_t row);
void OLED_Printf(uint8_t row, uint8_t col, const char *fmt, ...);
```

### C. 学习顺序总结

```
阶段1 基础概念    → 实验 1~3  (任务创建/延时/调度)
阶段2 任务通信    → 实验 4~7  (队列/信号量/互斥/事件)
阶段3 高级同步    → 实验 8~9  (任务通知/软件定时器)
阶段4 内存管理    → 实验 10   (堆监控)
阶段5 中断协作    → 实验 11   (ISR ↔ 任务)
阶段6 实战项目    → 实验 12~13 (综合项目)
阶段7 性能优化    → 实验 14~15 (监控/低功耗)
```

### D. 推荐参考资料

- 《FreeRTOS 内核实现与应用开发实战指南》— 野火
- 《Mastering the FreeRTOS Real Time Kernel》— 官方电子书 (免费)
- FreeRTOS 官网文档：freertos.org
- CubeMX 生成的 `freertos.c` 源码 + 注释
