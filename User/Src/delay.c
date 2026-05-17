#include "delay.h"
#include "stm32f1xx.h"

void delay_init(void) {
    /* 使能 DWT 调试模块 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    /* 清零周期计数器 */
    DWT->CYCCNT = 0;
    /* 使能周期计数器 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);  /* 72MHz → 72 ticks/us */
    while ((DWT->CYCCNT - start) < ticks);
}
