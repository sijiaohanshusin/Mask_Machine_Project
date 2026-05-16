#include "bsp_display_ltdc.h"

#include "bsp_lcd.h"
#include "bsp_log.h"
#include "bsp_sdram.h"
#include "bsp_touch.h"
#include "lvgl.h"
#include "stm32h7xx_hal.h"

#define BSP_DISPLAY_TOUCH_POLL_MS (20u)

static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t s_disp_drv;
static lv_indev_drv_t s_indev_drv;
static lv_disp_t *s_disp;
static lv_indev_t *s_indev;
static uint8_t s_ready;
static uint8_t s_touch_ready;
static uint32_t s_touch_retry_ms;
static uint32_t s_process_count;
static bsp_touch_state_t s_touch_cache;
static uint8_t s_touch_cache_valid;
static uint32_t s_touch_poll_ms;

static void Bsp_DisplayLtdc_Flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void Bsp_DisplayLtdc_ReadTouch(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static void Bsp_DisplayLtdc_TryInitTouch(void);

mm_status_t Bsp_DisplayLtdc_Init(void)
{
    mm_status_t ret;

    if (s_ready != 0u)
    {
        return MM_OK;
    }

    (void)Bsp_Log_Printf("[display] backlight probe on PD12...\r\n");
    Bsp_Lcd_ProbeBacklightOn();

    (void)Bsp_Log_Printf("[display] SDRAM init...\r\n");
    ret = Bsp_Sdram_Init();
    if (ret != MM_OK)
    {
        (void)Bsp_Log_Printf("[display] SDRAM init failed: %s\r\n", Mm_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] SDRAM self-test...\r\n");
    ret = Bsp_Sdram_SelfTest();
    if (ret != MM_OK)
    {
        (void)Bsp_Log_Printf("[display] SDRAM self-test failed: %s\r\n", Mm_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] LTDC panel init...\r\n");
    ret = Bsp_Lcd_Init();
    if (ret != MM_OK)
    {
        (void)Bsp_Log_Printf("[display] LTDC panel init failed: %s\r\n", Mm_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] LVGL register display...\r\n");
    lv_init();
    lv_disp_draw_buf_init(&s_draw_buf,
                          (void *)BSP_LCD_LVGL_BUF1_ADDR,
                          (void *)BSP_LCD_LVGL_BUF2_ADDR,
                          BSP_LCD_DRAW_BUF_PIXELS);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = BSP_LCD_WIDTH;
    s_disp_drv.ver_res = BSP_LCD_HEIGHT;
    s_disp_drv.flush_cb = Bsp_DisplayLtdc_Flush;
    s_disp_drv.draw_buf = &s_draw_buf;
    s_disp = lv_disp_drv_register(&s_disp_drv);

    Bsp_DisplayLtdc_TryInitTouch();
    if (s_touch_ready == 0u)
    {
        (void)Bsp_Log_Printf("[display] touch not detected; display continues and will retry\r\n");
    }

    s_ready = (s_disp != NULL) ? 1u : 0u;
    return (s_ready != 0u) ? MM_OK : MM_ERR_HW;
}

void Bsp_DisplayLtdc_Tick(uint32_t elapsed_ms)
{
    if (s_ready != 0u)
    {
        lv_tick_inc(elapsed_ms);
        if (s_touch_ready == 0u)
        {
            s_touch_retry_ms += elapsed_ms;
        }
        else
        {
            s_touch_poll_ms += elapsed_ms;
        }
    }
}

void Bsp_DisplayLtdc_Process(void)
{
    uint32_t start_ms;
    uint32_t elapsed_ms;
    uint32_t next_ms;

    if (s_ready != 0u)
    {
        if ((s_touch_ready == 0u) && (s_touch_retry_ms >= 1000u))
        {
            s_touch_retry_ms = 0u;
            Bsp_DisplayLtdc_TryInitTouch();
        }

        if ((s_touch_ready != 0u) && (s_touch_poll_ms >= BSP_DISPLAY_TOUCH_POLL_MS))
        {
            s_touch_poll_ms = 0u;
            s_touch_cache_valid = (Bsp_Touch_Read(&s_touch_cache) == MM_OK) ? 1u : 0u;
        }

        if (s_process_count < 3u)
        {
            (void)Bsp_Log_Printf("[display] lv_timer_handler enter %lu\r\n",
                                 (unsigned long)s_process_count);
        }

        start_ms = HAL_GetTick();
        next_ms = lv_timer_handler();
        elapsed_ms = HAL_GetTick() - start_ms;

        if ((s_process_count < 3u) || (elapsed_ms > 100u))
        {
            (void)Bsp_Log_Printf("[display] lv_timer_handler exit %lu elapsed=%lu next=%lu\r\n",
                                 (unsigned long)s_process_count,
                                 (unsigned long)elapsed_ms,
                                 (unsigned long)next_ms);
        }
        s_process_count++;
    }
}

uint8_t Bsp_DisplayLtdc_IsReady(void)
{
    return s_ready;
}

uint8_t Bsp_DisplayLtdc_IsTouchReady(void)
{
    return s_touch_ready;
}

static void Bsp_DisplayLtdc_Flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;
    uint16_t width;
    uint16_t height;

    if ((x2 < 0) || (y2 < 0) || (x1 >= (int32_t)BSP_LCD_WIDTH) || (y1 >= (int32_t)BSP_LCD_HEIGHT))
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    if (x1 < 0)
    {
        x1 = 0;
    }
    if (y1 < 0)
    {
        y1 = 0;
    }
    if (x2 >= (int32_t)BSP_LCD_WIDTH)
    {
        x2 = (int32_t)BSP_LCD_WIDTH - 1;
    }
    if (y2 >= (int32_t)BSP_LCD_HEIGHT)
    {
        y2 = (int32_t)BSP_LCD_HEIGHT - 1;
    }

    width = (uint16_t)(x2 - x1 + 1);
    height = (uint16_t)(y2 - y1 + 1);
    Bsp_Lcd_CopyRgb565Rect((uint16_t)x1,
                           (uint16_t)y1,
                           width,
                           height,
                           (const uint16_t *)color_p,
                           width);
    lv_disp_flush_ready(disp_drv);
}

static void Bsp_DisplayLtdc_ReadTouch(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_point_t last_point;
    bsp_touch_state_t touch;

    (void)indev_drv;

    if (s_touch_cache_valid != 0u)
    {
        touch = s_touch_cache;
    }
    else if (Bsp_Touch_Read(&touch) != MM_OK)
    {
        data->point = last_point;
        data->state = LV_INDEV_STATE_REL;
        data->continue_reading = false;
        return;
    }

    if (touch.pressed != 0u)
    {
        last_point.x = (lv_coord_t)touch.x;
        last_point.y = (lv_coord_t)touch.y;
        data->point = last_point;
        data->state = LV_INDEV_STATE_PR;
        data->continue_reading = false;
        return;
    }

    data->point = last_point;
    data->state = LV_INDEV_STATE_REL;
    data->continue_reading = false;
}

static void Bsp_DisplayLtdc_TryInitTouch(void)
{
    if (s_touch_ready != 0u)
    {
        return;
    }

    if (Bsp_Touch_Init() != MM_OK)
    {
        return;
    }

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = Bsp_DisplayLtdc_ReadTouch;
    s_indev = lv_indev_drv_register(&s_indev_drv);
    s_touch_ready = (s_indev != NULL) ? 1u : 0u;

    if (s_touch_ready != 0u)
    {
        (void)Bsp_Log_Printf("[display] touch ready\r\n");
    }
}
