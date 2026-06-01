#ifndef BSP_ENV_UART_H
#define BSP_ENV_UART_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_ENV_UART_BAUDRATE 9600u

app_status_t Bsp_EnvUart_Init(void);
app_status_t Bsp_EnvUart_Transmit(const uint8_t *data, uint16_t length, uint32_t timeout_ms);
app_status_t Bsp_EnvUart_Receive(uint8_t *data, uint16_t length, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ENV_UART_H */
