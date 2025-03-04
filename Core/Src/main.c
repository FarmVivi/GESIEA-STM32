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
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
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
#define NOTE_CS5 554.37  // Do#5/Ré♭5
#define NOTE_D5  587.33  // Ré5
#define NOTE_DS5 622.25  // Ré#5/Mi♭5
#define NOTE_E5  659.25  // Mi5

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
#define MUSIC_TIM TIM22
#define MUSIC_CHANNEL LL_TIM_CHANNEL_CH1
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

// Mélodie de mise en pause
Note pause_melody[] = {
    {NOTE_C5, 100}, {NOTE_A4, 100}, {NOTE_F4, 100}, {NOTE_D4, 200}
};
size_t pause_length = sizeof(pause_melody) / sizeof(Note);

// Mélodie de reprise
Note resume_melody[] = {
    {NOTE_D4, 100}, {NOTE_F4, 100}, {NOTE_A4, 100}, {NOTE_C5, 200}
};
size_t resume_length = sizeof(resume_melody) / sizeof(Note);

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

// Mélodie de fond pour le jeu (boucle)
Note background_melody[] = {
    {NOTE_C4, 200}, {NOTE_E4, 200}, {NOTE_G4, 200}, {NOTE_C5, 200}, {NOTE_G4, 200}, {NOTE_E4, 200},
    {NOTE_D4, 200}, {NOTE_F4, 200}, {NOTE_A4, 200}, {NOTE_D5, 200}, {NOTE_A4, 200}, {NOTE_F4, 200},
    {NOTE_E4, 200}, {NOTE_G4, 200}, {NOTE_B4, 200}, {NOTE_E5, 200}, {NOTE_B4, 200}, {NOTE_G4, 200}
};
size_t background_length = sizeof(background_melody) / sizeof(Note);

// Index de la mélodie de fond
size_t background_index = 0;

// Jeu
Game game =	{
	.ball_x = GAME_GRID_SIZE / 2,
	.ball_y = GAME_GRID_SIZE / 2,
	.ball_dx = 1,
	.ball_dy = 1,
	.paddle_left = GAME_GRID_SIZE / 2,
	.paddle_right = GAME_GRID_SIZE / 2,
	.status = GAME_STATUS_NONE
};
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

// Fonction qui génère un nombre pseudo-aléatoire
uint32_t get_random_number()
{
	static uint32_t seed = 0;
	seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
	return seed;
}

// Renvoie un nombre aléatoire compris entre min et max (inclus)
uint32_t get_random_number_range(uint32_t min, uint32_t max)
{
    return min + (get_random_number() % (max - min + 1));
}

// Fonction permettant de lire la valeur numérique depuis une entrée analogique
uint16_t Read_ADC_Value(ADC_TypeDef *ADCx, uint32_t Channel) {
	// Sélectionner le canal
	LL_ADC_REG_SetSequencerChRem(ADC1, LL_ADC_CHANNEL_0);
	// Démarrer la conversion
	LL_ADC_REG_StartConversion(ADCx);
	// Attendre la fin de la conversion
	while (!LL_ADC_IsActiveFlag_EOC(ADCx));
	// Lire la valeur
	uint16_t value = LL_ADC_REG_ReadConversionData12(ADCx);
	// Effacer le flag de fin de conversion
	LL_ADC_ClearFlag_EOC(ADCx);
	return value;
}

// Fonction pour définir la fréquence du buzzer
void Set_Buzzer_Frequency(TIM_TypeDef *TIMx, uint32_t Channels, uint32_t frequency) {
    if (frequency == 0) {
        LL_TIM_CC_DisableChannel(TIMx, Channels);
        return;
    }
    LL_TIM_CC_EnableChannel(TIMx, Channels);
    uint32_t timer_clock = 16000000; // Fréquence du timer (16 MHz)
    uint32_t arr_value = (timer_clock / frequency) - 1;
    LL_TIM_SetAutoReload(TIMx, arr_value);
    //LL_TIM_OC_SetCompareCH2(TIMx, arr_value / 2); // Rapport cyclique de 50%
    // Lire sur les bons channels en fonction de Channels qui peut être LL_TIM_CHANNEL_CH1 ou LL_TIM_CHANNEL_CH2 ou bien une combinaison de plusieurs (comparer Channels avec les masques (constantes LL_TIM_CHANNEL_CH1 et LL_TIM_CHANNEL_CH2))
    if (Channels & LL_TIM_CHANNEL_CH1) {
		LL_TIM_OC_SetCompareCH1(TIMx, arr_value / 2);
	}
	if (Channels & LL_TIM_CHANNEL_CH2) {
		LL_TIM_OC_SetCompareCH2(TIMx, arr_value / 2);
	}
	if (Channels & LL_TIM_CHANNEL_CH3) {
		LL_TIM_OC_SetCompareCH3(TIMx, arr_value / 2);
	}
	if (Channels & LL_TIM_CHANNEL_CH4) {
		LL_TIM_OC_SetCompareCH4(TIMx, arr_value / 2);
	}
}

// Fonction pour jouer une note
void Play_Note(TIM_TypeDef *TIMx, uint32_t Channels, Note note) {
    Set_Buzzer_Frequency(TIMx, Channels, note.frequency);
    LL_mDelay(note.duration);
    Set_Buzzer_Frequency(TIMx, Channels, 0); // Arrêter le son
    LL_mDelay(50); // Pause entre les notes
}

// Fonction pour jouer une mélodie
void Play_Melody(TIM_TypeDef *TIMx, uint32_t Channels, Note* melody, size_t length) {
    for (size_t i = 0; i < length; i++) {
        Play_Note(TIMx, Channels, melody[i]);
    }
}

// Fonction pour envoyer toutes les données du jeu à l'IHH
void Send_Game_All_Data() {
	// Format: "game:all:status,grip_size,paddle_size,ball_x,ball_y,ball_dx,ball_dy,paddle_left,paddle_right"
	printf("game:all:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
		game.status, GAME_GRID_SIZE, GAME_PADDLE_SIZE,
		game.ball_x, game.ball_y, game.ball_dx, game.ball_dy,
		game.paddle_left, game.paddle_right);
}

// Fonction pour envoyer les données de refresh en cours de jeu à l'IHM
void Send_Game_Run_Data() {
	// Format: "game:run:x,y,dx,dy,left,right,status"
	printf("game:run:%d,%d,%d,%d,%d,%d,%d\r\n",
		game.ball_x, game.ball_y, game.ball_dx, game.ball_dy,
		game.paddle_left, game.paddle_right, game.status);
}

// Fonction pour arrêter la mélodie de fond
void Stop_Play_Melody() {
	Set_Buzzer_Frequency(MUSIC_TIM, MUSIC_CHANNEL, 0);
}

// Fonction pour mettre à jour la mélodie de fond
void Update_Play_Melody() {
	// Récupérer la note
	Note note = background_melody[background_index];
	// Jouer la note suivante (sans pause)
	Set_Buzzer_Frequency(MUSIC_TIM, MUSIC_CHANNEL, note.frequency);
	// Incrémenter l'index
	background_index = (background_index + 1) % background_length;
}

// Fonction pour initialiser le jeu Pong
void Init_Game() {
    // Réinitialiser la position de la balle au centre
    game.ball_x = GAME_GRID_SIZE / 2;
    game.ball_y = GAME_GRID_SIZE / 2;
    
    // Utiliser notre fonction random pour déterminer la direction horizontale de la balle
    // On choisit 2 ou -2 de façon aléatoire
    if (get_random_number_range(0, 1) == 0) {
        game.ball_dx = 2;  // Vers la droite
    } else {
        game.ball_dx = -2; // Vers la gauche
    }
    
    // Pour la vitesse verticale, on choisit aléatoirement parmi 1, 2 ou -1
    uint32_t r = get_random_number_range(0, 2);
    if (r == 0) {
        game.ball_dy = 1;  // Vers le bas, lentement
    } else if (r == 1) {
        game.ball_dy = 2;  // Vers le bas, rapidement
    } else {
        game.ball_dy = -1; // Vers le haut
    }
    
    // Positionner les raquettes au centre
    game.paddle_left = (GAME_GRID_SIZE - GAME_PADDLE_SIZE) / 2;
    game.paddle_right = (GAME_GRID_SIZE - GAME_PADDLE_SIZE) / 2;
    
    // Statut initial
    game.status = GAME_STATUS_NONE;
}

// Fonction pour lancer la partie
void Start_Game() {
    // Initialiser le jeu
    Init_Game();
    
    // Jouer la mélodie de démarrage
    Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2, game_start_melody, game_start_length);
    
    // Définir le statut à "running"
    game.status = GAME_STATUS_RUNNING;
    
    // Envoyer les données du jeu à l'IHM
    Send_Game_All_Data();
    
    // Activer le timer qui fera les appels réguliers à Update_Game()
    LL_SYSTICK_EnableIT();
    
    // Démarrer la musique de fond
    background_index = 0;
    Update_Play_Melody();
}

// Fonction pour mettre en pause la partie
void Pause_Game() {
	LL_SYSTICK_DisableIT();
	game.status = GAME_STATUS_PAUSED;
	Send_Game_All_Data();
	Set_Buzzer_Frequency(MUSIC_TIM, MUSIC_CHANNEL, 0);
	Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2, pause_melody, pause_length);
}

// Fonction pour reprendre la partie
void Resume_Game() {
	Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2, resume_melody, resume_length);
	game.status = GAME_STATUS_RUNNING;
	Send_Game_All_Data();
	LL_SYSTICK_EnableIT();
}

// Fonction pour réinitialiser le jeu
void Reset_Game() {
    // Réinitialiser le jeu
    Init_Game();

    // Envoyer les données à l'IHM pour actualiser l'affichage
    Send_Game_All_Data();

    // Arrêter la musique de fond
    Stop_Play_Melody();
}

// Fonction pour mettre à jour le jeu (lire les entrées, mettre à jour les positions, gérer victoire/défaite, envoyer les données à l'IHM)
void Update_Game() {
    // Lire les entrées des joysticks pour les deux joueurs
    uint16_t joystick_y_p1 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P1);
    uint16_t joystick_y_p2 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P2);
    
    // Convertir les valeurs ADC en mouvements de raquette (ADC renvoie typiquement 0-4095)
    // Si joystick_y > 2048 + seuil, monter la raquette
    // Si joystick_y < 2048 - seuil, descendre la raquette
    const uint16_t threshold = 500; // Zone morte pour éviter des mouvements involontaires
    const uint8_t paddle_speed = 5; // Vitesse de déplacement des raquettes
    
    // Mise à jour de la position de la raquette gauche (P1)
    if (joystick_y_p1 > 2048 + threshold) {
        // Monter la raquette (diminuer y)
        if (game.paddle_left >= paddle_speed) {
            game.paddle_left -= paddle_speed;
        } else {
            game.paddle_left = 0;
        }
    } else if (joystick_y_p1 < 2048 - threshold) {
        // Descendre la raquette (augmenter y)
        if (game.paddle_left + GAME_PADDLE_SIZE + paddle_speed <= GAME_GRID_SIZE) {
            game.paddle_left += paddle_speed;
        } else {
            game.paddle_left = GAME_GRID_SIZE - GAME_PADDLE_SIZE;
        }
    }
    
    // Mise à jour de la position de la raquette droite (P2)
    if (joystick_y_p2 > 2048 + threshold) {
        // Monter la raquette (diminuer y)
        if (game.paddle_right >= paddle_speed) {
            game.paddle_right -= paddle_speed;
        } else {
            game.paddle_right = 0;
        }
    } else if (joystick_y_p2 < 2048 - threshold) {
        // Descendre la raquette (augmenter y)
        if (game.paddle_right + GAME_PADDLE_SIZE + paddle_speed <= GAME_GRID_SIZE) {
            game.paddle_right += paddle_speed;
        } else {
            game.paddle_right = GAME_GRID_SIZE - GAME_PADDLE_SIZE;
        }
    }
    
    // Mise à jour de la position de la balle
    game.ball_x += game.ball_dx;
    game.ball_y += game.ball_dy;
    
    // Gestion des collisions avec les bords supérieur et inférieur
    if (game.ball_y <= 0 || game.ball_y + GAME_BALL_SIZE >= GAME_GRID_SIZE) {
        game.ball_dy = -game.ball_dy; // Inverser la direction verticale
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2, pong_hit_sound, pong_hit_length);
    }
    
    // Gestion des collisions avec les raquettes
    const uint8_t paddle_margin = 10; // Distance entre le bord et la raquette
    
    // Collision avec la raquette gauche
    if (game.ball_x <= paddle_margin + GAME_BALL_SIZE && 
        game.ball_y + GAME_BALL_SIZE >= game.paddle_left && 
        game.ball_y <= game.paddle_left + GAME_PADDLE_SIZE) {
        game.ball_dx = -game.ball_dx; // Inverser la direction horizontale
        game.ball_x = paddle_margin + GAME_BALL_SIZE; // Repositionner la balle pour éviter les collisions multiples
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1, pong_hit_sound, pong_hit_length);
    }
    
    // Collision avec la raquette droite
    if (game.ball_x + GAME_BALL_SIZE >= GAME_GRID_SIZE - paddle_margin && 
        game.ball_y + GAME_BALL_SIZE >= game.paddle_right && 
        game.ball_y <= game.paddle_right + GAME_PADDLE_SIZE) {
        game.ball_dx = -game.ball_dx; // Inverser la direction horizontale
        game.ball_x = GAME_GRID_SIZE - paddle_margin - GAME_BALL_SIZE; // Repositionner la balle
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P2, pong_hit_sound, pong_hit_length);
    }
    
    // Gestion des conditions de victoire/défaite
    if (game.ball_x <= 0) {
        // Joueur 2 gagne
        game.status = GAME_STATUS_NONE;
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P2, victory_melody, victory_length);
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1, defeat_melody, defeat_length);
        // Réinitialiser le jeu
        Reset_Game();
        return; // Sortir pour éviter d'envoyer les données
    }
    
    if (game.ball_x + GAME_BALL_SIZE >= GAME_GRID_SIZE) {
        // Joueur 1 gagne
        game.status = GAME_STATUS_NONE;
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1, victory_melody, victory_length);
        Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P2, defeat_melody, defeat_length);
        // Réinitialiser le jeu
        Reset_Game();
        return; // Sortir pour éviter d'envoyer les données
    }
    
    // Envoyer les données à l'IHM
    Send_Game_Run_Data();
}

// Fonction de callback pour les ordres reçues par l'UART
void UART_Callback(char* msg, size_t length) {
    uint32_t buzzer_channels = BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2;
    char* colon_pos = strchr(msg, ':');

    // Vérifier s'il y a un troisième argument (sélection du buzzer)
    if (colon_pos) {
        char* third_arg = strchr(colon_pos + 1, ':');
        if (third_arg) {
            // Convertir le troisième argument en entier
            int buzzer_select = atoi(third_arg + 1);
            switch (buzzer_select) {
                case 1:
                    buzzer_channels = BUZZER_CHANNEL_P1;
                    break;
                case 2:
                    buzzer_channels = BUZZER_CHANNEL_P2;
                    break;
                default:
                    // Si argument invalide, utiliser les deux buzzers
                    buzzer_channels = BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2;
                    break;
            }
        }
    }

    // Commandes avec sélection de buzzer possible
    if (strncmp(msg, "play:connect", 12) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, connection_melody, connection_length);
    }
    else if (strncmp(msg, "play:disconnect", 15) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, disconnection_melody, disconnection_length);
    }
    else if (strncmp(msg, "play:gamestart", 14) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, game_start_melody, game_start_length);
    }
    else if (strncmp(msg, "play:pause", 10) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, pause_melody, pause_length);
    }
    else if (strncmp(msg, "play:resume", 11) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, resume_melody, resume_length);
    }
    else if (strncmp(msg, "play:hit", 8) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, pong_hit_sound, pong_hit_length);
    }
    else if (strncmp(msg, "play:victory", 12) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, victory_melody, victory_length);
    }
    else if (strncmp(msg, "play:defeat", 11) == 0) {
        Play_Melody(BUZZER_TIM, buzzer_channels, defeat_melody, defeat_length);
    }
    else if (strncmp(msg, "echo:", 5) == 0) {
        // Renvoyer le message (pour tester)
        printf("%s\r\n", msg + 5);
    }
    else if (strncmp(msg, "beep:", 5) == 0) {
        // Formats:
        // "beep:duration" - utilise les deux buzzers
        // "beep:duration:1" - utilise le buzzer 1
        // "beep:duration:2" - utilise le buzzer 2
        char* duration_str = msg + 5;
        uint32_t duration = atoi(duration_str);

        // Vérifier et ajuster duration si un troisième argument existe
        char* third_arg = strchr(duration_str, ':');
        if (third_arg) {
            duration = atoi(duration_str);
            int buzzer_select = atoi(third_arg + 1);

            switch (buzzer_select) {
                case 1:
                    buzzer_channels = BUZZER_CHANNEL_P1;
                    break;
                case 2:
                    buzzer_channels = BUZZER_CHANNEL_P2;
                    break;
                default:
                    buzzer_channels = BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2;
                    break;
            }
        }

        if (duration > 0 && duration < 5000) {  // Limiter à 5 secondes
            Set_Buzzer_Frequency(BUZZER_TIM, buzzer_channels, 1000);  // Fréquence fixe de 1000Hz
            LL_mDelay(duration);
            Set_Buzzer_Frequency(BUZZER_TIM, buzzer_channels, 0);  // Arrêter le son
        }
    }
}

// Fonction de callback pour le bouton poussoir bleu sur la carte
void Blue_Button_Callback() {
	// Si la partie n'est pas en cours, démarrer la partie
	if (game.status == GAME_STATUS_NONE) {
		Start_Game();
	} else if (game.status == GAME_STATUS_RUNNING) {
		Pause_Game();
	} else if (game.status == GAME_STATUS_PAUSED) {
		Resume_Game();
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
  MX_TIM22_Init();
  /* USER CODE BEGIN 2 */
  // Configurer l'UART pour la communication série
  LL_USART_EnableIT_RXNE(USART2);

  // Configurer le timer pour les buzzer
  LL_TIM_EnableCounter(TIM2);
  LL_TIM_EnableCounter(TIM22);

  // Configurer l'ADC
  LL_ADC_Enable(ADC1);
  while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {
    /* Attente de l'activation de l'ADC */
  }

  // Jouer la mélodie de démarrage
  Play_Melody(BUZZER_TIM, BUZZER_CHANNEL_P1 | BUZZER_CHANNEL_P2, init_melody, init_length);

  // Message de démarrage
  printf("\r\n\r\n---- GESIEA v1.0.0 - STM32 GamePad Initialized ----\r\n");
  printf("Available commands:\r\n");
  printf("  play:connect    - Play connection melody\r\n");
  printf("  play:disconnect - Play disconnection melody\r\n");
  printf("  play:gamestart  - Play game start melody\r\n");
  printf("  play:pause      - Play pause melody\r\n");
  printf("  play:resume     - Play resume melody\r\n");
  printf("  play:hit        - Play hit sound\r\n");
  printf("  play:hit:buzz   - Play hit sound using buzzer (1 or 2)\r\n");
  printf("  play:victory    - Play victory melody\r\n");
  printf("  play:defeat     - Play defeat melody\r\n");
  printf("  beep:dura       - Beep for duration (ms)\r\n");
  printf("  beep:dura:buzz  - Beep for duration (ms) using buzzer (1 or 2)\r\n");
  printf("  echo:message    - Echo back message\r\n");
  printf("-------------------------------------\r\n\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
