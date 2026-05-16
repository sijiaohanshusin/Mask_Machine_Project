#include "mm_runtime.h"

#include <string.h>

#include "mm_config.h"
#include "bsp_led.h"
#include "bsp_log.h"
#include "bsp_time.h"
#include "svc_auth.h"
#include "svc_diag.h"
#include "svc_dispenser.h"
#include "svc_environment.h"
#include "svc_inventory.h"
#include "svc_ui.h"
#include "stm32h7xx_hal.h"

static osMessageQueueId_t s_event_queue;
static uint8_t s_initialized;

static void Mm_HandleEvent(const mm_event_t *event);
static void Mm_UpdateUiSnapshot(void);

void Mm_Runtime_SetEventQueue(osMessageQueueId_t queue_id)
{
    s_event_queue = queue_id;
}

mm_status_t Mm_Runtime_Init(void)
{
    if (s_initialized != 0u)
    {
        return MM_OK;
    }

    (void)Bsp_Led_Init();
    (void)Bsp_Log_Init();
    (void)Svc_Diag_Init();
    (void)Svc_Inventory_Init();
    (void)Svc_Dispenser_Init();
    (void)Svc_Auth_Init();
    (void)Svc_Environment_Init();

    s_initialized = 1u;

    (void)Bsp_Log_Printf("\r\n[boot] %s v%s\r\n", MM_FW_NAME, MM_FW_VERSION);
    (void)Bsp_Log_Printf("[boot] SYSCLK=%lu Hz USART1=%lu baud\r\n",
                         (unsigned long)HAL_RCC_GetSysClockFreq(),
                         (unsigned long)MM_LOG_BAUDRATE);
    (void)Bsp_Log_Printf("[boot] base runtime ready; display init is deferred to UI task\r\n");
    return MM_OK;
}

mm_status_t Mm_Runtime_PostEvent(const mm_event_t *event, uint32_t timeout_ms)
{
    osStatus_t os_ret;

    if (event == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    if (s_event_queue == NULL)
    {
        return MM_ERR_NOT_READY;
    }

    os_ret = osMessageQueuePut(s_event_queue, event, 0u, timeout_ms);
    if (os_ret == osOK)
    {
        return MM_OK;
    }

    return (os_ret == osErrorTimeout) ? MM_ERR_TIMEOUT : MM_ERR_BUSY;
}

void Mm_Task_Control(void *argument)
{
    mm_event_t event;
    osStatus_t os_ret;

    (void)argument;
    (void)Bsp_Log_Printf("[task] control started\r\n");

    for (;;)
    {
        if (s_event_queue != NULL)
        {
            os_ret = osMessageQueueGet(s_event_queue, &event, NULL, MM_CONTROL_PERIOD_MS);
            if (os_ret == osOK)
            {
                Mm_HandleEvent(&event);
            }
        }
        else
        {
            osDelay(MM_CONTROL_PERIOD_MS);
        }

        Svc_Dispenser_Process();
    }
}

void Mm_Task_InputPoll(void *argument)
{
    svc_auth_result_t auth;
    uint32_t last_env_ms;

    (void)argument;
    last_env_ms = Bsp_Time_NowMs() - MM_ENV_POLL_PERIOD_MS;
    (void)Bsp_Log_Printf("[task] input poll started\r\n");

    for (;;)
    {
        if (Svc_Auth_Poll(&auth) == MM_OK && auth.state == SVC_AUTH_CARD_PRESENT)
        {
            mm_event_t event = {
                .type = MM_EVENT_AUTH_PRESENT,
                .arg0 = 0u,
                .arg1 = 0u
            };
            (void)Mm_Runtime_PostEvent(&event, 0u);
        }

        if ((Bsp_Time_NowMs() - last_env_ms) >= MM_ENV_POLL_PERIOD_MS)
        {
            last_env_ms = Bsp_Time_NowMs();
            if (Svc_Environment_Poll() == MM_OK)
            {
                mm_event_t event = {
                    .type = MM_EVENT_ENV_UPDATED,
                    .arg0 = 0u,
                    .arg1 = 0u
                };
                (void)Mm_Runtime_PostEvent(&event, 0u);
            }
        }

        osDelay(MM_AUTH_POLL_PERIOD_MS);
    }
}

void Mm_Task_Ui(void *argument)
{
    mm_status_t ui_ret = MM_ERR_NOT_READY;
    uint8_t ui_ready = 0u;
    uint32_t last_snapshot_ms = 0u;
    uint32_t last_alive_log_ms = 0u;
    uint32_t now_ms;

    (void)argument;
    (void)Bsp_Log_Printf("[task] ui started\r\n");
    osDelay(MM_DISPLAY_INIT_DELAY_MS);

    for (;;)
    {
        if (ui_ready == 0u)
        {
            ui_ret = Svc_Ui_Init();
            if (ui_ret == MM_OK)
            {
                ui_ready = 1u;
                last_snapshot_ms = Bsp_Time_NowMs() - MM_UI_SNAPSHOT_PERIOD_MS;
                last_alive_log_ms = Bsp_Time_NowMs();
                (void)Bsp_Log_Printf("[display] ui init complete\r\n");
            }
            else
            {
                Svc_Diag_SetLastError(ui_ret);
                (void)Bsp_Log_Printf("[display] ui init failed: %s\r\n", Mm_Status_ToString(ui_ret));
                osDelay(1000u);
                continue;
            }
        }

        now_ms = Bsp_Time_NowMs();
        if ((now_ms - last_snapshot_ms) >= MM_UI_SNAPSHOT_PERIOD_MS)
        {
            last_snapshot_ms = now_ms;
            Mm_UpdateUiSnapshot();
        }

        Svc_Ui_Process(MM_UI_PERIOD_MS);

        now_ms = Bsp_Time_NowMs();
        if ((now_ms - last_alive_log_ms) >= MM_UI_ALIVE_LOG_PERIOD_MS)
        {
            last_alive_log_ms = now_ms;
            (void)Bsp_Log_Printf("[ui] alive tick=%lu\r\n", (unsigned long)now_ms);
        }

        osDelay(MM_UI_PERIOD_MS);
    }
}

void Mm_Task_Diag(void *argument)
{
    uint32_t heartbeat_log_count = 0u;

    (void)argument;
    (void)Bsp_Log_Printf("[task] diag started\r\n");

    for (;;)
    {
        Svc_Diag_Heartbeat();
        (void)Bsp_Led_Toggle(BSP_LED_0);
        heartbeat_log_count++;
        if ((heartbeat_log_count % 10u) == 0u)
        {
            (void)Bsp_Log_Printf("[diag] heartbeat=%lu\r\n", (unsigned long)heartbeat_log_count);
        }
        osDelay(MM_DIAG_LED_PERIOD_MS);
    }
}

void Mm_Task_Idle(void *argument)
{
    (void)argument;

    for (;;)
    {
        osDelay(1000u);
    }
}

static void Mm_HandleEvent(const mm_event_t *event)
{
    mm_status_t ret = MM_OK;

    if (event == NULL)
    {
        return;
    }

    switch (event->type)
    {
        case MM_EVENT_DISPENSE_REQUEST:
            ret = Svc_Dispenser_RequestOne();
            (void)Bsp_Log_Printf("[dispense] request result=%s\r\n", Mm_Status_ToString(ret));
            break;

        case MM_EVENT_AUTH_PRESENT:
            (void)Bsp_Log_Printf("[auth] card present event\r\n");
            break;

        case MM_EVENT_ENV_UPDATED:
            break;

        case MM_EVENT_FAULT:
            ret = MM_ERR_HW;
            break;

        case MM_EVENT_NONE:
        default:
            break;
    }

    Svc_Ui_PostEvent(event);
    if (ret != MM_OK)
    {
        Svc_Diag_SetLastError(ret);
    }
}

static void Mm_UpdateUiSnapshot(void)
{
    svc_ui_snapshot_t snapshot;

    (void)memset(&snapshot, 0, sizeof(snapshot));

    (void)Svc_Dispenser_GetSnapshot(&snapshot.dispenser);
    (void)Svc_Inventory_GetSnapshot(&snapshot.inventory);
    (void)Svc_Environment_GetSnapshot(&snapshot.environment);
    (void)Svc_Diag_GetSnapshot(&snapshot.diag);
    snapshot.connectivity.mqtt_known = 0u;
    snapshot.connectivity.signal_known = 0u;
    (void)Svc_Ui_SetSnapshot(&snapshot);
}
