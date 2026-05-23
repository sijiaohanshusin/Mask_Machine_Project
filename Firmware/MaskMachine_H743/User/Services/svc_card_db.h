#ifndef SVC_CARD_DB_H
#define SVC_CARD_DB_H

#include <stdint.h>
#include "app_status.h"
#include "drv_rfid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SVC_CARD_DB_MAX_RECORDS 8u

typedef struct
{
    uint8_t uid_len;
    uint8_t uid[DRV_RFID_UID_MAX_LEN];
    uint8_t enabled;
} svc_card_db_record_t;

typedef struct
{
    uint8_t ready;
    uint8_t empty;
    uint8_t count;
    uint8_t max_records;
    uint32_t sequence;
    app_status_t last_status;
} svc_card_db_snapshot_t;

typedef struct
{
    uint8_t found;
    uint8_t authorized;
    uint8_t empty;
    uint8_t index;
    app_status_t status;
} svc_card_db_check_t;

app_status_t Svc_CardDb_Init(void);
app_status_t Svc_CardDb_CheckUid(const drv_rfid_card_t *card, svc_card_db_check_t *result);
app_status_t Svc_CardDb_AddOrUpdateUid(const drv_rfid_card_t *card, uint8_t enabled);
app_status_t Svc_CardDb_ReplaceAll(const svc_card_db_record_t *records, uint8_t count, uint32_t source_seq);
app_status_t Svc_CardDb_GetSnapshot(svc_card_db_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SVC_CARD_DB_H */
