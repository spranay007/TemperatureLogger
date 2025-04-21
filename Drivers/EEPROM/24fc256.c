/*
 * 24fc256.c
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */

#include "24fc256.h"
#include "string.h"

#define EEPROM_ADDR (0x50 << 1)

void EEPROM_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t memAddr, uint8_t *data, uint16_t size)
{
    uint8_t buffer[size + 2];
    buffer[0] = (uint8_t)(memAddr >> 8);
    buffer[1] = (uint8_t)(memAddr & 0xFF);
    memcpy(&buffer[2], data, size);

    HAL_I2C_Master_Transmit(hi2c, EEPROM_ADDR, buffer, size + 2, HAL_MAX_DELAY);
    HAL_Delay(5);  // 5ms write time
}
