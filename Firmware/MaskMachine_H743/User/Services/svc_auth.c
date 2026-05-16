#include "svc_auth.h"
#include <string.h>

mm_status_t Svc_Auth_Init(void)
{
    (void)Drv_Rfid_Init();
    return MM_OK;
}

mm_status_t Svc_Auth_Poll(svc_auth_result_t *result)
{
    mm_status_t ret;

    if (result == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    (void)memset(result, 0, sizeof(*result));
    ret = Drv_Rfid_Poll(&result->card);
    result->last_status = ret;

    if (ret == MM_OK && result->card.present != 0u)
    {
        result->state = SVC_AUTH_CARD_PRESENT;
        return MM_OK;
    }

    result->state = (ret == MM_ERR_NOT_IMPLEMENTED) ? SVC_AUTH_UNAVAILABLE : SVC_AUTH_NONE;
    return ret;
}
