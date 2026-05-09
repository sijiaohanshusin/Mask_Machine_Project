#include "drv_rfid.h"
#include <string.h>

app_status_t Drv_Rfid_Init(void)
{
    return APP_ERR_NOT_IMPLEMENTED;
}

app_status_t Drv_Rfid_Poll(drv_rfid_card_t *card)
{
    if (card == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(card, 0, sizeof(*card));
    return APP_ERR_NOT_IMPLEMENTED;
}
