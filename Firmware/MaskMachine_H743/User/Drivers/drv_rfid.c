#include "drv_rfid.h"

#include <string.h>

#include "bsp_rc522_port.h"
#include "stm32h7xx_hal.h"

#define RC522_MAX_FRAME_LEN         18u

#define RC522_CMD_IDLE              0x00u
#define RC522_CMD_AUTHENT           0x0Eu
#define RC522_CMD_TRANSCEIVE        0x0Cu
#define RC522_CMD_RESETPHASE        0x0Fu
#define RC522_CMD_CALCCRC           0x03u

#define RC522_PICC_REQIDL           0x26u
#define RC522_PICC_ANTICOLL1        0x93u
#define RC522_PICC_HALT             0x50u

#define RC522_REG_COMMAND           0x01u
#define RC522_REG_COMIEN            0x02u
#define RC522_REG_COMIRQ            0x04u
#define RC522_REG_DIVIRQ            0x05u
#define RC522_REG_ERROR             0x06u
#define RC522_REG_STATUS2           0x08u
#define RC522_REG_FIFO_DATA         0x09u
#define RC522_REG_FIFO_LEVEL        0x0Au
#define RC522_REG_CONTROL           0x0Cu
#define RC522_REG_BIT_FRAMING       0x0Du
#define RC522_REG_COLL              0x0Eu
#define RC522_REG_MODE              0x11u
#define RC522_REG_TX_CONTROL        0x14u
#define RC522_REG_TX_AUTO           0x15u
#define RC522_REG_CRC_RESULT_M      0x21u
#define RC522_REG_CRC_RESULT_L      0x22u
#define RC522_REG_T_MODE            0x2Au
#define RC522_REG_T_PRESCALER       0x2Bu
#define RC522_REG_T_RELOAD_H        0x2Cu
#define RC522_REG_T_RELOAD_L        0x2Du
#define RC522_REG_VERSION           0x37u

typedef enum
{
    RC522_RES_OK = 0,
    RC522_RES_NO_TAG,
    RC522_RES_ERR
} rc522_result_t;

static uint8_t s_initialized;
static uint8_t s_card_latched;
static uint8_t s_cached_uid[DRV_RFID_UID_MAX_LEN];
static uint8_t s_cached_uid_len;

static app_status_t Rc522_WriteReg(uint8_t reg, uint8_t value);
static app_status_t Rc522_ReadReg(uint8_t reg, uint8_t *value);
static app_status_t Rc522_SetBitMask(uint8_t reg, uint8_t mask);
static app_status_t Rc522_ClearBitMask(uint8_t reg, uint8_t mask);
static app_status_t Rc522_CalculateCrc(const uint8_t *data, uint8_t length, uint8_t *crc_out);
static rc522_result_t Rc522_Transceive(uint8_t command,
                                       const uint8_t *tx_data,
                                       uint8_t tx_length,
                                       uint8_t *rx_data,
                                       uint16_t *rx_bits);
static rc522_result_t Rc522_Request(uint8_t req_code, uint8_t *tag_type);
static rc522_result_t Rc522_Anticoll(uint8_t *uid_out);
static rc522_result_t Rc522_Select(const uint8_t *uid);
static void Rc522_Halt(void);
static app_status_t Rc522_AntennaOn(void);
static void Rc522_ClearCachedCard(void);

app_status_t Drv_Rfid_Init(void)
{
    uint8_t version = 0u;
    app_status_t ret;

    if (s_initialized != 0u)
    {
        return APP_OK;
    }

    ret = Bsp_Rc522Port_Init();
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Bsp_Rc522Port_Reset();
    if (ret != APP_OK)
    {
        return ret;
    }

    HAL_Delay(5u);

    ret = Rc522_WriteReg(RC522_REG_COMMAND, RC522_CMD_RESETPHASE);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_MODE, 0x3Du);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_T_RELOAD_L, 30u);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_T_RELOAD_H, 0u);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_T_MODE, 0x8Du);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_T_PRESCALER, 0x3Eu);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_TX_AUTO, 0x40u);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_AntennaOn();
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_ReadReg(RC522_REG_VERSION, &version);
    if (ret != APP_OK)
    {
        return ret;
    }

    if ((version == 0x00u) || (version == 0xFFu))
    {
        return APP_ERR_HW;
    }

    s_initialized = 1u;
    Rc522_ClearCachedCard();
    return APP_OK;
}

app_status_t Drv_Rfid_Poll(drv_rfid_card_t *card)
{
    uint8_t tag_type[2] = {0};
    uint8_t uid[5] = {0};
    rc522_result_t rc522_ret;
    app_status_t ret;

    if (card == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(card, 0, sizeof(*card));

    if (s_initialized == 0u)
    {
        ret = Drv_Rfid_Init();
        if (ret != APP_OK)
        {
            return ret;
        }
    }

    rc522_ret = Rc522_Request(RC522_PICC_REQIDL, tag_type);
    if (rc522_ret == RC522_RES_NO_TAG)
    {
        Rc522_ClearCachedCard();
        return APP_OK;
    }

    if (rc522_ret != RC522_RES_OK)
    {
        return APP_ERR_HW;
    }

    rc522_ret = Rc522_Anticoll(uid);
    if (rc522_ret != RC522_RES_OK)
    {
        Rc522_ClearCachedCard();
        return APP_ERR_HW;
    }

    if (Rc522_Select(uid) != RC522_RES_OK)
    {
        Rc522_ClearCachedCard();
        return APP_ERR_HW;
    }

    Rc522_Halt();

    if ((s_card_latched != 0u) &&
        (s_cached_uid_len == 4u) &&
        (memcmp(s_cached_uid, uid, 4u) == 0))
    {
        return APP_OK;
    }

    card->present = 1u;
    card->uid_len = 4u;
    (void)memcpy(card->uid, uid, 4u);

    s_card_latched = 1u;
    s_cached_uid_len = 4u;
    (void)memcpy(s_cached_uid, uid, 4u);
    return APP_OK;
}

static app_status_t Rc522_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2];

    tx_data[0] = (uint8_t)((reg << 1) & 0x7Eu);
    tx_data[1] = value;
    return Bsp_Rc522Port_Transfer(tx_data, NULL, 2u);
}

static app_status_t Rc522_ReadReg(uint8_t reg, uint8_t *value)
{
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    app_status_t ret;

    if (value == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    tx_data[0] = (uint8_t)(((reg << 1) & 0x7Eu) | 0x80u);
    tx_data[1] = 0u;

    ret = Bsp_Rc522Port_Transfer(tx_data, rx_data, 2u);
    if (ret != APP_OK)
    {
        return ret;
    }

    *value = rx_data[1];
    return APP_OK;
}

static app_status_t Rc522_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t value = 0u;
    app_status_t ret;

    ret = Rc522_ReadReg(reg, &value);
    if (ret != APP_OK)
    {
        return ret;
    }

    return Rc522_WriteReg(reg, (uint8_t)(value | mask));
}

static app_status_t Rc522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t value = 0u;
    app_status_t ret;

    ret = Rc522_ReadReg(reg, &value);
    if (ret != APP_OK)
    {
        return ret;
    }

    return Rc522_WriteReg(reg, (uint8_t)(value & (uint8_t)(~mask)));
}

static app_status_t Rc522_CalculateCrc(const uint8_t *data, uint8_t length, uint8_t *crc_out)
{
    uint32_t timeout = 0xFFu;
    uint8_t irq = 0u;
    uint8_t index;
    app_status_t ret;

    if ((data == NULL) || (crc_out == NULL))
    {
        return APP_ERR_INVALID_ARG;
    }

    ret = Rc522_ClearBitMask(RC522_REG_DIVIRQ, 0x04u);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_WriteReg(RC522_REG_COMMAND, RC522_CMD_IDLE);
    if (ret != APP_OK)
    {
        return ret;
    }

    ret = Rc522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80u);
    if (ret != APP_OK)
    {
        return ret;
    }

    for (index = 0u; index < length; index++)
    {
        ret = Rc522_WriteReg(RC522_REG_FIFO_DATA, data[index]);
        if (ret != APP_OK)
        {
            return ret;
        }
    }

    ret = Rc522_WriteReg(RC522_REG_COMMAND, RC522_CMD_CALCCRC);
    if (ret != APP_OK)
    {
        return ret;
    }

    do
    {
        ret = Rc522_ReadReg(RC522_REG_DIVIRQ, &irq);
        if (ret != APP_OK)
        {
            return ret;
        }
        timeout--;
    } while ((timeout > 0u) && ((irq & 0x04u) == 0u));

    if (timeout == 0u)
    {
        return APP_ERR_TIMEOUT;
    }

    ret = Rc522_ReadReg(RC522_REG_CRC_RESULT_L, &crc_out[0]);
    if (ret != APP_OK)
    {
        return ret;
    }

    return Rc522_ReadReg(RC522_REG_CRC_RESULT_M, &crc_out[1]);
}

static rc522_result_t Rc522_Transceive(uint8_t command,
                                       const uint8_t *tx_data,
                                       uint8_t tx_length,
                                       uint8_t *rx_data,
                                       uint16_t *rx_bits)
{
    uint8_t irq_en = 0u;
    uint8_t wait_for = 0u;
    uint8_t last_bits = 0u;
    uint8_t irq = 0u;
    uint8_t error = 0u;
    uint8_t fifo_level = 0u;
    uint16_t timeout = 600u;
    uint8_t index;

    if ((tx_data == NULL) || (rx_bits == NULL))
    {
        return RC522_RES_ERR;
    }

    switch (command)
    {
        case RC522_CMD_AUTHENT:
            irq_en = 0x12u;
            wait_for = 0x10u;
            break;

        case RC522_CMD_TRANSCEIVE:
            irq_en = 0x77u;
            wait_for = 0x30u;
            break;

        default:
            return RC522_RES_ERR;
    }

    if (Rc522_WriteReg(RC522_REG_COMIEN, (uint8_t)(irq_en | 0x80u)) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_ClearBitMask(RC522_REG_COMIRQ, 0x80u) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_WriteReg(RC522_REG_COMMAND, RC522_CMD_IDLE) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80u) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    for (index = 0u; index < tx_length; index++)
    {
        if (Rc522_WriteReg(RC522_REG_FIFO_DATA, tx_data[index]) != APP_OK)
        {
            return RC522_RES_ERR;
        }
    }

    if (Rc522_WriteReg(RC522_REG_COMMAND, command) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (command == RC522_CMD_TRANSCEIVE)
    {
        if (Rc522_SetBitMask(RC522_REG_BIT_FRAMING, 0x80u) != APP_OK)
        {
            return RC522_RES_ERR;
        }
    }

    do
    {
        if (Rc522_ReadReg(RC522_REG_COMIRQ, &irq) != APP_OK)
        {
            return RC522_RES_ERR;
        }
        timeout--;
    } while ((timeout > 0u) && ((irq & 0x01u) == 0u) && ((irq & wait_for) == 0u));

    if (Rc522_ClearBitMask(RC522_REG_BIT_FRAMING, 0x80u) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (timeout == 0u)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_ReadReg(RC522_REG_ERROR, &error) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if ((error & 0x1Bu) != 0u)
    {
        return RC522_RES_ERR;
    }

    if ((irq_en & irq & 0x01u) != 0u)
    {
        return RC522_RES_NO_TAG;
    }

    if (command != RC522_CMD_TRANSCEIVE)
    {
        *rx_bits = 0u;
        return RC522_RES_OK;
    }

    if ((Rc522_ReadReg(RC522_REG_FIFO_LEVEL, &fifo_level) != APP_OK) ||
        (Rc522_ReadReg(RC522_REG_CONTROL, &last_bits) != APP_OK))
    {
        return RC522_RES_ERR;
    }

    last_bits &= 0x07u;
    if (last_bits != 0u)
    {
        *rx_bits = (uint16_t)(((uint16_t)fifo_level - 1u) * 8u + last_bits);
    }
    else
    {
        *rx_bits = (uint16_t)fifo_level * 8u;
    }

    if (fifo_level == 0u)
    {
        fifo_level = 1u;
    }
    else if (fifo_level > RC522_MAX_FRAME_LEN)
    {
        fifo_level = RC522_MAX_FRAME_LEN;
    }

    if (rx_data != NULL)
    {
        for (index = 0u; index < fifo_level; index++)
        {
            if (Rc522_ReadReg(RC522_REG_FIFO_DATA, &rx_data[index]) != APP_OK)
            {
                return RC522_RES_ERR;
            }
        }
    }

    (void)Rc522_SetBitMask(RC522_REG_CONTROL, 0x80u);
    (void)Rc522_WriteReg(RC522_REG_COMMAND, RC522_CMD_IDLE);
    return RC522_RES_OK;
}

static rc522_result_t Rc522_Request(uint8_t req_code, uint8_t *tag_type)
{
    uint16_t rx_bits = 0u;
    uint8_t buffer[RC522_MAX_FRAME_LEN] = {0};
    rc522_result_t rc522_ret;

    if (tag_type == NULL)
    {
        return RC522_RES_ERR;
    }

    if ((Rc522_ClearBitMask(RC522_REG_STATUS2, 0x08u) != APP_OK) ||
        (Rc522_WriteReg(RC522_REG_BIT_FRAMING, 0x07u) != APP_OK) ||
        (Rc522_SetBitMask(RC522_REG_TX_CONTROL, 0x03u) != APP_OK))
    {
        return RC522_RES_ERR;
    }

    buffer[0] = req_code;
    rc522_ret = Rc522_Transceive(RC522_CMD_TRANSCEIVE, buffer, 1u, buffer, &rx_bits);
    if (rc522_ret != RC522_RES_OK)
    {
        return rc522_ret;
    }

    if (rx_bits != 0x10u)
    {
        return RC522_RES_ERR;
    }

    tag_type[0] = buffer[0];
    tag_type[1] = buffer[1];
    return RC522_RES_OK;
}

static rc522_result_t Rc522_Anticoll(uint8_t *uid_out)
{
    uint16_t rx_bits = 0u;
    uint8_t buffer[RC522_MAX_FRAME_LEN] = {0};
    uint8_t check = 0u;
    uint8_t index;

    if (uid_out == NULL)
    {
        return RC522_RES_ERR;
    }

    if ((Rc522_ClearBitMask(RC522_REG_STATUS2, 0x08u) != APP_OK) ||
        (Rc522_WriteReg(RC522_REG_BIT_FRAMING, 0x00u) != APP_OK) ||
        (Rc522_ClearBitMask(RC522_REG_COLL, 0x80u) != APP_OK))
    {
        return RC522_RES_ERR;
    }

    buffer[0] = RC522_PICC_ANTICOLL1;
    buffer[1] = 0x20u;
    if (Rc522_Transceive(RC522_CMD_TRANSCEIVE, buffer, 2u, buffer, &rx_bits) != RC522_RES_OK)
    {
        return RC522_RES_ERR;
    }

    for (index = 0u; index < 4u; index++)
    {
        uid_out[index] = buffer[index];
        check ^= buffer[index];
    }

    (void)Rc522_SetBitMask(RC522_REG_COLL, 0x80u);
    return (check == buffer[4]) ? RC522_RES_OK : RC522_RES_ERR;
}

static rc522_result_t Rc522_Select(const uint8_t *uid)
{
    uint16_t rx_bits = 0u;
    uint8_t buffer[RC522_MAX_FRAME_LEN] = {0};
    uint8_t index;

    if (uid == NULL)
    {
        return RC522_RES_ERR;
    }

    buffer[0] = RC522_PICC_ANTICOLL1;
    buffer[1] = 0x70u;
    buffer[6] = 0u;

    for (index = 0u; index < 4u; index++)
    {
        buffer[index + 2u] = uid[index];
        buffer[6] ^= uid[index];
    }

    if (Rc522_CalculateCrc(buffer, 7u, &buffer[7]) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_ClearBitMask(RC522_REG_STATUS2, 0x08u) != APP_OK)
    {
        return RC522_RES_ERR;
    }

    if (Rc522_Transceive(RC522_CMD_TRANSCEIVE, buffer, 9u, buffer, &rx_bits) != RC522_RES_OK)
    {
        return RC522_RES_ERR;
    }

    return (rx_bits == 0x18u) ? RC522_RES_OK : RC522_RES_ERR;
}

static void Rc522_Halt(void)
{
    uint16_t rx_bits = 0u;
    uint8_t buffer[4] = {0};

    buffer[0] = RC522_PICC_HALT;
    buffer[1] = 0u;

    if (Rc522_CalculateCrc(buffer, 2u, &buffer[2]) != APP_OK)
    {
        return;
    }

    (void)Rc522_Transceive(RC522_CMD_TRANSCEIVE, buffer, 4u, buffer, &rx_bits);
}

static app_status_t Rc522_AntennaOn(void)
{
    uint8_t value = 0u;
    app_status_t ret;

    ret = Rc522_ReadReg(RC522_REG_TX_CONTROL, &value);
    if (ret != APP_OK)
    {
        return ret;
    }

    if ((value & 0x03u) == 0u)
    {
        return Rc522_SetBitMask(RC522_REG_TX_CONTROL, 0x03u);
    }

    return APP_OK;
}

static void Rc522_ClearCachedCard(void)
{
    s_card_latched = 0u;
    s_cached_uid_len = 0u;
    (void)memset(s_cached_uid, 0, sizeof(s_cached_uid));
}
