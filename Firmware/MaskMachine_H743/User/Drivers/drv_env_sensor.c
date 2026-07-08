#include "drv_env_sensor.h"

#include <stddef.h>
#include <string.h>

#include "bsp_env_uart.h"

#define ENV_SENSOR_FRAME_LEN       17u
#define ENV_SENSOR_HEADER_0        0x3Cu
#define ENV_SENSOR_HEADER_1        0x02u
#define ENV_SENSOR_MAX_BYTES_POLL  96u

static uint8_t s_frame[ENV_SENSOR_FRAME_LEN];
static uint8_t s_frame_index;

static app_status_t EnvSensor_PushByte(uint8_t byte, drv_env_sample_t *sample);
static app_status_t EnvSensor_DecodeFrame(const uint8_t *frame, drv_env_sample_t *sample);
static uint16_t EnvSensor_U16Be(uint8_t msb, uint8_t lsb);

app_status_t Drv_EnvSensor_Init(void)
{
    s_frame_index = 0u;
    (void)memset(s_frame, 0, sizeof(s_frame));
    return Bsp_EnvUart_Init();
}

app_status_t Drv_EnvSensor_Read(drv_env_sample_t *sample)
{
    app_status_t ret;
    app_status_t last_error = APP_ERR_TIMEOUT;
    uint8_t byte;
    uint16_t count;

    if (sample == NULL)
    {
        return APP_ERR_INVALID_ARG;
    }

    (void)memset(sample, 0, sizeof(*sample));
    sample->valid = 0u;

    if (Bsp_EnvUart_Init() != APP_OK)
    {
        return APP_ERR_NOT_READY;
    }

    for (count = 0u; count < ENV_SENSOR_MAX_BYTES_POLL; count++)
    {
        ret = Bsp_EnvUart_ReadByte(&byte);
        if (ret == APP_ERR_TIMEOUT)
        {
            break;
        }

        if (ret != APP_OK)
        {
            return ret;
        }

        ret = EnvSensor_PushByte(byte, sample);
        if (ret == APP_OK)
        {
            return APP_OK;
        }

        if (ret == APP_ERR_HW)
        {
            last_error = APP_ERR_HW;
        }
    }

    return last_error;
}

static app_status_t EnvSensor_PushByte(uint8_t byte, drv_env_sample_t *sample)
{
    if (s_frame_index == 0u)
    {
        if (byte == ENV_SENSOR_HEADER_0)
        {
            s_frame[s_frame_index++] = byte;
        }
        return APP_ERR_TIMEOUT;
    }

    if (s_frame_index == 1u)
    {
        if (byte == ENV_SENSOR_HEADER_1)
        {
            s_frame[s_frame_index++] = byte;
        }
        else
        {
            s_frame_index = (byte == ENV_SENSOR_HEADER_0) ? 1u : 0u;
            s_frame[0] = (s_frame_index == 1u) ? byte : 0u;
        }
        return APP_ERR_TIMEOUT;
    }

    s_frame[s_frame_index++] = byte;
    if (s_frame_index < ENV_SENSOR_FRAME_LEN)
    {
        return APP_ERR_TIMEOUT;
    }

    s_frame_index = 0u;
    return EnvSensor_DecodeFrame(s_frame, sample);
}

static app_status_t EnvSensor_DecodeFrame(const uint8_t *frame, drv_env_sample_t *sample)
{
    uint16_t index;
    uint8_t checksum = 0u;
    int16_t temp_x10;

    if ((frame == NULL) || (sample == NULL))
    {
        return APP_ERR_INVALID_ARG;
    }

    for (index = 0u; index < (ENV_SENSOR_FRAME_LEN - 1u); index++)
    {
        checksum = (uint8_t)(checksum + frame[index]);
    }

    if (checksum != frame[ENV_SENSOR_FRAME_LEN - 1u])
    {
        return APP_ERR_HW;
    }

    sample->co2_ppm = EnvSensor_U16Be(frame[2], frame[3]);
    sample->ch2o_ugm3 = EnvSensor_U16Be(frame[4], frame[5]);
    sample->tvoc_ugm3 = EnvSensor_U16Be(frame[6], frame[7]);
    sample->pm25_ugm3 = EnvSensor_U16Be(frame[8], frame[9]);
    sample->pm10_ugm3 = EnvSensor_U16Be(frame[10], frame[11]);
    sample->pm25_ugm3_x10 = (uint16_t)(sample->pm25_ugm3 * 10u);

    temp_x10 = (int16_t)(((uint16_t)(frame[12] & 0x7Fu) * 10u) + frame[13]);
    if ((frame[12] & 0x80u) != 0u)
    {
        temp_x10 = (int16_t)-temp_x10;
    }
    sample->temperature_c_x10 = temp_x10;
    sample->humidity_rh_x10 = (uint16_t)(((uint16_t)frame[14] * 10u) + frame[15]);
    sample->valid = 1u;
    return APP_OK;
}

static uint16_t EnvSensor_U16Be(uint8_t msb, uint8_t lsb)
{
    return (uint16_t)(((uint16_t)msb << 8) | lsb);
}
