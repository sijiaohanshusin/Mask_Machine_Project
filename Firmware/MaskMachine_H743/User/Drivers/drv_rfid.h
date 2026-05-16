#ifndef DRV_RFID_H
#define DRV_RFID_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_RFID_UID_MAX_LEN 10u

typedef struct
{
    uint8_t present;
    uint8_t uid_len;
    uint8_t uid[DRV_RFID_UID_MAX_LEN];
} drv_rfid_card_t;

app_status_t Drv_Rfid_Init(void);
app_status_t Drv_Rfid_Poll(drv_rfid_card_t *card);

#ifdef __cplusplus
}
#endif

#endif /* DRV_RFID_H */
