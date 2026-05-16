#ifndef BSP_LCD_H
#define BSP_LCD_H

#include <stdint.h>

#include "app_status.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_LCD_WIDTH                   (1024U)
#define BSP_LCD_HEIGHT                  (600U)
#define BSP_LCD_PIXEL_SIZE_BYTES        (2U)
#define BSP_LCD_FRAMEBUFFER_ADDR        (0xC0000000UL)
#define BSP_LCD_FRAMEBUFFER_SIZE_BYTES  (BSP_LCD_WIDTH * BSP_LCD_HEIGHT * BSP_LCD_PIXEL_SIZE_BYTES)
#define BSP_LCD_DRAW_BUF_LINES          (40U)
#define BSP_LCD_DRAW_BUF_PIXELS         (BSP_LCD_WIDTH * BSP_LCD_DRAW_BUF_LINES)
#define BSP_LCD_LVGL_BUF1_ADDR          (0xC0200000UL)
#define BSP_LCD_LVGL_BUF2_ADDR          (BSP_LCD_LVGL_BUF1_ADDR + (BSP_LCD_DRAW_BUF_PIXELS * BSP_LCD_PIXEL_SIZE_BYTES))

extern LTDC_HandleTypeDef g_ltdc_handle;
extern DMA2D_HandleTypeDef g_dma2d_handle;

app_status_t Bsp_Lcd_Init(void);
void Bsp_Lcd_ProbeBacklightOn(void);
void Bsp_Lcd_SetBacklight(uint8_t on);
uint8_t Bsp_Lcd_IsReady(void);
void Bsp_Lcd_FillRgb565(uint16_t color);
void Bsp_Lcd_CopyRgb565Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *src, uint16_t src_stride);
LTDC_HandleTypeDef *Bsp_Lcd_GetLtdcHandle(void);
DMA2D_HandleTypeDef *Bsp_Lcd_GetDma2dHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LCD_H */
