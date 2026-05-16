#ifndef DRV_ACTUATOR_H
#define DRV_ACTUATOR_H

#include <stdint.h>
#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DRV_ACTUATOR_IDLE = 0,
    DRV_ACTUATOR_BUSY,
    DRV_ACTUATOR_FAULT
} drv_actuator_state_t;

mm_status_t Drv_Actuator_Init(void);
mm_status_t Drv_Actuator_DispenseOne(void);
drv_actuator_state_t Drv_Actuator_GetState(void);
void Drv_Actuator_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_ACTUATOR_H */
