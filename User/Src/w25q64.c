#include "w25q64.h"
#include "main.h"
#include "spi.h"
#include "stm32f1xx_hal_gpio.h"

/* W25Q64 指令集 */
#define W25Q64_CMD_READ_DATA      0x03
#define W25Q64_CMD_PAGE_PROGRAM   0x02
#define W25Q64_CMD_WRITE_ENABLE   0x06
#define W25Q64_CMD_READ_STATUS1   0x05
#define W25Q64_CMD_SECTOR_ERASE   0x20
#define W25Q64_CMD_BLOCK_ERASE    0xD8
#define W25Q64_CMD_READ_JEDEC_ID  0x9F

/* CS 控制 */
#define W25Q64_CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET)
#define W25Q64_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;

/* 等待写完成 */
static void W25Q64_WaitReady(void) {
  uint8_t status;
  W25Q64_CS_LOW();
  uint8_t cmd = W25Q64_CMD_READ_STATUS1;
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
  do {
    HAL_SPI_Receive(&hspi1, &status, 1, 100);
  } while (status & 0x01);  /* WIP=1 表示正在写 */
  W25Q64_CS_HIGH();
}

/* 写使能 */
static void W25Q64_WriteEnable(void) {
  W25Q64_CS_LOW();
  uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
  W25Q64_CS_HIGH();
}

void W25Q64_Init(void) {
  /* PA15 设为推挽输出（CS） */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  W25Q64_CS_HIGH();
}

void W25Q64_ReadID(uint8_t *manufacturer, uint8_t *device) {
  W25Q64_CS_LOW();
  uint8_t cmd = W25Q64_CMD_READ_JEDEC_ID;
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
  uint8_t dummy = 0;
  HAL_SPI_Receive(&hspi1, &dummy, 1, 100);  /* 跳过 0x00 */
  HAL_SPI_Receive(&hspi1, manufacturer, 1, 100);
  HAL_SPI_Receive(&hspi1, device, 1, 100);
  W25Q64_CS_HIGH();
}

void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len) {
  W25Q64_CS_LOW();
  uint8_t cmd[4] = {
    W25Q64_CMD_READ_DATA,
    (addr >> 16) & 0xFF,
    (addr >> 8) & 0xFF,
    addr & 0xFF
  };
  HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
  HAL_SPI_Receive(&hspi1, buf, len, 1000);
  W25Q64_CS_HIGH();
}

void W25Q64_PageWrite(uint32_t addr, const uint8_t *buf, uint16_t len) {
  if (len > W25Q64_PAGE_SIZE) len = W25Q64_PAGE_SIZE;

  W25Q64_WriteEnable();

  W25Q64_CS_LOW();
  uint8_t cmd[4] = {
    W25Q64_CMD_PAGE_PROGRAM,
    (addr >> 16) & 0xFF,
    (addr >> 8) & 0xFF,
    addr & 0xFF
  };
  HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, len, 1000);
  W25Q64_CS_HIGH();

  W25Q64_WaitReady();
}

void W25Q64_SectorErase(uint32_t addr) {
  W25Q64_WriteEnable();

  W25Q64_CS_LOW();
  uint8_t cmd[4] = {
    W25Q64_CMD_SECTOR_ERASE,
    (addr >> 16) & 0xFF,
    (addr >> 8) & 0xFF,
    addr & 0xFF
  };
  HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
  W25Q64_CS_HIGH();

  W25Q64_WaitReady();
}

void W25Q64_BlockErase(uint32_t addr) {
  W25Q64_WriteEnable();

  W25Q64_CS_LOW();
  uint8_t cmd[4] = {
    W25Q64_CMD_BLOCK_ERASE,
    (addr >> 16) & 0xFF,
    (addr >> 8) & 0xFF,
    addr & 0xFF
  };
  HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
  W25Q64_CS_HIGH();

  W25Q64_WaitReady();
}
