#ifndef MM_RUNTIME_H
#define MM_RUNTIME_H

#include <stdint.h>
#include "mm_status.h"
#include "mm_types.h"
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void Mm_Runtime_SetEventQueue(osMessageQueueId_t queue_id);
mm_status_t Mm_Runtime_Init(void);
mm_status_t Mm_Runtime_PostEvent(const mm_event_t *event, uint32_t timeout_ms);

void Mm_Task_Control(void *argument);
void Mm_Task_InputPoll(void *argument);
void Mm_Task_Ui(void *argument);
void Mm_Task_Diag(void *argument);
void Mm_Task_Idle(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* MM_RUNTIME_H */
