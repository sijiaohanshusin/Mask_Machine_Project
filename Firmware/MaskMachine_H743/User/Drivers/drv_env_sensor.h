#ifndef DRV_ENV_SENSOR_H
#define DRV_ENV_SENSOR_H

#include <stdint.h>
#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int16_t temperature_c_x10;
    uint16_t humidity_rh_x10;
    uint16_t pm25_ugm3_x10;
    uint8_t valid;
} drv_env_sample_t;

mm_status_t Drv_EnvSensor_Init(void);
mm_status_t Drv_EnvSensor_Read(drv_env_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRV_ENV_SENSOR_H */
