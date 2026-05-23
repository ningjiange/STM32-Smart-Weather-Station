#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdint.h>

/* 日志记录结构体（10 字节） */
typedef struct {
  uint16_t temp;       /* 温度 ×10 */
  uint16_t humi;       /* 湿度 ×10 */
  uint16_t light;      /* 光照原始值 */
  int16_t  pitch;      /* 俯仰角 */
  uint16_t timestamp;  /* 开机后分钟数 */
} LogRecord_t;

/* 日志配置 */
#define LOG_SECTOR_ADDR   0x000000  /* W25Q64 起始地址 */
#define LOG_SECTOR_SIZE   4096      /* 1 个 Sector */
#define LOG_MAX_RECORDS   (LOG_SECTOR_SIZE / sizeof(LogRecord_t))  /* 409 条 */

/* 初始化 */
void Logger_Init(void);

/* 写入一条日志（从 g_state 读取数据） */
void Logger_Write(void);

/* 读取日志 */
uint16_t Logger_GetCount(void);
void Logger_ReadRecord(uint16_t index, LogRecord_t *record);

/* 清空日志 */
void Logger_Clear(void);

/* 通过蓝牙发送所有日志 */
void Logger_DumpToBT(void);

#endif
