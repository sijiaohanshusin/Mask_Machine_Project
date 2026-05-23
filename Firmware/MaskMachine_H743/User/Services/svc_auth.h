#ifndef SVC_AUTH_H
#define SVC_AUTH_H

#include <stdint.h>
#include "app_status.h"
#include "drv_rfid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SVC_AUTH_NONE = 0,
    SVC_AUTH_CARD_PRESENT,
    SVC_AUTH_AUTHORIZED,
    SVC_AUTH_DENIED,
    SVC_AUTH_DB_UNAVAILABLE,
    SVC_AUTH_SESSION_ACTIVE,
    SVC_AUTH_SESSION_TIMEOUT,
    SVC_AUTH_UNAVAILABLE
} svc_auth_state_t;

typedef struct
{
    svc_auth_state_t state;
    drv_rfid_card_t card;
    uint8_t learned;
    uint8_t session_active;
    uint32_t session_remaining_ms;
    app_status_t last_status;
} svc_auth_result_t;

typedef struct
{
    svc_auth_state_t state;
    drv_rfid_card_t last_card;
    uint8_t last_uid_valid;
    uint8_t last_authorized;
    uint8_t last_learned;
    uint8_t session_active;
    uint32_t session_remaining_ms;
    app_status_t last_status;
} svc_auth_snapshot_t;

app_status_t Svc_Auth_Init(void);
app_status_t Svc_Auth_Poll(svc_auth_result_t *result);
app_status_t Svc_Auth_ConsumeSession(drv_rfid_card_t *card);
app_status_t Svc_Auth_GetSnapshot(svc_auth_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SVC_AUTH_H */
