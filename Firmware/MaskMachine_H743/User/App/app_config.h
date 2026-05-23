#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_FW_NAME                     "MaskMachine_H743"
#define APP_FW_VERSION                  "0.1.0"

#define APP_LOG_BAUDRATE                115200u
#define APP_INITIAL_INVENTORY_TOTAL     50u
#define APP_INVENTORY_LOW_THRESHOLD     5u

#define APP_CONTROL_TASK_STACK_WORDS    512u
#define APP_UI_TASK_STACK_WORDS         4096u
#define APP_DIAG_TASK_STACK_WORDS       384u
#define APP_POLL_TASK_STACK_WORDS       512u

#define APP_CONTROL_TASK_PRIO           4u
#define APP_UI_TASK_PRIO                3u
#define APP_DIAG_TASK_PRIO              1u
#define APP_POLL_TASK_PRIO              2u

#define APP_EVENT_QUEUE_DEPTH           8u
#define APP_CONTROL_PERIOD_MS           50u
#define APP_UI_PERIOD_MS                20u
#define APP_UI_SNAPSHOT_PERIOD_MS       500u
#define APP_UI_ALIVE_LOG_PERIOD_MS      5000u
#define APP_DISPLAY_INIT_DELAY_MS       1500u
#define APP_DIAG_LED_PERIOD_MS          500u
#define APP_ENV_POLL_PERIOD_MS          30000u
#define APP_AUTH_POLL_PERIOD_MS         200u
#define APP_AUTH_SESSION_VALID_MS       15000u
#define APP_CARD_DB_LEARN_FIRST_CARD    1u

#endif /* APP_CONFIG_H */
