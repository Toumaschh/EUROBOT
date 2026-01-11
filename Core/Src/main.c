/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (VL53L5CX + I2C1 fix)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "custom_ranging_sensor.h"
#include "stm32h7xx_nucleo_bus.h"
#include "VL53L5CX.h"
#include <stdio.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
COM_InitTypeDef BspCOMInit;
extern I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Helpers I2C */
void I2C_Bus_Recovery(void);
HAL_StatusTypeDef MX_I2C1_Init(I2C_HandleTypeDef* hi2c);

/* MSP for I2C1 (AF4 PB8/PB9 + RCC enable) */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);

/* USER CODE BEGIN 0 */
/* Redirect printf to USART3 (BSP COM1) */
__weak int __io_putchar(int ch)
{
  extern UART_HandleTypeDef hcom_uart[];
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();                   /* Reset des périphériques + Systick */
  SystemClock_Config();         /* 1) Clock tree d'abord */

  MX_GPIO_Init();               /* 2) GPIO (PWREN, LPn, INT, LEDs, etc.) */

  /* 3) Init COM pour printf */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);




  /* 1) INITIALISER le capteur (tout est configuré DANS la librairie) */
    if (VL53L5CX_Init_All() != BSP_ERROR_NONE) {
      printf("Init capteur KO\r\n");
      Error_Handler();
    }


  while (1)
  {
	  VL53L5CX_PrintMatrix();
  }
}

/* =========================
 * Clocks
 * ========================= */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* =========================
 * GPIO
 * ========================= */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* PB0 output */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = GPIO_PIN_0;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* I2C_R + LPn => outputs */
  HAL_GPIO_WritePin(GPIOF, I2C_R_Pin|LPn_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = I2C_R_Pin|LPn_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* PWREN high (enable power if used) */
  HAL_GPIO_WritePin(PWREN_GPIO_Port, PWREN_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin   = PWREN_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWREN_GPIO_Port, &GPIO_InitStruct);

  /* USER button */
  GPIO_InitStruct.Pin  = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* INT pin du capteur */
  GPIO_InitStruct.Pin  = INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INT_GPIO_Port, &GPIO_InitStruct);

  /* USART3 (PD8/PD9) pour COM1 (BSP) */
  GPIO_InitStruct.Pin       = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* LD2 */
  GPIO_InitStruct.Pin   = LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

/* =========================
 * I2C Bus Recovery
 * ========================= */
void I2C_Bus_Recovery(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Reset I2C1 peripheral to clear BUSY state */
  __HAL_RCC_I2C1_FORCE_RESET();
  HAL_Delay(2);
  __HAL_RCC_I2C1_RELEASE_RESET();
  HAL_Delay(2);

  /* Disable I2C1 clock (on va bit-bang le bus) */
  __HAL_RCC_I2C1_CLK_DISABLE();

  /* PB8=SCL, PB9=SDA en GPIO output open-drain + pull-up */
  GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Ligne SDA HIGH */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

  /* Toggle SCL 9 fois pour vider esclave bloqué */
  for (int i = 0; i < 9; i++)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(1);
  }

  /* Générer STOP: SCL low, SDA low -> SCL high -> SDA high */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_Delay(1);

  /* Vérifier lignes hautes */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET) {
    printf("Warning: SDA low après recovery!\r\n");
  }
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_RESET) {
    printf("Warning: SCL low après recovery!\r\n");
  }
}

/* =========================
 * I2C1 Init (+ Timing)
 * ========================= */
HAL_StatusTypeDef MX_I2C1_Init(I2C_HandleTypeDef* hi2c)
{
  HAL_StatusTypeDef ret = HAL_OK;

  printf("MX_I2C1_Init...\r\n");

  hi2c->Instance             = I2C1;

  /* Timing (exemple 64MHz kernel, 100kHz bus) :
     Recalcule via CubeMX I2C Timing Calculator si ta clock diffère !
     0x1042C3C7 est une valeur typique mais à vérifier avec ton clock tree.
  */
  hi2c->Init.Timing          = 0x1042C3C7;

  hi2c->Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hi2c->Init.OwnAddress1     = 0;
  hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c->Init.OwnAddress2     = 0;
  hi2c->Init.OwnAddress2Masks= I2C_OA2_NOMASK;
  hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(hi2c) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  /* Filtres : par défaut sur H7, l’analog filter est utile → ENABLE */
  if (HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    ret = HAL_ERROR;
  }
  /* Digital filter désactivé (0) */
  if (HAL_I2CEx_ConfigDigitalFilter(hi2c, 0) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

/* =========================
 * MSP pour I2C1 (AF4 PB8/PB9 + RCC)
 * ========================= */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin       = GPIO_PIN_8 | GPIO_PIN_9;   /* PB8=SCL, PB9=SDA */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();   /* Re-enable I2C clock après recovery */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
  }
}

/* =========================
 * Error handler
 * ========================= */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    BSP_LED_Toggle(LED_RED);
    HAL_Delay(100);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Report assert */
}
#endif /* USE_FULL_ASSERT */
