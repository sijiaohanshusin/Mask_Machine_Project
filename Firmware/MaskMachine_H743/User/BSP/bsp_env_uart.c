#include "bsp_env_uart.h"

#include <stddef.h>

#include "usart.h"

static uint8_t s_env_uart_ready;

static app_status_t EnvUart_MapHalStatus(HAL_StatusTypeDef status);

app_status_t Bsp_EnvUart_Init(void)
{
    if ((s_env_uart_ready != 0u) || (huart2.Instance == USART2))
    {
        s_env_uart_ready = 1u;
        return APP_OK;
    }

    MX_USART2_UART_Init();
    s_env_uart_ready = 1u;
    return APP_OK;
}

app_status_t Bsp_EnvUart_Transmit(const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    if ((data == NULL) || (length == 0u))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EnvUart_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    return EnvUart_MapHalStatus(HAL_UART_Transmit(&huart2, (uint8_t *)data, length, timeout_ms));
}

app_status_t Bsp_EnvUart_Receive(uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    if ((data == NULL) || (length == 0u))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EnvUart_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    return EnvUart_MapHalStatus(HAL_UART_Receive(&huart2, data, length, timeout_ms));
}

static app_status_t EnvUart_MapHalStatus(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK:
            return APP_OK;
        case HAL_TIMEOUT:
            return APP_ERR_TIMEOUT;
        case HAL_BUSY:
            return APP_ERR_BUSY;
        case HAL_ERROR:
        default:
            return APP_ERR_HW;
    }
}
