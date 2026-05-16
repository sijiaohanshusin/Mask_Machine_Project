#ifndef BSP_RC522_PORT_H
#define BSP_RC522_PORT_H

#include <stdint.h>

#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

app_status_t Bsp_Rc522Port_Init(void);
app_status_t Bsp_Rc522Port_Reset(void);
app_status_t Bsp_Rc522Port_Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RC522_PORT_H */
