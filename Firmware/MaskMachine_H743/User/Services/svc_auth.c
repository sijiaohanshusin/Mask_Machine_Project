#include "svc_auth.h"

#include <string.h>

#include "app_config.h"
#include "bsp_time.h"
#include "svc_card_db.h"

static svc_auth_snapshot_t s_snapshot;
static uint8_t s_session_active;
static uint32_t s_session_expires_ms;
static drv_rfid_card_t s_session_card;

static uint8_t Auth_GetSessionRemaining(uint32_t now_ms, uint32_t *remaining_ms);
static void Auth_ClearSession(void);
static void Auth_ActivateSession(const drv_rfid_card_t *card);
static void Auth_UpdateSnapshot(svc_auth_state_t state,
                                const drv_rfid_card_t *card,
                                app_status_t status,
                                uint8_t learned);

app_status_t Svc_Auth_Init(void)
{
    return Drv_Rfid_Init();
}

app_status_t Svc_Auth_Poll(svc_auth_result_t *result)
{
    app_status_t ret;
    svc_card_db_check_t db_check;
    uint32_t remaining_ms = 0u;
    uint32_t now_ms;

    if (result == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(result, 0, sizeof(*result));
    ret = Drv_Rfid_Poll(&result->card);
    result->last_status = ret;

    if (ret == APP_OK && result->card.present != 0u)
    {
        ret = Svc_CardDb_CheckUid(&result->card, &db_check);
        if (ret != APP_OK)
        {
            result->state = SVC_AUTH_DB_UNAVAILABLE;
            result->last_status = ret;
            Auth_UpdateSnapshot(SVC_AUTH_DB_UNAVAILABLE, &result->card, ret, 0u);
            return APP_OK;
        }

        if (db_check.authorized != 0u)
        {
            Auth_ActivateSession(&result->card);
            result->state = SVC_AUTH_AUTHORIZED;
            result->session_active = 1u;
            result->session_remaining_ms = APP_AUTH_SESSION_VALID_MS;
            Auth_UpdateSnapshot(SVC_AUTH_AUTHORIZED, &result->card, APP_OK, 0u);
            return APP_OK;
        }

#if (APP_CARD_DB_LEARN_FIRST_CARD != 0u)
        if (db_check.empty != 0u)
        {
            ret = Svc_CardDb_AddOrUpdateUid(&result->card, 1u);
            if (ret == APP_OK)
            {
                Auth_ActivateSession(&result->card);
                result->state = SVC_AUTH_AUTHORIZED;
                result->learned = 1u;
                result->session_active = 1u;
                result->session_remaining_ms = APP_AUTH_SESSION_VALID_MS;
                Auth_UpdateSnapshot(SVC_AUTH_AUTHORIZED, &result->card, APP_OK, 1u);
                return APP_OK;
            }

            result->state = SVC_AUTH_DB_UNAVAILABLE;
            result->last_status = ret;
            Auth_UpdateSnapshot(SVC_AUTH_DB_UNAVAILABLE, &result->card, ret, 0u);
            return APP_OK;
        }
#endif

        result->state = SVC_AUTH_DENIED;
        result->last_status = APP_ERR_NOT_READY;
        Auth_UpdateSnapshot(SVC_AUTH_DENIED, &result->card, APP_ERR_NOT_READY, 0u);
        return APP_OK;
    }

    if (ret != APP_OK)
    {
        result->state = SVC_AUTH_UNAVAILABLE;
        Auth_UpdateSnapshot(SVC_AUTH_UNAVAILABLE, NULL, ret, 0u);
        return ret;
    }

    now_ms = Bsp_Time_NowMs();
    if (Auth_GetSessionRemaining(now_ms, &remaining_ms) != 0u)
    {
        result->state = SVC_AUTH_SESSION_ACTIVE;
        result->card = s_session_card;
        result->session_active = 1u;
        result->session_remaining_ms = remaining_ms;
        result->last_status = APP_OK;
        Auth_UpdateSnapshot(SVC_AUTH_SESSION_ACTIVE, &s_session_card, APP_OK, 0u);
        return APP_OK;
    }

    if (s_session_active != 0u)
    {
        Auth_ClearSession();
        result->state = SVC_AUTH_SESSION_TIMEOUT;
        result->last_status = APP_ERR_TIMEOUT;
        Auth_UpdateSnapshot(SVC_AUTH_SESSION_TIMEOUT, NULL, APP_ERR_TIMEOUT, 0u);
        return APP_OK;
    }

    result->state = SVC_AUTH_NONE;
    Auth_UpdateSnapshot(SVC_AUTH_NONE, NULL, APP_OK, 0u);
    return ret;
}

app_status_t Svc_Auth_ConsumeSession(drv_rfid_card_t *card)
{
    uint32_t remaining_ms;

    if (Auth_GetSessionRemaining(Bsp_Time_NowMs(), &remaining_ms) == 0u)
    {
        if (s_session_active != 0u)
        {
            Auth_ClearSession();
            Auth_UpdateSnapshot(SVC_AUTH_SESSION_TIMEOUT, NULL, APP_ERR_TIMEOUT, 0u);
            return APP_ERR_TIMEOUT;
        }

        Auth_UpdateSnapshot(SVC_AUTH_NONE, NULL, APP_ERR_NOT_READY, 0u);
        return APP_ERR_NOT_READY;
    }

    if (card != NULL)
    {
        *card = s_session_card;
    }

    Auth_ClearSession();
    Auth_UpdateSnapshot(SVC_AUTH_NONE, NULL, APP_OK, 0u);
    return APP_OK;
}

app_status_t Svc_Auth_GetSnapshot(svc_auth_snapshot_t *snapshot)
{
    uint32_t remaining_ms;

    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Auth_GetSessionRemaining(Bsp_Time_NowMs(), &remaining_ms) != 0u)
    {
        s_snapshot.session_active = 1u;
        s_snapshot.session_remaining_ms = remaining_ms;
    }
    else
    {
        s_snapshot.session_active = 0u;
        s_snapshot.session_remaining_ms = 0u;
    }

    *snapshot = s_snapshot;
    return APP_OK;
}

static uint8_t Auth_GetSessionRemaining(uint32_t now_ms, uint32_t *remaining_ms)
{
    int32_t delta;

    if (s_session_active == 0u)
    {
        if (remaining_ms != NULL)
        {
            *remaining_ms = 0u;
        }
        return 0u;
    }

    delta = (int32_t)(s_session_expires_ms - now_ms);
    if (delta <= 0)
    {
        if (remaining_ms != NULL)
        {
            *remaining_ms = 0u;
        }
        return 0u;
    }

    if (remaining_ms != NULL)
    {
        *remaining_ms = (uint32_t)delta;
    }
    return 1u;
}

static void Auth_ClearSession(void)
{
    s_session_active = 0u;
    s_session_expires_ms = 0u;
    (void)memset(&s_session_card, 0, sizeof(s_session_card));
}

static void Auth_ActivateSession(const drv_rfid_card_t *card)
{
    if (card == NULL)
    {
        return;
    }

    s_session_active = 1u;
    s_session_expires_ms = Bsp_Time_NowMs() + APP_AUTH_SESSION_VALID_MS;
    s_session_card = *card;
}

static void Auth_UpdateSnapshot(svc_auth_state_t state,
                                const drv_rfid_card_t *card,
                                app_status_t status,
                                uint8_t learned)
{
    uint32_t remaining_ms;

    s_snapshot.state = state;
    s_snapshot.last_status = status;
    s_snapshot.last_learned = learned;
    s_snapshot.last_authorized = (state == SVC_AUTH_AUTHORIZED) ? 1u : 0u;

    if ((card != NULL) && (card->present != 0u) && (card->uid_len != 0u))
    {
        s_snapshot.last_card = *card;
        s_snapshot.last_uid_valid = 1u;
    }

    if (Auth_GetSessionRemaining(Bsp_Time_NowMs(), &remaining_ms) != 0u)
    {
        s_snapshot.session_active = 1u;
        s_snapshot.session_remaining_ms = remaining_ms;
    }
    else
    {
        s_snapshot.session_active = 0u;
        s_snapshot.session_remaining_ms = 0u;
    }
}
