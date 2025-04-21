/*
 * 24fc256.h
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */

#ifndef EEPROM_24FC256_H_
#define EEPROM_24FC256_H_

#include "stm32f1xx_hal.h"
void EEPROM_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t memAddr, uint8_t *data, uint16_t size);

#endif /* EEPROM_24FC256_H_ */
