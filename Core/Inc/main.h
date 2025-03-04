/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_ll_crs.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"
#include "stm32l0xx_ll_tim.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
// Structure pour une note
typedef struct {
    uint32_t frequency;
    uint32_t duration; // en ms
} Note;

// Structure pour le status du jeu
typedef struct {
    uint8_t ball_x;
    uint8_t ball_y;
    uint8_t ball_dx;
    uint8_t ball_dy;
    uint8_t paddle_left;
    uint8_t paddle_right;
    uint8_t status;
} Game;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
// Status du jeu
#define GAME_STATUS_NONE 0
#define GAME_STATUS_RUNNING 1
#define GAME_STATUS_PAUSED 2

// Taille de la grille du jeu
#define GAME_GRID_SIZE 250

// Taille de la raquette
#define GAME_PADDLE_SIZE 6

// Taille de la balle
#define GAME_BALL_SIZE 3

// Définir les buzzers, boutons et joystick de chaque joystick
#define BUZZER_TIM TIM2
#define BUZZER_CHANNEL_P1 LL_TIM_CHANNEL_CH3
#define BUZZER_CHANNEL_P2 LL_TIM_CHANNEL_CH2
#define BUTTON_GPIO GPIOA
#define BUTTON_PIN_P1 LL_GPIO_PIN_8
#define BUTTON_PIN_P2 LL_GPIO_PIN_10
#define JOYSTICK_ADC ADC1
#define JOYSTICK_X_CHANNEL_P1 LL_ADC_CHANNEL_0
#define JOYSTICK_Y_CHANNEL_P1 LL_ADC_CHANNEL_1
#define JOYSTICK_X_CHANNEL_P2 LL_ADC_CHANNEL_11
#define JOYSTICK_Y_CHANNEL_P2 LL_ADC_CHANNEL_12
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void UART_Callback(char* msg, size_t length);
void Play_Melody(TIM_TypeDef *TIMx, uint32_t Channels, Note* melody, size_t length);
void Send_Game_All_Data();
void Send_Game_Run_Data();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI4_15_IRQn
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin LL_GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */
// Variables externes pour les mélodies
extern Note init_melody[];
extern size_t init_length;

extern Note connection_melody[];
extern size_t connection_length;

extern Note disconnection_melody[];
extern size_t disconnection_length;

extern Note game_start_melody[];
extern size_t game_start_length;

extern Note pause_melody[];
extern size_t pause_length;

extern Note resume_melody[];
extern size_t resume_length;

extern Note pong_hit_sound[];
extern size_t pong_hit_length;

extern Note victory_melody[];
extern size_t victory_length;

extern Note defeat_melody[];
extern size_t defeat_length;

// Variable externe pour le jeu
extern Game game;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
