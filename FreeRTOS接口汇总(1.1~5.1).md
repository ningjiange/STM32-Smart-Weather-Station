# FreeRTOS/CMSIS 函数接口汇总 (1.1~5.1)

---

## 任务管理

| 函数 | 作用 | 参数 |
|------|------|------|
| `osThreadNew()` | 创建任务 | `func` 任务函数, `argument` 传参, `attr` 属性(栈大小/优先级/名称) |
| `osThreadTerminate()` | 删除任务 | `thread_id` 任务句柄 |
| `xTaskCreate()` | 创建任务(原生) | `pvTaskCode` 函数指针, `pcName` 名称, `usStackDepth` 栈深度(words), `pvParameters` 传参, `uxPriority` 优先级, `pxCreatedTask` 句柄指针 |
| `vTaskDelete()` | 删除任务(原生) | `xTaskToDelete` 句柄，`NULL` 表示删除自己 |

---

## 延时与调度

| 函数 | 作用 | 参数 |
|------|------|------|
| `osDelay()` | 相对延时(阻塞) | `ticks` 延时 tick 数 |
| `osDelayUntil()` | 绝对时间延时(周期精确) | `prev_wake` 上次唤醒时间指针, `delay` 延时 tick 数 |
| `vTaskDelay()` | 相对延时(原生) | `xTicksToDelay` 延时 tick 数 |
| `vTaskDelayUntil()` | 绝对时间延时(原生) | `pxPreviousWakeTime` 周期计时指针, `xTimeIncrement` 周期 tick 数 |
| `portYIELD()` | 主动让出 CPU | 无参数 |
| `taskYIELD()` | 主动让出 CPU(原生) | 无参数 |

---

## 队列

| 函数 | 作用 | 参数 |
|------|------|------|
| `osMessageQueueNew()` | 创建队列 | `msg_count` 深度, `msg_size` 每个元素大小(bytes), `attr` 属性 |
| `osMessageQueuePut()` | 发送消息到队列 | `queue_id` 队列句柄, `msg_ptr` 数据指针, `msg_prio` 优先级, `timeout` 超时(ms), 0=不等待 |
| `osMessageQueueGet()` | 从队列接收消息 | `queue_id` 队列句柄, `msg_ptr` 接收缓冲指针, `msg_prio` 返回优先级, `timeout` 超时(ms), `osWaitForever`=永久等待 |
| `xQueueSend()` | 发送(原生) | `xQueue` 句柄, `pvItemToQueue` 数据指针, `xTicksToWait` 超时 tick |
| `xQueueSendFromISR()` | 中断中发送 | `xQueue` 句柄, `pvItemToQueue` 数据, `pxHigherPriorityTaskWoken` 是否需要切换出参 |
| `xQueueReceive()` | 接收(原生) | `xQueue` 句柄, `pvBuffer` 接收缓冲, `xTicksToWait` 超时 |

---

## 信号量

| 函数 | 作用 | 参数 |
|------|------|------|
| `osSemaphoreNew()` | 创建信号量 | `max_count` 最大计数值, `initial_count` 初始计数值, `attr` 属性 |
| `osSemaphoreAcquire()` | 获取信号量 | `semaphore_id` 句柄, `timeout` 超时(ms), `osWaitForever`=永久等待 |
| `osSemaphoreRelease()` | 释放信号量 | `semaphore_id` 句柄 |
| `xSemaphoreCreateBinary()` | 创建二值信号量 | 无参数，返回句柄 |
| `xSemaphoreCreateCounting()` | 创建计数信号量 | `uxMaxCount` 最大值, `uxInitialCount` 初始值 |
| `xSemaphoreGive()` | 释放(原生) | `xSemaphore` 句柄 |
| `xSemaphoreGiveFromISR()` | 中断中释放 | `xSemaphore` 句柄, `pxHigherPriorityTaskWoken` 出参 |

---

## 互斥量

| 函数 | 作用 | 参数 |
|------|------|------|
| `osMutexNew()` | 创建互斥量 | `attr` 属性 |
| `xSemaphoreCreateMutex()` | 创建互斥量(原生) | 无参数 |
| `osMutexRecursiveNew()` | 创建递归互斥量 | `attr` 属性 |

---

## 事件组

| 函数 | 作用 | 参数 |
|------|------|------|
| `osEventFlagsNew()` | 创建事件组 | `attr` 属性 |
| `osEventFlagsSet()` | 置位事件标志 | `event_flags_id` 句柄, `flags` 要置位的 bit |
| `osEventFlagsWait()` | 等待事件标志 | `event_flags_id` 句柄, `flags` 等待的 bit, `options` `osFlagsWaitAll`/`osFlagsWaitAny`, `timeout` 超时 |
| `xEventGroupCreate()` | 创建(原生) | 无参数 |

---

## 任务通知

| 函数 | 作用 | 参数 |
|------|------|------|
| `osThreadFlagsSet()` | 发送通知(置位线程标志) | `thread_id` 任务句柄, `flags` 要置位的 bit |
| `osThreadFlagsWait()` | 等待通知 | `flags` 等待的 bit, `options` ALL/ANY, `timeout` 超时 |

---

## 软件定时器

| 函数 | 作用 | 参数 |
|------|------|------|
| `osTimerNew()` | 创建定时器 | `func` 回调函数, `type` `osTimerPeriodic`/`osTimerOnce`, `argument` 回调参数, `attr` 属性 |
| `osTimerStart()` | 启动定时器 | `timer_id` 句柄, `ticks` 定时 tick 数 |
| `osTimerStop()` | 停止定时器 | `timer_id` 句柄 |
| `osTimerDelete()` | 删除定时器 | `timer_id` 句柄 |
| `xTimerCreate()` | 创建(原生) | `pcTimerName` 名称, `xTimerPeriodInTicks` 周期, `uxAutoReload` 自动重载, `pvTimerID` ID, `pxCallbackFunction` 回调 |

---

## 内存管理

| 函数 | 作用 | 参数 |
|------|------|------|
| `pvPortMalloc()` | 动态分配内存 | `xWantedSize` 字节数，返回指针 |
| `vPortFree()` | 释放内存 | `pv` 指针 |
| `xPortGetFreeHeapSize()` | 当前剩余堆空间 | 无参数，返回 `size_t` |
| `xPortGetMinimumEverFreeHeapSize()` | 历史最低剩余堆 | 无参数，返回 `size_t` |
| `uxTaskGetStackHighWaterMark()` | 任务栈高水位线(最低剩余栈) | `xTask` 任务句柄，`NULL`=当前任务，返回剩余 words 数 |

---

## 内核调度(启动)

| 函数 | 作用 | 参数 |
|------|------|------|
| `osKernelStart()` | 启动调度器 | 无参数，返回状态 |

---

## 规律总结

- **CMSIS V2 接口**（`osXxx`）：在任务和中断中写法一致，底层自动处理安全问题，推荐项目中统一使用。
- **原生接口**（`xXxx`）：需要区分普通版本和 `FromISR` 版本（如 `xQueueSend` vs `xQueueSendFromISR`），中断中必须用后者。
