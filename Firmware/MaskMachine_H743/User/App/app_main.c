#include "app_main.h"

#include "app_config.h"
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

static void App_HandleEvent(const app_event_t *event);
static void App_UpdateUiSnapshot(void);

void App_Main_SetEventQueue(osMessageQueueId_t queue_id)
{
    s_event_queue = queue_id;
}

app_status_t App_Main_Init(void)
{
    app_status_t ret;

    if (s_initialized != 0u)
    {
        return APP_OK;
    }

    (void)Bsp_Led_Init();
    (void)Bsp_Log_Init();
    (void)Svc_Diag_Init();
    (void)Svc_Inventory_Init();
    (void)Svc_Dispenser_Init();
    (void)Svc_Auth_Init();
    (void)Svc_Environment_Init();

    ret = Svc_Ui_Init();
    if (ret != APP_OK)
    {
        Svc_Diag_SetLastError(ret);
        return ret;
    }

    App_UpdateUiSnapshot();
    s_initialized = 1u;

    (void)Bsp_Log_Printf("\r\n[boot] %s v%s\r\n", APP_FW_NAME, APP_FW_VERSION);
    (void)Bsp_Log_Printf("[boot] SYSCLK=%lu Hz USART1=%lu baud\r\n",
                         (unsigned long)HAL_RCC_GetSysClockFreq(),
                         (unsigned long)APP_LOG_BAUDRATE);
    (void)Bsp_Log_Printf("[boot] FreeRTOS task framework and LVGL dummy display ready\r\n");
    return APP_OK;
}

app_status_t App_Main_PostEvent(const app_event_t *event, uint32_t timeout_ms)
{
    osStatus_t os_ret;

    if (event == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    if (s_event_queue == NULL)
    {
        return APP_ERR_NOT_READY;
    }

    os_ret = osMessageQueuePut(s_event_queue, event, 0u, timeout_ms);
    if (os_ret == osOK)
    {
        return APP_OK;
    }

    return (os_ret == osErrorTimeout) ? APP_ERR_TIMEOUT : APP_ERR_BUSY;
}

void App_Task_Control(void *argument)
{
    app_event_t event;
    osStatus_t os_ret;

    (void)argument;
    (void)Bsp_Log_Printf("[task] control started\r\n");

    for (;;)
    {
        if (s_event_queue != NULL)
        {
            os_ret = osMessageQueueGet(s_event_queue, &event, NULL, APP_CONTROL_PERIOD_MS);
            if (os_ret == osOK)
            {
                App_HandleEvent(&event);
            }
        }
        else
        {
            osDelay(APP_CONTROL_PERIOD_MS);
        }

        Svc_Dispenser_Process();
    }
}

void App_Task_InputPoll(void *argument)
{
    svc_auth_result_t auth;
    uint32_t last_env_ms;

    (void)argument;
    last_env_ms = Bsp_Time_NowMs() - APP_ENV_POLL_PERIOD_MS;
    (void)Bsp_Log_Printf("[task] input poll started\r\n");

    for (;;)
    {
        if (Svc_Auth_Poll(&auth) == APP_OK && auth.state == SVC_AUTH_CARD_PRESENT)
        {
            app_event_t event = {
                .type = APP_EVENT_AUTH_PRESENT,
                .arg0 = 0u,
                .arg1 = 0u
            };
            (void)App_Main_PostEvent(&event, 0u);
        }

        if ((Bsp_Time_NowMs() - last_env_ms) >= APP_ENV_POLL_PERIOD_MS)
        {
            last_env_ms = Bsp_Time_NowMs();
            if (Svc_Environment_Poll() == APP_OK)
            {
                app_event_t event = {
                    .type = APP_EVENT_ENV_UPDATED,
                    .arg0 = 0u,
                    .arg1 = 0u
                };
                (void)App_Main_PostEvent(&event, 0u);
            }
        }

        osDelay(APP_AUTH_POLL_PERIOD_MS);
    }
}

void App_Task_Ui(void *argument)
{
    (void)argument;
    (void)Bsp_Log_Printf("[task] ui started\r\n");

    for (;;)
    {
        App_UpdateUiSnapshot();
        Svc_Ui_Process(APP_UI_PERIOD_MS);
        osDelay(APP_UI_PERIOD_MS);
    }
}

void App_Task_Diag(void *argument)
{
    (void)argument;
    (void)Bsp_Log_Printf("[task] diag started\r\n");

    for (;;)
    {
        Svc_Diag_Heartbeat();
        (void)Bsp_Led_Toggle(BSP_LED_0);
        osDelay(APP_DIAG_LED_PERIOD_MS);
    }
}

void App_Task_Idle(void *argument)
{
    (void)argument;

    for (;;)
    {
        osDelay(1000u);
    }
}

static void App_HandleEvent(const app_event_t *event)
{
    app_status_t ret = APP_OK;

    if (event == NULL)
    {
        return;
    }

    switch (event->type)
    {
        case APP_EVENT_DISPENSE_REQUEST:
            ret = Svc_Dispenser_RequestOne();
            (void)Bsp_Log_Printf("[dispense] request result=%s\r\n", App_Status_ToString(ret));
            break;

        case APP_EVENT_AUTH_PRESENT:
            (void)Bsp_Log_Printf("[auth] card present event\r\n");
            break;

        case APP_EVENT_ENV_UPDATED:
            break;

        case APP_EVENT_FAULT:
            ret = APP_ERR_HW;
            break;

        case APP_EVENT_NONE:
        default:
            break;
    }

    Svc_Ui_PostEvent(event);
    if (ret != APP_OK)
    {
        Svc_Diag_SetLastError(ret);
    }
}

static void App_UpdateUiSnapshot(void)
{
    svc_ui_snapshot_t snapshot;

    (void)Svc_Dispenser_GetSnapshot(&snapshot.dispenser);
    (void)Svc_Inventory_GetSnapshot(&snapshot.inventory);
    (void)Svc_Environment_GetSnapshot(&snapshot.environment);
    (void)Svc_Diag_GetSnapshot(&snapshot.diag);
    (void)Svc_Ui_SetSnapshot(&snapshot);
}
