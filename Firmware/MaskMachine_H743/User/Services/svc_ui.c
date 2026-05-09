#include "svc_ui.h"

#include <stdio.h>
#include <string.h>
#include "bsp_display_stub.h"
#include "lvgl.h"

static svc_ui_snapshot_t s_snapshot;
static lv_obj_t *s_status_label = NULL;

app_status_t Svc_Ui_Init(void)
{
    app_status_t ret = Bsp_DisplayStub_Init();
    if (ret != APP_OK)
    {
        return ret;
    }

    s_status_label = lv_label_create(lv_scr_act());
    lv_obj_align(s_status_label, LV_ALIGN_TOP_LEFT, 4, 4);
    lv_label_set_text(s_status_label, "MaskMachine H743\nUI dummy port");
    return APP_OK;
}

app_status_t Svc_Ui_PostEvent(const app_event_t *event)
{
    (void)event;
    return APP_OK;
}

app_status_t Svc_Ui_SetSnapshot(const svc_ui_snapshot_t *snapshot)
{
    char text[192];

    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    s_snapshot = *snapshot;

    if (s_status_label != NULL)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "MaskMachine H743\nup=%lu ms\nstock=%u/%u\nreq=%lu ok=%lu\nlast=%s",
                       (unsigned long)s_snapshot.diag.uptime_ms,
                       (unsigned int)s_snapshot.inventory.remaining,
                       (unsigned int)s_snapshot.inventory.total,
                       (unsigned long)s_snapshot.dispenser.request_count,
                       (unsigned long)s_snapshot.dispenser.success_count,
                       App_Status_ToString(s_snapshot.dispenser.last_status));
        lv_label_set_text(s_status_label, text);
    }

    return APP_OK;
}

void Svc_Ui_Process(uint32_t elapsed_ms)
{
    Bsp_DisplayStub_Tick(elapsed_ms);
    Bsp_DisplayStub_Process();
}
