#ifndef MM_CONFIG_H
#define MM_CONFIG_H

#define MM_FW_NAME                     "MaskMachine_H743"
#define MM_FW_VERSION                  "0.1.0"

#define MM_LOG_BAUDRATE                115200u
#define MM_INITIAL_INVENTORY_TOTAL     50u
#define MM_INVENTORY_LOW_THRESHOLD     5u

#define MM_CONTROL_TASK_STACK_WORDS    512u
#define MM_UI_TASK_STACK_WORDS         4096u
#define MM_DIAG_TASK_STACK_WORDS       384u
#define MM_POLL_TASK_STACK_WORDS       512u

#define MM_CONTROL_TASK_PRIO           4u
#define MM_UI_TASK_PRIO                3u
#define MM_DIAG_TASK_PRIO              1u
#define MM_POLL_TASK_PRIO              2u

#define MM_EVENT_QUEUE_DEPTH           8u
#define MM_CONTROL_PERIOD_MS           50u
#define MM_UI_PERIOD_MS                20u
#define MM_UI_SNAPSHOT_PERIOD_MS       500u
#define MM_UI_ALIVE_LOG_PERIOD_MS      5000u
#define MM_DISPLAY_INIT_DELAY_MS       1500u
#define MM_DIAG_LED_PERIOD_MS          500u
#define MM_ENV_POLL_PERIOD_MS          30000u
#define MM_AUTH_POLL_PERIOD_MS         200u

#endif /* MM_CONFIG_H */
