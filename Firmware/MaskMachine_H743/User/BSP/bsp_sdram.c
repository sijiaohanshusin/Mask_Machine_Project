#include "bsp_sdram.h"

#define SDRAM_REFRESH_COUNT                 (917U)
#define SDRAM_TIMEOUT_MS                    (1000U)
#define SDRAM_MODEREG_BURST_LENGTH_1        ((uint16_t)0x0000U)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000U)
#define SDRAM_MODEREG_CAS_LATENCY_2         ((uint16_t)0x0020U)
#define SDRAM_MODEREG_OPERATING_MODE_STD    ((uint16_t)0x0000U)
#define SDRAM_MODEREG_WRITEBURST_SINGLE     ((uint16_t)0x0200U)

SDRAM_HandleTypeDef g_sdram_handle;

static uint8_t s_ready;

static app_status_t Bsp_Sdram_SendCommand(uint32_t mode, uint32_t refresh_number, uint32_t mode_register);

app_status_t Bsp_Sdram_Init(void)
{
    FMC_SDRAM_TimingTypeDef timing = {0};
    app_status_t ret;

    if (s_ready != 0u)
    {
        return APP_OK;
    }

    g_sdram_handle.Instance = FMC_SDRAM_DEVICE;
    g_sdram_handle.Init.SDBank = FMC_SDRAM_BANK1;
    g_sdram_handle.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    g_sdram_handle.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
    g_sdram_handle.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    g_sdram_handle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    g_sdram_handle.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    g_sdram_handle.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    g_sdram_handle.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    g_sdram_handle.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    g_sdram_handle.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;

    timing.LoadToActiveDelay = 2U;
    timing.ExitSelfRefreshDelay = 9U;
    timing.SelfRefreshTime = 6U;
    timing.RowCycleDelay = 8U;
    timing.WriteRecoveryTime = 2U;
    timing.RPDelay = 3U;
    timing.RCDDelay = 3U;

    if (HAL_SDRAM_Init(&g_sdram_handle, &timing) != HAL_OK)
    {
        return APP_ERR_HW;
    }

    ret = Bsp_Sdram_SendCommand(FMC_SDRAM_CMD_CLK_ENABLE, 1U, 0U);
    if (ret != APP_OK)
    {
        return ret;
    }

    HAL_Delay(1U);

    ret = Bsp_Sdram_SendCommand(FMC_SDRAM_CMD_PALL, 1U, 0U);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Bsp_Sdram_SendCommand(FMC_SDRAM_CMD_AUTOREFRESH_MODE, 8U, 0U);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Bsp_Sdram_SendCommand(FMC_SDRAM_CMD_LOAD_MODE,
                                1U,
                                SDRAM_MODEREG_BURST_LENGTH_1 |
                                SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                                SDRAM_MODEREG_CAS_LATENCY_2 |
                                SDRAM_MODEREG_OPERATING_MODE_STD |
                                SDRAM_MODEREG_WRITEBURST_SINGLE);
    if (ret != APP_OK)
    {
        return ret;
    }

    if (HAL_SDRAM_ProgramRefreshRate(&g_sdram_handle, SDRAM_REFRESH_COUNT) != HAL_OK)
    {
        return APP_ERR_HW;
    }

    s_ready = 1u;
    return APP_OK;
}

app_status_t Bsp_Sdram_SelfTest(void)
{
    static const uint32_t patterns[] = {
        0x00000000UL,
        0xFFFFFFFFUL,
        0xA5A55A5AUL,
        0x5A5AA5A5UL,
        0x12345678UL
    };
    volatile uint32_t *mem = (volatile uint32_t *)BSP_SDRAM_BASE_ADDR;
    uint32_t i;
    uint32_t p;
    uint32_t original[16];

    if (s_ready == 0u)
    {
        return APP_ERR_NOT_READY;
    }

    for (i = 0U; i < 16U; i++)
    {
        original[i] = mem[i];
    }

    for (p = 0U; p < (uint32_t)(sizeof(patterns) / sizeof(patterns[0])); p++)
    {
        for (i = 0U; i < 1024U; i++)
        {
            mem[i] = patterns[p] ^ i;
        }

        for (i = 0U; i < 1024U; i++)
        {
            if (mem[i] != (patterns[p] ^ i))
            {
                for (i = 0U; i < 16U; i++)
                {
                    mem[i] = original[i];
                }
                return APP_ERR_HW;
            }
        }
    }

    for (i = 0U; i < 16U; i++)
    {
        mem[i] = original[i];
    }

    return APP_OK;
}

uint8_t Bsp_Sdram_IsReady(void)
{
    return s_ready;
}

SDRAM_HandleTypeDef *Bsp_Sdram_GetHandle(void)
{
    return &g_sdram_handle;
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef gpio = {0};

    if (hsdram->Instance != FMC_SDRAM_DEVICE)
    {
        return;
    }

    __HAL_RCC_FMC_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF12_FMC;

    gpio.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOC, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
               GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 |
               GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
               GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
               GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 |
               GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |
               GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &gpio);

    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOH, &gpio);

    __HAL_RCC_CSI_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_EnableCompensationCell();
}

static app_status_t Bsp_Sdram_SendCommand(uint32_t mode, uint32_t refresh_number, uint32_t mode_register)
{
    FMC_SDRAM_CommandTypeDef command = {0};

    command.CommandMode = mode;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.AutoRefreshNumber = refresh_number;
    command.ModeRegisterDefinition = mode_register;

    return (HAL_SDRAM_SendCommand(&g_sdram_handle, &command, SDRAM_TIMEOUT_MS) == HAL_OK) ? APP_OK : APP_ERR_HW;
}
