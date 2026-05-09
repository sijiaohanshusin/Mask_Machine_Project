#include "app_status.h"

const char *App_Status_ToString(app_status_t status)
{
    switch (status)
    {
        case APP_OK:                  return "ok";
        case APP_ERR_NOT_READY:       return "not_ready";
        case APP_ERR_NOT_IMPLEMENTED: return "not_implemented";
        case APP_ERR_TIMEOUT:         return "timeout";
        case APP_ERR_INVALID_ARG:     return "invalid_arg";
        case APP_ERR_BUSY:            return "busy";
        case APP_ERR_HW:              return "hw_error";
        default:                      return "unknown";
    }
}
