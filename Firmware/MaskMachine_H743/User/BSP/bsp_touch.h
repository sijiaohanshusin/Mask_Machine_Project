#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include <stdint.h>

#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t ready;
    uint8_t pressed;
    uint8_t count;
    uint16_t x;
    uint16_t y;
    uint16_t raw_x;
    uint16_t raw_y;
    uint8_t status;
    char product_id[5];
} bsp_touch_state_t;

app_status_t Bsp_Touch_Init(void);
app_status_t Bsp_Touch_Read(bsp_touch_state_t *state);
uint8_t Bsp_Touch_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TOUCH_H */
