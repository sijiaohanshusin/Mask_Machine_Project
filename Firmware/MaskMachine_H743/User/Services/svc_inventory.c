#include "svc_inventory.h"

#include <stddef.h>

#include "mm_config.h"

static svc_inventory_snapshot_t s_inventory;

mm_status_t Svc_Inventory_Init(void)
{
    s_inventory.total = MM_INITIAL_INVENTORY_TOTAL;
    s_inventory.remaining = MM_INITIAL_INVENTORY_TOTAL;
    s_inventory.low_threshold = MM_INVENTORY_LOW_THRESHOLD;
    s_inventory.low = 0u;
    return MM_OK;
}

mm_status_t Svc_Inventory_SetTotal(uint16_t total)
{
    s_inventory.total = total;
    s_inventory.remaining = total;
    s_inventory.low = (s_inventory.remaining <= s_inventory.low_threshold) ? 1u : 0u;
    return MM_OK;
}

mm_status_t Svc_Inventory_Decrement(uint16_t count)
{
    if (count > s_inventory.remaining)
    {
        return MM_ERR_INVALID_ARG;
    }

    s_inventory.remaining = (uint16_t)(s_inventory.remaining - count);
    s_inventory.low = (s_inventory.remaining <= s_inventory.low_threshold) ? 1u : 0u;
    return MM_OK;
}

mm_status_t Svc_Inventory_GetSnapshot(svc_inventory_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    *snapshot = s_inventory;
    return MM_OK;
}
