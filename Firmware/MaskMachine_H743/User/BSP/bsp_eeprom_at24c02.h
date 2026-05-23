#ifndef BSP_EEPROM_AT24C02_H
#define BSP_EEPROM_AT24C02_H

#include <stdint.h>
#include "app_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_EEPROM_AT24C02_SIZE_BYTES   256u
#define BSP_EEPROM_AT24C02_PAGE_BYTES   8u

app_status_t Bsp_EepromAt24c02_Init(void);
app_status_t Bsp_EepromAt24c02_Read(uint8_t offset, uint8_t *data, uint16_t length);
app_status_t Bsp_EepromAt24c02_Write(uint8_t offset, const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* BSP_EEPROM_AT24C02_H */
