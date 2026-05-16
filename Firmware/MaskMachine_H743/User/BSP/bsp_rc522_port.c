#include "bsp_rc522_port.h"

#include <string.h>

/*
 * The RC522 transport stays intentionally unbound for now.
 * We have not yet confirmed the real SPI / NSS / RST wiring on the mask machine.
 * Once the hardware mapping is fixed, this file is the only place that needs
 * a board-specific implementation.
 */

app_status_t Bsp_Rc522Port_Init(void)
{
    return APP_ERR_NOT_READY;
}

app_status_t Bsp_Rc522Port_Reset(void)
{
    return APP_ERR_NOT_READY;
}

app_status_t Bsp_Rc522Port_Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length)
{
    (void)tx_data;

    if ((rx_data != NULL) && (length > 0u))
    {
        (void)memset(rx_data, 0, length);
    }

    return APP_ERR_NOT_READY;
}
