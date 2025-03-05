/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32l0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#define UART_BUFFER_SIZE 256
static char uartBuffer[UART_BUFFER_SIZE];
static uint16_t uartBufferIndex = 0;
#define SYSTICK_TIME 1000 // 1ms
static uint32_t systick = 0;
#define GAMETICK_FREQ 30 // 30Hz (30ms)
#define GAMETICK_PERIOD (SYSTICK_TIME / GAMETICK_FREQ)
#define MUSICTICK_FREQ 4 // 4Hz (250ms)
#define MUSICTICK_PERIOD (SYSTICK_TIME / MUSICTICK_FREQ)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  systick++;

  // Ticks pour le jeu
  if(systick % GAMETICK_PERIOD == 0)
  {
	  Update_Game();
  }

  // Ticks pour la musique (4Hz - toutes les 250ms)
  if(systick % MUSICTICK_PERIOD == 0)
  {
      Update_Sound();
  }

  // Ticks pour arrêter le son (4Hz - toutes les 250ms, avec 50ms de décalage)
  if(systick % MUSICTICK_PERIOD == 200)
  {
      Stop_Sound();
  }
  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 4 to 15 interrupts.
  */
void EXTI4_15_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_15_IRQn 0 */

  /* USER CODE END EXTI4_15_IRQn 0 */
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    /* USER CODE BEGIN LL_EXTI_LINE_13 */
    // Désactiver l'interruption pour éviter les rebonds
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_13);

    // Appeler la fonction de callback
    Blue_Button_Callback();

    // Réactiver l'interruption
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    /* USER CODE END LL_EXTI_LINE_13 */
  }
  /* USER CODE BEGIN EXTI4_15_IRQn 1 */

  /* USER CODE END EXTI4_15_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  if(LL_USART_IsActiveFlag_RXNE(USART2))
  {
	// Lire le caractère reçu
	char receivedChar = LL_USART_ReceiveData8(USART2);

	// Ajouter le caractère au buffer si il y a de la place
	if(uartBufferIndex < UART_BUFFER_SIZE - 1)
	{
	  uartBuffer[uartBufferIndex++] = receivedChar;

	  // Si c'est un caractère de fin de ligne, traiter le message
	  if(receivedChar == '\n')
	  {
		// Ajouter un caractère nul à la fin
		uartBuffer[uartBufferIndex] = '\0';

		// Trim du message (enlever les espaces et caractères de contrôle en début et fin)
		char* trimmedStart = uartBuffer;
		size_t trimmedLength = uartBufferIndex;

		// Trim du début (avancer le pointeur)
		while(*trimmedStart && (*trimmedStart == ' ' || *trimmedStart == '\r' || *trimmedStart == '\t'))
		{
		  trimmedStart++;
		  trimmedLength--;
		}

		// Trim de la fin (réduire la longueur)
		char* trimmedEnd = trimmedStart + trimmedLength - 1;
		while(trimmedLength > 0 && (*trimmedEnd == ' ' || *trimmedEnd == '\r' || *trimmedEnd == '\n' || *trimmedEnd == '\t'))
		{
		  *trimmedEnd = '\0';
		  trimmedEnd--;
		  trimmedLength--;
		}

		// Appeler la fonction de callback avec le message trimé
		if(trimmedLength > 0)
		{
		  UART_Callback(trimmedStart, trimmedLength);
		}

		// Réinitialiser l'index pour le prochain message
		uartBufferIndex = 0;
	  }
	}
	else
	{
	  // Buffer overflow, réinitialiser
	  uartBufferIndex = 0;
	}
  }
  /* USER CODE END USART2_IRQn 0 */
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
