#include "mm_status.h"

const char *Mm_Status_ToString(mm_status_t status)
{
    switch (status)
    {
        case MM_OK:                  return "ok";
        case MM_ERR_NOT_READY:       return "not_ready";
        case MM_ERR_NOT_IMPLEMENTED: return "not_implemented";
        case MM_ERR_TIMEOUT:         return "timeout";
        case MM_ERR_INVALID_ARG:     return "invalid_arg";
        case MM_ERR_BUSY:            return "busy";
        case MM_ERR_HW:              return "hw_error";
        default:                      return "unknown";
    }
}
