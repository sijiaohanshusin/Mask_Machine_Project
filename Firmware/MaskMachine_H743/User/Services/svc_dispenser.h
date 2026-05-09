#ifndef SVC_DISPENSER_H
#define SVC_DISPENSER_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SVC_DISPENSER_STATE_IDLE = 0,
    SVC_DISPENSER_STATE_BUSY,
    SVC_DISPENSER_STATE_FAULT
} svc_dispenser_state_t;

typedef struct
{
    svc_dispenser_state_t state;
    app_status_t last_status;
    uint32_t request_count;
    uint32_t success_count;
    uint32_t blocked_count;
} svc_dispenser_snapshot_t;

app_status_t Svc_Dispenser_Init(void);
app_status_t Svc_Dispenser_RequestOne(void);
svc_dispenser_state_t Svc_Dispenser_GetState(void);
app_status_t Svc_Dispenser_GetSnapshot(svc_dispenser_snapshot_t *snapshot);
void Svc_Dispenser_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* SVC_DISPENSER_H */
