/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SIM_PWR_KEY_Pin GPIO_PIN_0
#define SIM_PWR_KEY_GPIO_Port GPIOA
#define SIM_RESET_Pin GPIO_PIN_1
#define SIM_RESET_GPIO_Port GPIOA
#define SIM_TX_Pin GPIO_PIN_2
#define SIM_TX_GPIO_Port GPIOA
#define SIM_RX_Pin GPIO_PIN_3
#define SIM_RX_GPIO_Port GPIOA
#define SIM_DTR_Pin GPIO_PIN_4
#define SIM_DTR_GPIO_Port GPIOA
#define SIM_RI_Pin GPIO_PIN_5
#define SIM_RI_GPIO_Port GPIOA
#define SIM_ON_OFF_Pin GPIO_PIN_6
#define SIM_ON_OFF_GPIO_Port GPIOA
#define V_IN_12V_Pin GPIO_PIN_7
#define V_IN_12V_GPIO_Port GPIOA
#define V_IN_Pin GPIO_PIN_4
#define V_IN_GPIO_Port GPIOC
#define V_OFFSET_Pin GPIO_PIN_5
#define V_OFFSET_GPIO_Port GPIOC
#define I_IN_Pin GPIO_PIN_0
#define I_IN_GPIO_Port GPIOB
#define BLE_ON_OFF_Pin GPIO_PIN_1
#define BLE_ON_OFF_GPIO_Port GPIOB
#define BLE_WAKEUP_Pin GPIO_PIN_2
#define BLE_WAKEUP_GPIO_Port GPIOB
#define DEBUG_TX_Pin GPIO_PIN_10
#define DEBUG_TX_GPIO_Port GPIOB
#define DEBUG_RX_Pin GPIO_PIN_11
#define DEBUG_RX_GPIO_Port GPIOB
#define FLASH_ON_OFF_Pin GPIO_PIN_12
#define FLASH_ON_OFF_GPIO_Port GPIOB
#define FLASH_SCK_Pin GPIO_PIN_13
#define FLASH_SCK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_14
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_15
#define FLASH_MOSI_GPIO_Port GPIOB
#define FLASH_RESET_Pin GPIO_PIN_6
#define FLASH_RESET_GPIO_Port GPIOC
#define FLASH_CS_Pin GPIO_PIN_7
#define FLASH_CS_GPIO_Port GPIOC
#define ON_OFF_12V_Pin GPIO_PIN_8
#define ON_OFF_12V_GPIO_Port GPIOC
#define RS485_ON_OFF_Pin GPIO_PIN_9
#define RS485_ON_OFF_GPIO_Port GPIOC
#define RS485_TXDE_Pin GPIO_PIN_8
#define RS485_TXDE_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define MAGNETIC_IRQ_Pin GPIO_PIN_11
#define MAGNETIC_IRQ_GPIO_Port GPIOA
#define LED_1_Pin GPIO_PIN_12
#define LED_1_GPIO_Port GPIOC
#define LED_2_Pin GPIO_PIN_2
#define LED_2_GPIO_Port GPIOD
#define LED_3_Pin GPIO_PIN_3
#define LED_3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
