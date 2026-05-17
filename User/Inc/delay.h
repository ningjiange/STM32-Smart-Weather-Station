#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

void delay_init(void);    /* DWT 初始化，main 中调用一次 */
void delay_us(uint32_t us); /* 微秒级延时 */

#endif
