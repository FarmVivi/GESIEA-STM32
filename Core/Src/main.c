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
    uint32_t frequency;    // Fréquence en Hz
} Note;

// Structure pour gérer une mélodie en lecture non-bloquante
typedef struct {
    Note* melody;              // Tableau de notes pour la mélodie
    size_t length;             // Longueur de la mélodie
    size_t currentIndex;       // Index de la note en cours
    uint32_t channels;         // Canaux sur lesquels jouer
    TIM_TypeDef *timer;        // Timer à utiliser
    uint8_t isPlaying;         // Indique si une mélodie est en cours
    uint8_t loopMode;          // Indique si la mélodie doit être jouée en boucle
} MelodyPlayer;

// Structure pour le status du jeu
typedef struct {
    uint16_t grid_width;
    uint16_t grid_height;
    uint16_t ball_x;
    uint16_t ball_y;
    int8_t ball_dx;
    int8_t ball_dy;
    uint16_t paddle_left_x;
    uint16_t paddle_left_y;
    uint16_t paddle_right_x;
    uint16_t paddle_right_y;
    uint8_t paddle_left_size;
    uint8_t paddle_right_size;
    uint8_t paddle_width;
    uint8_t ball_size;
    uint8_t paddle_speed;
    uint8_t status;
    uint8_t max_points;
    uint8_t player1_points;
    uint8_t player2_points;
    uint16_t left_zone_width;
    uint16_t right_zone_width;
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
#define GAME_STATUS_FINISHED 3

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
#define JOYSTICK_X_CHANNEL_P2 LL_ADC_CHANNEL_10
#define JOYSTICK_Y_CHANNEL_P2 LL_ADC_CHANNEL_11
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Mélodie de démarrage
Note init_melody[] = {
    {NOTE_G4}, {NOTE_A4}, {NOTE_B4}, {NOTE_C5}
};
size_t init_length = sizeof(init_melody) / sizeof(Note);

// Mélodie de connexion
Note connection_melody[] = {
    {NOTE_C4}, {NOTE_E4}, {NOTE_G4}, {NOTE_C5}, {NOTE_G4}, {NOTE_E4}
};
size_t connection_length = sizeof(connection_melody) / sizeof(Note);

// Mélodie de déconnexion
Note disconnection_melody[] = {
    {NOTE_C5}, {NOTE_G4}, {NOTE_E4}, {NOTE_C4}
};
size_t disconnection_length = sizeof(disconnection_melody) / sizeof(Note);

// Mélodie de début de partie
Note game_start_melody[] = {
    {NOTE_E4}, {NOTE_F4}, {NOTE_G4},
    {NOTE_A4}, {NOTE_B4}, {NOTE_C5}
};
size_t game_start_length = sizeof(game_start_melody) / sizeof(Note);

// Mélodie de mise en pause
Note pause_melody[] = {
    {NOTE_C5}, {NOTE_A4}, {NOTE_F4}, {NOTE_D4}
};
size_t pause_length = sizeof(pause_melody) / sizeof(Note);

// Mélodie de reprise
Note resume_melody[] = {
    {NOTE_D4}, {NOTE_F4}, {NOTE_A4}, {NOTE_C5}
};
size_t resume_length = sizeof(resume_melody) / sizeof(Note);

// Mélodie de touche
Note pong_hit_sound[] = {
    {NOTE_E4}, {NOTE_G4}
};
size_t pong_hit_length = sizeof(pong_hit_sound) / sizeof(Note);

// Mélodie de victoire
Note victory_melody[] = {
    {NOTE_C4}, {NOTE_E4}, {NOTE_G4}, {NOTE_C5}
};
size_t victory_length = sizeof(victory_melody) / sizeof(Note);

// Mélodie de défaite
Note defeat_melody[] = {
    {NOTE_C5}, {NOTE_G4}, {NOTE_E4}, {NOTE_C4}
};
size_t defeat_length = sizeof(defeat_melody) / sizeof(Note);

// Mélodie de fond pour le jeu (boucle)
Note background_melody[] = {
    {NOTE_C4}, {NOTE_E4}, {NOTE_G4}, {NOTE_C5}, {NOTE_G4}, {NOTE_E4},
    {NOTE_D4}, {NOTE_F4}, {NOTE_A4}, {NOTE_D5}, {NOTE_A4}, {NOTE_F4},
    {NOTE_E4}, {NOTE_G4}, {NOTE_B4}, {NOTE_E5}, {NOTE_B4}, {NOTE_G4}
};
size_t background_length = sizeof(background_melody) / sizeof(Note);

// Variables globales pour la gestion des mélodies - un player par buzzer
MelodyPlayer buzzer1Player = {0};      // Pour le buzzer du joueur 1
MelodyPlayer buzzer2Player = {0};      // Pour le buzzer du joueur 2
MelodyPlayer backgroundPlayer = {0};   // Pour la musique de fond

// Jeu
Game game =	{
	.grid_width = 400,
	.grid_height = 250,
	.ball_x = 200,
	.ball_y = 125,
	.ball_dx = 1,
	.ball_dy = 1,
    .paddle_left_x = 20,
    .paddle_left_y = 125,
    .paddle_right_x = 380,
    .paddle_right_y = 125,
	.paddle_left_size = 6,
	.paddle_right_size = 6,
    .paddle_width = 8,
	.ball_size = 3,
	.paddle_speed = 5,
	.status = GAME_STATUS_NONE,
    .max_points = 5,
    .player1_points = 0,
    .player2_points = 0,
    .left_zone_width = 100,
    .right_zone_width = 100
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
	LL_ADC_REG_SetSequencerChannels(ADCx, Channel);
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

// Fonction pour démarrer une mélodie
void Start_Melody(MelodyPlayer *player, TIM_TypeDef *TIMx, uint32_t Channels, Note* melody, size_t length, uint8_t loop) {
    // Si une mélodie est déjà en cours sur ce player, l'arrêter
    if (player->isPlaying) {
        Set_Buzzer_Frequency(player->timer, player->channels, 0);
    }
    
    // Configurer la nouvelle mélodie
    player->melody = melody;
    player->length = length;
    player->currentIndex = 0;
    player->channels = Channels;
    player->timer = TIMx;
    player->isPlaying = 1;
    player->loopMode = loop;
    
    // La note sera jouée au prochain appel à Update_Sound
}

// Fonction pour arrêter une mélodie
void Stop_Melody(MelodyPlayer *player) {
    if (player->isPlaying) {
        Set_Buzzer_Frequency(player->timer, player->channels, 0);
        player->isPlaying = 0;
    }
}

// Jouer un son sur le buzzer 1
void Play_Sound_P1(Note* melody, size_t length) {
    Start_Melody(&buzzer1Player, BUZZER_TIM, BUZZER_CHANNEL_P1, melody, length, 0);
}

// Jouer un son sur le buzzer 2
void Play_Sound_P2(Note* melody, size_t length) {
    Start_Melody(&buzzer2Player, BUZZER_TIM, BUZZER_CHANNEL_P2, melody, length, 0);
}

// Jouer un son sur les deux buzzers
void Play_Sound(Note* melody, size_t length) {
    Play_Sound_P1(melody, length);
    Play_Sound_P2(melody, length);
}

// Fonction pour démarrer la musique de fond
void Start_Music(Note* melody, size_t length) {
    Start_Melody(&backgroundPlayer, MUSIC_TIM, MUSIC_CHANNEL, melody, length, 1);
}

// Fonction pour arrêter la musique de fond
void Stop_Music() {
    Stop_Melody(&backgroundPlayer);
}

// Fonction pour reprendre la musique de fond
void Resume_Music() {
	backgroundPlayer.isPlaying = 1;
}

// Fonction pour jouer les sons - appelée toutes les 250ms
void Update_Sound() {
    // Mise à jour du buzzer 1
    if (buzzer1Player.isPlaying) {
        // Jouer la note actuelle
        if (buzzer1Player.currentIndex < buzzer1Player.length) {
            Note currentNote = buzzer1Player.melody[buzzer1Player.currentIndex];
            Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, currentNote.frequency);
            buzzer1Player.currentIndex++;

            // Vérifier si nous avons atteint la fin de la mélodie
            if (buzzer1Player.currentIndex >= buzzer1Player.length) {
                if (!buzzer1Player.loopMode) {
                    // Marquer comme terminé et arrêter le son immédiatement pour la dernière note
                    buzzer1Player.isPlaying = 0;
                    // La note sera arrêtée par Stop_Sound()
                } else {
                    // Boucler au début
                    buzzer1Player.currentIndex = 0;
                }
            }
        }
    }

    // Mise à jour du buzzer 2
    if (buzzer2Player.isPlaying) {
        // Jouer la note actuelle
        if (buzzer2Player.currentIndex < buzzer2Player.length) {
            Note currentNote = buzzer2Player.melody[buzzer2Player.currentIndex];
            Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, currentNote.frequency);
            buzzer2Player.currentIndex++;
            
            // Vérifier si nous avons atteint la fin de la mélodie
            if (buzzer2Player.currentIndex >= buzzer2Player.length) {
                if (!buzzer2Player.loopMode) {
                    // Marquer comme terminé et arrêter le son immédiatement pour la dernière note
                    buzzer2Player.isPlaying = 0;
                    // La note sera arrêtée par Stop_Sound()
                } else {
                    // Boucler au début
                    buzzer2Player.currentIndex = 0;
                }
            }
        }
    }

    // Mise à jour de la musique de fond
    if (backgroundPlayer.isPlaying) {
        // Jouer la note actuelle de la musique de fond
        if (backgroundPlayer.currentIndex < backgroundPlayer.length) {
            Note currentNote = backgroundPlayer.melody[backgroundPlayer.currentIndex];
            Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, currentNote.frequency);
            backgroundPlayer.currentIndex++;
            
            // Vérifier si nous avons atteint la fin de la mélodie
            if (backgroundPlayer.currentIndex >= backgroundPlayer.length) {
                if (backgroundPlayer.loopMode) {
                    // Boucler au début
                    backgroundPlayer.currentIndex = 0;
                } else {
                    // Marquer comme terminé et arrêter le son immédiatement pour la dernière note
                    backgroundPlayer.isPlaying = 0;
                    // La note sera arrêtée par Stop_Sound()
                }
            }
        }
    }
}

// Fonction pour arrêter les sons - appelée 50ms après Update_Sound
void Stop_Sound() {
    // Arrêter les notes courantes pour créer une pause entre les notes
    if (buzzer1Player.isPlaying) {
        Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, 0);
    } else if (buzzer1Player.currentIndex >= buzzer1Player.length && !buzzer1Player.loopMode) {
        // Si la mélodie vient de se terminer, s'assurer que le son est arrêté
        Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, 0);
    }

    if (buzzer2Player.isPlaying) {
        Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, 0);
    } else if (buzzer2Player.currentIndex >= buzzer2Player.length && !buzzer2Player.loopMode) {
        // Si la mélodie vient de se terminer, s'assurer que le son est arrêté
        Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, 0);
    }

    if (backgroundPlayer.isPlaying) {
        Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, 0);
    } else if (backgroundPlayer.currentIndex >= backgroundPlayer.length && !backgroundPlayer.loopMode) {
        // Si la mélodie vient de se terminer, s'assurer que le son est arrêté
        Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, 0);
    }
}

// Fonction pour envoyer toutes les données du jeu à l'IHM
void Send_Game_All_Data() {
    // Format: "game:all:status,grid_width,grid_height,ball_size,ball_x,ball_y,ball_dx,ball_dy,
    //         paddle_left_x,paddle_left_y,paddle_left_size,paddle_width,
    //         paddle_right_x,paddle_right_y,paddle_right_size,max_points,player1_points,player2_points,
    //         left_zone_width,right_zone_width"
    printf("game:all:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        game.status, game.grid_width, game.grid_height, game.ball_size,
        game.ball_x, game.ball_y, game.ball_dx, game.ball_dy,
        game.paddle_left_x, game.paddle_left_y, game.paddle_left_size, game.paddle_width,
        game.paddle_right_x, game.paddle_right_y, game.paddle_right_size,
        game.max_points, game.player1_points, game.player2_points,
        game.left_zone_width, game.right_zone_width);
}

// Fonction pour envoyer les données de refresh en cours de jeu à l'IHM
void Send_Game_Run_Data() {
    // Format: "game:run:status,ball_x,ball_y,ball_dx,ball_dy,ball_size,
    //         paddle_left_x,paddle_left_y,paddle_left_size,paddle_width,
    //         paddle_right_x,paddle_right_y,paddle_right_size,p1points,p2points"
    printf("game:run:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        game.status, game.ball_x, game.ball_y, game.ball_dx, game.ball_dy, game.ball_size,
        game.paddle_left_x, game.paddle_left_y, game.paddle_left_size, game.paddle_width,
        game.paddle_right_x, game.paddle_right_y, game.paddle_right_size,
        game.player1_points, game.player2_points);
}

// Fonction pour initialiser le jeu Pong
void Init_Game(uint16_t width, uint16_t height, uint8_t max_points, uint8_t ball_velocity, uint8_t ball_size, uint8_t paddle_velocity, uint8_t paddle_size) {
    // Initialiser les dimensions de la grille
    game.grid_width = width;
    game.grid_height = height;

    // Initialiser la taille des raquettes
    game.paddle_left_size = paddle_size;
    game.paddle_right_size = paddle_size;
    game.paddle_width = 8;  // Largeur fixe des raquettes
    
    // Initialiser la taille de la balle
    game.ball_size = ball_size;
    
    // Initialiser la vitesse des raquettes
    game.paddle_speed = paddle_velocity;

    // Initialiser les points
    game.max_points = max_points;
    game.player1_points = 0;
    game.player2_points = 0;

    // Initialiser les zones de jeu des joueurs
    game.left_zone_width = width / 4;   // 25% du terrain à gauche
    game.right_zone_width = width / 4;  // 25% du terrain à droite

    // Réinitialiser la position de la balle au centre
    game.ball_x = game.grid_width / 2;
    game.ball_y = game.grid_height / 2;
    
    // Déterminer aléatoirement la direction horizontale de la balle
    if (get_random_number_range(0, 1) == 0) {
        game.ball_dx = 2 + ball_velocity;  // Vers la droite
    } else {
        game.ball_dx = -2 - ball_velocity; // Vers la gauche
    }
    
    // Choisir aléatoirement la vitesse verticale
    uint32_t r = get_random_number_range(0, 2);
    if (r == 0) {
        game.ball_dy = 1 + ball_velocity;  // Vers le bas, lentement
    } else if (r == 1) {
        game.ball_dy = 2 + ball_velocity;  // Vers le bas, rapidement
    } else {
        game.ball_dy = -1 - ball_velocity; // Vers le haut
    }
    
    // Positionner les raquettes
    game.paddle_left_x = 20;  // Position X initiale de la raquette gauche
    game.paddle_left_y = (game.grid_height - game.paddle_left_size) / 2;  // Centrer verticalement
    
    game.paddle_right_x = game.grid_width - 20 - game.paddle_width;  // Position X initiale de la raquette droite
    game.paddle_right_y = (game.grid_height - game.paddle_right_size) / 2;  // Centrer verticalement
    
    // Statut initial
    game.status = GAME_STATUS_NONE;
}

// Fonction pour lancer la partie
void Start_Game() {
    // Jouer la mélodie de démarrage
    Play_Sound(game_start_melody, game_start_length);
    
    // Définir le statut à "running"
    game.status = GAME_STATUS_RUNNING;
    
    // Allumer la LED du microcontrôleur
    LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

    // Envoyer les données du jeu à l'IHM
    Send_Game_All_Data();
    
    // Démarrer la musique de fond en boucle
    Start_Music(background_melody, background_length);
}

// Fonction pour mettre en pause la partie
void Pause_Game() {
    game.status = GAME_STATUS_PAUSED;
    LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
    Send_Game_All_Data();

    // Mettre en pause la musique de fond
    Stop_Music();

    // Jouer la mélodie de pause
    Play_Sound(pause_melody, pause_length);
}

// Fonction pour reprendre la partie
void Resume_Game() {
    // Jouer la mélodie de reprise
    Play_Sound(resume_melody, resume_length);

    game.status = GAME_STATUS_RUNNING;
    LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
    Send_Game_All_Data();

    // Reprendre la musique de fond
    Resume_Music();
}

// Fonction pour réinitialiser le jeu
void Stop_Game() {
    // Réinitialiser la LED
    LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

    // Arrêter la musique de fond
    Stop_Music();

    // Envoyer les données à l'IHM pour actualiser l'affichage
    Send_Game_All_Data();

    // Jouer la mélodie de fin de partie
    Play_Sound(disconnection_melody, disconnection_length);

    // Réinitialiser le jeu avec les valeurs actuelles
    Init_Game(game.grid_width, game.grid_height, 5, 1, 3, 5, 6);
}

// Fonction pour mettre à jour le jeu (lire les entrées, mettre à jour les positions, gérer victoire/défaite, envoyer les données à l'IHM)
void Update_Game() {
	// Si la partie est en cours
	if (game.status != GAME_STATUS_RUNNING) {
		return;
	}

    // Lire les entrées des joysticks pour les deux joueurs
	uint16_t joystick_y_p1 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P1);
	uint16_t joystick_y_p2 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P2);
	uint16_t joystick_x_p1 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_X_CHANNEL_P1);
	uint16_t joystick_x_p2 = Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_X_CHANNEL_P2);
    
    // Convertir les valeurs ADC en mouvements de raquette (ADC renvoie typiquement 0-4095)
    // Si joystick_y > 2048 + seuil, monter la raquette
    // Si joystick_y < 2048 - seuil, descendre la raquette
    const uint16_t threshold = 500; // Zone morte pour éviter des mouvements involontaires
    
    // Mise à jour de la position Y de la raquette gauche (P1)
    if (joystick_y_p1 > 2048 + threshold) {
        // Monter la raquette (diminuer y)
        if (game.paddle_left_y >= game.paddle_speed) {
            game.paddle_left_y -= game.paddle_speed;
        } else {
            game.paddle_left_y = 0;
        }
    } else if (joystick_y_p1 < 2048 - threshold) {
        // Descendre la raquette (augmenter y)
        if (game.paddle_left_y + game.paddle_left_size + game.paddle_speed <= game.grid_height) {
            game.paddle_left_y += game.paddle_speed;
        } else {
            game.paddle_left_y = game.grid_height - game.paddle_left_size;
        }
    }
    
    // Mise à jour de la position X de la raquette gauche (P1)
    if (joystick_x_p1 > 2048 + threshold) {
        // Déplacer la raquette vers la droite
        uint16_t max_x = game.left_zone_width - game.paddle_width;
        if (game.paddle_left_x + game.paddle_speed <= max_x) {
            game.paddle_left_x += game.paddle_speed;
        } else {
            game.paddle_left_x = max_x;
        }
    } else if (joystick_x_p1 < 2048 - threshold) {
        // Déplacer la raquette vers la gauche
        if (game.paddle_left_x >= game.paddle_speed) {
            game.paddle_left_x -= game.paddle_speed;
        } else {
            game.paddle_left_x = 0;
        }
    }
    
    // Mise à jour de la position Y de la raquette droite (P2)
    if (joystick_y_p2 > 2048 + threshold) {
        // Monter la raquette (diminuer y)
        if (game.paddle_right_y >= game.paddle_speed) {
            game.paddle_right_y -= game.paddle_speed;
        } else {
            game.paddle_right_y = 0;
        }
    } else if (joystick_y_p2 < 2048 - threshold) {
        // Descendre la raquette (augmenter y)
        if (game.paddle_right_y + game.paddle_right_size + game.paddle_speed <= game.grid_height) {
            game.paddle_right_y += game.paddle_speed;
        } else {
            game.paddle_right_y = game.grid_height - game.paddle_right_size;
        }
    }
    
    // Mise à jour de la position X de la raquette droite (P2)
    if (joystick_x_p2 > 2048 + threshold) {
        // Déplacer la raquette vers la droite
        if (game.paddle_right_x + game.paddle_speed <= game.grid_width - 1) {
            game.paddle_right_x += game.paddle_speed;
        } else {
            game.paddle_right_x = game.grid_width - 1;
        }
    } else if (joystick_x_p2 < 2048 - threshold) {
        // Déplacer la raquette vers la gauche
        uint16_t min_x = game.grid_width - game.right_zone_width;
        if (game.paddle_right_x >= min_x + game.paddle_speed) {
            game.paddle_right_x -= game.paddle_speed;
        } else {
            game.paddle_right_x = min_x;
        }
    }
    
    // Mise à jour de la position de la balle
    game.ball_x += game.ball_dx;
    game.ball_y += game.ball_dy;
    
    // Gestion des collisions avec les bords supérieur et inférieur
    if (game.ball_y <= 0 || game.ball_y + game.ball_size >= game.grid_height) {
        game.ball_dy = -game.ball_dy; // Inverser la direction verticale
        Play_Sound(pong_hit_sound, pong_hit_length);
    }
    
    // Gestion des collisions avec les raquettes
    
    // Collision avec la raquette gauche
    if (game.ball_dx < 0 // La balle se dirige vers la gauche
        && game.ball_x <= game.paddle_left_x + game.paddle_width
        && game.ball_x >= game.paddle_left_x
        && game.ball_y + game.ball_size >= game.paddle_left_y
        && game.ball_y <= game.paddle_left_y + game.paddle_left_size) {
        
        game.ball_dx = -game.ball_dx; // Inverser la direction horizontale
        
        // Repositionner la balle juste après la raquette pour éviter les collisions multiples
        game.ball_x = game.paddle_left_x + game.paddle_width;
        
        // Jouer le son de collision sur le buzzer du joueur 1
        Play_Sound_P1(pong_hit_sound, pong_hit_length);
    }
    
    // Collision avec la raquette droite
    if (game.ball_dx > 0 // La balle se dirige vers la droite
        && game.ball_x + game.ball_size >= game.paddle_right_x
        && game.ball_x + game.ball_size <= game.paddle_right_x + game.paddle_width
        && game.ball_y + game.ball_size >= game.paddle_right_y
        && game.ball_y <= game.paddle_right_y + game.paddle_right_size) {
        
        game.ball_dx = -game.ball_dx; // Inverser la direction horizontale
        
        // Repositionner la balle juste avant la raquette pour éviter les collisions multiples
        game.ball_x = game.paddle_right_x - game.ball_size;
        
        // Jouer le son de collision sur le buzzer du joueur 2
        Play_Sound_P2(pong_hit_sound, pong_hit_length);
    }
    
    // Gestion des conditions de score
    if (game.ball_x <= 0) {
        // Joueur 2 marque un point
        game.player2_points++;
        Play_Sound_P2(pong_hit_sound, pong_hit_length);
        
        // Vérifier si le joueur 2 a gagné la partie
        if (game.player2_points >= game.max_points) {
            // Fin de partie, le joueur 2 a gagné
            game.status = GAME_STATUS_FINISHED;

            // Jouer les mélodies de victoire/défaite en même temps sur les deux buzzers respectifs
            Play_Sound_P2(victory_melody, victory_length);
            Play_Sound_P1(defeat_melody, defeat_length);
            
            // Arrêter le jeu
            Stop_Game();
            return;
        }
        
        // Repositionner la balle au centre et inverser sa direction
        game.ball_x = game.grid_width / 2;
        game.ball_y = game.grid_height / 2;
        game.ball_dx = 2 + (game.player1_points + game.player2_points) / 3; // Augmenter légèrement la vitesse
        
        // Choisir aléatoirement la vitesse verticale
        uint32_t r = get_random_number_range(0, 2);
        if (r == 0) {
            game.ball_dy = 1 + (game.player1_points + game.player2_points) / 5;
        } else if (r == 1) {
            game.ball_dy = 2 + (game.player1_points + game.player2_points) / 5;
        } else {
            game.ball_dy = -1 - (game.player1_points + game.player2_points) / 5;
        }
    }
    
    if (game.ball_x + game.ball_size >= game.grid_width) {
        // Joueur 1 marque un point
        game.player1_points++;
        Play_Sound_P1(pong_hit_sound, pong_hit_length);
        
        // Vérifier si le joueur 1 a gagné la partie
        if (game.player1_points >= game.max_points) {
            // Fin de partie, le joueur 1 a gagné
            game.status = GAME_STATUS_FINISHED;

            // Jouer les mélodies de victoire/défaite en même temps sur les deux buzzers respectifs
            Play_Sound_P1(victory_melody, victory_length);
            Play_Sound_P2(defeat_melody, defeat_length);
            
            // Arrêter le jeu
            Stop_Game();
            return;
        }
        
        // Repositionner la balle au centre et inverser sa direction
        game.ball_x = game.grid_width / 2;
        game.ball_y = game.grid_height / 2;
        game.ball_dx = -2 - (game.player1_points + game.player2_points) / 3; // Augmenter légèrement la vitesse
        
        // Choisir aléatoirement la vitesse verticale
        uint32_t r = get_random_number_range(0, 2);
        if (r == 0) {
            game.ball_dy = 1 + (game.player1_points + game.player2_points) / 5;
        } else if (r == 1) {
            game.ball_dy = 2 + (game.player1_points + game.player2_points) / 5;
        } else {
            game.ball_dy = -1 - (game.player1_points + game.player2_points) / 5;
        }
    }
    
    // Envoyer les données à l'IHM
    Send_Game_Run_Data();
}

// Fonction de callback pour les ordres reçues par l'UART
void UART_Callback(char* msg, size_t length) {
    if (strncmp(msg, "echo:", 5) == 0) {
        // Renvoyer le message (pour tester)
        printf("%s\r\n", msg + 5);
    }
    else if (strncmp(msg, "game:start", 10) == 0) {
        // Format mis à jour: game:start:width:height:points:vb:sb:vp:sp:leftzone:rightzone
        // width = grid width, height = grid height, points = points to end the game, 
        // vb = ball velocity, sb = ball size, sp = paddle size, vp = paddle velocity
        // leftzone = largeur de la zone gauche, rightzone = largeur de la zone droite
        char* width_pos_str = strchr(msg + 10, ':'); // Skip "game:start:"
        if (width_pos_str) {
            char* height_pos_str = strchr(width_pos_str + 1, ':');
            if (height_pos_str) {
                char* points_pos_str = strchr(height_pos_str + 1, ':');
                if (points_pos_str) {
                    char* vb_pos_str = strchr(points_pos_str + 1, ':');
                    if (vb_pos_str) {
                        char* sb_pos_str = strchr(vb_pos_str + 1, ':');
                        if (sb_pos_str) {
                            char* vp_pos_str = strchr(sb_pos_str + 1, ':');
                            if (vp_pos_str) {
                                char* sp_pos_str = strchr(vp_pos_str + 1, ':');
                                if (sp_pos_str) {
                                    char* leftzone_pos_str = strchr(sp_pos_str + 1, ':');
                                    char* rightzone_pos_str = NULL;
                                    
                                    uint16_t grid_width = atoi(width_pos_str + 1);
                                    uint16_t grid_height = atoi(height_pos_str + 1);
                                    uint8_t max_points = atoi(points_pos_str + 1);
                                    uint8_t ball_velocity = atoi(vb_pos_str + 1);
                                    uint8_t ball_size = atoi(sb_pos_str + 1);
                                    uint8_t paddle_velocity = atoi(vp_pos_str + 1);
                                    uint8_t paddle_size = atoi(sp_pos_str + 1);
                                    
                                    // Initialiser le jeu avec les paramètres de base
                                    Init_Game(grid_width, grid_height, max_points, ball_velocity, ball_size, paddle_velocity, paddle_size);
                                    
                                    // Si les zones sont spécifiées, les mettre à jour
                                    if (leftzone_pos_str) {
                                        rightzone_pos_str = strchr(leftzone_pos_str + 1, ':');
                                        game.left_zone_width = atoi(leftzone_pos_str + 1);
                                        
                                        if (rightzone_pos_str) {
                                            game.right_zone_width = atoi(rightzone_pos_str + 1);
                                        }
                                    }
                                    
                                    // Verify that dimensions are valid
                                    if (grid_width > 50 && grid_height > 50) {
                                        // Assurez-vous que les zones ne sont pas trop grandes
                                        if (game.left_zone_width > grid_width / 2) {
                                            game.left_zone_width = grid_width / 2;
                                        }
                                        if (game.right_zone_width > grid_width / 2) {
                                            game.right_zone_width = grid_width / 2;
                                        }
                                        
                                        // Ajuster les positions X des raquettes en fonction des zones
                                        game.paddle_left_x = game.left_zone_width / 2;
                                        game.paddle_right_x = grid_width - game.right_zone_width / 2 - game.paddle_width;
                                        
                                        // Démarrer le jeu
                                        Start_Game();
                                    } else {
                                        printf("Invalid grid dimensions. Minimum size is 50x50.\r\n");
                                    }
                                } else {
                                    printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
                                }
                            } else {
                                printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
                            }
                        } else {
                            printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
                        }
                    } else {
                        printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
                    }
                } else {
                    printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
                }
            } else {
                printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
            }
        } else {
            printf("Invalid command format. Use: game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone]\r\n");
        }
    }
	else if (strncmp(msg, "game:pause", 10) == 0) {
		Pause_Game();
	}
	else if (strncmp(msg, "game:resume", 11) == 0) {
		Resume_Game();
	}
	else if (strncmp(msg, "game:stop", 9) == 0) {
	    Stop_Game();
	}
	else {
		printf("Unknown command: %s\r\n", msg);
	}
}

// Fonction de callback pour le bouton poussoir bleu sur la carte
void Blue_Button_Callback() {
	// Si la partie n'est pas en cours, démarrer la partie
	if (game.status == GAME_STATUS_NONE) {
		Init_Game(400, 250, 5, 1, 3, 5, 6);
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

  // Message de démarrage
  printf("\r\n\r\n---- GESIEA v1.0.0 - STM32 GamePad Initialized ----\r\n");
  printf("Available commands:\r\n");
  printf("  game:start:width:height:points:vb:sb:vp:sp[:leftzone:rightzone] - Start game with parameters:\r\n");
  printf("    width:     grid width\r\n");
  printf("    height:    grid height\r\n");
  printf("    points:    points to end the game\r\n");
  printf("    vb:        ball velocity\r\n");
  printf("    sb:        ball size\r\n");
  printf("    vp:        paddle velocity\r\n");
  printf("    sp:        paddle size\r\n");
  printf("    leftzone:  [optional] width of left player zone\r\n");
  printf("    rightzone: [optional] width of right player zone\r\n");
  printf("  game:pause       - Pause the game\r\n");
  printf("  game:resume      - Resume the game\r\n");
  printf("  game:stop        - Stop the game\r\n");
  printf("  echo:message     - Echo back message\r\n");
  printf("-------------------------------------\r\n\r\n");

  // Activer le timer qui fera les appels réguliers à Update_Game()
  LL_SYSTICK_EnableIT();

  // Jouer la mélodie de démarrage
  Play_Sound(init_melody, init_length);
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
