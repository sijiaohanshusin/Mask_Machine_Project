#include "svc_environment.h"

#include <stddef.h>
#include <string.h>

#include "app_config.h"
#include "bsp_time.h"

static svc_environment_snapshot_t s_env;

app_status_t Svc_Environment_Init(void)
{
    (void)memset(&s_env, 0, sizeof(s_env));
    s_env.updated_ms = 0u;
    s_env.last_status = Drv_EnvSensor_Init();
    return APP_OK;
}

app_status_t Svc_Environment_Poll(void)
{
    drv_env_sample_t sample;
    app_status_t ret;
    uint32_t now_ms;

    now_ms = Bsp_Time_NowMs();
    ret = Drv_EnvSensor_Read(&sample);
    if (ret == APP_OK)
    {
        s_env.sample = sample;
        s_env.updated_ms = now_ms;
        s_env.last_status = APP_OK;
        return APP_OK;
    }

    if ((s_env.sample.valid != 0u) && ((now_ms - s_env.updated_ms) <= APP_ENV_STALE_MS))
    {
        s_env.last_status = APP_OK;
        return ret;
    }

    s_env.sample.valid = 0u;
    s_env.last_status = ret;
    return ret;
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
