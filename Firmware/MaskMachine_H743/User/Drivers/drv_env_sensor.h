#ifndef DRV_ENV_SENSOR_H
#define DRV_ENV_SENSOR_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t co2_ppm;
    uint16_t ch2o_ugm3;
    uint16_t tvoc_ugm3;
    uint16_t pm25_ugm3;
    uint16_t pm10_ugm3;
    int16_t temperature_c_x10;
    uint16_t humidity_rh_x10;
    uint16_t pm25_ugm3_x10;
    uint8_t valid;
} drv_env_sample_t;

app_status_t Drv_EnvSensor_Init(void);
app_status_t Drv_EnvSensor_Read(drv_env_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRV_ENV_SENSOR_H */
