#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_EVENT_NONE = 0,
    APP_EVENT_DISPENSE_REQUEST,
    APP_EVENT_AUTH_PRESENT,
    APP_EVENT_AUTH_GRANTED,
    APP_EVENT_AUTH_DENIED,
    APP_EVENT_AUTH_REQUIRED,
    APP_EVENT_AUTH_TIMEOUT,
    APP_EVENT_AUTH_DB_ERROR,
    APP_EVENT_ENV_UPDATED,
    APP_EVENT_FAULT
} app_event_type_t;

typedef struct
{
    app_event_type_t type;
    uint32_t arg0;
    uint32_t arg1;
} app_event_t;

#ifdef __cplusplus
}
#endif

#endif /* APP_TYPES_H */
