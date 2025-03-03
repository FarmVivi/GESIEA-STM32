/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Structure pour une note
typedef struct {
    uint32_t frequency;
    uint32_t duration; // en ms
} Note;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Fréquences des notes (en Hz)
#define NOTE_C4  261.63  // Do4
#define NOTE_CS4 277.18  // Do#4/Ré♭4
#define NOTE_D4  293.66  // Ré4
#define NOTE_DS4 311.13  // Ré#4/Mi♭4
#define NOTE_E4  329.63  // Mi4
#define NOTE_F4  349.23  // Fa4
#define NOTE_FS4 369.99  // Fa#4/Sol♭4
#define NOTE_G4  392.00  // Sol4
#define NOTE_GS4 415.30  // Sol#4/La♭4
#define NOTE_A4  440.00  // La4
#define NOTE_AS4 466.16  // La#4/Si♭4
#define NOTE_B4  493.88  // Si4
#define NOTE_C5  523.25  // Do5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Mélodie de démarrage
Note init_melody[] = {
    {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_B4, 200}, {NOTE_C5, 400}
};
size_t init_length = sizeof(init_melody) / sizeof(Note);

// Mélodie de connexion
Note connection_melody[] = {
    {NOTE_C4, 100}, {NOTE_E4, 100}, {NOTE_G4, 100}, {NOTE_C5, 100}, {NOTE_G4, 100}, {NOTE_E4, 100}
};
size_t connection_length = sizeof(connection_melody) / sizeof(Note);

// Mélodie de déconnexion
Note disconnection_melody[] = {
    {NOTE_C5, 100}, {NOTE_G4, 100}, {NOTE_E4, 100}, {NOTE_C4, 200}
};
size_t disconnection_length = sizeof(disconnection_melody) / sizeof(Note);

// Mélodie de début de partie
Note game_start_melody[] = {
    {NOTE_E4, 150}, {NOTE_F4, 150}, {NOTE_G4, 150},
    {NOTE_A4, 150}, {NOTE_B4, 150}, {NOTE_C5, 300}
};
size_t game_start_length = sizeof(game_start_melody) / sizeof(Note);

// Mélodie de touche
Note pong_hit_sound[] = {
    {NOTE_E4, 100}, {NOTE_G4, 100}
};
size_t pong_hit_length = sizeof(pong_hit_sound) / sizeof(Note);

// Mélodie de victoire
Note victory_melody[] = {
    {NOTE_C4, 200}, {NOTE_E4, 200}, {NOTE_G4, 200}, {NOTE_C5, 400}
};
size_t victory_length = sizeof(victory_melody) / sizeof(Note);

// Mélodie de défaite
Note defeat_melody[] = {
    {NOTE_C5, 200}, {NOTE_G4, 200}, {NOTE_E4, 200}, {NOTE_C4, 400}
};
size_t defeat_length = sizeof(defeat_melody) / sizeof(Note);

uint32_t x_value = 0;
uint32_t y_value = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Redirection de printf vers l'UART2
int __io_putchar(int ch) {
    // Attendre que le buffer de transmission soit vide
    while (!LL_USART_IsActiveFlag_TXE(USART2));

    // Envoyer le caractère
    LL_USART_TransmitData8(USART2, ch);

    return ch;
}

// Necessaire pour stdio.h
int _write(int file, char *ptr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        __io_putchar(*ptr++);
    }
    return len;
}

// Fonction pour définir la fréquence du buzzer
void Set_Buzzer_Frequency(uint32_t frequency) {
    if (frequency == 0) {
        LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
        return;
    }
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    uint32_t timer_clock = 16000000; // Fréquence du timer (16 MHz)
    uint32_t prescaler = 0;
    uint32_t arr_value = (timer_clock / frequency) - 1;
    LL_TIM_SetAutoReload(TIM2, arr_value);
    LL_TIM_OC_SetCompareCH2(TIM2, arr_value / 2); // Rapport cyclique de 50%
}

// Fonction pour jouer une note
void Play_Note(Note note) {
    Set_Buzzer_Frequency(note.frequency);
    LL_mDelay(note.duration);
    Set_Buzzer_Frequency(0); // Arrêter le son
    LL_mDelay(25); // Pause entre les notes
}

// Fonction pour jouer une mélodie
void Play_Melody(Note* melody, size_t length) {
    for (size_t i = 0; i < length; i++) {
        Play_Note(melody[i]);
    }
}

// Fonction de callback pour les ordres reçues par l'UART
void UART_Callback(char* msg, size_t length) {
	// En fonction de la commande reçue
	if (strncmp(msg, "play:connect", 12) == 0) {
		// Jouer la mélodie de connexion
		Play_Melody(connection_melody, connection_length);
	}
	else if (strncmp(msg, "play:disconnect", 15) == 0) {
		// Jouer la mélodie de déconnexion
		Play_Melody(disconnection_melody, disconnection_length);
	}
	else if (strncmp(msg, "play:gamestart", 10) == 0) {
		// Jouer la mélodie de début de partie
		Play_Melody(game_start_melody, game_start_length);
	}
	else if (strncmp(msg, "play:hit", 8) == 0) {
		// Jouer le son de touche
		Play_Melody(pong_hit_sound, pong_hit_length);
	}
	else if (strncmp(msg, "play:victory", 12) == 0) {
		// Jouer la mélodie de victoire
		Play_Melody(victory_melody, victory_length);
	}
	else if (strncmp(msg, "play:defeat", 11) == 0) {
		// Jouer la mélodie de défaite
		Play_Melody(defeat_melody, defeat_length);
	}
    else if (strncmp(msg, "echo:", 5) == 0) {
        // Renvoyer le message (pour tester)
        printf("%s\r\n", msg + 5);
    }
    else if (strncmp(msg, "beep:", 5) == 0) {
        // Format simple: "beep:duree"
        // Exemple: "beep:500" pour un bip de 500ms à 1000Hz
        uint32_t duration = atoi(msg + 5);
        if (duration > 0 && duration < 5000) {  // Limiter à 5 secondes
            Set_Buzzer_Frequency(1000);  // Fréquence fixe de 1000Hz
            LL_mDelay(duration);
            Set_Buzzer_Frequency(0);  // Arrêter le son
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */
  // Configurer l'UART pour la communication série
  LL_USART_EnableIT_RXNE(USART2);

  // Configurer le timer pour le buzzer
  LL_TIM_EnableCounter(TIM2);

  // Configuration ADC avec API LL
  MX_ADC1_Init();
  ADC1_Calibration();
  LL_ADC_Enable(ADC1);
  while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {
    /* Attente de l'activation de l'ADC */
  }

  // Jouer la mélodie de démarrage
  Play_Melody(init_melody, init_length);

  // Message de démarrage
  printf("\r\n\r\n---- GESIEA v1.0.0 - STM32 GamePad Initialized ----\r\n");
  printf("Available commands:\r\n");
  printf("  play:connect    - Play connection melody\r\n");
  printf("  play:disconnect - Play disconnection melody\r\n");
  printf("  play:gamestart  - Play game start melody\r\n");
  printf("  play:hit        - Play hit sound\r\n");
  printf("  play:victory    - Play victory melody\r\n");
  printf("  play:defeat     - Play defeat melody\r\n");
  printf("  beep:duration   - Beep for duration (ms)\r\n");
  printf("  echo:message    - Echo back message\r\n");
  printf("-------------------------------------\r\n\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* --- Contrôle de la LED via les entrées numériques --- */
    if (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_5)) {
      LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
    } else {
      LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
    }

    if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_10)) {
      LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
    } else {
      LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
    }

    /* --- Lecture du joystick via l'ADC --- */
    // Pour STM32L0, la sélection du canal se fait par le registre CHSELR.
    // Lecture sur le canal 0 (PA0 pour X)
    ADC1->CHSELR = ADC_CHSELR_CHSEL0;  // Sélectionne uniquement le canal 0
    LL_ADC_REG_StartConversion(ADC1);
    while (!LL_ADC_IsActiveFlag_EOC(ADC1));
    x_value = LL_ADC_REG_ReadConversionData12(ADC1);
    LL_ADC_ClearFlag_EOC(ADC1);

    // Lecture sur le canal 1 (PA1 pour Y)
    ADC1->CHSELR = ADC_CHSELR_CHSEL1;  // Sélectionne uniquement le canal 1
    LL_ADC_REG_StartConversion(ADC1);
    while (!LL_ADC_IsActiveFlag_EOC(ADC1));
    y_value = LL_ADC_REG_ReadConversionData12(ADC1);
    LL_ADC_ClearFlag_EOC(ADC1);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  while (LL_PWR_IsActiveFlag_VOS() != 0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }

  LL_Init1msTick(16000000);

  LL_SetSystemCoreClock(16000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
