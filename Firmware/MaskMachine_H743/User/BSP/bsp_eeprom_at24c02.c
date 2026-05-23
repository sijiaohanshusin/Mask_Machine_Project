#include "bsp_eeprom_at24c02.h"

#include <string.h>

#include "stm32h7xx_hal.h"

#define AT24C02_ADDR_WRITE          0xA0u
#define AT24C02_ADDR_READ           0xA1u
#define AT24C02_ACK_POLL_ATTEMPTS   600u

#define AT24C02_SCL_PORT            GPIOH
#define AT24C02_SCL_PIN             GPIO_PIN_4
#define AT24C02_SDA_PORT            GPIOH
#define AT24C02_SDA_PIN             GPIO_PIN_5

static uint8_t s_eeprom_ready;

static void Eeprom_I2c_InitPins(void);
static void Eeprom_I2c_RecoverBus(void);
static void Eeprom_I2c_Delay(void);
static void Eeprom_I2c_WriteCycleDelay(void);
static void Eeprom_I2c_Scl(uint8_t level);
static void Eeprom_I2c_Sda(uint8_t level);
static uint8_t Eeprom_I2c_ReadSda(void);
static void Eeprom_I2c_Start(void);
static void Eeprom_I2c_Stop(void);
static uint8_t Eeprom_I2c_WriteByte(uint8_t byte);
static uint8_t Eeprom_I2c_ReadByte(uint8_t ack);
static app_status_t Eeprom_I2c_AckPoll(void);
static app_status_t Eeprom_I2c_Probe(void);

app_status_t Bsp_EepromAt24c02_Init(void)
{
    app_status_t ret;

    if (s_eeprom_ready != 0u)
    {
        return APP_OK;
    }

    Eeprom_I2c_InitPins();
    Eeprom_I2c_RecoverBus();

    ret = Eeprom_I2c_Probe();
    if (ret != APP_OK)
    {
        return ret;
    }

    s_eeprom_ready = 1u;
    return APP_OK;
}

app_status_t Bsp_EepromAt24c02_Read(uint8_t offset, uint8_t *data, uint16_t length)
{
    uint16_t index;

    if ((data == NULL) || (((uint16_t)offset + length) > BSP_EEPROM_AT24C02_SIZE_BYTES))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EepromAt24c02_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    if (length == 0u)
    {
        return APP_OK;
    }

    Eeprom_I2c_Start();
    if ((Eeprom_I2c_WriteByte(AT24C02_ADDR_WRITE) == 0u) ||
        (Eeprom_I2c_WriteByte(offset) == 0u))
    {
        Eeprom_I2c_Stop();
        return APP_ERR_TIMEOUT;
    }

    Eeprom_I2c_Start();
    if (Eeprom_I2c_WriteByte(AT24C02_ADDR_READ) == 0u)
    {
        Eeprom_I2c_Stop();
        return APP_ERR_TIMEOUT;
    }

    for (index = 0u; index < length; index++)
    {
        data[index] = Eeprom_I2c_ReadByte((index + 1u) < length ? 1u : 0u);
    }

    Eeprom_I2c_Stop();
    return APP_OK;
}

app_status_t Bsp_EepromAt24c02_Write(uint8_t offset, const uint8_t *data, uint16_t length)
{
    uint16_t written = 0u;
    uint16_t chunk;
    uint16_t page_left;
    uint16_t index;
    uint8_t current_offset;

    if ((data == NULL) || (((uint16_t)offset + length) > BSP_EEPROM_AT24C02_SIZE_BYTES))
    {
        return APP_ERR_INVALID_ARG;
    }

    if (Bsp_EepromAt24c02_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    while (written < length)
    {
        current_offset = (uint8_t)(offset + written);
        page_left = (uint16_t)(BSP_EEPROM_AT24C02_PAGE_BYTES -
                               (current_offset % BSP_EEPROM_AT24C02_PAGE_BYTES));
        chunk = (uint16_t)(length - written);
        if (chunk > page_left)
        {
            chunk = page_left;
        }

        Eeprom_I2c_Start();
        if ((Eeprom_I2c_WriteByte(AT24C02_ADDR_WRITE) == 0u) ||
            (Eeprom_I2c_WriteByte(current_offset) == 0u))
        {
            Eeprom_I2c_Stop();
            return APP_ERR_TIMEOUT;
        }

        for (index = 0u; index < chunk; index++)
        {
            if (Eeprom_I2c_WriteByte(data[written + index]) == 0u)
            {
                Eeprom_I2c_Stop();
                return APP_ERR_TIMEOUT;
            }
        }

        Eeprom_I2c_Stop();
        if (Eeprom_I2c_AckPoll() != APP_OK)
        {
            return APP_ERR_TIMEOUT;
        }

        written = (uint16_t)(written + chunk);
    }

    return APP_OK;
}

static void Eeprom_I2c_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOH_CLK_ENABLE();

    (void)memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));
    GPIO_InitStruct.Pin = AT24C02_SCL_PIN | AT24C02_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    Eeprom_I2c_Scl(1u);
    Eeprom_I2c_Sda(1u);
}

static void Eeprom_I2c_RecoverBus(void)
{
    uint8_t i;

    Eeprom_I2c_Sda(1u);
    for (i = 0u; i < 9u; i++)
    {
        Eeprom_I2c_Scl(0u);
        Eeprom_I2c_Delay();
        Eeprom_I2c_Scl(1u);
        Eeprom_I2c_Delay();
    }
    Eeprom_I2c_Stop();
}

static void Eeprom_I2c_Delay(void)
{
    volatile uint32_t delay;

    for (delay = 0u; delay < 1800u; delay++)
    {
        __NOP();
    }
}

static void Eeprom_I2c_WriteCycleDelay(void)
{
    volatile uint32_t delay;

    for (delay = 0u; delay < 5000u; delay++)
    {
        __NOP();
    }
}

static void Eeprom_I2c_Scl(uint8_t level)
{
    HAL_GPIO_WritePin(AT24C02_SCL_PORT,
                      AT24C02_SCL_PIN,
                      (level != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Eeprom_I2c_Sda(uint8_t level)
{
    HAL_GPIO_WritePin(AT24C02_SDA_PORT,
                      AT24C02_SDA_PIN,
                      (level != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t Eeprom_I2c_ReadSda(void)
{
    return (HAL_GPIO_ReadPin(AT24C02_SDA_PORT, AT24C02_SDA_PIN) == GPIO_PIN_SET) ? 1u : 0u;
}

static void Eeprom_I2c_Start(void)
{
    Eeprom_I2c_Sda(1u);
    Eeprom_I2c_Scl(1u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Sda(0u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Scl(0u);
}

static void Eeprom_I2c_Stop(void)
{
    Eeprom_I2c_Sda(0u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Scl(1u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Sda(1u);
    Eeprom_I2c_Delay();
}

static uint8_t Eeprom_I2c_WriteByte(uint8_t byte)
{
    uint8_t mask;
    uint8_t ack;

    for (mask = 0x80u; mask != 0u; mask >>= 1)
    {
        Eeprom_I2c_Sda((byte & mask) != 0u ? 1u : 0u);
        Eeprom_I2c_Delay();
        Eeprom_I2c_Scl(1u);
        Eeprom_I2c_Delay();
        Eeprom_I2c_Scl(0u);
    }

    Eeprom_I2c_Sda(1u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Scl(1u);
    Eeprom_I2c_Delay();
    ack = (Eeprom_I2c_ReadSda() == 0u) ? 1u : 0u;
    Eeprom_I2c_Scl(0u);
    return ack;
}

static uint8_t Eeprom_I2c_ReadByte(uint8_t ack)
{
    uint8_t value = 0u;
    uint8_t i;

    Eeprom_I2c_Sda(1u);
    for (i = 0u; i < 8u; i++)
    {
        value <<= 1;
        Eeprom_I2c_Delay();
        Eeprom_I2c_Scl(1u);
        Eeprom_I2c_Delay();
        if (Eeprom_I2c_ReadSda() != 0u)
        {
            value |= 1u;
        }
        Eeprom_I2c_Scl(0u);
    }

    Eeprom_I2c_Sda((ack != 0u) ? 0u : 1u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Scl(1u);
    Eeprom_I2c_Delay();
    Eeprom_I2c_Scl(0u);
    Eeprom_I2c_Sda(1u);
    return value;
}

static app_status_t Eeprom_I2c_AckPoll(void)
{
    uint16_t attempt;
    uint8_t ack;

    for (attempt = 0u; attempt < AT24C02_ACK_POLL_ATTEMPTS; attempt++)
    {
        Eeprom_I2c_Start();
        ack = Eeprom_I2c_WriteByte(AT24C02_ADDR_WRITE);
        Eeprom_I2c_Stop();
        if (ack != 0u)
        {
            return APP_OK;
        }
        Eeprom_I2c_WriteCycleDelay();
    }

    return APP_ERR_TIMEOUT;
}

static app_status_t Eeprom_I2c_Probe(void)
{
    uint8_t ack;

    Eeprom_I2c_Start();
    ack = Eeprom_I2c_WriteByte(AT24C02_ADDR_WRITE);
    Eeprom_I2c_Stop();
    return (ack != 0u) ? APP_OK : APP_ERR_NOT_READY;
}
