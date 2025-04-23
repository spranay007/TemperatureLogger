/*
 * 24fc256.c
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */

#include "24fc256.h"
#include "string.h"

/* Static function defs
 * */
static HAL_StatusTypeDef EEPROM_WaitForWriteCompletion(I2C_HandleTypeDef *hi2c);

/*
 * @brief waits for write completion
 * @param hi2c pointer to the I@C handle
 * @retvat HAL_Status
 *
 * */
static HAL_StatusTypeDef EEPROM_WaitForWriteCompletion(I2C_HandleTypeDef *hi2c)
{
    uint32_t startTick = HAL_GetTick();
    while (HAL_I2C_IsDeviceReady(hi2c, EEPROM_I2C_ADDR, 1, 10) != HAL_OK)
    {
        if ((HAL_GetTick() - startTick) > EEPROM_ACK_TIMEOUT_MS)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

/*
 * @brief Checks the EEPROM status and Initializes the EEPROM with meta data read from the reserved memory
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM handle
 * @retval HAL_Status
 *
 * */
HAL_StatusTypeDef EEPROM_Init(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle)
{
    handle->status = EEPROM_CheckStatus(hi2c);
    handle->state = EEPROM_IDLE;
    handle->write_ptr = EEPROM_DATA_START_ADDR;
    handle->read_ptr = EEPROM_DATA_START_ADDR;
    handle->used_size = 0;
    handle->has_wrapped = false;
    if(EEPROM_RestoreMetadata(hi2c, handle) != HAL_OK){
    	return HAL_ERROR;
    }
    return HAL_OK;
}

/*
 * @brief Checks the EEPROM Status if its present on the I2C bus or not
 * @param hi2c pointer to the I2C handle
 * @retval EEPROM Status
 *
 * */
EEPROM_Status EEPROM_CheckStatus(I2C_HandleTypeDef *hi2c)
{
    if (HAL_I2C_IsDeviceReady(hi2c, EEPROM_I2C_ADDR, 3, 100) == HAL_OK)
        return EEPROM_STATUS_PRESENT;
    return EEPROM_STATUS_NOT_PRESENT;
}

/*
 * @brief checks the EEPROM status if its busy or not
 * @param EEPROM Handle
 * @retval weather the EEPROM is busy or in bool
 *
 * */
bool EEPROM_IsBusy(EEPROM_Handle *handle)
{
    return (handle->state == EEPROM_BUSY);
}

/*
 * @brief Restores the meta data from the reserved memory
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM structure pointer
 * @retval HAL_Status
 *
 * */
HAL_StatusTypeDef EEPROM_RestoreMetadata(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle)
{
    uint8_t meta[5]; //left one address willingly empty for future addition
    if (HAL_I2C_Mem_Read(hi2c, EEPROM_I2C_ADDR, EEPROM_PTR_META_ADDR, I2C_MEMADD_SIZE_16BIT, meta, 5, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    handle->write_ptr = (meta[0] << 8) | meta[1];
    handle->used_size = (meta[2] << 8) | meta[3];
    handle->has_wrapped = (meta[4] != 0);

    if (handle->write_ptr < EEPROM_DATA_START_ADDR || handle->write_ptr >= EEPROM_TOTAL_SIZE)
        handle->write_ptr = EEPROM_DATA_START_ADDR;

    return HAL_OK;
}

/*
 * @brief Stores the Meta data into the reserved memory after each read and write
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM structure pointer
 * @retval void
 *
 * */
void EEPROM_StoreMetadata(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle)
{
    uint8_t meta[5];
    meta[0] = (handle->write_ptr >> 8);
    meta[1] = (handle->write_ptr & 0xFF);
    meta[2] = (handle->used_size >> 8);
    meta[3] = (handle->used_size & 0xFF);
    meta[4] = handle->has_wrapped ? 1 : 0;

    HAL_I2C_Mem_Write(hi2c, EEPROM_I2C_ADDR, EEPROM_PTR_META_ADDR, I2C_MEMADD_SIZE_16BIT, meta, 5, HAL_MAX_DELAY);
    EEPROM_WaitForWriteCompletion(hi2c);
}

/*
 * @brief Erases the EEPROM to the length specified
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM structure pointer
 * @param[3] Start address from there erase has to start
 * @param[4] length of data to be erased
 * @retval void
 *
 * */
void EEPROM_Erase(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint16_t start_addr, uint16_t length)
{
    if (handle->status != EEPROM_STATUS_PRESENT || handle->state == EEPROM_BUSY)
        return;

    handle->state = EEPROM_BUSY;

    uint8_t blank[EEPROM_PAGE_SIZE];
    memset(blank, 0xFF, EEPROM_PAGE_SIZE);

    uint16_t addr = start_addr;
    while (length > 0)
    {
        uint16_t chunk = (length > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : length;

        uint8_t buffer[chunk + 2];
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = addr & 0xFF;
        memcpy(&buffer[2], blank, chunk);

        HAL_I2C_Master_Transmit(hi2c, EEPROM_I2C_ADDR, buffer, chunk + 2, HAL_MAX_DELAY);
        EEPROM_WaitForWriteCompletion(hi2c);

        addr += chunk;
        length -= chunk;
    }

    if (start_addr == EEPROM_DATA_START_ADDR) {
        handle->write_ptr = EEPROM_DATA_START_ADDR;
        handle->used_size = 0;
        handle->has_wrapped = false;
        EEPROM_StoreMetadata(hi2c, handle);
    }

    handle->state = EEPROM_IDLE;
}

/*
 * @brief Erases the EEPROM to the length specified
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM structure pointer
 * @param[3] Start address from there erase has to start
 * @param[4] length of data to be erased
 * @retval void
 *
 * */
void EEPROM_EraseAll(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle)
{
    EEPROM_Erase(hi2c, handle, EEPROM_DATA_START_ADDR, EEPROM_TOTAL_SIZE - EEPROM_DATA_START_ADDR);
}

/*
 * @brief Writes the number of Bytes passed
 * @param[1] hi2c pointer to the I2C handle
 * @param[2] EEPROM structure pointer
 * @param[3] data to be written
 * @param[4] size of data to be written
 * @retval HAL_Status
 *
 * */
HAL_StatusTypeDef EEPROM_WriteBytes(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint8_t *data, uint16_t size)
{
    if (handle->status != EEPROM_STATUS_PRESENT || handle->state == EEPROM_BUSY)
        return HAL_ERROR;

    handle->state = EEPROM_BUSY;

    while (size > 0) {
        if (handle->write_ptr >= EEPROM_TOTAL_SIZE) {
            handle->write_ptr = EEPROM_DATA_START_ADDR;
            handle->has_wrapped = true;
        }

        uint16_t page_offset = handle->write_ptr % EEPROM_PAGE_SIZE;
        uint16_t space_in_page = EEPROM_PAGE_SIZE - page_offset;
        uint16_t chunk_size = (size < space_in_page) ? size : space_in_page;

        uint8_t buffer[chunk_size + 2];
        buffer[0] = (handle->write_ptr >> 8) & 0xFF;
        buffer[1] = handle->write_ptr & 0xFF;
        memcpy(&buffer[2], data, chunk_size);

        if (HAL_I2C_Master_Transmit(hi2c, EEPROM_I2C_ADDR, buffer, chunk_size + 2, HAL_MAX_DELAY) != HAL_OK) {
            handle->state = EEPROM_IDLE;
            return HAL_ERROR;
        }

        if (EEPROM_WaitForWriteCompletion(hi2c) != HAL_OK) {
                    handle->state = EEPROM_IDLE;
                    return HAL_TIMEOUT;
        }

        handle->write_ptr += chunk_size;
        handle->used_size += chunk_size;
        if (handle->used_size > EEPROM_MAX_USABLE_SIZE) {
            handle->used_size = EEPROM_MAX_USABLE_SIZE;
            handle->has_wrapped = true;
        }

        data += chunk_size;
        size -= chunk_size;
    }

    EEPROM_StoreMetadata(hi2c, handle);
    handle->state = EEPROM_IDLE;
    return HAL_OK;
}

HAL_StatusTypeDef EEPROM_ReadBytes(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint8_t *data, uint16_t size)
{
    if (handle->status != EEPROM_STATUS_PRESENT || handle->state == EEPROM_BUSY)
        return HAL_ERROR;

    handle->state = EEPROM_BUSY;

    uint8_t addr_bytes[2] = {
        (uint8_t)(handle->read_ptr >> 8),
        (uint8_t)(handle->read_ptr & 0xFF)
    };

    if (HAL_I2C_Master_Transmit(hi2c, EEPROM_I2C_ADDR, addr_bytes, 2, HAL_MAX_DELAY) != HAL_OK) {
        handle->state = EEPROM_IDLE;
        return HAL_ERROR;
    }

    if (HAL_I2C_Master_Receive(hi2c, EEPROM_I2C_ADDR, data, size, HAL_MAX_DELAY) != HAL_OK) {
        handle->state = EEPROM_IDLE;
        return HAL_ERROR;
    }

    handle->read_ptr += size;
    handle->state = EEPROM_IDLE;
    return HAL_OK;
}

