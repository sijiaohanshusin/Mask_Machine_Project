#include "svc_dispenser.h"

#include <stddef.h>

#include "drv_actuator.h"
#include "svc_inventory.h"

static svc_dispenser_snapshot_t s_disp;

app_status_t Svc_Dispenser_Init(void)
{
    (void)Drv_Actuator_Init();
    s_disp.state = SVC_DISPENSER_STATE_IDLE;
    s_disp.last_status = APP_OK;
    s_disp.request_count = 0u;
    s_disp.success_count = 0u;
    s_disp.blocked_count = 0u;
    return APP_OK;
}

app_status_t Svc_Dispenser_RequestOne(void)
{
    svc_inventory_snapshot_t inv;
    app_status_t ret;

    if (s_disp.state == SVC_DISPENSER_STATE_BUSY)
    {
        s_disp.blocked_count++;
        s_disp.last_status = APP_ERR_BUSY;
        return APP_ERR_BUSY;
    }

    ret = Svc_Inventory_GetSnapshot(&inv);
    if (ret != APP_OK)
    {
        s_disp.last_status = ret;
        return ret;
    }

    if (inv.remaining == 0u)
    {
        s_disp.blocked_count++;
        s_disp.last_status = APP_ERR_NOT_READY;
        return APP_ERR_NOT_READY;
    }

    s_disp.request_count++;
    ret = Drv_Actuator_DispenseOne();
    s_disp.last_status = ret;

    if (ret == APP_OK)
    {
        (void)Svc_Inventory_Decrement(1u);
        s_disp.success_count++;
    }

    return ret;
}

svc_dispenser_state_t Svc_Dispenser_GetState(void)
{
    return s_disp.state;
}

app_status_t Svc_Dispenser_GetSnapshot(svc_dispenser_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    *snapshot = s_disp;
    return APP_OK;
}

void Svc_Dispenser_Process(void)
{
    Drv_Actuator_Process();

    switch (Drv_Actuator_GetState())
    {
        case DRV_ACTUATOR_BUSY:
            s_disp.state = SVC_DISPENSER_STATE_BUSY;
            break;
        case DRV_ACTUATOR_FAULT:
            s_disp.state = SVC_DISPENSER_STATE_FAULT;
            break;
        case DRV_ACTUATOR_IDLE:
        default:
            s_disp.state = SVC_DISPENSER_STATE_IDLE;
            break;
    }
}
