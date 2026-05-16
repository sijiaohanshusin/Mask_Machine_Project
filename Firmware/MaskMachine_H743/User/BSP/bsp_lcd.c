#include "bsp_lcd.h"

#include <string.h>

#define BSP_LCD_HSW            (20U)
#define BSP_LCD_HBP            (140U)
#define BSP_LCD_HFP            (160U)
#define BSP_LCD_VSW            (3U)
#define BSP_LCD_VBP            (20U)
#define BSP_LCD_VFP            (12U)
#define BSP_LCD_BLACK_RGB565   (0x0000U)

LTDC_HandleTypeDef g_ltdc_handle;
DMA2D_HandleTypeDef g_dma2d_handle;

static uint8_t s_ready;
static volatile uint32_t s_ltdc_error_count;
static volatile uint32_t s_dma2d_error_count;

static mm_status_t Bsp_Lcd_ConfigPixelClock(void);
static mm_status_t Bsp_Lcd_InitDma2d(void);
static mm_status_t Bsp_Lcd_InitLtdc(void);
static void Bsp_Lcd_InitControlPins(void);
static void Bsp_Lcd_ResetPanel(void);

mm_status_t Bsp_Lcd_Init(void)
{
    mm_status_t ret;

    if (s_ready != 0u)
    {
        return MM_OK;
    }

    Bsp_Lcd_ProbeBacklightOn();

    ret = Bsp_Lcd_ConfigPixelClock();
    if (ret != MM_OK)
    {
        return ret;
    }

    Bsp_Lcd_FillRgb565(BSP_LCD_BLACK_RGB565);

    ret = Bsp_Lcd_InitDma2d();
    if (ret != MM_OK)
    {
        return ret;
    }

    ret = Bsp_Lcd_InitLtdc();
    if (ret != MM_OK)
    {
        return ret;
    }

    Bsp_Lcd_SetBacklight(1u);
    s_ready = 1u;
    return MM_OK;
}

void Bsp_Lcd_SetBacklight(uint8_t on)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, (on != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Bsp_Lcd_ProbeBacklightOn(void)
{
    Bsp_Lcd_InitControlPins();
    Bsp_Lcd_SetBacklight(0u);
    Bsp_Lcd_ResetPanel();
    Bsp_Lcd_SetBacklight(1u);
}

uint8_t Bsp_Lcd_IsReady(void)
{
    return s_ready;
}

void Bsp_Lcd_FillRgb565(uint16_t color)
{
    volatile uint16_t *fb = (volatile uint16_t *)BSP_LCD_FRAMEBUFFER_ADDR;
    uint32_t i;

    for (i = 0U; i < (BSP_LCD_WIDTH * BSP_LCD_HEIGHT); i++)
    {
        fb[i] = color;
    }
}

void Bsp_Lcd_CopyRgb565Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *src, uint16_t src_stride)
{
    uint16_t row;
    uint16_t col;
    uint16_t *dst;
    uint16_t *dst_row;
    const uint16_t *src_row;

    if ((src == NULL) || (x >= BSP_LCD_WIDTH) || (y >= BSP_LCD_HEIGHT) || (w == 0u) || (h == 0u))
    {
        return;
    }

    if ((uint32_t)x + w > BSP_LCD_WIDTH)
    {
        w = (uint16_t)(BSP_LCD_WIDTH - x);
    }

    if ((uint32_t)y + h > BSP_LCD_HEIGHT)
    {
        h = (uint16_t)(BSP_LCD_HEIGHT - y);
    }

    dst = (uint16_t *)(BSP_LCD_FRAMEBUFFER_ADDR + (((uint32_t)y * BSP_LCD_WIDTH + x) * BSP_LCD_PIXEL_SIZE_BYTES));
    for (row = 0u; row < h; row++)
    {
        dst_row = &dst[(uint32_t)row * BSP_LCD_WIDTH];
        src_row = &src[(uint32_t)row * src_stride];

        for (col = 0u; col < w; col++)
        {
            dst_row[col] = src_row[col];
        }
    }
}

LTDC_HandleTypeDef *Bsp_Lcd_GetLtdcHandle(void)
{
    return &g_ltdc_handle;
}

DMA2D_HandleTypeDef *Bsp_Lcd_GetDma2dHandle(void)
{
    return &g_dma2d_handle;
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
    GPIO_InitTypeDef gpio = {0};

    if (hltdc->Instance != LTDC)
    {
        return;
    }

    __HAL_RCC_LTDC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio.Alternate = GPIO_AF13_LTDC;
    gpio.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Alternate = GPIO_AF14_LTDC;
    gpio.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOF, &gpio);

    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOG, &gpio);

    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
               GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |
               GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &gpio);

    HAL_NVIC_SetPriority(LTDC_IRQn, 6U, 0U);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_NVIC_SetPriority(LTDC_ER_IRQn, 6U, 0U);
    HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);

    __HAL_RCC_CSI_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_EnableCompensationCell();
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d)
{
    if (hdma2d->Instance != DMA2D)
    {
        return;
    }

    __HAL_RCC_DMA2D_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2D_IRQn, 6U, 0U);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
}

void HAL_LTDC_ErrorCallback(LTDC_HandleTypeDef *hltdc)
{
    (void)hltdc;
    s_ltdc_error_count++;
}

void HAL_DMA2D_ErrorCallback(DMA2D_HandleTypeDef *hdma2d)
{
    (void)hdma2d;
    s_dma2d_error_count++;
}

static mm_status_t Bsp_Lcd_ConfigPixelClock(void)
{
    RCC_PeriphCLKInitTypeDef periph_clk = {0};

    periph_clk.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    periph_clk.PLL3.PLL3M = 25U;
    periph_clk.PLL3.PLL3N = 300U;
    periph_clk.PLL3.PLL3P = 2U;
    periph_clk.PLL3.PLL3Q = 2U;
    periph_clk.PLL3.PLL3R = 6U;
    periph_clk.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    periph_clk.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    periph_clk.PLL3.PLL3FRACN = 0U;

    return (HAL_RCCEx_PeriphCLKConfig(&periph_clk) == HAL_OK) ? MM_OK : MM_ERR_HW;
}

static mm_status_t Bsp_Lcd_InitDma2d(void)
{
    g_dma2d_handle.Instance = DMA2D;
    g_dma2d_handle.Init.Mode = DMA2D_M2M;
    g_dma2d_handle.Init.ColorMode = DMA2D_OUTPUT_RGB565;
    g_dma2d_handle.Init.OutputOffset = 0U;
    g_dma2d_handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
    g_dma2d_handle.Init.RedBlueSwap = DMA2D_RB_REGULAR;
    g_dma2d_handle.Init.LineOffsetMode = DMA2D_LOM_PIXELS;

    return (HAL_DMA2D_Init(&g_dma2d_handle) == HAL_OK) ? MM_OK : MM_ERR_HW;
}

static mm_status_t Bsp_Lcd_InitLtdc(void)
{
    LTDC_LayerCfgTypeDef layer = {0};

    g_ltdc_handle.Instance = LTDC;
    g_ltdc_handle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    g_ltdc_handle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    g_ltdc_handle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    g_ltdc_handle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    g_ltdc_handle.Init.HorizontalSync = BSP_LCD_HSW - 1U;
    g_ltdc_handle.Init.VerticalSync = BSP_LCD_VSW - 1U;
    g_ltdc_handle.Init.AccumulatedHBP = BSP_LCD_HSW + BSP_LCD_HBP - 1U;
    g_ltdc_handle.Init.AccumulatedVBP = BSP_LCD_VSW + BSP_LCD_VBP - 1U;
    g_ltdc_handle.Init.AccumulatedActiveW = BSP_LCD_HSW + BSP_LCD_HBP + BSP_LCD_WIDTH - 1U;
    g_ltdc_handle.Init.AccumulatedActiveH = BSP_LCD_VSW + BSP_LCD_VBP + BSP_LCD_HEIGHT - 1U;
    g_ltdc_handle.Init.TotalWidth = BSP_LCD_HSW + BSP_LCD_HBP + BSP_LCD_WIDTH + BSP_LCD_HFP - 1U;
    g_ltdc_handle.Init.TotalHeigh = BSP_LCD_VSW + BSP_LCD_VBP + BSP_LCD_HEIGHT + BSP_LCD_VFP - 1U;
    g_ltdc_handle.Init.Backcolor.Red = 0U;
    g_ltdc_handle.Init.Backcolor.Green = 0U;
    g_ltdc_handle.Init.Backcolor.Blue = 0U;

    if (HAL_LTDC_Init(&g_ltdc_handle) != HAL_OK)
    {
        return MM_ERR_HW;
    }

    layer.WindowX0 = 0U;
    layer.WindowX1 = BSP_LCD_WIDTH;
    layer.WindowY0 = 0U;
    layer.WindowY1 = BSP_LCD_HEIGHT;
    layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layer.Alpha = 255U;
    layer.Alpha0 = 0U;
    layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layer.FBStartAdress = BSP_LCD_FRAMEBUFFER_ADDR;
    layer.ImageWidth = BSP_LCD_WIDTH;
    layer.ImageHeight = BSP_LCD_HEIGHT;
    layer.Backcolor.Red = 0U;
    layer.Backcolor.Green = 0U;
    layer.Backcolor.Blue = 0U;

    if (HAL_LTDC_ConfigLayer(&g_ltdc_handle, &layer, 0U) != HAL_OK)
    {
        return MM_ERR_HW;
    }

    __HAL_LTDC_ENABLE_IT(&g_ltdc_handle, LTDC_IT_TE | LTDC_IT_FU);
    return MM_OK;
}

static void Bsp_Lcd_InitControlPins(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &gpio);
}

static void Bsp_Lcd_ResetPanel(void)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_Delay(20U);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_Delay(120U);
}
