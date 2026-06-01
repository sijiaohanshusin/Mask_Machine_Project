#include "drv_env_sensor.h"

#include <stddef.h>

#include "bsp_env_uart.h"

app_status_t Drv_EnvSensor_Init(void)
{
    return Bsp_EnvUart_Init();
}

app_status_t Drv_EnvSensor_Read(drv_env_sample_t *sample)
{
    if (sample == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    sample->temperature_c_x10 = 0;
    sample->humidity_rh_x10 = 0u;
    sample->pm25_ugm3_x10 = 0u;
    sample->valid = 0u;
    return APP_ERR_NOT_IMPLEMENTED;
}
