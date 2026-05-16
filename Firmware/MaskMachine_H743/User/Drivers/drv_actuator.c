#include "drv_actuator.h"

static drv_actuator_state_t s_state = DRV_ACTUATOR_IDLE;

mm_status_t Drv_Actuator_Init(void)
{
    s_state = DRV_ACTUATOR_IDLE;
    return MM_OK;
}

mm_status_t Drv_Actuator_DispenseOne(void)
{
    s_state = DRV_ACTUATOR_IDLE;
    return MM_OK;
}

drv_actuator_state_t Drv_Actuator_GetState(void)
{
    return s_state;
}

void Drv_Actuator_Process(void)
{
}
