#include "drv_rfid.h"
#include <string.h>

mm_status_t Drv_Rfid_Init(void)
{
    return MM_ERR_NOT_IMPLEMENTED;
}

mm_status_t Drv_Rfid_Poll(drv_rfid_card_t *card)
{
    if (card == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    (void)memset(card, 0, sizeof(*card));
    return MM_ERR_NOT_IMPLEMENTED;
}
