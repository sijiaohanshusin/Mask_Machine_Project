#ifndef BSP_SDRAM_H
#define BSP_SDRAM_H

#include <stdint.h>

#include "app_status.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_SDRAM_BASE_ADDR   (0xC0000000UL)
#define BSP_SDRAM_SIZE_BYTES  (0x02000000UL)

extern SDRAM_HandleTypeDef g_sdram_handle;

app_status_t Bsp_Sdram_Init(void);
app_status_t Bsp_Sdram_SelfTest(void);
uint8_t Bsp_Sdram_IsReady(void);
SDRAM_HandleTypeDef *Bsp_Sdram_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SDRAM_H */
