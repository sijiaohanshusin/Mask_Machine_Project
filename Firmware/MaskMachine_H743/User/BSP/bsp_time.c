#include "bsp_time.h"
#include "stm32h7xx_hal.h"

uint32_t Bsp_Time_NowMs(void)
{
    return HAL_GetTick();
}

void Bsp_Time_DelayMs(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}
