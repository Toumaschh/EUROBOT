################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/vl53l5cx/porting/platform.c 

OBJS += \
./Drivers/BSP/Components/vl53l5cx/porting/platform.o 

C_DEPS += \
./Drivers/BSP/Components/vl53l5cx/porting/platform.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/vl53l5cx/porting/%.o Drivers/BSP/Components/vl53l5cx/porting/%.su Drivers/BSP/Components/vl53l5cx/porting/%.cyclo: ../Drivers/BSP/Components/vl53l5cx/porting/%.c Drivers/BSP/Components/vl53l5cx/porting/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I../X-CUBE-TOF1/Target -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/Components/vl53l5cx/modules -I../Drivers/BSP/Components/vl53l5cx/porting -I../Drivers/BSP/Components/vl53l5cx -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-vl53l5cx-2f-porting

clean-Drivers-2f-BSP-2f-Components-2f-vl53l5cx-2f-porting:
	-$(RM) ./Drivers/BSP/Components/vl53l5cx/porting/platform.cyclo ./Drivers/BSP/Components/vl53l5cx/porting/platform.d ./Drivers/BSP/Components/vl53l5cx/porting/platform.o ./Drivers/BSP/Components/vl53l5cx/porting/platform.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-vl53l5cx-2f-porting

