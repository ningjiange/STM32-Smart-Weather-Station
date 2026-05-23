/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define Red_LED_Pin GPIO_PIN_13
#define Red_LED_GPIO_Port GPIOC
#define DHT11_Pin GPIO_PIN_0
#define DHT11_GPIO_Port GPIOA
#define LightSensorIN_Pin GPIO_PIN_1
#define LightSensorIN_GPIO_Port GPIOA
#define TB6612PWM_Pin GPIO_PIN_6
#define TB6612PWM_GPIO_Port GPIOA
#define AIN1_Pin GPIO_PIN_7
#define AIN1_GPIO_Port GPIOA
#define EncoderA_Pin GPIO_PIN_0
#define EncoderA_GPIO_Port GPIOB
#define EncoderA_EXTI_IRQn EXTI0_IRQn
#define EncoderB_Pin GPIO_PIN_1
#define EncoderB_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_12
#define KEY1_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_13
#define KEY2_GPIO_Port GPIOB
#define Buzzer_Pin GPIO_PIN_14
#define Buzzer_GPIO_Port GPIOB
#define EncoderC_Pin GPIO_PIN_15
#define EncoderC_GPIO_Port GPIOB
#define AIN2_Pin GPIO_PIN_8
#define AIN2_GPIO_Port GPIOA
#define SG_90PWM_Pin GPIO_PIN_11
#define SG_90PWM_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_6
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB
#define Green_LED_Pin GPIO_PIN_8
#define Green_LED_GPIO_Port GPIOB
#define Yellow_LED_Pin GPIO_PIN_9
#define Yellow_LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
