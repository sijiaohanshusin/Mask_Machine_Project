#include "svc_card_db.h"

#include <string.h>

#include "bsp_eeprom_at24c02.h"

#define CARD_DB_MAGIC0              'M'
#define CARD_DB_MAGIC1              'M'
#define CARD_DB_MAGIC2              'C'
#define CARD_DB_MAGIC3              'D'
#define CARD_DB_VERSION             1u
#define CARD_DB_SLOT_SIZE           128u
#define CARD_DB_SLOT0_OFFSET        0u
#define CARD_DB_SLOT1_OFFSET        128u
#define CARD_DB_HEADER_SIZE         12u
#define CARD_DB_RECORD_SIZE         12u
#define CARD_DB_CRC_OFFSET          126u

static uint8_t s_initialized;
static svc_card_db_record_t s_records[SVC_CARD_DB_MAX_RECORDS];
static svc_card_db_snapshot_t s_snapshot = {
    0u,
    1u,
    0u,
    SVC_CARD_DB_MAX_RECORDS,
    0u,
    APP_ERR_NOT_READY
};

static uint16_t CardDb_Crc16(const uint8_t *data, uint16_t length);
static uint32_t CardDb_ReadLe32(const uint8_t *data);
static void CardDb_WriteLe32(uint8_t *data, uint32_t value);
static uint16_t CardDb_ReadLe16(const uint8_t *data);
static void CardDb_WriteLe16(uint8_t *data, uint16_t value);
static uint8_t CardDb_UidEqual(const drv_rfid_card_t *card, const svc_card_db_record_t *record);
static app_status_t CardDb_LoadSlot(uint8_t offset,
                                    svc_card_db_record_t *records,
                                    uint8_t *count,
                                    uint32_t *sequence);
static void CardDb_EncodeSlot(uint8_t *buffer,
                              const svc_card_db_record_t *records,
                              uint8_t count,
                              uint32_t sequence);
static app_status_t CardDb_Commit(const svc_card_db_record_t *records, uint8_t count, uint32_t sequence);
static void CardDb_ApplyRecords(const svc_card_db_record_t *records, uint8_t count, uint32_t sequence);
static void CardDb_SetUnavailable(app_status_t status);

app_status_t Svc_CardDb_Init(void)
{
    svc_card_db_record_t records0[SVC_CARD_DB_MAX_RECORDS];
    svc_card_db_record_t records1[SVC_CARD_DB_MAX_RECORDS];
    uint8_t count0 = 0u;
    uint8_t count1 = 0u;
    uint32_t seq0 = 0u;
    uint32_t seq1 = 0u;
    app_status_t init_ret;
    app_status_t slot0_ret;
    app_status_t slot1_ret;

    if (s_initialized != 0u)
    {
        return s_snapshot.ready != 0u ? APP_OK : s_snapshot.last_status;
    }

    init_ret = Bsp_EepromAt24c02_Init();
    if (init_ret != APP_OK)
    {
        CardDb_SetUnavailable(init_ret);
        return init_ret;
    }

    slot0_ret = CardDb_LoadSlot(CARD_DB_SLOT0_OFFSET, records0, &count0, &seq0);
    slot1_ret = CardDb_LoadSlot(CARD_DB_SLOT1_OFFSET, records1, &count1, &seq1);

    if ((slot0_ret == APP_OK) && ((slot1_ret != APP_OK) || (seq0 >= seq1)))
    {
        CardDb_ApplyRecords(records0, count0, seq0);
    }
    else if (slot1_ret == APP_OK)
    {
        CardDb_ApplyRecords(records1, count1, seq1);
    }
    else
    {
        (void)memset(s_records, 0, sizeof(s_records));
        s_snapshot.ready = 1u;
        s_snapshot.empty = 1u;
        s_snapshot.count = 0u;
        s_snapshot.max_records = SVC_CARD_DB_MAX_RECORDS;
        s_snapshot.sequence = 0u;
        s_snapshot.last_status = APP_OK;
    }

    s_initialized = 1u;
    return APP_OK;
}

app_status_t Svc_CardDb_CheckUid(const drv_rfid_card_t *card, svc_card_db_check_t *result)
{
    uint8_t index;
    app_status_t ret;

    if ((card == NULL) || (result == NULL) || (card->uid_len == 0u) ||
        (card->uid_len > DRV_RFID_UID_MAX_LEN))
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(result, 0, sizeof(*result));
    ret = Svc_CardDb_Init();
    if (ret != APP_OK)
    {
        result->status = ret;
        return ret;
    }

    result->empty = s_snapshot.empty;
    result->index = 0xFFu;
    result->status = APP_OK;

    for (index = 0u; index < s_snapshot.count; index++)
    {
        if (CardDb_UidEqual(card, &s_records[index]) != 0u)
        {
            result->found = 1u;
            result->authorized = s_records[index].enabled != 0u ? 1u : 0u;
            result->index = index;
            return APP_OK;
        }
    }

    return APP_OK;
}

app_status_t Svc_CardDb_AddOrUpdateUid(const drv_rfid_card_t *card, uint8_t enabled)
{
    svc_card_db_record_t next_records[SVC_CARD_DB_MAX_RECORDS];
    uint8_t index;
    uint8_t count;

    if ((card == NULL) || (card->uid_len == 0u) || (card->uid_len > DRV_RFID_UID_MAX_LEN))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Svc_CardDb_Init() != APP_OK)
    {
        return s_snapshot.last_status;
    }

    (void)memcpy(next_records, s_records, sizeof(next_records));
    count = s_snapshot.count;

    for (index = 0u; index < count; index++)
    {
        if (CardDb_UidEqual(card, &next_records[index]) != 0u)
        {
            next_records[index].enabled = enabled != 0u ? 1u : 0u;
            return CardDb_Commit(next_records, count, s_snapshot.sequence + 1u);
        }
    }

    if (count >= SVC_CARD_DB_MAX_RECORDS)
    {
        s_snapshot.last_status = APP_ERR_BUSY;
        return APP_ERR_BUSY;
    }

    (void)memset(&next_records[count], 0, sizeof(next_records[count]));
    next_records[count].uid_len = card->uid_len;
    (void)memcpy(next_records[count].uid, card->uid, card->uid_len);
    next_records[count].enabled = enabled != 0u ? 1u : 0u;
    count++;

    return CardDb_Commit(next_records, count, s_snapshot.sequence + 1u);
}

app_status_t Svc_CardDb_ReplaceAll(const svc_card_db_record_t *records, uint8_t count, uint32_t source_seq)
{
    svc_card_db_record_t next_records[SVC_CARD_DB_MAX_RECORDS];
    uint8_t index;
    uint32_t sequence;

    if ((count > SVC_CARD_DB_MAX_RECORDS) || ((count != 0u) && (records == NULL)))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Svc_CardDb_Init() != APP_OK)
    {
        return s_snapshot.last_status;
    }

    (void)memset(next_records, 0, sizeof(next_records));
    for (index = 0u; index < count; index++)
    {
        if ((records[index].uid_len == 0u) || (records[index].uid_len > DRV_RFID_UID_MAX_LEN))
        {
            return APP_ERR_INVALID_ARG;
        }
        next_records[index] = records[index];
        next_records[index].enabled = records[index].enabled != 0u ? 1u : 0u;
    }

    sequence = (source_seq > s_snapshot.sequence) ? source_seq : (s_snapshot.sequence + 1u);
    return CardDb_Commit(next_records, count, sequence);
}

app_status_t Svc_CardDb_GetSnapshot(svc_card_db_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    *snapshot = s_snapshot;
    return APP_OK;
}

static uint16_t CardDb_Crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFu;
    uint16_t index;
    uint8_t bit;

    for (index = 0u; index < length; index++)
    {
        crc ^= (uint16_t)data[index] << 8;
        for (bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static uint32_t CardDb_ReadLe32(const uint8_t *data)
{
    return ((uint32_t)data[0]) |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void CardDb_WriteLe32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value & 0xFFu);
    data[1] = (uint8_t)((value >> 8) & 0xFFu);
    data[2] = (uint8_t)((value >> 16) & 0xFFu);
    data[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint16_t CardDb_ReadLe16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void CardDb_WriteLe16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value & 0xFFu);
    data[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static uint8_t CardDb_UidEqual(const drv_rfid_card_t *card, const svc_card_db_record_t *record)
{
    if ((card == NULL) || (record == NULL) || (card->uid_len != record->uid_len))
    {
        return 0u;
    }

    return (memcmp(card->uid, record->uid, card->uid_len) == 0) ? 1u : 0u;
}

static app_status_t CardDb_LoadSlot(uint8_t offset,
                                    svc_card_db_record_t *records,
                                    uint8_t *count,
                                    uint32_t *sequence)
{
    uint8_t buffer[CARD_DB_SLOT_SIZE];
    uint8_t record_count;
    uint8_t index;
    uint8_t record_offset;
    uint16_t stored_crc;
    uint16_t calc_crc;

    if (Bsp_EepromAt24c02_Read(offset, buffer, sizeof(buffer)) != APP_OK)
    {
        return APP_ERR_HW;
    }

    if ((buffer[0] != CARD_DB_MAGIC0) ||
        (buffer[1] != CARD_DB_MAGIC1) ||
        (buffer[2] != CARD_DB_MAGIC2) ||
        (buffer[3] != CARD_DB_MAGIC3) ||
        (buffer[4] != CARD_DB_VERSION))
    {
        return APP_ERR_NOT_READY;
    }

    stored_crc = CardDb_ReadLe16(&buffer[CARD_DB_CRC_OFFSET]);
    calc_crc = CardDb_Crc16(buffer, CARD_DB_CRC_OFFSET);
    if (stored_crc != calc_crc)
    {
        return APP_ERR_HW;
    }

    record_count = buffer[9];
    if (record_count > SVC_CARD_DB_MAX_RECORDS)
    {
        return APP_ERR_HW;
    }

    (void)memset(records, 0, sizeof(svc_card_db_record_t) * SVC_CARD_DB_MAX_RECORDS);
    for (index = 0u; index < record_count; index++)
    {
        record_offset = (uint8_t)(CARD_DB_HEADER_SIZE + (index * CARD_DB_RECORD_SIZE));
        records[index].uid_len = buffer[record_offset];
        if ((records[index].uid_len == 0u) || (records[index].uid_len > DRV_RFID_UID_MAX_LEN))
        {
            return APP_ERR_HW;
        }
        (void)memcpy(records[index].uid, &buffer[record_offset + 1u], DRV_RFID_UID_MAX_LEN);
        records[index].enabled = buffer[record_offset + 11u] != 0u ? 1u : 0u;
    }

    *count = record_count;
    *sequence = CardDb_ReadLe32(&buffer[5]);
    return APP_OK;
}

static void CardDb_EncodeSlot(uint8_t *buffer,
                              const svc_card_db_record_t *records,
                              uint8_t count,
                              uint32_t sequence)
{
    uint8_t index;
    uint8_t record_offset;
    uint16_t crc;

    (void)memset(buffer, 0xFF, CARD_DB_SLOT_SIZE);
    buffer[0] = CARD_DB_MAGIC0;
    buffer[1] = CARD_DB_MAGIC1;
    buffer[2] = CARD_DB_MAGIC2;
    buffer[3] = CARD_DB_MAGIC3;
    buffer[4] = CARD_DB_VERSION;
    CardDb_WriteLe32(&buffer[5], sequence);
    buffer[9] = count;
    buffer[10] = SVC_CARD_DB_MAX_RECORDS;
    buffer[11] = 0u;

    for (index = 0u; index < count; index++)
    {
        record_offset = (uint8_t)(CARD_DB_HEADER_SIZE + (index * CARD_DB_RECORD_SIZE));
        buffer[record_offset] = records[index].uid_len;
        (void)memcpy(&buffer[record_offset + 1u], records[index].uid, DRV_RFID_UID_MAX_LEN);
        buffer[record_offset + 11u] = records[index].enabled != 0u ? 1u : 0u;
    }

    crc = CardDb_Crc16(buffer, CARD_DB_CRC_OFFSET);
    CardDb_WriteLe16(&buffer[CARD_DB_CRC_OFFSET], crc);
}

static app_status_t CardDb_Commit(const svc_card_db_record_t *records, uint8_t count, uint32_t sequence)
{
    uint8_t buffer[CARD_DB_SLOT_SIZE];
    uint8_t slot_offset;
    app_status_t ret;

    if ((records == NULL) || (count > SVC_CARD_DB_MAX_RECORDS))
    {
        return APP_ERR_INVALID_ARG;
    }

    CardDb_EncodeSlot(buffer, records, count, sequence);
    slot_offset = ((sequence & 1u) == 0u) ? CARD_DB_SLOT0_OFFSET : CARD_DB_SLOT1_OFFSET;
    ret = Bsp_EepromAt24c02_Write(slot_offset, buffer, sizeof(buffer));
    if (ret != APP_OK)
    {
        s_snapshot.last_status = ret;
        return ret;
    }

    CardDb_ApplyRecords(records, count, sequence);
    return APP_OK;
}

static void CardDb_ApplyRecords(const svc_card_db_record_t *records, uint8_t count, uint32_t sequence)
{
    (void)memset(s_records, 0, sizeof(s_records));
    if ((records != NULL) && (count != 0u))
    {
        (void)memcpy(s_records, records, sizeof(svc_card_db_record_t) * count);
    }

    s_snapshot.ready = 1u;
    s_snapshot.empty = (count == 0u) ? 1u : 0u;
    s_snapshot.count = count;
    s_snapshot.max_records = SVC_CARD_DB_MAX_RECORDS;
    s_snapshot.sequence = sequence;
    s_snapshot.last_status = APP_OK;
}

static void CardDb_SetUnavailable(app_status_t status)
{
    s_snapshot.ready = 0u;
    s_snapshot.empty = 0u;
    s_snapshot.count = 0u;
    s_snapshot.max_records = SVC_CARD_DB_MAX_RECORDS;
    s_snapshot.last_status = status;
}
