#include "bsp_display_ltdc.h"

#include "bsp_lcd.h"
#include "bsp_log.h"
#include "bsp_sdram.h"
#include "bsp_touch.h"
#include "lvgl.h"
#include "stm32h7xx_hal.h"

#define BSP_DISPLAY_TOUCH_POLL_MS (20u)

static lv_display_t *s_disp;
static lv_indev_t *s_indev;
static uint8_t s_ready;
static uint8_t s_touch_ready;
static uint32_t s_touch_retry_ms;
static uint32_t s_process_count;
static bsp_touch_state_t s_touch_cache;
static uint8_t s_touch_cache_valid;
static uint32_t s_touch_poll_ms;

static void Bsp_DisplayLtdc_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void Bsp_DisplayLtdc_ReadTouch(lv_indev_t *indev, lv_indev_data_t *data);
static void Bsp_DisplayLtdc_TryInitTouch(void);

app_status_t Bsp_DisplayLtdc_Init(void)
{
    app_status_t ret;

    if (s_ready != 0u)
    {
        return APP_OK;
    }

    (void)Bsp_Log_Printf("[display] backlight probe on PD12...\r\n");
    Bsp_Lcd_ProbeBacklightOn();

    (void)Bsp_Log_Printf("[display] SDRAM init...\r\n");
    ret = Bsp_Sdram_Init();
    if (ret != APP_OK)
    {
        (void)Bsp_Log_Printf("[display] SDRAM init failed: %s\r\n", App_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] SDRAM self-test...\r\n");
    ret = Bsp_Sdram_SelfTest();
    if (ret != APP_OK)
    {
        (void)Bsp_Log_Printf("[display] SDRAM self-test failed: %s\r\n", App_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] LTDC panel init...\r\n");
    ret = Bsp_Lcd_Init();
    if (ret != APP_OK)
    {
        (void)Bsp_Log_Printf("[display] LTDC panel init failed: %s\r\n", App_Status_ToString(ret));
        return ret;
    }

    (void)Bsp_Log_Printf("[display] LVGL register display...\r\n");
    lv_init();
    s_disp = lv_display_create(BSP_LCD_WIDTH, BSP_LCD_HEIGHT);
    if (s_disp == NULL)
    {
        return APP_ERR_HW;
    }

    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(s_disp, Bsp_DisplayLtdc_Flush);
    lv_display_set_buffers(s_disp,
                           (void *)BSP_LCD_LVGL_BUF1_ADDR,
                           (void *)BSP_LCD_LVGL_BUF2_ADDR,
                           (uint32_t)(BSP_LCD_DRAW_BUF_PIXELS * sizeof(lv_color_t)),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    Bsp_DisplayLtdc_TryInitTouch();
    if (s_touch_ready == 0u)
    {
        (void)Bsp_Log_Printf("[display] touch not detected; display continues and will retry\r\n");
    }

    s_ready = (s_disp != NULL) ? 1u : 0u;
    return (s_ready != 0u) ? APP_OK : APP_ERR_HW;
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
            s_touch_cache_valid = (Bsp_Touch_Read(&s_touch_cache) == APP_OK) ? 1u : 0u;
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

static void Bsp_DisplayLtdc_Flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;
    uint16_t width;
    uint16_t height;

    if ((x2 < 0) || (y2 < 0) || (x1 >= (int32_t)BSP_LCD_WIDTH) || (y1 >= (int32_t)BSP_LCD_HEIGHT))
    {
        lv_display_flush_ready(disp);
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
                           (const uint16_t *)px_map,
                           width);
    lv_display_flush_ready(disp);
}

static void Bsp_DisplayLtdc_ReadTouch(lv_indev_t *indev, lv_indev_data_t *data)
{
    static lv_point_t last_point;
    bsp_touch_state_t touch;

    (void)indev;

    if (s_touch_cache_valid != 0u)
    {
        touch = s_touch_cache;
    }
    else if (Bsp_Touch_Read(&touch) != APP_OK)
    {
        data->point = last_point;
        data->state = LV_INDEV_STATE_RELEASED;
        data->continue_reading = false;
        return;
    }

    if (touch.pressed != 0u)
    {
        last_point.x = (lv_coord_t)touch.x;
        last_point.y = (lv_coord_t)touch.y;
        data->point = last_point;
        data->state = LV_INDEV_STATE_PRESSED;
        data->continue_reading = false;
        return;
    }

    data->point = last_point;
    data->state = LV_INDEV_STATE_RELEASED;
    data->continue_reading = false;
}

static void Bsp_DisplayLtdc_TryInitTouch(void)
{
    if (s_touch_ready != 0u)
    {
        return;
    }

    if (Bsp_Touch_Init() != APP_OK)
    {
        return;
    }

    s_indev = lv_indev_create();
    if (s_indev != NULL)
    {
        lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_display(s_indev, s_disp);
        lv_indev_set_read_cb(s_indev, Bsp_DisplayLtdc_ReadTouch);
    }
    s_touch_ready = (s_indev != NULL) ? 1u : 0u;

    if (s_touch_ready != 0u)
    {
        (void)Bsp_Log_Printf("[display] touch ready\r\n");
    }
}
