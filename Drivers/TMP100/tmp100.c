/*
 * tmp100.c
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */
#include "tmp100.h"

/*Static function declaration
 * */
static float TMP100_ConvertRawTemp(int16_t raw);
/*
 * check if the device is present or not on the i2c bus
 * */
TMP100_STATUS TMP100_CheckStatus(I2C_HandleTypeDef *hi2c)
{
    TMP100_STATUS retStatus = TMP_ERROR;

    for (uint8_t attempt = 0; attempt < TMP100_I2C_RETRIES ; attempt++) {
        if (HAL_I2C_IsDeviceReady(hi2c, TMP100_I2C_ADDR, 3, HAL_MAX_DELAY) == HAL_OK) {
            retStatus = TMP_READY;
            break;
        }
        HAL_Delay(TMP100_RETRY_DELAY_MS);  // delay between retries
    }

    return retStatus;
}

/*
 * By default we are using the continues conversion mode and 12 bit resolution which is okay for the 10 min logging
 * */
TMP100_STATUS TMP100_ReadTemperature(I2C_HandleTypeDef *hi2c, float *readVal)
{
    uint8_t reg = TMP100_TEMP_REG;
    uint8_t data[2];

    if(HAL_I2C_Master_Transmit(hi2c, TMP100_I2C_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK){
    	return TMP_ERROR;
    }
    if(HAL_I2C_Master_Receive(hi2c, TMP100_I2C_ADDR, data, 2, HAL_MAX_DELAY) != HAL_OK){
		return TMP_ERROR;
    }
    //HAL_I2C_Mem_Read(hi2c, TMP100_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    int16_t raw = (data[0] << 4) | (data[1] >> 4); //12 bit to 16 bit conversion

    *readVal = TMP100_ConvertRawTemp(raw);
	return (*readVal == TMP100_INVALID_TEMP) ? TMP_ERROR : TMP_READY;
}
/*
 * Incase we need to save some power as the logging has to be done in 10 mins interval we can go the one shot approach and sleep rest of the period
 * */
TMP100_STATUS TMP100_ReadTemperature_OneShot(I2C_HandleTypeDef *hi2c, float *readVal)
{
    uint8_t config;
    // Valid raw range: 0x800 (−55°C) to 0x7FF (+127°C)

    // Seting shutdown + 12-bit resolution
    config = TMP100_CONFIG_SHUTDOWN_12BIT;
    if(HAL_I2C_Mem_Write(hi2c, TMP100_I2C_ADDR, TMP100_CONFIG_REG, 1, &config, 1, HAL_MAX_DELAY) != HAL_OK){
    	return TMP_ERROR;
    }
    // Triggering one-shot conversion
    config = TMP100_CONFIG_ONESHOT_12BIT;
    if(HAL_I2C_Mem_Write(hi2c, TMP100_I2C_ADDR, TMP100_CONFIG_REG, 1, &config, 1, HAL_MAX_DELAY) != HAL_OK){
    	return TMP_ERROR;
    }
    // Waiting for conversion (max 600ms for 12-bit)
    //HAL_Delay(600);

    //Poll OS bit until cleared or conversion complete is detected
	uint32_t tickStart = HAL_GetTick();
	do {
		if(HAL_I2C_Mem_Read(hi2c, TMP100_I2C_ADDR, TMP100_CONFIG_REG, 1, &config, 1, HAL_MAX_DELAY) != HAL_OK){
			return TMP_ERROR;
		}

		if ((HAL_GetTick() - tickStart) > TMP100_CONV_TIMEOUT_MS) {
			return TMP_TIMEOUT;
		}
	} while (config & TMP100_OS_BIT_MASK);  // Wait until OS=0

    // Read temperature
    uint8_t temp_reg = TMP100_TEMP_REG;
    uint8_t data[2];
    if(HAL_I2C_Master_Transmit(hi2c, TMP100_I2C_ADDR, &temp_reg, 1, HAL_MAX_DELAY) != HAL_OK){
    	return TMP_ERROR;
	}
    if(HAL_I2C_Master_Receive(hi2c, TMP100_I2C_ADDR, data, 2, HAL_MAX_DELAY) != HAL_OK){
    	return TMP_ERROR;
    }

    int16_t raw = (data[0] << 4) | (data[1] >> 4);

    *readVal = TMP100_ConvertRawTemp(raw);
    if(*readVal == TMP100_INVALID_TEMP){
    	return TMP_ERROR;
    }
    return TMP_READY;
}
/*
 * Static function to calculate the temp value from raw values
 * */
static float TMP100_ConvertRawTemp(int16_t raw){

	//checking for the signed bit before copying the 12bit number to 16bit
    if (raw & 0x800) raw |= 0xF000;

    float temp = raw * 0.0625f;
    if (temp < -55.0f || temp > 125.0f){
		return TMP100_INVALID_TEMP;
    }
	return temp;
}
