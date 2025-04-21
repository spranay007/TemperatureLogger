################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/TMP100/tmp100.c 

OBJS += \
./Drivers/TMP100/tmp100.o 

C_DEPS += \
./Drivers/TMP100/tmp100.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/TMP100/%.o Drivers/TMP100/%.su Drivers/TMP100/%.cyclo: ../Drivers/TMP100/%.c Drivers/TMP100/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/EEPROM -I../Drivers/TMP100 -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-TMP100

clean-Drivers-2f-TMP100:
	-$(RM) ./Drivers/TMP100/tmp100.cyclo ./Drivers/TMP100/tmp100.d ./Drivers/TMP100/tmp100.o ./Drivers/TMP100/tmp100.su

.PHONY: clean-Drivers-2f-TMP100

