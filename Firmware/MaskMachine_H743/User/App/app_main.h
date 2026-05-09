#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#include "app_status.h"
#include "app_types.h"
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void App_Main_SetEventQueue(osMessageQueueId_t queue_id);
app_status_t App_Main_Init(void);
app_status_t App_Main_PostEvent(const app_event_t *event, uint32_t timeout_ms);

void App_Task_Control(void *argument);
void App_Task_InputPoll(void *argument);
void App_Task_Ui(void *argument);
void App_Task_Diag(void *argument);
void App_Task_Idle(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
