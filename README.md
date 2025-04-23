# TemperatureLogger STM32 Temperature Data Logger (TMP100 + 24FC256)

## Overview

This project is a periodic temperature logging system built on the STM32F103C8T6 microcontroller. It reads temperature data from a **TMP100 temperature sensor** and logs it to a **24FC256 EEPROM** over I2C. The system is designed to operate autonomously with minimal power consumption using **one-shot temperature reads every 10 minutes**.

## System Details

- **Controller Used**: STM32F103C8T6  
- **IDE Used**: STM32CubeIDE  
- **Language**: C (HAL-based drivers)  
- **Clock Source**: HSI (8Mhz)
- **Timer Used**: TIM2 (1s periodic interrupt)  
- **I2C Configuration**:
  - `I2C1`: 24FC256 EEPROM
  - `I2C2`: TMP100 Temperature Sensor

## Peripherals

### TMP100 (Temperature Sensor)
- Interface: I2C2
- Address: `0x48`
- Operating Mode: One-shot, 12-bit resolution
- Features:
  - Power-efficient temperature reads
  - Configurable timeout for conversion
  - Range check for valid temperature data
  - Raw-to-float conversion and error detection

### 24FC256 (EEPROM)
- Interface: I2C1
- Address: `0x50`
- Total Capacity: 32KB
- Page Size: 64 bytes
- Features:
  - Write pointer and metadata tracking
  - Support for wraparound writes
  - EEPROM erase (selective or full)
  - Restore metadata after power cycle
  - ACK polling with timeout

## System Behavior

1. On startup:
   - Checks TMP100 availability on I2C2.
   - Initializes EEPROM and restores metadata (write pointer, used size, etc.)

2. Every 1 second:
   - A counter is incremented via TIM2 interrupt.

3. Every 10 minutes (600 seconds):
   - TMP100 performs a one-shot temperature conversion.
   - The result is scaled and stored in EEPROM as a 2-byte signed integer.
   - EEPROM metadata is updated to support wraparound and power-failure recovery.

## Example Logging Flow

If temperature = `65.89°C`:
- Scaled and stored as `6589 → 0x19AB`
- Written to EEPROM sequentially in 2-byte blocks

---
**Author**: spran  

