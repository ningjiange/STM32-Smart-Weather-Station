#ifndef __W25Q64_H
#define __W25Q64_H

#include <stdint.h>

/* W25Q64 容量 */
#define W25Q64_PAGE_SIZE     256
#define W25Q64_SECTOR_SIZE   4096
#define W25Q64_BLOCK_SIZE    65536
#define W25Q64_TOTAL_SIZE    (8 * 1024 * 1024)  /* 8MB */

/* 初始化 */
void W25Q64_Init(void);

/* 读取厂商ID和设备ID */
void W25Q64_ReadID(uint8_t *manufacturer, uint8_t *device);

/* 读写操作 */
void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len);
void W25Q64_PageWrite(uint32_t addr, const uint8_t *buf, uint16_t len);
void W25Q64_SectorErase(uint32_t addr);
void W25Q64_BlockErase(uint32_t addr);

#endif
