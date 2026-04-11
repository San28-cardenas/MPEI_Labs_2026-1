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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Boton_Diagnostico_Pin GPIO_PIN_13
#define Boton_Diagnostico_GPIO_Port GPIOC
#define Interruptor_Pin GPIO_PIN_14
#define Interruptor_GPIO_Port GPIOC
#define Puente_H_IN1_Pin GPIO_PIN_0
#define Puente_H_IN1_GPIO_Port GPIOA
#define Puente_H_IN2_Pin GPIO_PIN_1
#define Puente_H_IN2_GPIO_Port GPIOA
#define Sensor_sharp_Pin GPIO_PIN_4
#define Sensor_sharp_GPIO_Port GPIOA
#define IR_Izq_Pin GPIO_PIN_0
#define IR_Izq_GPIO_Port GPIOB
#define IR_Cen_Pin GPIO_PIN_1
#define IR_Cen_GPIO_Port GPIOB
#define Ir_Der_Pin GPIO_PIN_11
#define Ir_Der_GPIO_Port GPIOB
#define Sensor_impacto_Pin GPIO_PIN_12
#define Sensor_impacto_GPIO_Port GPIOB
#define Puente_H_IN3_Pin GPIO_PIN_13
#define Puente_H_IN3_GPIO_Port GPIOB
#define Puente_H_IN4_Pin GPIO_PIN_14
#define Puente_H_IN4_GPIO_Port GPIOB
#define TRIG_Ultra_Pin GPIO_PIN_8
#define TRIG_Ultra_GPIO_Port GPIOA
#define ECHO_Ultra_Pin GPIO_PIN_11
#define ECHO_Ultra_GPIO_Port GPIOA
#define Led_RGB_Pin GPIO_PIN_15
#define Led_RGB_GPIO_Port GPIOA
#define Buzzer_Pin GPIO_PIN_8
#define Buzzer_GPIO_Port GPIOB
#define Servo_Pin GPIO_PIN_9
#define Servo_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
