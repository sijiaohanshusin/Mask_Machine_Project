#ifndef DRV_COMM_H
#define DRV_COMM_H

#include <stddef.h>
#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

app_status_t Drv_Comm_Init(void);
app_status_t Drv_Comm_SendHealth(const uint8_t *payload, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* DRV_COMM_H */
