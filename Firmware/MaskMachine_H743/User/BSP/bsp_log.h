#ifndef BSP_LOG_H
#define BSP_LOG_H

#include "mm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

mm_status_t Bsp_Log_Init(void);
int Bsp_Log_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LOG_H */
