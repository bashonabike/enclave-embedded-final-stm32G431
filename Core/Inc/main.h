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
#include "stm32g4xx_hal.h"

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
#define Fld1G_Pin GPIO_PIN_0
#define Fld1G_GPIO_Port GPIOF
#define Fld1B_Pin GPIO_PIN_1
#define Fld1B_GPIO_Port GPIOF
#define Knob1_Pin GPIO_PIN_0
#define Knob1_GPIO_Port GPIOA
#define Knob2_Pin GPIO_PIN_1
#define Knob2_GPIO_Port GPIOA
#define Knob8_Pin GPIO_PIN_2
#define Knob8_GPIO_Port GPIOA
#define Knob3_Pin GPIO_PIN_3
#define Knob3_GPIO_Port GPIOA
#define Knob4_Pin GPIO_PIN_4
#define Knob4_GPIO_Port GPIOA
#define Knob5_Pin GPIO_PIN_5
#define Knob5_GPIO_Port GPIOA
#define Knob6_Pin GPIO_PIN_6
#define Knob6_GPIO_Port GPIOA
#define Knob7_Pin GPIO_PIN_7
#define Knob7_GPIO_Port GPIOA
#define But4_Pin GPIO_PIN_0
#define But4_GPIO_Port GPIOB
#define But4_EXTI_IRQn EXTI0_IRQn
#define Fld2R_Pin GPIO_PIN_8
#define Fld2R_GPIO_Port GPIOA
#define But1_Pin GPIO_PIN_9
#define But1_GPIO_Port GPIOA
#define But1_EXTI_IRQn EXTI9_5_IRQn
#define But2_Pin GPIO_PIN_10
#define But2_GPIO_Port GPIOA
#define But2_EXTI_IRQn EXTI15_10_IRQn
#define Fld2G_Pin GPIO_PIN_11
#define Fld2G_GPIO_Port GPIOA
#define But3_Pin GPIO_PIN_12
#define But3_GPIO_Port GPIOA
#define But3_EXTI_IRQn EXTI15_10_IRQn
#define But6_Pin GPIO_PIN_15
#define But6_GPIO_Port GPIOA
#define But6_EXTI_IRQn EXTI15_10_IRQn
#define Mains_Pin GPIO_PIN_4
#define Mains_GPIO_Port GPIOB
#define Mains_EXTI_IRQn EXTI4_IRQn
#define Fld2B_Pin GPIO_PIN_5
#define Fld2B_GPIO_Port GPIOB
#define Fld1R_Pin GPIO_PIN_6
#define Fld1R_GPIO_Port GPIOB
#define But5_Pin GPIO_PIN_7
#define But5_GPIO_Port GPIOB
#define But5_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
