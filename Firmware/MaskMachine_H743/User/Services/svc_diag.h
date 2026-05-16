#ifndef SVC_DIAG_H
#define SVC_DIAG_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t uptime_ms;
    uint32_t heartbeat_count;
    app_status_t last_error;
} svc_diag_snapshot_t;

app_status_t Svc_Diag_Init(void);
void Svc_Diag_Heartbeat(void);
app_status_t Svc_Diag_GetSnapshot(svc_diag_snapshot_t *snapshot);
void Svc_Diag_SetLastError(app_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* SVC_DIAG_H */
