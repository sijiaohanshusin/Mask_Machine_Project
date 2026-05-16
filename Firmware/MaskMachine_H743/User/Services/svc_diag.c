#include "svc_diag.h"

#include <stddef.h>

#include "bsp_time.h"

static svc_diag_snapshot_t s_diag;

mm_status_t Svc_Diag_Init(void)
{
    s_diag.uptime_ms = 0u;
    s_diag.heartbeat_count = 0u;
    s_diag.last_error = MM_OK;
    return MM_OK;
}

void Svc_Diag_Heartbeat(void)
{
    s_diag.uptime_ms = Bsp_Time_NowMs();
    s_diag.heartbeat_count++;
}

mm_status_t Svc_Diag_GetSnapshot(svc_diag_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    s_diag.uptime_ms = Bsp_Time_NowMs();
    *snapshot = s_diag;
    return MM_OK;
}

void Svc_Diag_SetLastError(mm_status_t status)
{
    if (status != MM_OK)
    {
        s_diag.last_error = status;
    }
}
