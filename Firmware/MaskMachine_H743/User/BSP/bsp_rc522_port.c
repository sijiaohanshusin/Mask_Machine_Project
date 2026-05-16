#include "bsp_rc522_port.h"

#include <string.h>

#include "main.h"
#include "spi.h"
#include "stm32h7xx_hal.h"

#define RC522_SPI_TIMEOUT_MS    20u
#define RC522_RESET_LOW_MS      5u
#define RC522_RESET_HIGH_MS     10u

static uint8_t s_port_ready;

static app_status_t Rc522_MapHalStatus(HAL_StatusTypeDef hal_status);

app_status_t Bsp_Rc522Port_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    if (s_port_ready != 0u)
    {
        return APP_OK;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio.Pin = RC522_RST_Pin;
    HAL_GPIO_Init(RC522_RST_GPIO_Port, &gpio);

    gpio.Pin = RC522_CS_Pin;
    HAL_GPIO_Init(RC522_CS_GPIO_Port, &gpio);

    MX_SPI1_Init();

    s_port_ready = 1u;
    return APP_OK;
}

app_status_t Bsp_Rc522Port_Reset(void)
{
    if (s_port_ready == 0u)
    {
        return APP_ERR_NOT_READY;
    }

    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(RC522_RESET_LOW_MS);
    HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(RC522_RESET_HIGH_MS);
    return APP_OK;
}

app_status_t Bsp_Rc522Port_Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length)
{
    HAL_StatusTypeDef hal_ret;

    if ((s_port_ready == 0u) || (tx_data == NULL) || (length == 0u))
    {
        return (s_port_ready == 0u) ? APP_ERR_NOT_READY : APP_ERR_INVALID_ARG;
    }

    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);

    if (rx_data != NULL)
    {
        hal_ret = HAL_SPI_TransmitReceive(&hspi1,
                                          (uint8_t *)tx_data,
                                          rx_data,
                                          length,
                                          RC522_SPI_TIMEOUT_MS);
    }
    else
    {
        hal_ret = HAL_SPI_Transmit(&hspi1,
                                   (uint8_t *)tx_data,
                                   length,
                                   RC522_SPI_TIMEOUT_MS);
    }

    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);

    if ((hal_ret != HAL_OK) && (rx_data != NULL))
    {
        (void)memset(rx_data, 0, length);
    }

    return Rc522_MapHalStatus(hal_ret);
}

static app_status_t Rc522_MapHalStatus(HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
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
