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
    SVC_AUTH_UNAVAILABLE
} svc_auth_state_t;

typedef struct
{
    svc_auth_state_t state;
    drv_rfid_card_t card;
    app_status_t last_status;
} svc_auth_result_t;

app_status_t Svc_Auth_Init(void);
app_status_t Svc_Auth_Poll(svc_auth_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* SVC_AUTH_H */
