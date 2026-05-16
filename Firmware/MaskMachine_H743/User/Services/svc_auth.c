#include "svc_auth.h"
#include <string.h>

app_status_t Svc_Auth_Init(void)
{
    return Drv_Rfid_Init();
}

app_status_t Svc_Auth_Poll(svc_auth_result_t *result)
{
    app_status_t ret;

    if (result == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(result, 0, sizeof(*result));
    ret = Drv_Rfid_Poll(&result->card);
    result->last_status = ret;

    if (ret == APP_OK && result->card.present != 0u)
    {
        result->state = SVC_AUTH_CARD_PRESENT;
        return APP_OK;
    }

    result->state = (ret == APP_OK) ? SVC_AUTH_NONE : SVC_AUTH_UNAVAILABLE;
    return ret;
}
