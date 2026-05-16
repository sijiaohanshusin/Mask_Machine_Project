#include "svc_environment.h"

#include <stddef.h>

#include "bsp_time.h"

static svc_environment_snapshot_t s_env;

app_status_t Svc_Environment_Init(void)
{
    s_env.updated_ms = 0u;
    s_env.last_status = Drv_EnvSensor_Init();
    return APP_OK;
}

app_status_t Svc_Environment_Poll(void)
{
    s_env.last_status = Drv_EnvSensor_Read(&s_env.sample);
    s_env.updated_ms = Bsp_Time_NowMs();
    return s_env.last_status;
}

app_status_t Svc_Environment_GetSnapshot(svc_environment_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    *snapshot = s_env;
    return APP_OK;
}
