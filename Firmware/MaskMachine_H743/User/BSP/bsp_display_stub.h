#ifndef BSP_DISPLAY_STUB_H
#define BSP_DISPLAY_STUB_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_DISPLAY_STUB_HOR_RES 320u
#define BSP_DISPLAY_STUB_VER_RES 240u

app_status_t Bsp_DisplayStub_Init(void);
void Bsp_DisplayStub_Tick(uint32_t elapsed_ms);
void Bsp_DisplayStub_Process(void);
uint8_t Bsp_DisplayStub_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_STUB_H */
