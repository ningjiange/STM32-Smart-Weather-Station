#include "logger.h"
#include "w25q64.h"
#include "system_state.h"
#include "BT_WIFI.h"
#include "stdio.h"
#include "string.h"

/* 当前写入位置 */
static uint16_t log_count = 0;     /* 已写入记录数 */
static uint32_t write_addr = 0;    /* 下次写入的 Flash 地址 */

void Logger_Init(void) {
  /* 读取 Sector 中已有的记录，找到写入位置 */
  LogRecord_t record;
  log_count = 0;

  for (uint16_t i = 0; i < LOG_MAX_RECORDS; i++) {
    W25Q64_ReadData(LOG_SECTOR_ADDR + i * sizeof(LogRecord_t),
                    (uint8_t *)&record, sizeof(LogRecord_t));
    /* 有效的记录 timestamp 不为 0xFFFF（擦除后是 0xFF） */
    if (record.timestamp == 0xFFFF)
      break;
    log_count = i + 1;
  }

  write_addr = LOG_SECTOR_ADDR + log_count * sizeof(LogRecord_t);

  /* 如果 Sector 满了，擦除后从头开始 */
  if (log_count >= LOG_MAX_RECORDS) {
    W25Q64_SectorErase(LOG_SECTOR_ADDR);
    log_count = 0;
    write_addr = LOG_SECTOR_ADDR;
  }
}

void Logger_Write(void) {
  LogRecord_t record;

  record.temp = (uint16_t)(g_state.temp * 10);
  record.humi = (uint16_t)(g_state.humi * 10);
  record.light = g_state.light_raw;
  record.pitch = g_state.pitch;
  record.timestamp = (uint16_t)(HAL_GetTick() / 60000);  /* 毫秒 → 分钟 */

  /* 写入 Flash */
  W25Q64_PageWrite(write_addr, (uint8_t *)&record, sizeof(LogRecord_t));

  log_count++;
  write_addr += sizeof(LogRecord_t);

  /* Sector 满了，擦除并从头开始 */
  if (log_count >= LOG_MAX_RECORDS) {
    W25Q64_SectorErase(LOG_SECTOR_ADDR);
    log_count = 0;
    write_addr = LOG_SECTOR_ADDR;
  }
}

uint16_t Logger_GetCount(void) {
  return log_count;
}

void Logger_ReadRecord(uint16_t index, LogRecord_t *record) {
  if (index >= log_count) {
    memset(record, 0, sizeof(LogRecord_t));
    return;
  }
  W25Q64_ReadData(LOG_SECTOR_ADDR + index * sizeof(LogRecord_t),
                  (uint8_t *)record, sizeof(LogRecord_t));
}

void Logger_Clear(void) {
  W25Q64_SectorErase(LOG_SECTOR_ADDR);
  log_count = 0;
  write_addr = LOG_SECTOR_ADDR;
}

void Logger_DumpToBT(void) {
  char line[64];
  LogRecord_t record;

  if (log_count == 0) {
    DebugPrint("LOG: empty\r\n");
    return;
  }

  snprintf(line, sizeof(line), "LOG: %d records\r\n", log_count);
  DebugPrint(line);

  for (uint16_t i = 0; i < log_count; i++) {
    Logger_ReadRecord(i, &record);
    snprintf(line, sizeof(line), "[%03d] %d.%dC %d.%d%% L:%d P:%d %dmin\r\n",
             i,
             record.temp / 10, record.temp % 10,
             record.humi / 10, record.humi % 10,
             record.light,
             record.pitch,
             record.timestamp);
    DebugPrint(line);
  }
}
