#include "svc_ui.h"

#include <stdio.h>
#include <string.h>

#include "bsp_display_ltdc.h"
#include "bsp_lcd.h"
#include "lvgl.h"
#include "app_main.h"

LV_FONT_DECLARE(mm_font_zh_16);

#define UI_FRAME_X                 0
#define UI_FRAME_Y                 0
#define UI_FRAME_W                 480
#define UI_FRAME_H                 272
#define UI_BASE_W                  480
#define UI_BASE_H                  272
#define UI_HEADER_H                34
#define UI_DEMO_VERIFY_MS          1000u
#define UI_DEMO_DISPENSE_MS        1300u
#define UI_DEMO_SUCCESS_MS         2600u
#define UI_LOG_TEXT_MAX            96u

#define UI_TXT_SYS_READY "\347\263\273\347\273\237\345\260\261\347\273\252\357\274\214\347\255\211\345\276\205\350\247\246\345\217\221\343\200\202"
#define UI_TXT_LOG_REQ "\345\217\226\347\275\251\350\257\267\346\261\202\345\267\262\350\277\233\345\205\245\350\277\220\350\241\214\351\230\237\345\210\227\343\200\202"
#define UI_TXT_LOG_AUTH "\346\243\200\346\265\213\345\210\260\350\256\244\350\257\201\344\272\213\344\273\266\357\274\214RFID \346\216\245\345\217\243\345\267\262\351\242\204\347\225\231\343\200\202"
#define UI_TXT_FAULT_EVENT "\350\277\220\350\241\214\345\261\202\344\270\212\346\212\245\346\225\205\351\232\234\357\274\214\350\257\267\346\243\200\346\237\245\344\270\262\345\217\243\346\227\245\345\277\227\343\200\202"
#define UI_TXT_TITLE "\346\231\272\350\203\275\345\217\243\347\275\251\345\210\206\345\217\221\345\276\256\347\273\210\347\253\257"
#define UI_TXT_MQTT_UNSET "MQTT \346\234\252\346\216\245\345\205\245"
#define UI_TXT_MQTT_ONLINE "MQTT \345\234\250\347\272\277"
#define UI_TXT_MQTT_OFFLINE "MQTT \347\246\273\347\272\277"
#define UI_TXT_PROMPT_IDLE "\350\257\267\345\205\210\345\210\267\111\103\345\215\241"
#define UI_TXT_SUBPROMPT_IDLE "\345\210\267\345\215\241\351\200\232\350\277\207\345\220\216\357\274\214\347\202\271\345\207\273\345\261\217\345\271\225\345\217\226\347\275\251\343\200\202"
#define UI_TXT_CUR_STATE "\345\275\223\345\211\215\347\212\266\346\200\201"
#define UI_TXT_WAIT_TRIGGER "\347\255\211\345\276\205\350\247\246\345\217\221"
#define UI_TXT_QUOTA "\344\273\212\346\227\245\345\211\251\344\275\231\351\205\215\351\242\235"
#define UI_TXT_DETAIL_IDLE "\347\255\211\345\276\205\346\234\254\345\234\260\111\103\345\215\241\350\256\244\350\257\201\357\274\214\346\216\210\346\235\203\345\220\216\350\247\246\346\221\270\345\267\246\344\276\247\345\217\226\347\275\251\343\200\202"
#define UI_TXT_ENV_TITLE "\345\256\236\346\227\266\347\216\257\345\242\203\346\204\237\347\237\245"
#define UI_TXT_REFRESH "30s \345\210\267\346\226\260"
#define UI_TXT_UNKNOWN "\346\234\252\347\237\245"
#define UI_TXT_ENV_NOT_CONNECTED "\347\251\272\346\260\224\350\264\250\351\207\217\346\216\245\345\217\243\346\234\252\346\216\245\345\205\245"
#define UI_TXT_DEVICE_TITLE "\346\234\272\347\224\265\344\270\216\345\272\223\345\255\230\347\212\266\346\200\201"
#define UI_TXT_CUR_STOCK "\345\275\223\345\211\215\345\272\223\345\255\230"
#define UI_TXT_STOCK_WAIT "\345\272\223\345\255\230\346\216\245\345\217\243\347\255\211\345\276\205\345\277\253\347\205\247"
#define UI_TXT_CTRL_PLACE "H743 \346\216\247\345\210\266  --"
#define UI_TXT_MOTOR_PLACE "\347\224\265\346\234\272  --"
#define UI_TXT_ENV_PLACE "\347\216\257\345\242\203  --"
#define UI_TXT_VERIFY_REQ "\346\255\243\345\234\250\346\240\241\351\252\214\345\217\226\347\275\251\350\257\267\346\261\202..."
#define UI_TXT_SUBPROMPT_VERIFY "\350\201\224\347\275\221\343\200\201RFID\343\200\201\344\277\241\345\217\267\345\274\272\345\272\246\346\216\245\345\217\243\351\242\204\347\225\231\357\274\233\345\275\223\345\211\215\344\275\277\347\224\250\350\247\246\346\221\270\346\274\224\347\244\272"
#define UI_TXT_VERIFY_TITLE "\350\272\253\344\273\275/\351\205\215\351\242\235\346\240\241\351\252\214"
#define UI_TXT_VERIFY_DETAIL "\346\216\247\345\210\266\344\273\273\345\212\241\345\244\204\347\220\206\344\270\255\357\274\214\347\234\237\345\256\236\351\211\264\346\235\203\346\234\252\346\216\245\345\205\245\343\200\202"
#define UI_TXT_LOG_TOUCH "\350\247\246\346\221\270\350\247\246\345\217\221\346\274\224\347\244\272\345\217\226\347\275\251\346\265\201\347\250\213\343\200\202"
#define UI_TXT_QUEUE_NOT_READY "\350\277\220\350\241\214\351\230\237\345\210\227\346\234\252\345\260\261\347\273\252\343\200\202"
#define UI_TXT_VERIFY_OK "\346\240\241\351\252\214\351\200\232\350\277\207\357\274\214\346\250\241\346\213\237\345\207\272\347\275\251\344\270\255"
#define UI_TXT_DISPENSING "\347\224\265\346\234\272\345\217\221\346\224\276\344\270\255"
#define UI_TXT_DISPENSE_DETAIL "\347\234\237\345\256\236\347\224\265\346\234\272\346\234\252\346\216\245\345\205\245\357\274\214\345\275\223\345\211\215\344\275\277\347\224\250 stub \346\216\245\345\217\243\343\200\202"
#define UI_TXT_LOG_DISPENSING "\346\274\224\347\244\272\345\217\221\346\224\276\350\267\257\345\276\204\350\277\220\350\241\214\344\270\255\357\274\214\347\255\211\345\276\205\347\273\223\346\236\234\343\200\202"
#define UI_TXT_PROMPT_SUCCESS "\345\217\221\346\224\276\346\210\220\345\212\237\357\274\214\350\257\267\345\217\226\350\265\260\345\217\243\347\275\251"
#define UI_TXT_SUCCESS_TITLE "\345\217\221\346\224\276\346\210\220\345\212\237"
#define UI_TXT_SUCCESS_DETAIL "\345\272\223\345\255\230\345\267\262\351\200\222\345\207\217\357\274\214\347\255\211\345\276\205\344\270\213\344\270\200\346\254\241\350\247\246\345\217\221\343\200\202"
#define UI_TXT_LOG_SUCCESS "\345\215\225\346\254\241\345\217\221\346\224\276\345\256\214\346\210\220\357\274\214\345\272\223\345\255\230\345\277\253\347\205\247\345\267\262\346\233\264\346\226\260\343\200\202"
#define UI_TXT_REQ_FAILED "\350\257\267\346\261\202\346\234\252\345\256\214\346\210\220\357\274\214\350\257\267\346\243\200\346\237\245\347\212\266\346\200\201"
#define UI_TXT_FLOW_ERROR "\346\265\201\347\250\213\345\274\202\345\270\270"
#define UI_TXT_LOG_ERROR "\346\265\201\347\250\213\346\234\252\345\256\214\346\210\220\357\274\214\346\237\245\347\234\213\346\216\245\345\217\243\347\212\266\346\200\201\343\200\202"
#define UI_TXT_AIR_GOOD_BADGE "\344\274\230"
#define UI_TXT_AIR_OK_BADGE "\350\211\257"
#define UI_TXT_AIR_BAD_BADGE "\345\267\256"
#define UI_TXT_AIR_GOOD "\347\251\272\346\260\224\350\264\250\351\207\217\350\211\257\345\245\275"
#define UI_TXT_AIR_OK "\347\251\272\346\260\224\350\264\250\351\207\217\344\270\200\350\210\254"
#define UI_TXT_AIR_BAD "\347\251\272\346\260\224\350\264\250\351\207\217\345\274\202\345\270\270"
#define UI_TXT_STOCK_LOW "\345\272\223\345\255\230\345\201\217\344\275\216"
#define UI_TXT_STOCK_OK "\344\275\231\351\207\217\345\205\205\350\266\263"
#define UI_TXT_CTRL_OK "H743 \346\216\247\345\210\266  OK"
#define UI_TXT_MOTOR_DEMO "\347\224\265\346\234\272  DEMO"
#define UI_TXT_MOTOR_ERR "\347\224\265\346\234\272  ERR"
#define UI_TXT_ENV_OK "\347\216\257\345\242\203  OK"
#define UI_TXT_ENV_ERR "\347\216\257\345\242\203  ERR"
#define UI_TXT_AUTH_READY "\350\256\244\350\257\201\351\200\232\350\277\207\357\274\214\350\257\267\347\202\271\345\207\273\345\217\226\347\275\251"
#define UI_TXT_AUTH_READY_DETAIL "\111\103\345\215\241\345\267\262\346\216\210\346\235\203\357\274\214\061\065\347\247\222\345\206\205\347\202\271\345\207\273\345\261\217\345\271\225\345\256\214\346\210\220\345\217\226\347\275\251\343\200\202"
#define UI_TXT_LOG_AUTH_GRANTED "\111\103\345\215\241\350\256\244\350\257\201\351\200\232\350\277\207\357\274\214\347\255\211\345\276\205\350\247\246\346\221\270\347\241\256\350\256\244\343\200\202"
#define UI_TXT_LOG_AUTH_LEARNED "\347\251\272\345\272\223\351\246\226\345\215\241\345\267\262\347\231\273\350\256\260\345\271\266\346\216\210\346\235\203\343\200\202"
#define UI_TXT_AUTH_DENIED "\346\234\252\346\216\210\346\235\203\111\103\345\215\241"
#define UI_TXT_AUTH_DENIED_DETAIL "\346\234\254\345\234\260\345\215\241\345\272\223\346\234\252\346\211\276\345\210\260\350\257\245\345\215\241\357\274\214\350\257\267\350\201\224\347\263\273\347\256\241\347\220\206\345\221\230\347\231\273\350\256\260\343\200\202"
#define UI_TXT_LOG_AUTH_DENIED "\111\103\345\215\241\346\234\252\346\216\210\346\235\203\357\274\214\345\267\262\346\213\222\347\273\235\345\217\226\347\275\251\343\200\202"
#define UI_TXT_AUTH_REQUIRED "\350\257\267\345\205\210\345\210\267\111\103\345\215\241"
#define UI_TXT_AUTH_REQUIRED_DETAIL "\345\217\226\347\275\251\345\211\215\351\234\200\350\246\201\345\256\214\346\210\220\346\234\254\345\234\260\111\103\345\215\241\350\256\244\350\257\201\343\200\202"
#define UI_TXT_LOG_AUTH_REQUIRED "\346\234\252\346\243\200\346\265\213\345\210\260\346\234\211\346\225\210\350\256\244\350\257\201\357\274\214\344\274\232\350\257\235\346\234\252\345\273\272\347\253\213\343\200\202"
#define UI_TXT_AUTH_TIMEOUT "\350\256\244\350\257\201\345\267\262\350\266\205\346\227\266"
#define UI_TXT_AUTH_TIMEOUT_DETAIL "\350\257\267\351\207\215\346\226\260\345\210\267\345\215\241\345\220\216\345\234\250\061\065\347\247\222\345\206\205\347\202\271\345\207\273\345\217\226\347\275\251\343\200\202"
#define UI_TXT_LOG_AUTH_TIMEOUT "\111\103\345\215\241\350\256\244\350\257\201\344\274\232\350\257\235\345\267\262\350\266\205\346\227\266\343\200\202"
#define UI_TXT_AUTH_DB_ERROR "\345\215\241\345\272\223\344\270\215\345\217\257\347\224\250"
#define UI_TXT_AUTH_DB_ERROR_DETAIL "\346\234\254\345\234\260\105\105\120\122\117\115\345\215\241\345\272\223\350\257\273\345\217\226\345\244\261\350\264\245\357\274\214\350\256\244\350\257\201\346\232\202\344\270\215\345\217\257\347\224\250\343\200\202"
#define UI_TXT_LOG_AUTH_DB_ERROR "\346\234\254\345\234\260\345\215\241\345\272\223\345\274\202\345\270\270\357\274\214\350\256\244\350\257\201\344\270\215\345\217\257\347\224\250\343\200\202"

typedef enum
{
    UI_DEMO_IDLE = 0,
    UI_DEMO_VERIFY,
    UI_DEMO_DISPENSE,
    UI_DEMO_SUCCESS,
    UI_DEMO_ERROR
} ui_demo_state_t;

static svc_ui_snapshot_t s_snapshot;
static uint32_t s_last_success_count;
static uint32_t s_last_blocked_count;
static uint8_t s_ui_ready;
static ui_demo_state_t s_demo_state;
static uint32_t s_demo_elapsed_ms;
static char s_log_text[UI_LOG_TEXT_MAX] = UI_TXT_SYS_READY;

static lv_obj_t *s_header_mqtt_label;
static lv_obj_t *s_header_signal_label;
static lv_obj_t *s_clock_label;
static lv_obj_t *s_prompt_label;
static lv_obj_t *s_subprompt_label;
static lv_obj_t *s_scan_line;
static lv_obj_t *s_success_label;
static lv_obj_t *s_status_title_label;
static lv_obj_t *s_status_detail_label;
static lv_obj_t *s_quota_label;
static lv_obj_t *s_progress_bar;
static lv_obj_t *s_progress_percent_label;
static lv_obj_t *s_co2_value_label;
static lv_obj_t *s_hcho_value_label;
static lv_obj_t *s_tvoc_value_label;
static lv_obj_t *s_pm25_value_label;
static lv_obj_t *s_pm10_value_label;
static lv_obj_t *s_aqi_badge_label;
static lv_obj_t *s_temp_label;
static lv_obj_t *s_humidity_label;
static lv_obj_t *s_env_state_label;
static lv_obj_t *s_inventory_arc;
static lv_obj_t *s_inventory_value_label;
static lv_obj_t *s_inventory_note_label;
static lv_obj_t *s_controller_status_label;
static lv_obj_t *s_actuator_status_label;
static lv_obj_t *s_env_module_status_label;
static lv_obj_t *s_log_label;

static void Ui_Create(void);
static void Ui_CreateHeader(lv_obj_t *frame);
static void Ui_CreateLeftPanel(lv_obj_t *frame);
static void Ui_CreateEnvironmentPanel(lv_obj_t *frame);
static void Ui_CreateDevicePanel(lv_obj_t *frame);
static void Ui_CreateCameraMock(lv_obj_t *parent);
static lv_obj_t *Ui_CreatePanel(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h,
                                uint32_t bg, uint32_t border, int16_t radius, int16_t pad);
static lv_obj_t *Ui_CreateLabel(lv_obj_t *parent, const char *text, int16_t x, int16_t y,
                                const lv_font_t *font, uint32_t color);
static lv_obj_t *Ui_CreatePill(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h,
                               uint32_t bg, uint32_t border);
static lv_obj_t *Ui_CreateDot(lv_obj_t *parent, int16_t x, int16_t y, uint32_t color);
static int16_t Ui_ScaleX(int16_t value);
static int16_t Ui_ScaleY(int16_t value);
static uint16_t Ui_ScaleW(uint16_t value);
static uint16_t Ui_ScaleH(uint16_t value);
static const lv_font_t *Ui_FontForScreen(const lv_font_t *font);
static void Ui_SetObjPos(lv_obj_t *obj, int16_t x, int16_t y);
static void Ui_SetObjSize(lv_obj_t *obj, uint16_t w, uint16_t h);
static void Ui_SetLabelTextIfChanged(lv_obj_t *label, const char *text);
static void Ui_SetBarValueIfChanged(lv_obj_t *bar, int32_t value, lv_anim_enable_t anim);
static void Ui_PrepareWrappedLabel(lv_obj_t *label, int16_t width);
static void Ui_SetLog(const char *text);
static void Ui_RefreshLogLine(void);
static void Ui_StartTouchDemo(void);
static void Ui_LeftPanelEvent(lv_event_t *event);
static void Ui_UpdateDemo(uint32_t elapsed_ms);
static void Ui_ShowAuthReady(uint8_t learned);
static void Ui_ShowIdle(void);
static void Ui_ShowSuccess(void);
static void Ui_ShowError(const char *detail);
static void Ui_SetProgress(uint8_t value);
static void Ui_UpdateAuth(void);
static void Ui_UpdateEnvironment(void);
static void Ui_UpdateConnectivity(void);
static void Ui_UpdateInventory(void);
static void Ui_UpdateDeviceStatus(void);
static const char *Ui_StatusShort(app_status_t status);
static uint8_t Ui_Percent(uint16_t remaining, uint16_t total);
static void Ui_FormatClock(uint32_t uptime_ms, char *buffer, uint32_t buffer_size);

app_status_t Svc_Ui_Init(void)
{
    app_status_t ret;

    if (s_ui_ready != 0u)
    {
        return APP_OK;
    }

    ret = Bsp_DisplayLtdc_Init();
    if (ret != APP_OK)
    {
        return ret;
    }

    Ui_Create();
    s_ui_ready = 1u;
    return APP_OK;
}

app_status_t Svc_Ui_PostEvent(const app_event_t *event)
{
    if (event == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    if (s_ui_ready == 0u)
    {
        return APP_ERR_NOT_READY;
    }

    switch (event->type)
    {
        case APP_EVENT_DISPENSE_REQUEST:
            Ui_SetLog(UI_TXT_LOG_REQ);
            break;

        case APP_EVENT_AUTH_PRESENT:
            Ui_SetLog(UI_TXT_LOG_AUTH);
            break;

        case APP_EVENT_AUTH_GRANTED:
            Ui_ShowAuthReady((uint8_t)event->arg0);
            break;

        case APP_EVENT_AUTH_DENIED:
            Ui_ShowError(UI_TXT_AUTH_DENIED_DETAIL);
            Ui_SetLog(UI_TXT_LOG_AUTH_DENIED);
            break;

        case APP_EVENT_AUTH_REQUIRED:
            Ui_ShowError(UI_TXT_AUTH_REQUIRED_DETAIL);
            Ui_SetLog(UI_TXT_LOG_AUTH_REQUIRED);
            break;

        case APP_EVENT_AUTH_TIMEOUT:
            Ui_ShowError(UI_TXT_AUTH_TIMEOUT_DETAIL);
            Ui_SetLog(UI_TXT_LOG_AUTH_TIMEOUT);
            break;

        case APP_EVENT_AUTH_DB_ERROR:
            Ui_ShowError(UI_TXT_AUTH_DB_ERROR_DETAIL);
            Ui_SetLog(UI_TXT_LOG_AUTH_DB_ERROR);
            break;

        case APP_EVENT_ENV_UPDATED:
            Ui_UpdateEnvironment();
            break;

        case APP_EVENT_FAULT:
            Ui_ShowError(UI_TXT_FAULT_EVENT);
            break;

        case APP_EVENT_NONE:
        default:
            break;
    }

    return APP_OK;
}

app_status_t Svc_Ui_SetSnapshot(const svc_ui_snapshot_t *snapshot)
{
    char text[96];

    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    s_snapshot = *snapshot;

    if (s_ui_ready == 0u)
    {
        return APP_ERR_NOT_READY;
    }

    Ui_FormatClock(s_snapshot.diag.uptime_ms, text, sizeof(text));
    Ui_SetLabelTextIfChanged(s_clock_label, text);

    Ui_UpdateAuth();
    Ui_UpdateConnectivity();
    Ui_UpdateInventory();
    Ui_UpdateEnvironment();
    Ui_UpdateDeviceStatus();
    Ui_RefreshLogLine();

    if (s_snapshot.dispenser.success_count != s_last_success_count)
    {
        s_last_success_count = s_snapshot.dispenser.success_count;
        Ui_ShowSuccess();
    }
    else if (s_snapshot.dispenser.blocked_count != s_last_blocked_count)
    {
        s_last_blocked_count = s_snapshot.dispenser.blocked_count;
        Ui_ShowError(Ui_StatusShort(s_snapshot.dispenser.last_status));
    }

    return APP_OK;
}

void Svc_Ui_Process(uint32_t elapsed_ms)
{
    if (s_ui_ready != 0u)
    {
        Bsp_DisplayLtdc_Tick(elapsed_ms);
        Ui_UpdateDemo(elapsed_ms);
        Bsp_DisplayLtdc_Process();
    }
}

static void Ui_Create(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_t *frame;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x02050A), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    frame = Ui_CreatePanel(screen, UI_FRAME_X, UI_FRAME_Y, UI_FRAME_W, UI_FRAME_H,
                           0x172235, 0x172235, 10, 0);

    Ui_CreateHeader(frame);
    Ui_CreateLeftPanel(frame);
    Ui_CreateEnvironmentPanel(frame);
    Ui_CreateDevicePanel(frame);
    Ui_ShowIdle();
}

static void Ui_CreateHeader(lv_obj_t *frame)
{
    lv_obj_t *header;
    lv_obj_t *mark;
    lv_obj_t *chip;

    header = Ui_CreatePanel(frame, 0, 0, UI_FRAME_W, UI_HEADER_H, 0x111A2C, 0x26354A, 0, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);

    mark = Ui_CreatePill(header, 8, 7, 20, 20, 0x22D3EE, 0x22D3EE);
    (void)Ui_CreateLabel(mark, "M", 5, 3, &lv_font_montserrat_14, 0x06101D);

    (void)Ui_CreateLabel(header, UI_TXT_TITLE, 36, 8, &mm_font_zh_16, 0xF8FAFC);

    chip = Ui_CreatePill(header, 156, 7, 58, 20, 0x10384A, 0x0E7490);
    (void)Ui_CreateLabel(chip, "H743", 13, 4, &lv_font_montserrat_14, 0x22D3EE);

    (void)Ui_CreateDot(header, 226, 10, 0x4ADE80);
    s_header_mqtt_label = Ui_CreateLabel(header, UI_TXT_MQTT_UNSET, 242, 8, &mm_font_zh_16, 0xE2E8F0);

    (void)Ui_CreateDot(header, 340, 10, 0x22D3EE);
    s_header_signal_label = Ui_CreateLabel(header, "--", 356, 8, &lv_font_montserrat_14, 0xE2E8F0);

    s_clock_label = Ui_CreateLabel(header, "00:00", 414, 7, &lv_font_montserrat_14, 0xFFFFFF);
}

static void Ui_CreateLeftPanel(lv_obj_t *frame)
{
    lv_obj_t *left_panel;
    lv_obj_t *prompt;
    lv_obj_t *info;
    lv_obj_t *icon_bg;
    lv_obj_t *touch_layer;

    left_panel = Ui_CreatePanel(frame, 6, 42, 292, 188, 0x1A2738, 0x334155, 8, 0);
    lv_obj_add_flag(left_panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(left_panel, Ui_LeftPanelEvent, LV_EVENT_CLICKED, NULL);

    Ui_CreateCameraMock(left_panel);

    prompt = Ui_CreatePill(left_panel, 12, 10, 126, 28, 0x111A2C, 0x475569);
    s_prompt_label = Ui_CreateLabel(prompt, UI_TXT_PROMPT_IDLE, 18, 6, &mm_font_zh_16, 0x67E8F9);

    s_subprompt_label = Ui_CreateLabel(left_panel, UI_TXT_SUBPROMPT_IDLE,
                                       150, 14, &mm_font_zh_16, 0x94A3B8);
    Ui_PrepareWrappedLabel(s_subprompt_label, 124);

    s_scan_line = lv_obj_create(left_panel);
    Ui_SetObjPos(s_scan_line, 16, 68);
    Ui_SetObjSize(s_scan_line, 258, 2);
    lv_obj_set_style_bg_color(s_scan_line, lv_color_hex(0x22D3EE), 0);
    lv_obj_set_style_bg_opa(s_scan_line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_scan_line, 0, 0);
    lv_obj_add_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);

    s_success_label = Ui_CreateLabel(left_panel, "OK", 130, 80, &lv_font_montserrat_20, 0x22C55E);
    lv_obj_add_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);

    info = Ui_CreatePanel(left_panel, 14, 96, 264, 44, 0x111A2C, 0x26354A, 8, 0);
    lv_obj_set_style_bg_opa(info, LV_OPA_80, 0);

    icon_bg = Ui_CreatePill(info, 8, 8, 28, 28, 0x0E7490, 0x22D3EE);
    (void)Ui_CreateLabel(icon_bg, "ID", 6, 7, &lv_font_montserrat_14, 0x67E8F9);

    (void)Ui_CreateLabel(info, UI_TXT_CUR_STATE, 46, 5, &mm_font_zh_16, 0x94A3B8);
    s_status_title_label = Ui_CreateLabel(info, UI_TXT_WAIT_TRIGGER, 46, 24, &mm_font_zh_16, 0xFFFFFF);

    (void)Ui_CreateLabel(info, UI_TXT_QUOTA, 176, 5, &mm_font_zh_16, 0x94A3B8);
    s_quota_label = Ui_CreateLabel(info, "--/--", 190, 23, &lv_font_montserrat_20, 0x22D3EE);

    s_status_detail_label = Ui_CreateLabel(left_panel, UI_TXT_DETAIL_IDLE,
                                           16, 146, &mm_font_zh_16, 0xCBD5E1);
    Ui_PrepareWrappedLabel(s_status_detail_label, 258);

    s_progress_bar = lv_bar_create(left_panel);
    Ui_SetObjPos(s_progress_bar, 16, 170);
    Ui_SetObjSize(s_progress_bar, 198, 8);
    lv_bar_set_range(s_progress_bar, 0, 100);
    lv_bar_set_value(s_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_hex(0x334155), 0);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_hex(0x22D3EE), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_progress_bar, 6, 0);
    lv_obj_set_style_radius(s_progress_bar, 6, LV_PART_INDICATOR);

    s_progress_percent_label = Ui_CreateLabel(left_panel, "0%", 222, 164, &lv_font_montserrat_14, 0x22D3EE);

    touch_layer = lv_obj_create(left_panel);
    Ui_SetObjPos(touch_layer, 0, 0);
    Ui_SetObjSize(touch_layer, 292, 188);
    lv_obj_set_style_bg_opa(touch_layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(touch_layer, 0, 0);
    lv_obj_clear_flag(touch_layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(touch_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(touch_layer, Ui_LeftPanelEvent, LV_EVENT_CLICKED, NULL);
}

static void Ui_CreateEnvironmentPanel(lv_obj_t *frame)
{
    lv_obj_t *env;
    lv_obj_t *refresh;
    lv_obj_t *badge;
    lv_obj_t *tile;

    env = Ui_CreatePanel(frame, 306, 42, 168, 118, 0x153A42, 0x0F766E, 8, 0);
    lv_obj_set_style_bg_opa(env, LV_OPA_90, 0);

    (void)Ui_CreateDot(env, 10, 12, 0x4ADE80);
    (void)Ui_CreateLabel(env, UI_TXT_ENV_TITLE, 26, 9, &mm_font_zh_16, 0xD7DEE8);

    refresh = Ui_CreatePill(env, 104, 8, 54, 22, 0x1E293B, 0x475569);
    (void)Ui_CreateLabel(refresh, "U2", 18, 4, &lv_font_montserrat_14, 0xA7B4C4);

    tile = Ui_CreatePanel(env, 8, 36, 48, 28, 0x112733, 0x256D85, 5, 0);
    (void)Ui_CreateLabel(tile, "CO2", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_co2_value_label = Ui_CreateLabel(tile, "--", 6, 14, &lv_font_montserrat_14, 0xE2E8F0);

    tile = Ui_CreatePanel(env, 60, 36, 48, 28, 0x112733, 0x0F766E, 5, 0);
    (void)Ui_CreateLabel(tile, "PM25", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_pm25_value_label = Ui_CreateLabel(tile, "--", 6, 14, &lv_font_montserrat_14, 0x10B981);

    tile = Ui_CreatePanel(env, 112, 36, 48, 28, 0x112733, 0x256D85, 5, 0);
    (void)Ui_CreateLabel(tile, "PM10", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_pm10_value_label = Ui_CreateLabel(tile, "--", 6, 14, &lv_font_montserrat_14, 0xE2E8F0);

    tile = Ui_CreatePanel(env, 8, 68, 48, 28, 0x112733, 0x7C5A1A, 5, 0);
    (void)Ui_CreateLabel(tile, "HCHO", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_hcho_value_label = Ui_CreateLabel(tile, "--", 6, 14, &lv_font_montserrat_14, 0xE2E8F0);

    tile = Ui_CreatePanel(env, 60, 68, 48, 28, 0x112733, 0x6D5DA8, 5, 0);
    (void)Ui_CreateLabel(tile, "TVOC", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_tvoc_value_label = Ui_CreateLabel(tile, "--", 6, 14, &lv_font_montserrat_14, 0xE2E8F0);

    tile = Ui_CreatePanel(env, 112, 68, 48, 28, 0x112733, 0x475569, 5, 0);
    (void)Ui_CreateLabel(tile, "T/RH", 6, 3, &lv_font_montserrat_14, 0x94A3B8);
    s_temp_label = Ui_CreateLabel(tile, "--.-C", 6, 13, &lv_font_montserrat_14, 0xE2E8F0);

    badge = Ui_CreatePill(env, 8, 98, 38, 18, 0x134E4A, 0x0F766E);
    s_aqi_badge_label = Ui_CreateLabel(badge, UI_TXT_UNKNOWN, 5, 1, &mm_font_zh_16, 0xA7F3D0);
    s_env_state_label = Ui_CreateLabel(env, UI_TXT_ENV_NOT_CONNECTED, 52, 98, &mm_font_zh_16, 0x94A3B8);
    Ui_PrepareWrappedLabel(s_env_state_label, 66);
    s_humidity_label = Ui_CreateLabel(env, "--%", 124, 98, &lv_font_montserrat_14, 0xE2E8F0);
}

static void Ui_CreateDevicePanel(lv_obj_t *frame)
{
    lv_obj_t *dev;
    lv_obj_t *log_box;

    dev = Ui_CreatePanel(frame, 306, 166, 168, 64, 0x1B293A, 0x26354A, 8, 0);
    lv_obj_set_style_bg_opa(dev, LV_OPA_90, 0);

    (void)Ui_CreateDot(dev, 10, 10, 0x60A5FA);
    (void)Ui_CreateLabel(dev, UI_TXT_DEVICE_TITLE, 26, 7, &mm_font_zh_16, 0xD7DEE8);

    s_inventory_arc = lv_arc_create(dev);
    Ui_SetObjPos(s_inventory_arc, 10, 26);
    Ui_SetObjSize(s_inventory_arc, 34, 34);
    lv_arc_set_range(s_inventory_arc, 0, 100);
    lv_arc_set_value(s_inventory_arc, 0);
    lv_arc_set_bg_angles(s_inventory_arc, 0, 360);
    lv_arc_set_rotation(s_inventory_arc, 270);
    lv_obj_remove_style(s_inventory_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(s_inventory_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(s_inventory_arc, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_inventory_arc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_inventory_arc, lv_color_hex(0x3B82F6), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_inventory_arc, 4, LV_PART_INDICATOR);

    s_inventory_value_label = Ui_CreateLabel(dev, "--", 18, 36, &lv_font_montserrat_14, 0xFFFFFF);
    (void)Ui_CreateLabel(dev, UI_TXT_CUR_STOCK, 52, 25, &mm_font_zh_16, 0x94A3B8);
    s_inventory_note_label = Ui_CreateLabel(dev, UI_TXT_STOCK_WAIT, 52, 43, &mm_font_zh_16, 0x60A5FA);
    Ui_PrepareWrappedLabel(s_inventory_note_label, 58);

    s_controller_status_label = Ui_CreateLabel(dev, UI_TXT_CTRL_PLACE, 112, 22, &mm_font_zh_16, 0x94A3B8);
    s_actuator_status_label = Ui_CreateLabel(dev, UI_TXT_MOTOR_PLACE, 112, 38, &mm_font_zh_16, 0x94A3B8);
    s_env_module_status_label = Ui_CreateLabel(dev, UI_TXT_ENV_PLACE, 112, 52, &mm_font_zh_16, 0x94A3B8);

    log_box = Ui_CreatePanel(frame, 6, 236, 468, 28, 0x111827, 0x26354A, 4, 0);
    s_log_label = Ui_CreateLabel(log_box, "> [SYS] --", 8, 6, &mm_font_zh_16, 0x4ADE80);
    lv_label_set_long_mode(s_log_label, LV_LABEL_LONG_MODE_CLIP);
}

static void Ui_CreateCameraMock(lv_obj_t *parent)
{
    lv_obj_t *obj;

    obj = Ui_CreatePanel(parent, 0, 0, 292, 188, 0x243244, 0x243244, 8, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    obj = Ui_CreatePanel(parent, 0, 0, 110, 188, 0x60483B, 0x60483B, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);

    obj = Ui_CreatePanel(parent, 150, 0, 142, 188, 0x4B5563, 0x4B5563, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_40, 0);

    obj = Ui_CreatePanel(parent, 198, 14, 18, 28, 0x111827, 0x111827, 4, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
    obj = Ui_CreatePanel(parent, 236, 16, 18, 28, 0x111827, 0x111827, 4, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
    obj = Ui_CreatePanel(parent, 270, 24, 14, 32, 0x111827, 0x111827, 4, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);

    obj = Ui_CreatePill(parent, 118, 44, 54, 54, 0x111827, 0x111827);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
    obj = Ui_CreatePanel(parent, 108, 94, 72, 76, 0x111827, 0x111827, 10, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);

    obj = Ui_CreatePanel(parent, 0, 0, 292, 188, 0x0F172A, 0x0F172A, 8, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_60, 0);
}

static int16_t Ui_ScaleX(int16_t value)
{
    return (int16_t)(((int32_t)value * (int32_t)BSP_LCD_WIDTH + (UI_BASE_W / 2)) / UI_BASE_W);
}

static int16_t Ui_ScaleY(int16_t value)
{
    return (int16_t)(((int32_t)value * (int32_t)BSP_LCD_HEIGHT + (UI_BASE_H / 2)) / UI_BASE_H);
}

static uint16_t Ui_ScaleW(uint16_t value)
{
    uint32_t scaled = ((uint32_t)value * BSP_LCD_WIDTH + (UI_BASE_W / 2U)) / UI_BASE_W;
    return (uint16_t)((scaled == 0U) ? 1U : scaled);
}

static uint16_t Ui_ScaleH(uint16_t value)
{
    uint32_t scaled = ((uint32_t)value * BSP_LCD_HEIGHT + (UI_BASE_H / 2U)) / UI_BASE_H;
    return (uint16_t)((scaled == 0U) ? 1U : scaled);
}

static const lv_font_t *Ui_FontForScreen(const lv_font_t *font)
{
#if (BSP_LCD_WIDTH <= 480U)
    if (font == &lv_font_montserrat_28)
    {
        return &lv_font_montserrat_20;
    }
    if (font == &lv_font_montserrat_20)
    {
        return &lv_font_montserrat_14;
    }
#endif
    return font;
}

static void Ui_SetObjPos(lv_obj_t *obj, int16_t x, int16_t y)
{
    if (obj != NULL)
    {
        lv_obj_set_pos(obj, Ui_ScaleX(x), Ui_ScaleY(y));
    }
}

static void Ui_SetObjSize(lv_obj_t *obj, uint16_t w, uint16_t h)
{
    if (obj != NULL)
    {
        lv_obj_set_size(obj, Ui_ScaleW(w), Ui_ScaleH(h));
    }
}

static lv_obj_t *Ui_CreatePanel(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h,
                                uint32_t bg, uint32_t border, int16_t radius, int16_t pad)
{
    lv_obj_t *panel = lv_obj_create(parent);

    Ui_SetObjPos(panel, x, y);
    Ui_SetObjSize(panel, (uint16_t)w, (uint16_t)h);
    lv_obj_set_style_bg_color(panel, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(border), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, Ui_ScaleY(radius), 0);
    lv_obj_set_style_pad_all(panel, Ui_ScaleY(pad), 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}

static lv_obj_t *Ui_CreateLabel(lv_obj_t *parent, const char *text, int16_t x, int16_t y,
                                const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    Ui_SetObjPos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(label, Ui_FontForScreen(font), 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    return label;
}

static lv_obj_t *Ui_CreatePill(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h,
                               uint32_t bg, uint32_t border)
{
    return Ui_CreatePanel(parent, x, y, w, h, bg, border, h / 2, 0);
}

static lv_obj_t *Ui_CreateDot(lv_obj_t *parent, int16_t x, int16_t y, uint32_t color)
{
    return Ui_CreatePill(parent, x, y, 14, 14, color, color);
}

static void Ui_SetLabelTextIfChanged(lv_obj_t *label, const char *text)
{
    const char *current;

    if ((label == NULL) || (text == NULL))
    {
        return;
    }

    current = lv_label_get_text(label);
    if ((current == NULL) || (strcmp(current, text) != 0))
    {
        lv_label_set_text(label, text);
    }
}

static void Ui_SetBarValueIfChanged(lv_obj_t *bar, int32_t value, lv_anim_enable_t anim)
{
    if (bar == NULL)
    {
        return;
    }

    if (lv_bar_get_value(bar) != value)
    {
        lv_bar_set_value(bar, value, anim);
    }
}

static void Ui_PrepareWrappedLabel(lv_obj_t *label, int16_t width)
{
    if (label == NULL)
    {
        return;
    }

    lv_obj_set_width(label, Ui_ScaleW((uint16_t)width));
    lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_WRAP);
}

static void Ui_SetLog(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    (void)strncpy(s_log_text, text, sizeof(s_log_text) - 1u);
    s_log_text[sizeof(s_log_text) - 1u] = '\0';
    Ui_RefreshLogLine();
}

static void Ui_RefreshLogLine(void)
{
    char text[160];

    if (s_log_label == NULL)
    {
        return;
    }

    (void)snprintf(text,
                   sizeof(text),
                   "> [SYS] %s  HB:%lu",
                   s_log_text,
                   (unsigned long)s_snapshot.diag.heartbeat_count);
    Ui_SetLabelTextIfChanged(s_log_label, text);
}

static void Ui_StartTouchDemo(void)
{
    app_event_t app_event = {
        .type = APP_EVENT_DISPENSE_REQUEST,
        .arg0 = 0u,
        .arg1 = 0u
    };

    if ((s_demo_state == UI_DEMO_VERIFY) || (s_demo_state == UI_DEMO_DISPENSE))
    {
        return;
    }

    s_demo_state = UI_DEMO_VERIFY;
    s_demo_elapsed_ms = 0u;
    Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_VERIFY_REQ);
    Ui_SetLabelTextIfChanged(s_subprompt_label, UI_TXT_SUBPROMPT_VERIFY);
    Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_VERIFY_TITLE);
    Ui_SetLabelTextIfChanged(s_status_detail_label, UI_TXT_VERIFY_DETAIL);
    Ui_SetProgress(12u);
    lv_obj_clear_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);
    Ui_SetLog(UI_TXT_LOG_TOUCH);

    if (App_Main_PostEvent(&app_event, 0u) != APP_OK)
    {
        Ui_ShowError(UI_TXT_QUEUE_NOT_READY);
    }
}

static void Ui_LeftPanelEvent(lv_event_t *event)
{
    (void)event;
    Ui_StartTouchDemo();
}

static void Ui_UpdateDemo(uint32_t elapsed_ms)
{
    uint32_t pos;
    uint8_t progress;

    if (s_demo_state == UI_DEMO_IDLE)
    {
        return;
    }

    s_demo_elapsed_ms += elapsed_ms;

    if ((s_demo_state == UI_DEMO_VERIFY) || (s_demo_state == UI_DEMO_DISPENSE))
    {
        pos = 68u + ((s_demo_elapsed_ms % 1200u) * 60u) / 1200u;
        lv_obj_set_y(s_scan_line, (lv_coord_t)Ui_ScaleY((int16_t)pos));
    }

    if (s_demo_state == UI_DEMO_VERIFY)
    {
        progress = (uint8_t)(12u + ((s_demo_elapsed_ms * 38u) / UI_DEMO_VERIFY_MS));
        if (progress > 50u)
        {
            progress = 50u;
        }
        Ui_SetProgress(progress);

        if (s_demo_elapsed_ms >= UI_DEMO_VERIFY_MS)
        {
            s_demo_state = UI_DEMO_DISPENSE;
            s_demo_elapsed_ms = 0u;
            Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_VERIFY_OK);
            Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_DISPENSING);
            Ui_SetLabelTextIfChanged(s_status_detail_label, UI_TXT_DISPENSE_DETAIL);
            Ui_SetLog(UI_TXT_LOG_DISPENSING);
        }
    }
    else if (s_demo_state == UI_DEMO_DISPENSE)
    {
        progress = (uint8_t)(50u + ((s_demo_elapsed_ms * 45u) / UI_DEMO_DISPENSE_MS));
        if (progress > 95u)
        {
            progress = 95u;
        }
        Ui_SetProgress(progress);
    }
    else if ((s_demo_state == UI_DEMO_SUCCESS) && (s_demo_elapsed_ms >= UI_DEMO_SUCCESS_MS))
    {
        Ui_ShowIdle();
    }
    else if ((s_demo_state == UI_DEMO_ERROR) && (s_demo_elapsed_ms >= UI_DEMO_SUCCESS_MS))
    {
        Ui_ShowIdle();
    }
    else
    {
    }
}

static void Ui_ShowAuthReady(uint8_t learned)
{
    char text[160];
    const drv_rfid_card_t *card = &s_snapshot.auth.last_card;

    s_demo_state = UI_DEMO_IDLE;
    s_demo_elapsed_ms = 0u;
    Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_AUTH_READY);
    Ui_SetLabelTextIfChanged(s_subprompt_label, UI_TXT_SUBPROMPT_IDLE);
    Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_AUTH_READY);

    if ((s_snapshot.auth.last_uid_valid != 0u) && (card->uid_len >= 4u))
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%s UID **** %02X %02X %02X %02X",
                       UI_TXT_AUTH_READY_DETAIL,
                       (unsigned int)card->uid[card->uid_len - 4u],
                       (unsigned int)card->uid[card->uid_len - 3u],
                       (unsigned int)card->uid[card->uid_len - 2u],
                       (unsigned int)card->uid[card->uid_len - 1u]);
        Ui_SetLabelTextIfChanged(s_status_detail_label, text);
    }
    else
    {
        Ui_SetLabelTextIfChanged(s_status_detail_label, UI_TXT_AUTH_READY_DETAIL);
    }

    Ui_SetProgress(35u);
    lv_obj_add_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);
    Ui_SetLog((learned != 0u) ? UI_TXT_LOG_AUTH_LEARNED : UI_TXT_LOG_AUTH_GRANTED);
}

static void Ui_ShowIdle(void)
{
    s_demo_state = UI_DEMO_IDLE;
    s_demo_elapsed_ms = 0u;
    Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_PROMPT_IDLE);
    Ui_SetLabelTextIfChanged(s_subprompt_label, UI_TXT_SUBPROMPT_IDLE);
    Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_WAIT_TRIGGER);
    Ui_SetLabelTextIfChanged(s_status_detail_label, UI_TXT_DETAIL_IDLE);
    Ui_SetProgress(0u);
    lv_obj_add_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);
}

static void Ui_ShowSuccess(void)
{
    s_demo_state = UI_DEMO_SUCCESS;
    s_demo_elapsed_ms = 0u;
    Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_PROMPT_SUCCESS);
    Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_SUCCESS_TITLE);
    Ui_SetLabelTextIfChanged(s_status_detail_label, UI_TXT_SUCCESS_DETAIL);
    Ui_SetProgress(100u);
    lv_obj_add_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);
    Ui_SetLog(UI_TXT_LOG_SUCCESS);
}

static void Ui_ShowError(const char *detail)
{
    s_demo_state = UI_DEMO_ERROR;
    s_demo_elapsed_ms = 0u;
    Ui_SetLabelTextIfChanged(s_prompt_label, UI_TXT_REQ_FAILED);
    Ui_SetLabelTextIfChanged(s_status_title_label, UI_TXT_FLOW_ERROR);
    Ui_SetLabelTextIfChanged(s_status_detail_label, detail);
    Ui_SetProgress(0u);
    lv_obj_add_flag(s_scan_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_success_label, LV_OBJ_FLAG_HIDDEN);
    Ui_SetLog(UI_TXT_LOG_ERROR);
}

static void Ui_UpdateAuth(void)
{
    if ((s_demo_state == UI_DEMO_IDLE) && (s_snapshot.auth.session_active != 0u))
    {
        Ui_ShowAuthReady(s_snapshot.auth.last_learned);
    }
}

static void Ui_SetProgress(uint8_t value)
{
    char text[12];

    if (value > 100u)
    {
        value = 100u;
    }

    Ui_SetBarValueIfChanged(s_progress_bar, value, LV_ANIM_OFF);
    (void)snprintf(text, sizeof(text), "%u%%", (unsigned int)value);
    Ui_SetLabelTextIfChanged(s_progress_percent_label, text);
}

static void Ui_UpdateEnvironment(void)
{
    char text[32];
    uint8_t env_ready;
    uint16_t pm25;
    int16_t temp_abs;

    env_ready = ((s_snapshot.environment.last_status == APP_OK) &&
                 (s_snapshot.environment.sample.valid != 0u)) ? 1u : 0u;

    if (env_ready == 0u)
    {
        Ui_SetLabelTextIfChanged(s_co2_value_label, "--");
        Ui_SetLabelTextIfChanged(s_hcho_value_label, "--");
        Ui_SetLabelTextIfChanged(s_tvoc_value_label, "--");
        Ui_SetLabelTextIfChanged(s_pm25_value_label, "--");
        Ui_SetLabelTextIfChanged(s_pm10_value_label, "--");
        Ui_SetLabelTextIfChanged(s_aqi_badge_label, UI_TXT_UNKNOWN);
        Ui_SetLabelTextIfChanged(s_temp_label, "--.- C");
        Ui_SetLabelTextIfChanged(s_humidity_label, "--.- %");
        Ui_SetLabelTextIfChanged(s_env_state_label, UI_TXT_ENV_NOT_CONNECTED);
        return;
    }

    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.environment.sample.co2_ppm);
    Ui_SetLabelTextIfChanged(s_co2_value_label, text);

    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.environment.sample.ch2o_ugm3);
    Ui_SetLabelTextIfChanged(s_hcho_value_label, text);

    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.environment.sample.tvoc_ugm3);
    Ui_SetLabelTextIfChanged(s_tvoc_value_label, text);

    pm25 = s_snapshot.environment.sample.pm25_ugm3;
    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.environment.sample.pm25_ugm3);
    Ui_SetLabelTextIfChanged(s_pm25_value_label, text);

    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.environment.sample.pm10_ugm3);
    Ui_SetLabelTextIfChanged(s_pm10_value_label, text);

    if (pm25 <= 35u)
    {
        Ui_SetLabelTextIfChanged(s_aqi_badge_label, UI_TXT_AIR_GOOD_BADGE);
        Ui_SetLabelTextIfChanged(s_env_state_label, UI_TXT_AIR_GOOD);
    }
    else if (pm25 <= 75u)
    {
        Ui_SetLabelTextIfChanged(s_aqi_badge_label, UI_TXT_AIR_OK_BADGE);
        Ui_SetLabelTextIfChanged(s_env_state_label, UI_TXT_AIR_OK);
    }
    else
    {
        Ui_SetLabelTextIfChanged(s_aqi_badge_label, UI_TXT_AIR_BAD_BADGE);
        Ui_SetLabelTextIfChanged(s_env_state_label, UI_TXT_AIR_BAD);
    }

    temp_abs = s_snapshot.environment.sample.temperature_c_x10;
    if (temp_abs < 0)
    {
        temp_abs = (int16_t)-temp_abs;
    }

    (void)snprintf(text,
                   sizeof(text),
                   "%s%d.%d C",
                   (s_snapshot.environment.sample.temperature_c_x10 < 0) ? "-" : "",
                   (int)(temp_abs / 10),
                   (int)(temp_abs % 10));
    Ui_SetLabelTextIfChanged(s_temp_label, text);

    (void)snprintf(text,
                   sizeof(text),
                   "%u.%u %%",
                   (unsigned int)(s_snapshot.environment.sample.humidity_rh_x10 / 10u),
                   (unsigned int)(s_snapshot.environment.sample.humidity_rh_x10 % 10u));
    Ui_SetLabelTextIfChanged(s_humidity_label, text);
}

static void Ui_UpdateConnectivity(void)
{
    char text[32];

    if (s_snapshot.connectivity.mqtt_known == 0u)
    {
        Ui_SetLabelTextIfChanged(s_header_mqtt_label, UI_TXT_MQTT_UNSET);
    }
    else
    {
        Ui_SetLabelTextIfChanged(s_header_mqtt_label,
                                 s_snapshot.connectivity.mqtt_online != 0u ? UI_TXT_MQTT_ONLINE : UI_TXT_MQTT_OFFLINE);
    }

    if (s_snapshot.connectivity.signal_known == 0u)
    {
        Ui_SetLabelTextIfChanged(s_header_signal_label, "WiFi -- dBm");
    }
    else
    {
        (void)snprintf(text,
                       sizeof(text),
                       "WiFi %d dBm",
                       (int)s_snapshot.connectivity.signal_dbm);
        Ui_SetLabelTextIfChanged(s_header_signal_label, text);
    }
}

static void Ui_UpdateInventory(void)
{
    char text[48];
    uint8_t stock_percent;

    stock_percent = Ui_Percent(s_snapshot.inventory.remaining, s_snapshot.inventory.total);
    lv_arc_set_value(s_inventory_arc, stock_percent);

    if (s_snapshot.inventory.total == 0u)
    {
        Ui_SetLabelTextIfChanged(s_inventory_value_label, "--");
        Ui_SetLabelTextIfChanged(s_inventory_note_label, UI_TXT_STOCK_WAIT);
        Ui_SetLabelTextIfChanged(s_quota_label, "--/--");
        return;
    }

    (void)snprintf(text, sizeof(text), "%u", (unsigned int)s_snapshot.inventory.remaining);
    Ui_SetLabelTextIfChanged(s_inventory_value_label, text);

    (void)snprintf(text,
                   sizeof(text),
                   "%u/%u",
                   (unsigned int)s_snapshot.inventory.remaining,
                   (unsigned int)s_snapshot.inventory.total);
    Ui_SetLabelTextIfChanged(s_quota_label, text);

    if (s_snapshot.inventory.low != 0u)
    {
        Ui_SetLabelTextIfChanged(s_inventory_note_label, UI_TXT_STOCK_LOW);
    }
    else
    {
        Ui_SetLabelTextIfChanged(s_inventory_note_label, UI_TXT_STOCK_OK);
    }
}

static void Ui_UpdateDeviceStatus(void)
{
    char text[32];

    Ui_SetLabelTextIfChanged(s_controller_status_label, UI_TXT_CTRL_OK);
    Ui_SetLabelTextIfChanged(s_actuator_status_label,
                             (s_snapshot.dispenser.last_status == APP_OK) ? UI_TXT_MOTOR_DEMO : UI_TXT_MOTOR_ERR);
    if (s_snapshot.card_db.ready != 0u)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "CARD DB %u/%u",
                       (unsigned int)s_snapshot.card_db.count,
                       (unsigned int)s_snapshot.card_db.max_records);
        Ui_SetLabelTextIfChanged(s_env_module_status_label, text);
    }
    else
    {
        Ui_SetLabelTextIfChanged(s_env_module_status_label, "CARD DB ERR");
    }
}

static const char *Ui_StatusShort(app_status_t status)
{
    switch (status)
    {
        case APP_OK:
            return "OK";
        case APP_ERR_NOT_READY:
            return "NOT READY";
        case APP_ERR_NOT_IMPLEMENTED:
            return "NOT IMPLEMENTED";
        case APP_ERR_TIMEOUT:
            return "TIMEOUT";
        case APP_ERR_INVALID_ARG:
            return "INVALID ARG";
        case APP_ERR_BUSY:
            return "BUSY";
        case APP_ERR_HW:
        default:
            return "HW ERROR";
    }
}

static uint8_t Ui_Percent(uint16_t remaining, uint16_t total)
{
    if (total == 0u)
    {
        return 0u;
    }

    return (uint8_t)(((uint32_t)remaining * 100u) / total);
}

static void Ui_FormatClock(uint32_t uptime_ms, char *buffer, uint32_t buffer_size)
{
    uint32_t total_seconds;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;

    if ((buffer == NULL) || (buffer_size == 0u))
    {
        return;
    }

    total_seconds = uptime_ms / 1000u;
    hours = (total_seconds / 3600u) % 24u;
    minutes = (total_seconds / 60u) % 60u;
    seconds = total_seconds % 60u;

    (void)snprintf(buffer,
                   buffer_size,
                   "%02lu:%02lu:%02lu",
                   (unsigned long)hours,
                   (unsigned long)minutes,
                   (unsigned long)seconds);
}
