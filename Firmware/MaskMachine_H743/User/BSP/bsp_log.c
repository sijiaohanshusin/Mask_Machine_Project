#include "bsp_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"

#ifndef BSP_LOG_BUFFER_SIZE
#define BSP_LOG_BUFFER_SIZE 192u
#endif

mm_status_t Bsp_Log_Init(void)
{
    return MM_OK;
}

int Bsp_Log_Printf(const char *fmt, ...)
{
    char buffer[BSP_LOG_BUFFER_SIZE];
    va_list args;
    int len;

    if (fmt == NULL)
    {
        return 0;
    }

    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len <= 0)
    {
        return len;
    }

    if ((uint32_t)len >= sizeof(buffer))
    {
        len = (int)sizeof(buffer) - 1;
    }

    (void)HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)len, 100u);
    return len;
}

int fputc(int ch, FILE *stream)
{
    uint8_t c = (uint8_t)ch;
    (void)stream;
    (void)HAL_UART_Transmit(&huart1, &c, 1u, 100u);
    return ch;
}
