#ifndef __DELAY_H
#define __DELAY_H
#include<oled_font.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t NonBlocking_Delay(uint32_t *last_time, uint32_t delay_ms);

#endif
