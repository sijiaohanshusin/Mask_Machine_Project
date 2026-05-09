#ifndef APP_STATUS_H
#define APP_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_OK = 0,
    APP_ERR_NOT_READY,
    APP_ERR_NOT_IMPLEMENTED,
    APP_ERR_TIMEOUT,
    APP_ERR_INVALID_ARG,
    APP_ERR_BUSY,
    APP_ERR_HW
} app_status_t;

const char *App_Status_ToString(app_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATUS_H */
