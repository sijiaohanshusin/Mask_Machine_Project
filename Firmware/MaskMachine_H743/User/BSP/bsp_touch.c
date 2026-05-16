#include "bsp_touch.h"

#include <string.h>

#include "bsp_lcd.h"
#include "bsp_log.h"
#include "stm32h7xx_hal.h"

#define GT9XXX_ADDR0_WRITE      (0x28U)
#define GT9XXX_ADDR0_READ       (0x29U)
#define GT9XXX_ADDR1_WRITE      (0xBAU)
#define GT9XXX_ADDR1_READ       (0xBBU)
#define GT9XXX_REG_CTRL         (0x8040U)
#define GT9XXX_REG_PID          (0x8140U)
#define GT9XXX_REG_STATUS       (0x814EU)
#define GT9XXX_REG_POINT1       (0x8150U)
#define GT9XXX_MAX_POINTS       (5U)
#define GT9XXX_POINT_BYTES      (8U)

static uint8_t s_ready;
static uint8_t s_addr_write = GT9XXX_ADDR0_WRITE;
static uint8_t s_addr_read = GT9XXX_ADDR0_READ;
static uint8_t s_last_pressed;
static char s_product_id[5];
static uint32_t s_read_count;
static uint8_t s_last_status_log = 0xFFu;
static uint8_t s_dwt_ready;

static void Touch_I2c_InitPins(void);
static void Touch_I2c_ConfigIntPin(uint32_t pull);
static void Touch_I2c_RecoverBus(void);
static void Touch_I2c_Delay(void);
static void Touch_I2c_Scl(uint8_t level);
static void Touch_I2c_Sda(uint8_t level);
static uint8_t Touch_I2c_ReadSda(void);
static uint8_t Touch_I2c_ReadInt(void);
static void Touch_I2c_Start(void);
static void Touch_I2c_Stop(void);
static uint8_t Touch_I2c_WriteByte(uint8_t byte);
static uint8_t Touch_I2c_ReadByte(uint8_t ack);
static uint8_t Gt9xxx_WriteReg(uint16_t reg, const uint8_t *data, uint8_t len);
static uint8_t Gt9xxx_ReadReg(uint16_t reg, uint8_t *data, uint8_t len);
static uint8_t Gt9xxx_ReadPidAt(uint8_t write_addr, uint8_t read_addr, char *pid);
static void Gt9xxx_ResetPulse(void);
static uint8_t Gt9xxx_MapPoint(uint16_t raw_x, uint16_t raw_y, uint16_t *x, uint16_t *y);
static uint8_t Gt9xxx_TryMap(uint16_t cand_x, uint16_t cand_y, uint16_t *x, uint16_t *y, uint8_t *valid);

mm_status_t Bsp_Touch_Init(void)
{
    uint8_t ctrl;
    uint8_t clear = 0u;

    Touch_I2c_InitPins();
    Gt9xxx_ResetPulse();

    (void)memset(s_product_id, 0, sizeof(s_product_id));
    if (Gt9xxx_ReadPidAt(GT9XXX_ADDR0_WRITE, GT9XXX_ADDR0_READ, s_product_id) != 0u)
    {
        (void)Bsp_Log_Printf("[touch] no ACK at addr 0x%02X, trying 0x%02X\r\n",
                             GT9XXX_ADDR0_WRITE,
                             GT9XXX_ADDR1_WRITE);
        if (Gt9xxx_ReadPidAt(GT9XXX_ADDR1_WRITE, GT9XXX_ADDR1_READ, s_product_id) != 0u)
        {
            s_ready = 0u;
            (void)Bsp_Log_Printf("[touch] GT9XXX PID read failed on both addresses\r\n");
            return MM_ERR_NOT_READY;
        }
    }

    (void)Bsp_Log_Printf("[touch] GT9XXX PID %02X %02X %02X %02X addr 0x%02X\r\n",
                         (unsigned int)(uint8_t)s_product_id[0],
                         (unsigned int)(uint8_t)s_product_id[1],
                         (unsigned int)(uint8_t)s_product_id[2],
                         (unsigned int)(uint8_t)s_product_id[3],
                         (unsigned int)s_addr_write);

    ctrl = 0x02u;
    (void)Gt9xxx_WriteReg(GT9XXX_REG_CTRL, &ctrl, 1U);
    HAL_Delay(10U);

    ctrl = 0x00u;
    (void)Gt9xxx_WriteReg(GT9XXX_REG_CTRL, &ctrl, 1U);
    HAL_Delay(10U);

    (void)Gt9xxx_WriteReg(GT9XXX_REG_STATUS, &clear, 1U);

    s_ready = 1u;
    return MM_OK;
}

mm_status_t Bsp_Touch_Read(bsp_touch_state_t *state)
{
    uint8_t status;
    uint8_t point[GT9XXX_POINT_BYTES];
    uint8_t clear = 0u;
    uint8_t point_count;
    uint16_t raw_x;
    uint16_t raw_y;

    if (state == NULL)
    {
        return MM_ERR_INVALID_ARG;
    }

    (void)memset(state, 0, sizeof(*state));
    state->ready = s_ready;
    (void)memcpy(state->product_id, s_product_id, sizeof(state->product_id));

    if (s_ready == 0u)
    {
        return MM_ERR_NOT_READY;
    }

    if (Gt9xxx_ReadReg(GT9XXX_REG_STATUS, &status, 1U) != 0u)
    {
        return MM_ERR_HW;
    }
    state->status = status;
    s_read_count++;

    if ((status != s_last_status_log) || ((s_read_count % 100u) == 0u))
    {
        s_last_status_log = status;
        (void)Bsp_Log_Printf("[touch] poll status=0x%02X int=%u\r\n",
                             (unsigned int)status,
                             (unsigned int)Touch_I2c_ReadInt());
    }

    if ((status & 0x80u) == 0u)
    {
        s_last_pressed = 0u;
        return MM_OK;
    }

    point_count = status & 0x0Fu;
    if (point_count > GT9XXX_MAX_POINTS)
    {
        (void)Gt9xxx_WriteReg(GT9XXX_REG_STATUS, &clear, 1U);
        s_last_pressed = 0u;
        return MM_OK;
    }

    if (point_count == 0u)
    {
        (void)Gt9xxx_WriteReg(GT9XXX_REG_STATUS, &clear, 1U);
        (void)Bsp_Log_Printf("[touch] ready with zero points status=0x%02X int=%u\r\n",
                             (unsigned int)status,
                             (unsigned int)Touch_I2c_ReadInt());
        s_last_pressed = 0u;
        return MM_OK;
    }

    if (Gt9xxx_ReadReg(GT9XXX_REG_POINT1, point, sizeof(point)) != 0u)
    {
        return MM_ERR_HW;
    }
    (void)Gt9xxx_WriteReg(GT9XXX_REG_STATUS, &clear, 1U);

    if (s_last_pressed == 0u)
    {
        (void)Bsp_Log_Printf("[touch] data n=%u bytes=%02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                             (unsigned int)point_count,
                             (unsigned int)point[0],
                             (unsigned int)point[1],
                             (unsigned int)point[2],
                             (unsigned int)point[3],
                             (unsigned int)point[4],
                             (unsigned int)point[5],
                             (unsigned int)point[6],
                             (unsigned int)point[7]);
    }

    raw_x = (uint16_t)point[0] | ((uint16_t)point[1] << 8);
    raw_y = (uint16_t)point[2] | ((uint16_t)point[3] << 8);
    state->raw_x = raw_x;
    state->raw_y = raw_y;

    if (Gt9xxx_MapPoint(raw_x, raw_y, &state->x, &state->y) == 0u)
    {
        if (s_last_pressed == 0u)
        {
            (void)Bsp_Log_Printf("[touch] raw only raw=(%u,%u) status=0x%02X\r\n",
                                 (unsigned int)raw_x,
                                 (unsigned int)raw_y,
                                 (unsigned int)status);
        }
        s_last_pressed = 0u;
        return MM_OK;
    }

    state->pressed = 1u;
    state->count = point_count;
    if (s_last_pressed == 0u)
    {
        (void)Bsp_Log_Printf("[touch] press raw=(%u,%u) map=(%u,%u) status=0x%02X\r\n",
                             (unsigned int)raw_x,
                             (unsigned int)raw_y,
                             (unsigned int)state->x,
                             (unsigned int)state->y,
                             (unsigned int)status);
    }
    s_last_pressed = 1u;
    return MM_OK;
}

uint8_t Bsp_Touch_IsReady(void)
{
    return s_ready;
}

static void Touch_I2c_InitPins(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_12;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_14;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio);

    Touch_I2c_Scl(1u);
    Touch_I2c_Sda(1u);
    Touch_I2c_RecoverBus();
}

static void Touch_I2c_ConfigIntPin(uint32_t pull)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = pull;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOH, &gpio);
}

static void Touch_I2c_RecoverBus(void)
{
    uint8_t i;

    Touch_I2c_Sda(1u);
    for (i = 0u; i < 9u; i++)
    {
        Touch_I2c_Scl(0u);
        Touch_I2c_Delay();
        Touch_I2c_Scl(1u);
        Touch_I2c_Delay();
    }
    Touch_I2c_Stop();
}

static void Touch_I2c_Delay(void)
{
    uint32_t start;
    uint32_t ticks;

    if (s_dwt_ready == 0u)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        DWT->CYCCNT = 0u;
        s_dwt_ready = 1u;
    }

    ticks = (SystemCoreClock / 1000000u) * 2u;
    start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < ticks)
    {
    }
}

static void Touch_I2c_Scl(uint8_t level)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (level != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Touch_I2c_Sda(uint8_t level)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (level != 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t Touch_I2c_ReadSda(void)
{
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) ? 1u : 0u;
}

static uint8_t Touch_I2c_ReadInt(void)
{
    return (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_7) == GPIO_PIN_SET) ? 1u : 0u;
}

static void Touch_I2c_Start(void)
{
    Touch_I2c_Sda(1u);
    Touch_I2c_Scl(1u);
    Touch_I2c_Delay();
    Touch_I2c_Sda(0u);
    Touch_I2c_Delay();
    Touch_I2c_Scl(0u);
    Touch_I2c_Delay();
}

static void Touch_I2c_Stop(void)
{
    Touch_I2c_Scl(0u);
    Touch_I2c_Sda(0u);
    Touch_I2c_Delay();
    Touch_I2c_Scl(1u);
    Touch_I2c_Delay();
    Touch_I2c_Sda(1u);
    Touch_I2c_Delay();
}

static uint8_t Touch_I2c_WriteByte(uint8_t byte)
{
    uint8_t bit;
    uint8_t ack;
    uint8_t wait_time = 0u;

    for (bit = 0u; bit < 8u; bit++)
    {
        Touch_I2c_Sda((byte & 0x80u) != 0u);
        byte <<= 1;
        Touch_I2c_Delay();
        Touch_I2c_Scl(1u);
        Touch_I2c_Delay();
        Touch_I2c_Scl(0u);
        Touch_I2c_Delay();
    }

    Touch_I2c_Sda(1u);
    Touch_I2c_Delay();
    Touch_I2c_Scl(1u);
    Touch_I2c_Delay();
    while (Touch_I2c_ReadSda() != 0u)
    {
        wait_time++;
        if (wait_time > 250u)
        {
            Touch_I2c_Scl(0u);
            Touch_I2c_Stop();
            return 1u;
        }
        Touch_I2c_Delay();
    }
    ack = 0u;
    Touch_I2c_Scl(0u);
    Touch_I2c_Delay();
    return ack;
}

static uint8_t Touch_I2c_ReadByte(uint8_t ack)
{
    uint8_t bit;
    uint8_t value = 0u;

    Touch_I2c_Sda(1u);
    for (bit = 0u; bit < 8u; bit++)
    {
        value <<= 1;
        Touch_I2c_Scl(1u);
        Touch_I2c_Delay();
        if (Touch_I2c_ReadSda() != 0u)
        {
            value |= 1u;
        }
        Touch_I2c_Scl(0u);
        Touch_I2c_Delay();
    }

    Touch_I2c_Sda((ack != 0u) ? 0u : 1u);
    Touch_I2c_Delay();
    Touch_I2c_Scl(1u);
    Touch_I2c_Delay();
    Touch_I2c_Scl(0u);
    Touch_I2c_Sda(1u);
    Touch_I2c_Delay();
    return value;
}

static uint8_t Gt9xxx_WriteReg(uint16_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    Touch_I2c_Start();
    if (Touch_I2c_WriteByte(s_addr_write) != 0u)
    {
        Touch_I2c_Stop();
        return 1u;
    }

    if ((Touch_I2c_WriteByte((uint8_t)(reg >> 8)) != 0u) ||
        (Touch_I2c_WriteByte((uint8_t)(reg & 0xFFu)) != 0u))
    {
        Touch_I2c_Stop();
        return 1u;
    }

    for (i = 0u; i < len; i++)
    {
        if (Touch_I2c_WriteByte(data[i]) != 0u)
        {
            Touch_I2c_Stop();
            return 1u;
        }
    }

    Touch_I2c_Stop();
    return 0u;
}

static uint8_t Gt9xxx_ReadReg(uint16_t reg, uint8_t *data, uint8_t len)
{
    uint8_t i;

    Touch_I2c_Start();
    if (Touch_I2c_WriteByte(s_addr_write) != 0u)
    {
        Touch_I2c_Stop();
        return 1u;
    }

    if ((Touch_I2c_WriteByte((uint8_t)(reg >> 8)) != 0u) ||
        (Touch_I2c_WriteByte((uint8_t)(reg & 0xFFu)) != 0u))
    {
        Touch_I2c_Stop();
        return 1u;
    }

    Touch_I2c_Stop();
    Touch_I2c_Start();
    if (Touch_I2c_WriteByte(s_addr_read) != 0u)
    {
        Touch_I2c_Stop();
        return 1u;
    }

    for (i = 0u; i < len; i++)
    {
        data[i] = Touch_I2c_ReadByte((i + 1u) < len);
    }

    Touch_I2c_Stop();
    return 0u;
}

static uint8_t Gt9xxx_ReadPidAt(uint8_t write_addr, uint8_t read_addr, char *pid)
{
    s_addr_write = write_addr;
    s_addr_read = read_addr;
    (void)memset(pid, 0, 5U);
    return Gt9xxx_ReadReg(GT9XXX_REG_PID, (uint8_t *)pid, 4U);
}

static void Gt9xxx_ResetPulse(void)
{
    Touch_I2c_ConfigIntPin(GPIO_PULLUP);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_Delay(30U);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_Delay(30U);
    Touch_I2c_ConfigIntPin(GPIO_NOPULL);
    HAL_Delay(100U);
}

static uint8_t Gt9xxx_MapPoint(uint16_t raw_x, uint16_t raw_y, uint16_t *x, uint16_t *y)
{
    uint8_t valid = 0u;

    (void)Gt9xxx_TryMap(raw_x, raw_y, x, y, &valid);
    (void)Gt9xxx_TryMap((uint16_t)((raw_y < BSP_LCD_WIDTH) ? (BSP_LCD_WIDTH - 1U - raw_y) : BSP_LCD_WIDTH),
                        raw_x,
                        x,
                        y,
                        &valid);
    (void)Gt9xxx_TryMap(raw_y,
                        (uint16_t)((raw_x < BSP_LCD_HEIGHT) ? (BSP_LCD_HEIGHT - 1U - raw_x) : BSP_LCD_HEIGHT),
                        x,
                        y,
                        &valid);
    (void)Gt9xxx_TryMap((uint16_t)((raw_x < BSP_LCD_WIDTH) ? (BSP_LCD_WIDTH - 1U - raw_x) : BSP_LCD_WIDTH),
                        (uint16_t)((raw_y < BSP_LCD_HEIGHT) ? (BSP_LCD_HEIGHT - 1U - raw_y) : BSP_LCD_HEIGHT),
                        x,
                        y,
                        &valid);
    return valid;
}

static uint8_t Gt9xxx_TryMap(uint16_t cand_x, uint16_t cand_y, uint16_t *x, uint16_t *y, uint8_t *valid)
{
    if ((valid == NULL) || (*valid != 0u))
    {
        return 0u;
    }

    if ((cand_x < BSP_LCD_WIDTH) && (cand_y < BSP_LCD_HEIGHT))
    {
        *x = cand_x;
        *y = cand_y;
        *valid = 1u;
        return 1u;
    }

    return 0u;
}
