#ifndef SVC_ENVIRONMENT_H
#define SVC_ENVIRONMENT_H

#include <stdint.h>
#include "app_status.h"
#include "drv_env_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    drv_env_sample_t sample;
    uint32_t updated_ms;
    app_status_t last_status;
} svc_environment_snapshot_t;

app_status_t Svc_Environment_Init(void);
app_status_t Svc_Environment_Poll(void);
app_status_t Svc_Environment_GetSnapshot(svc_environment_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SVC_ENVIRONMENT_H */
