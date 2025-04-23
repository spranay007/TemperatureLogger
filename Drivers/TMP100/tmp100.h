/*
 * tmp100.h
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */

#ifndef TMP100_TMP100_H_
#define TMP100_TMP100_H_

#include "stm32f1xx_hal.h"  // or your specific HAL

typedef enum{
	TMP_READY = 0,
	TMP_ERROR = 1,
	TMP_TIMEOUT
}TMP100_STATUS;

#define TMP100_I2C_ADDR  				(0x48 << 1)
#define TMP100_CONFIG_SHUTDOWN_12BIT	0xA0	// 1010 0000 SD=1, OS=0, 12-bit
#define TMP100_CONFIG_ONESHOT_12BIT		0xE0	// 1110 0000 SD=1, OS=1, 12-bit
#define TMP100_TEMP_REG					0x00	// Temperature register address in TMP100
#define TMP100_CONFIG_REG				0x01	// Configuration register of TMP100
#define TMP100_OS_BIT_MASK				0x80	// Bitmask to isolate bit 7 (OS) in config register
#define TMP100_CONV_TIMEOUT_MS			1000 	// Timeout after 1000 ms
#define TMP100_RETRY_DELAY_MS			10		// Delay between retries
#define TMP100_I2C_RETRIES				5		// Number of retries
#define TMP100_INVALID_TEMP				-1000.0f// Invalid temp return

TMP100_STATUS TMP100_CheckStatus(I2C_HandleTypeDef *hi2c);

TMP100_STATUS TMP100_ReadTemperature(I2C_HandleTypeDef *hi2c, float *readVal);
TMP100_STATUS TMP100_ReadTemperature_OneShot(I2C_HandleTypeDef *hi2c, float *readVal);

#endif /* TMP100_TMP100_H_ */
