#ifndef SVC_INVENTORY_H
#define SVC_INVENTORY_H

#include <stdint.h>
#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t total;
    uint16_t remaining;
    uint16_t low_threshold;
    uint8_t low;
} svc_inventory_snapshot_t;

mm_status_t Svc_Inventory_Init(void);
mm_status_t Svc_Inventory_SetTotal(uint16_t total);
mm_status_t Svc_Inventory_Decrement(uint16_t count);
mm_status_t Svc_Inventory_GetSnapshot(svc_inventory_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SVC_INVENTORY_H */
