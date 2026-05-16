#include "bsp_led.h"
#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} bsp_led_hw_t;

static const bsp_led_hw_t s_leds[BSP_LED_COUNT] = {
    { LED0_GPIO_Port, LED0_Pin },
    { LED1_GPIO_Port, LED1_Pin },
};

app_status_t Bsp_Led_Init(void)
{
    (void)Bsp_Led_Set(BSP_LED_0, 0u);
    (void)Bsp_Led_Set(BSP_LED_1, 0u);
    return APP_OK;
}

app_status_t Bsp_Led_Set(bsp_led_id_t led, uint8_t on)
{
    if ((uint32_t)led >= (uint32_t)BSP_LED_COUNT)
    {
        return APP_ERR_INVALID_ARG;
    }

    /* Board LEDs are active low in the vendor examples. */
    HAL_GPIO_WritePin(s_leds[led].port,
                      s_leds[led].pin,
                      (on != 0u) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    return APP_OK;
}

app_status_t Bsp_Led_Toggle(bsp_led_id_t led)
{
    if ((uint32_t)led >= (uint32_t)BSP_LED_COUNT)
    {
        return APP_ERR_INVALID_ARG;
    }

    HAL_GPIO_TogglePin(s_leds[led].port, s_leds[led].pin);
    return APP_OK;
}
