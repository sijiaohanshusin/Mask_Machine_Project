#ifndef MM_STATUS_H
#define MM_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MM_OK = 0,
    MM_ERR_NOT_READY,
    MM_ERR_NOT_IMPLEMENTED,
    MM_ERR_TIMEOUT,
    MM_ERR_INVALID_ARG,
    MM_ERR_BUSY,
    MM_ERR_HW
} mm_status_t;

const char *Mm_Status_ToString(mm_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* MM_STATUS_H */
