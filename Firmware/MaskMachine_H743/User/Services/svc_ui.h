#ifndef SVC_UI_H
#define SVC_UI_H

#include "app_types.h"
#include "svc_diag.h"
#include "svc_dispenser.h"
#include "svc_environment.h"
#include "svc_inventory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    svc_dispenser_snapshot_t dispenser;
    svc_inventory_snapshot_t inventory;
    svc_environment_snapshot_t environment;
    svc_diag_snapshot_t diag;
} svc_ui_snapshot_t;

app_status_t Svc_Ui_Init(void);
app_status_t Svc_Ui_PostEvent(const app_event_t *event);
app_status_t Svc_Ui_SetSnapshot(const svc_ui_snapshot_t *snapshot);
void Svc_Ui_Process(uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif /* SVC_UI_H */
