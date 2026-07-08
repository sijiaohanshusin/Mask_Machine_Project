#include "bsp_env_uart.h"

#include <stddef.h>

#include "usart.h"

#define BSP_ENV_UART_RX_RING_SIZE 256u

static uint8_t s_env_uart_ready;
static uint8_t s_rx_byte;
static volatile uint8_t s_rx_ring[BSP_ENV_UART_RX_RING_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static volatile uint32_t s_rx_bytes;
static volatile uint32_t s_rx_overflow;
static volatile uint32_t s_rx_errors;
static volatile uint32_t s_last_error_code;
static volatile uint8_t s_last_byte;

static app_status_t EnvUart_MapHalStatus(HAL_StatusTypeDef status);
static app_status_t EnvUart_StartReceiveIt(void);
static void EnvUart_ResetRing(void);
static uint16_t EnvUart_GetAvailable(void);
static void EnvUart_PushByteFromIsr(uint8_t byte);

app_status_t Bsp_EnvUart_Init(void)
{
    if (huart2.Instance != USART2)
    {
        MX_USART2_UART_Init();
    }

    if (s_env_uart_ready == 0u)
    {
        EnvUart_ResetRing();
        HAL_NVIC_SetPriority(USART2_IRQn, 6u, 0u);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        s_env_uart_ready = 1u;
    }

    return EnvUart_StartReceiveIt();
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
    uint32_t start_ms;
    uint16_t offset = 0u;
    app_status_t ret;

    if ((data == NULL) || (length == 0u))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EnvUart_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    start_ms = HAL_GetTick();
    while (offset < length)
    {
        ret = Bsp_EnvUart_ReadByte(&data[offset]);
        if (ret == APP_OK)
        {
            offset++;
            continue;
        }

        if (ret != APP_ERR_TIMEOUT)
        {
            return ret;
        }

        if ((timeout_ms == 0u) || ((HAL_GetTick() - start_ms) >= timeout_ms))
        {
            return APP_ERR_TIMEOUT;
        }
    }

    return APP_OK;
}

app_status_t Bsp_EnvUart_ReadByte(uint8_t *data)
{
    uint16_t tail;

    if (data == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EnvUart_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    tail = s_rx_tail;
    if (tail == s_rx_head)
    {
        return APP_ERR_TIMEOUT;
    }

    *data = s_rx_ring[tail];
    tail++;
    if (tail >= BSP_ENV_UART_RX_RING_SIZE)
    {
        tail = 0u;
    }
    s_rx_tail = tail;
    return APP_OK;
}

app_status_t Bsp_EnvUart_GetDiag(bsp_env_uart_diag_t *diag)
{
    if (diag == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    diag->rx_bytes = s_rx_bytes;
    diag->rx_overflow = s_rx_overflow;
    diag->rx_errors = s_rx_errors;
    diag->last_error_code = s_last_error_code;
    diag->rx_available = EnvUart_GetAvailable();
    diag->last_byte = s_last_byte;
    return APP_OK;
}

void Bsp_EnvUart_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void Bsp_EnvUart_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->Instance != USART2))
    {
        return;
    }

    EnvUart_PushByteFromIsr(s_rx_byte);
    (void)EnvUart_StartReceiveIt();
}

void Bsp_EnvUart_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->Instance != USART2))
    {
        return;
    }

    s_rx_errors++;
    s_last_error_code = HAL_UART_GetError(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_OREFLAG(huart);
    (void)EnvUart_StartReceiveIt();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    Bsp_EnvUart_RxCpltCallback(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    Bsp_EnvUart_ErrorCallback(huart);
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

static app_status_t EnvUart_StartReceiveIt(void)
{
    HAL_StatusTypeDef status;

    if (huart2.Instance != USART2)
    {
        return APP_ERR_NOT_READY;
    }

    status = HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1u);
    if (status == HAL_BUSY)
    {
        return APP_OK;
    }

    return EnvUart_MapHalStatus(status);
}

static void EnvUart_ResetRing(void)
{
    uint32_t primask;

    primask = __get_PRIMASK();
    __disable_irq();
    s_rx_head = 0u;
    s_rx_tail = 0u;
    s_rx_bytes = 0u;
    s_rx_overflow = 0u;
    s_rx_errors = 0u;
    s_last_error_code = 0u;
    s_last_byte = 0u;
    if (primask == 0u)
    {
        __enable_irq();
    }
}

static uint16_t EnvUart_GetAvailable(void)
{
    uint16_t head;
    uint16_t tail;

    head = s_rx_head;
    tail = s_rx_tail;
    if (head >= tail)
    {
        return (uint16_t)(head - tail);
    }

    return (uint16_t)((BSP_ENV_UART_RX_RING_SIZE - tail) + head);
}

static void EnvUart_PushByteFromIsr(uint8_t byte)
{
    uint16_t head;
    uint16_t next;

    head = s_rx_head;
    next = (uint16_t)(head + 1u);
    if (next >= BSP_ENV_UART_RX_RING_SIZE)
    {
        next = 0u;
    }

    if (next == s_rx_tail)
    {
        s_rx_overflow++;
    }
    else
    {
        s_rx_ring[head] = byte;
        s_rx_head = next;
    }

    s_rx_bytes++;
    s_last_byte = byte;
}
