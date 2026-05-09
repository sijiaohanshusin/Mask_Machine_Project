#include "bsp_display_stub.h"

#include "lvgl.h"

#define BSP_DISPLAY_STUB_BUF_LINES 10u
#define BSP_DISPLAY_STUB_BUF_PIXELS (BSP_DISPLAY_STUB_HOR_RES * BSP_DISPLAY_STUB_BUF_LINES)

static lv_color_t s_draw_buf_1[BSP_DISPLAY_STUB_BUF_PIXELS];
static lv_color_t s_draw_buf_2[BSP_DISPLAY_STUB_BUF_PIXELS];
static lv_disp_draw_buf_t s_draw_buf;
static uint8_t s_ready = 0u;

static void s_flush_cb(lv_disp_drv_t *disp_drv,
                       const lv_area_t *area,
                       lv_color_t *color_p)
{
    (void)area;
    (void)color_p;
    lv_disp_flush_ready(disp_drv);
}

app_status_t Bsp_DisplayStub_Init(void)
{
    static lv_disp_drv_t disp_drv;

    if (s_ready != 0u)
    {
        return APP_OK;
    }

    lv_init();
    lv_disp_draw_buf_init(&s_draw_buf,
                          s_draw_buf_1,
                          s_draw_buf_2,
                          BSP_DISPLAY_STUB_BUF_PIXELS);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = (lv_coord_t)BSP_DISPLAY_STUB_HOR_RES;
    disp_drv.ver_res = (lv_coord_t)BSP_DISPLAY_STUB_VER_RES;
    disp_drv.flush_cb = s_flush_cb;
    disp_drv.draw_buf = &s_draw_buf;
    (void)lv_disp_drv_register(&disp_drv);

    s_ready = 1u;
    return APP_OK;
}

void Bsp_DisplayStub_Tick(uint32_t elapsed_ms)
{
    if (s_ready != 0u)
    {
        lv_tick_inc(elapsed_ms);
    }
}

void Bsp_DisplayStub_Process(void)
{
    if (s_ready != 0u)
    {
        (void)lv_timer_handler();
    }
}

uint8_t Bsp_DisplayStub_IsReady(void)
{
    return s_ready;
}
