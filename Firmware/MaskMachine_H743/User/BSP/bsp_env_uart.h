#ifndef BSP_ENV_UART_H
#define BSP_ENV_UART_H

#include <stdint.h>
#include "app_status.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_ENV_UART_BAUDRATE 9600u

typedef struct
{
    uint32_t rx_bytes;
    uint32_t rx_overflow;
    uint32_t rx_errors;
    uint32_t last_error_code;
    uint16_t rx_available;
    uint8_t last_byte;
} bsp_env_uart_diag_t;

app_status_t Bsp_EnvUart_Init(void);
app_status_t Bsp_EnvUart_Transmit(const uint8_t *data, uint16_t length, uint32_t timeout_ms);
app_status_t Bsp_EnvUart_Receive(uint8_t *data, uint16_t length, uint32_t timeout_ms);
app_status_t Bsp_EnvUart_ReadByte(uint8_t *data);
app_status_t Bsp_EnvUart_GetDiag(bsp_env_uart_diag_t *diag);
void Bsp_EnvUart_IRQHandler(void);
void Bsp_EnvUart_RxCpltCallback(UART_HandleTypeDef *huart);
void Bsp_EnvUart_ErrorCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ENV_UART_H */
