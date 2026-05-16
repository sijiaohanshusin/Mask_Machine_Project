#ifndef MM_TYPES_H
#define MM_TYPES_H

#include <stdint.h>
#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MM_EVENT_NONE = 0,
    MM_EVENT_DISPENSE_REQUEST,
    MM_EVENT_AUTH_PRESENT,
    MM_EVENT_ENV_UPDATED,
    MM_EVENT_FAULT
} mm_event_type_t;

typedef struct
{
    mm_event_type_t type;
    uint32_t arg0;
    uint32_t arg1;
} mm_event_t;

#ifdef __cplusplus
}
#endif

#endif /* MM_TYPES_H */
