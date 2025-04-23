/*
 * 24fc256.h
 *
 *  Created on: Apr 20, 2025
 *      Author: spran
 */

#ifndef EEPROM_24FC256_H_
#define EEPROM_24FC256_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// I2C config and EEPROM parameters
#define EEPROM_I2C_ADDR              (0x50 << 1)   // 7-bit base address (0x50) shifted left
#define EEPROM_TOTAL_SIZE            32768         // 32KB = 256Kb
#define EEPROM_PAGE_SIZE             64            // Max bytes per page write
#define EEPROM_WRITE_CYCLE_TIME      5             // ms delay after write
#define EEPROM_PTR_META_ADDR         0x0000        // Reserve first 2 bytes for write pointer
#define EEPROM_DATA_START_ADDR       0x0002        // Start writing data after pointer storage
#define EEPROM_MAX_ADDR              (EEPROM_TOTAL_SIZE - 1)
#define EEPROM_MAX_USABLE_SIZE       (EEPROM_TOTAL_SIZE - EEPROM_DATA_START_ADDR)

// EEPROM presence/status
typedef enum {
    EEPROM_STATUS_UNKNOWN = 0,
    EEPROM_STATUS_PRESENT,
    EEPROM_STATUS_NOT_PRESENT
} EEPROM_Status;

// EEPROM operation state
typedef enum {
    EEPROM_IDLE = 0,
    EEPROM_BUSY
} EEPROM_State;

// EEPROM handle struct
typedef struct {
    EEPROM_Status status;         // Whether EEPROM is detected
    EEPROM_State  state;          // Busy or idle
    uint16_t      write_ptr;      // Current write pointer
    uint16_t      read_ptr;       // Current read pointer
    uint16_t      used_size;      // Total bytes written (up to max)
    bool          has_wrapped;    // True if write pointer wrapped around
} EEPROM_Handle;

//Initialization and state check functions
void EEPROM_Init(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle);
EEPROM_Status EEPROM_CheckStatus(I2C_HandleTypeDef *hi2c);
bool EEPROM_IsBusy(EEPROM_Handle *handle);

//Pointer Handling
HAL_StatusTypeDef EEPROM_RestoreWritePointer(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle);
void EEPROM_StoreWritePointer(I2C_HandleTypeDef *hi2c, uint16_t current_ptr);

//Read/Write Operations
HAL_StatusTypeDef EEPROM_WriteBytes(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint8_t *data, uint16_t size);
HAL_StatusTypeDef EEPROM_ReadBytes(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint8_t *data, uint16_t size);

//Erase Functionality
void EEPROM_Erase(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle, uint16_t start_addr, uint16_t length);
void EEPROM_EraseAll(I2C_HandleTypeDef *hi2c, EEPROM_Handle *handle);
#endif /* EEPROM_24FC256_H_ */
