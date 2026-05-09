#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t Bsp_Time_NowMs(void);
void Bsp_Time_DelayMs(uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIME_H */
