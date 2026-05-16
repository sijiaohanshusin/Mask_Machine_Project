#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BSP_LED_0 = 0,
    BSP_LED_1,
    BSP_LED_COUNT
} bsp_led_id_t;

app_status_t Bsp_Led_Init(void);
app_status_t Bsp_Led_Set(bsp_led_id_t led, uint8_t on);
app_status_t Bsp_Led_Toggle(bsp_led_id_t led);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LED_H */
