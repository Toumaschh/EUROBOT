################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../X-CUBE-TOF1/Target/custom_ranging_sensor.c 

OBJS += \
./X-CUBE-TOF1/Target/custom_ranging_sensor.o 

C_DEPS += \
./X-CUBE-TOF1/Target/custom_ranging_sensor.d 


# Each subdirectory must supply rules for building sources it contributes
X-CUBE-TOF1/Target/%.o X-CUBE-TOF1/Target/%.su X-CUBE-TOF1/Target/%.cyclo: ../X-CUBE-TOF1/Target/%.c X-CUBE-TOF1/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I../X-CUBE-TOF1/Target -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/Components/vl53l5cx/modules -I../Drivers/BSP/Components/vl53l5cx/porting -I../Drivers/BSP/Components/vl53l5cx -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-X-2d-CUBE-2d-TOF1-2f-Target

clean-X-2d-CUBE-2d-TOF1-2f-Target:
	-$(RM) ./X-CUBE-TOF1/Target/custom_ranging_sensor.cyclo ./X-CUBE-TOF1/Target/custom_ranging_sensor.d ./X-CUBE-TOF1/Target/custom_ranging_sensor.o ./X-CUBE-TOF1/Target/custom_ranging_sensor.su

.PHONY: clean-X-2d-CUBE-2d-TOF1-2f-Target

