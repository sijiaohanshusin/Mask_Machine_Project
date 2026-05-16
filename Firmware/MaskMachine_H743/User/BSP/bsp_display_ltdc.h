#ifndef BSP_DISPLAY_LTDC_H
#define BSP_DISPLAY_LTDC_H

#include <stdint.h>

#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

mm_status_t Bsp_DisplayLtdc_Init(void);
void Bsp_DisplayLtdc_Tick(uint32_t elapsed_ms);
void Bsp_DisplayLtdc_Process(void);
uint8_t Bsp_DisplayLtdc_IsReady(void);
uint8_t Bsp_DisplayLtdc_IsTouchReady(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_LTDC_H */
