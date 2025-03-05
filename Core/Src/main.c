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
#include "gesiea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
 * @brief Structure représentant une note musicale.
 */
typedef struct {
    uint32_t frequency;    /**< Fréquence en Hz */
} Note;

/**
 * @brief Structure pour gérer une mélodie en lecture non-bloquante.
 *
 * Cette structure permet de jouer des mélodies sans bloquer l'exécution
 * du programme principal, idéal pour un système temps réel.
 */
typedef struct {
    Note* melody;              /**< Tableau de notes pour la mélodie */
    size_t length;             /**< Longueur de la mélodie */
    size_t currentIndex;       /**< Index de la note en cours */
    uint32_t channels;         /**< Canaux sur lesquels jouer */
    TIM_TypeDef *timer;        /**< Timer à utiliser */
    uint8_t isPlaying;         /**< Indique si une mélodie est en cours */
    uint8_t loopMode;          /**< Indique si la mélodie doit être jouée en boucle */
    uint32_t currentFrequency; /**< Fréquence actuellement jouée */
    uint8_t continuePlaying;   /**< Indique si on doit continuer à jouer sans coupure */
} MelodyPlayer;

/**
 * @brief Structure pour la gestion des LED de victoire.
 *
 * Permet de gérer l'allumage, l'extinction et le clignotement des LED
 * pour indiquer les points marqués et la victoire.
 */
typedef struct {
    GPIO_TypeDef *gpio;     /**< Port GPIO */
    uint32_t pin;           /**< Pin de la LED */
    uint32_t duration;      /**< Durée d'allumage en ticks (4Hz) */
    uint32_t counter;       /**< Compteur de ticks */
    uint8_t isActive;       /**< Indique si la LED est active */
    uint8_t blinkMode;      /**< Indique si la LED doit clignoter */
    uint32_t blinkPeriod;   /**< Période de clignotement en ticks */
    uint8_t blinkState;     /**< État actuel du clignotement (1=allumé, 0=éteint) */
    uint8_t infiniteMode;   /**< Mode infini (1=infini, 0=durée limitée) */
} LEDPlayer;

/**
 * @brief Structure pour le status du jeu.
 *
 * Cette structure contient toutes les données du jeu : positions des raquettes et de la balle,
 * scores, état du jeu, dimensions du terrain, etc.
 */
typedef struct {
    uint16_t grid_width;               /**< Largeur de la grille de jeu */
    uint16_t grid_height;              /**< Hauteur de la grille de jeu */
    uint16_t ball_x;                   /**< Position X de la balle */
    uint16_t ball_y;                   /**< Position Y de la balle */
    int8_t ball_dx;                    /**< Vitesse horizontale de la balle */
    int8_t ball_dy;                    /**< Vitesse verticale de la balle */
    uint8_t initial_ball_velocity;     /**< Vitesse initiale de la balle */
    uint16_t paddle_left_x;            /**< Position X de la raquette gauche */
    uint16_t paddle_left_y;            /**< Position Y de la raquette gauche */
    uint16_t paddle_right_x;           /**< Position X de la raquette droite */
    uint16_t paddle_right_y;           /**< Position Y de la raquette droite */
    uint8_t paddle_left_size;          /**< Taille de la raquette gauche */
    uint8_t paddle_right_size;         /**< Taille de la raquette droite */
    uint8_t paddle_width;              /**< Largeur des raquettes */
    uint8_t ball_size;                 /**< Taille de la balle */
    uint8_t paddle_speed;              /**< Vitesse des raquettes */
    uint8_t status;                    /**< Statut du jeu (NONE, RUNNING, PAUSED, FINISHED) */
    uint8_t max_points;                /**< Nombre de points nécessaires pour gagner */
    uint8_t player1_points;            /**< Points du joueur 1 */
    uint8_t player2_points;            /**< Points du joueur 2 */
    uint16_t left_zone_width;          /**< Largeur de la zone du joueur gauche */
    uint16_t right_zone_width;         /**< Largeur de la zone du joueur droit */
    uint8_t player1_button_state;      /**< État actuel du bouton du joueur 1 */
    uint8_t player1_prev_button_state; /**< État précédent du bouton du joueur 1 */
    uint8_t player1_boost_ready;       /**< Indique si le boost du joueur 1 est prêt */
    uint8_t player1_boost_counter;     /**< Compteur pour le joueur 1 */
    uint8_t player2_button_state;      /**< État actuel du bouton du joueur 2 */
    uint8_t player2_prev_button_state; /**< État précédent du bouton du joueur 2 */
    uint8_t player2_boost_ready;       /**< Indique si le boost du joueur 2 est prêt */
    uint8_t player2_boost_counter;     /**< Compteur pour le joueur 2 */
    uint8_t boost_window;              /**< Fenêtre de temps pour le boost (en ticks) */
    uint8_t boost_factor;              /**< Facteur de boost (en pourcentage) */
    uint16_t tick;                     /**< Compteur de ticks pour le jeu */
} Game;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Fréquences des notes musicales (en Hz) */
#define NOTE_G3  196.00  /**< Sol3 */
#define NOTE_GS3 207.65  /**< Sol#3/La♭3 */
#define NOTE_A3  220.00  /**< La3 */
#define NOTE_AS3 233.08  /**< La#3/Si♭3 */
#define NOTE_B3  246.94  /**< Si3 */
#define NOTE_C4  261.63  /**< Do4 */
#define NOTE_CS4 277.18  /**< Do#4/Ré♭4 */
#define NOTE_D4  293.66  /**< Ré4 */
#define NOTE_DS4 311.13  /**< Ré#4/Mi♭4 */
#define NOTE_E4  329.63  /**< Mi4 */
#define NOTE_F4  349.23  /**< Fa4 */
#define NOTE_FS4 369.99  /**< Fa#4/Sol♭4 */
#define NOTE_G4  392.00  /**< Sol4 */
#define NOTE_GS4 415.30  /**< Sol#4/La♭4 */
#define NOTE_A4  440.00  /**< La4 */
#define NOTE_AS4 466.16  /**< La#4/Si♭4 */
#define NOTE_B4  493.88  /**< Si4 */
#define NOTE_C5  523.25  /**< Do5 */
#define NOTE_CS5 554.37  /**< Do#5/Ré♭5 */
#define NOTE_D5  587.33  /**< Ré5 */
#define NOTE_DS5 622.25  /**< Ré#5/Mi♭5 */
#define NOTE_E5  659.25  /**< Mi5 */
#define NOTE_F5  698.46  /**< Fa5 */
#define NOTE_FS5 739.99  /**< Fa#5/Sol♭5 */
#define NOTE_G5  783.99  /**< Sol5 */
#define NOTE_GS5 830.61  /**< Sol#5/La♭5 */
#define NOTE_A5  880.00  /**< La5 */
#define NOTE_AS5 932.33  /**< La#5/Si♭5 */
#define NOTE_B5  987.77  /**< Si5 */
#define NOTE_C6  1046.50 /**< Do6 */
#define NOTE_CS6 1108.73 /**< Do#6/Ré♭6 */
#define NOTE_D6  1174.66 /**< Ré6 */
#define NOTE_DS6 1244.51 /**< Ré#6/Mi♭6 */
#define NOTE_E6  1318.51 /**< Mi6 */

/* Status du jeu */
#define GAME_STATUS_NONE 0     /**< Aucun jeu en cours */
#define GAME_STATUS_RUNNING 1  /**< Jeu en cours */
#define GAME_STATUS_PAUSED 2   /**< Jeu en pause */
#define GAME_STATUS_FINISHED 3 /**< Jeu terminé */

/* Définitions des broches et périphériques */
#define MUSIC_TIM TIM22                         /**< Timer pour la musique */
#define MUSIC_CHANNEL LL_TIM_CHANNEL_CH1        /**< Canal pour la musique */
#define BUZZER_TIM TIM2                         /**< Timer pour les buzzers des joueurs */
#define BUZZER_CHANNEL_P1 LL_TIM_CHANNEL_CH3    /**< Canal pour le buzzer du joueur 1 */
#define BUZZER_CHANNEL_P2 LL_TIM_CHANNEL_CH2    /**< Canal pour le buzzer du joueur 2 */
#define LED_GPIO GPIOA                          /**< Port GPIO pour les LED */
#define LED_PIN_P1_VICTORY LL_GPIO_PIN_7        /**< Broche pour la LED de victoire du joueur 1 */
#define LED_PIN_P2_VICTORY LL_GPIO_PIN_9        /**< Broche pour la LED de victoire du joueur 2 */
#define BUTTON_GPIO GPIOA                       /**< Port GPIO pour les boutons */
#define BUTTON_PIN_P1 LL_GPIO_PIN_8             /**< Broche pour le bouton du joueur 1 */
#define BUTTON_PIN_P2 LL_GPIO_PIN_10            /**< Broche pour le bouton du joueur 2 */
#define JOYSTICK_ADC ADC1                       /**< ADC pour les joysticks */
#define JOYSTICK_X_CHANNEL_P1 LL_ADC_CHANNEL_0  /**< Canal pour l'axe X du joystick du joueur 1 */
#define JOYSTICK_Y_CHANNEL_P1 LL_ADC_CHANNEL_1  /**< Canal pour l'axe Y du joystick du joueur 1 */
#define JOYSTICK_X_CHANNEL_P2 LL_ADC_CHANNEL_10 /**< Canal pour l'axe X du joystick du joueur 2 */
#define JOYSTICK_Y_CHANNEL_P2 LL_ADC_CHANNEL_11 /**< Canal pour l'axe Y du joystick du joueur 2 */

/* Définir des limites pour la vitesse verticale de la balle */
#define MAX_VERTICAL_SPEED 6  /* Vitesse verticale maximale (valeur absolue) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Mélodies pour les buzzers et la musique */
Note init_melody[] = {
    {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5},
    {NOTE_E5}, {NOTE_E5}, {NOTE_G5}, {NOTE_G5},
    {NOTE_E5}, {NOTE_E5}, {NOTE_C5}, {NOTE_C5}
};                                                                          /**< Mélodie de démarrage */
size_t init_length = sizeof(init_melody) / sizeof(Note);                    /**< Longueur de la mélodie */

Note connection_melody[] = {
    {NOTE_C4}, {NOTE_C4}, {NOTE_E4}, {NOTE_E4},
    {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5},
    {NOTE_G4}, {NOTE_G4}, {NOTE_E4}, {NOTE_E4},
    {NOTE_C4}, {NOTE_E4}, {NOTE_G4}, {NOTE_C5}
};                                                                         /**< Mélodie de connexion */
size_t connection_length = sizeof(connection_melody) / sizeof(Note);       /**< Longueur de la mélodie */

Note disconnection_melody[] = {
    {NOTE_C5}, {NOTE_C5}, {NOTE_A4}, {NOTE_A4},
    {NOTE_F4}, {NOTE_F4}, {NOTE_D4}, {NOTE_D4},
    {NOTE_C4}, {NOTE_C4}, {0}, {0}
};                                                                         /**< Mélodie de déconnexion */
size_t disconnection_length = sizeof(disconnection_melody) / sizeof(Note); /**< Longueur de la mélodie */

Note game_start_melody[] = {
    {NOTE_E4}, {NOTE_E4}, {NOTE_G4}, {NOTE_G4},
    {NOTE_A4}, {NOTE_A4}, {NOTE_C5}, {NOTE_C5},
    {NOTE_B4}, {NOTE_A4}, {NOTE_G4}, {NOTE_E4},
    {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5}
};                                                                         /**< Mélodie de début de jeu */
size_t game_start_length = sizeof(game_start_melody) / sizeof(Note);       /**< Longueur de la mélodie */

Note pause_melody[] = {
    {NOTE_E5}, {NOTE_E5}, {NOTE_C5}, {NOTE_C5},
    {NOTE_A4}, {NOTE_A4}, {NOTE_E4}, {NOTE_E4},
    {NOTE_C4}, {NOTE_C4}, {0}, {0}
};                                                                         /**< Mélodie de pause */
size_t pause_length = sizeof(pause_melody) / sizeof(Note);                 /**< Longueur de la mélodie */

Note resume_melody[] = {
    {NOTE_C4}, {NOTE_C4}, {NOTE_E4}, {NOTE_E4},
    {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5},
    {NOTE_E5}, {NOTE_E5}, {NOTE_G5}, {NOTE_G5}
};                                                                         /**< Mélodie de reprise */
size_t resume_length = sizeof(resume_melody) / sizeof(Note);               /**< Longueur de la mélodie */

Note hit_sound[] = {
    {NOTE_A4}, {NOTE_E5}, {NOTE_A4}, {0}
};                                                                         /**< Son de collision */
size_t hit_length = sizeof(hit_sound) / sizeof(Note);                      /**< Longueur du son */

Note boost_sound[] = {
    {NOTE_E5}, {NOTE_G5}, {NOTE_C6}, {NOTE_G5}, {NOTE_E6}
};                                                                         /**< Son de boost */
size_t boost_sound_length = sizeof(boost_sound) / sizeof(Note);            /**< Longueur du son */

Note victory_melody[] = {
    {NOTE_C5}, {NOTE_C5}, {NOTE_G4}, {NOTE_G4},
    {NOTE_C5}, {NOTE_C5}, {NOTE_E5}, {NOTE_E5},
    {NOTE_G5}, {NOTE_G5}, {NOTE_G5}, {NOTE_G5},
    {NOTE_E5}, {NOTE_C5}, {NOTE_G4}, {NOTE_C5},
    {NOTE_C5}, {NOTE_C5}, {0}, {0}
};                                                                         /**< Mélodie de victoire */
size_t victory_length = sizeof(victory_melody) / sizeof(Note);             /**< Longueur de la mélodie */

Note defeat_melody[] = {
    {NOTE_C5}, {NOTE_C5}, {NOTE_G4}, {NOTE_G4},
    {NOTE_E4}, {NOTE_E4}, {NOTE_C4}, {NOTE_C4},
    {NOTE_B3}, {NOTE_B3}, {NOTE_G3}, {NOTE_G3},
    {NOTE_G3}, {0}, {0}, {0}
};                                                                         /**< Mélodie de défaite */
size_t defeat_length = sizeof(defeat_melody) / sizeof(Note);               /**< Longueur de la mélodie */

Note background_melody[] = {
    // Partie 1: thème principal
    {NOTE_C4}, {NOTE_C4}, {NOTE_E4}, {NOTE_E4}, {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5},
    {NOTE_G4}, {NOTE_G4}, {NOTE_E4}, {NOTE_E4}, {NOTE_C4}, {NOTE_C4}, {NOTE_E4}, {NOTE_G4},

    // Partie 2: variation
    {NOTE_D4}, {NOTE_D4}, {NOTE_F4}, {NOTE_F4}, {NOTE_A4}, {NOTE_A4}, {NOTE_D5}, {NOTE_D5},
    {NOTE_A4}, {NOTE_A4}, {NOTE_F4}, {NOTE_F4}, {NOTE_D4}, {NOTE_D4}, {NOTE_F4}, {NOTE_A4},

    // Partie 3: montée
    {NOTE_E4}, {NOTE_E4}, {NOTE_G4}, {NOTE_G4}, {NOTE_B4}, {NOTE_B4}, {NOTE_E5}, {NOTE_E5},
    {NOTE_B4}, {NOTE_B4}, {NOTE_G4}, {NOTE_G4}, {NOTE_E4}, {NOTE_E4}, {NOTE_G4}, {NOTE_B4},

    // Partie 4: finale
    {NOTE_C5}, {NOTE_C5}, {NOTE_A4}, {NOTE_A4}, {NOTE_F4}, {NOTE_F4}, {NOTE_D4}, {NOTE_D4},
    {NOTE_E4}, {NOTE_E4}, {NOTE_G4}, {NOTE_G4}, {NOTE_C5}, {NOTE_C5}, {NOTE_G4}, {NOTE_E4}
};                                                                         /**< Mélodie de fond */
size_t background_length = sizeof(background_melody) / sizeof(Note);       /**< Longueur de la mélodie */

/* Variables pour les players de mélodies */
MelodyPlayer buzzer1Player = {0};    /**< Player pour le buzzer 1 */
MelodyPlayer buzzer2Player = {0};    /**< Player pour le buzzer 2 */
MelodyPlayer backgroundPlayer = {0}; /**< Player pour la musique de fond */

/* Variables pour les players de LED */
LEDPlayer led1Player = {0}; /**< Player pour la LED de victoire du joueur 1 */
LEDPlayer led2Player = {0}; /**< Player pour la LED de victoire du joueur 2 */

/* Variables pour le jeu */
Game game =	{0}; /**< Status du jeu */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Redirection de printf vers l'UART2.
 *
 * Cette fonction est appelée par les fonctions printf et permet
 * de rediriger la sortie vers l'UART2.
 *
 * @param ch Caractère à envoyer
 * @return Le caractère envoyé
 */
int __io_putchar(int ch) {
    // Attendre que le buffer de transmission soit vide
    while (!LL_USART_IsActiveFlag_TXE(USART2));

    // Envoyer le caractère
    LL_USART_TransmitData8(USART2, ch);

    return ch;
}

/**
 * @brief Fonction d'écriture pour stdio.h.
 *
 * Implémentation de la fonction _write utilisée par stdio.h
 * pour la sortie standard.
 *
 * @param file Descripteur de fichier (non utilisé)
 * @param ptr Pointeur vers les données à écrire
 * @param len Longueur des données
 * @return Nombre d'octets écrits
 */
int _write(int file, char *ptr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        __io_putchar(*ptr++);
    }
    return len;
}

/**
 * @brief Lit une valeur depuis une entrée analogique.
 *
 * @param ADCx Périphérique ADC à utiliser
 * @param Channel Canal ADC à lire
 * @return Valeur lue (0-4095)
 */
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

/**
 * @brief Génère un nombre pseudo-aléatoire avec entropie améliorée.
 *
 * Utilise les sources d'entropie matérielles (ADC, timers) et un
 * algorithme amélioré pour une meilleure distribution des valeurs aléatoires.
 *
 * @return Nombre aléatoire généré
 */
uint32_t get_random_number()
{
    static uint32_t seed = 0;
    static uint8_t initialized = 0;

    // Initialiser la graine avec diverses sources d'entropie au premier appel
    if (!initialized) {
        // Ajouter de l'entropie avec les valeurs des joysticks
        seed ^= Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_X_CHANNEL_P1);
        seed ^= Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P1) << 8;
        seed ^= Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_X_CHANNEL_P2) << 16;
        seed ^= Read_ADC_Value(JOYSTICK_ADC, JOYSTICK_Y_CHANNEL_P2) << 24;

        // Ajouter les valeurs des timers
        seed ^= LL_TIM_GetCounter(TIM2);
        seed ^= LL_TIM_GetCounter(TIM22) << 8;

        // Éviter la valeur zéro
        if (seed == 0) seed = 0x12345678;

        initialized = 1;
    }

    // Algorithme amélioré (combinaison d'un LCG avec des opérations XOR)
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;

    // Appliquer une transformation supplémentaire pour améliorer la distribution
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;

    return seed;
}

/**
 * @brief Génère un nombre aléatoire dans une plage donnée.
 *
 * @param min Valeur minimale (incluse)
 * @param max Valeur maximale (incluse)
 * @return Nombre aléatoire dans la plage [min, max]
 */
uint32_t get_random_number_range(uint32_t min, uint32_t max)
{
    return min + (get_random_number() % (max - min + 1));
}

/**
 * @brief Configure la fréquence d'un buzzer.
 *
 * @param TIMx Timer à utiliser
 * @param Channels Canaux du timer à configurer
 * @param frequency Fréquence en Hz (0 pour désactiver)
 */
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

/**
 * @brief Démarre la lecture d'une mélodie.
 *
 * @param player Pointeur vers la structure MelodyPlayer
 * @param TIMx Timer à utiliser
 * @param Channels Canaux du timer
 * @param melody Tableau de notes à jouer
 * @param length Longueur de la mélodie
 * @param loop Mode boucle (1=actif, 0=inactif)
 */
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
    player->currentFrequency = 0;
    player->continuePlaying = 0;
    
    // La note sera jouée au prochain appel à Update_Sound
}

/**
 * @brief Arrête la lecture d'une mélodie.
 *
 * @param player Pointeur vers la structure MelodyPlayer
 */
void Stop_Melody(MelodyPlayer *player) {
    if (player->isPlaying) {
        Set_Buzzer_Frequency(player->timer, player->channels, 0);
        player->isPlaying = 0;
    }
}

/**
 * @brief Joue un son sur le buzzer du joueur 1.
 *
 * @param melody Tableau de notes à jouer
 * @param length Longueur de la mélodie
 */
void Play_Sound_P1(Note* melody, size_t length) {
    Start_Melody(&buzzer1Player, BUZZER_TIM, BUZZER_CHANNEL_P1, melody, length, 0);
}

/**
 * @brief Joue un son sur le buzzer du joueur 2.
 *
 * @param melody Tableau de notes à jouer
 * @param length Longueur de la mélodie
 */
void Play_Sound_P2(Note* melody, size_t length) {
    Start_Melody(&buzzer2Player, BUZZER_TIM, BUZZER_CHANNEL_P2, melody, length, 0);
}

/**
 * @brief Joue un son sur les deux buzzers.
 *
 * @param melody Tableau de notes à jouer
 * @param length Longueur de la mélodie
 */
void Play_Sound(Note* melody, size_t length) {
    Play_Sound_P1(melody, length);
    Play_Sound_P2(melody, length);
}

/**
 * @brief Démarre la musique de fond.
 *
 * @param melody Tableau de notes à jouer
 * @param length Longueur de la mélodie
 */
void Start_Music(Note* melody, size_t length) {
    Start_Melody(&backgroundPlayer, MUSIC_TIM, MUSIC_CHANNEL, melody, length, 1);
}

/**
 * @brief Arrête la musique de fond.
 */
void Stop_Music() {
    Stop_Melody(&backgroundPlayer);
}

/**
 * @brief Reprend la lecture de la musique de fond.
 */
void Resume_Music() {
	backgroundPlayer.isPlaying = 1;
}

/**
 * @brief Met à jour les sons - appelée périodiquement.
 *
 * Cette fonction est appelée toutes les 125ms par le timer système
 * pour mettre à jour la lecture des mélodies.
 */
void Update_Sound() {
    // Mise à jour du buzzer 1
    if (buzzer1Player.isPlaying) {
        // Jouer la note actuelle
        if (buzzer1Player.currentIndex < buzzer1Player.length) {
            Note currentNote = buzzer1Player.melody[buzzer1Player.currentIndex];

            // Vérifier si la prochaine note est la même que l'actuelle
            uint8_t skipSilence = 0;
            if (buzzer1Player.currentIndex + 1 < buzzer1Player.length) {
                Note nextNote = buzzer1Player.melody[buzzer1Player.currentIndex + 1];
                if (nextNote.frequency == currentNote.frequency && currentNote.frequency != 0) {
                    skipSilence = 1;
                }
            }

            // Sauvegarder la fréquence actuelle
            buzzer1Player.currentFrequency = currentNote.frequency;
            buzzer1Player.continuePlaying = skipSilence;

            // Jouer la note (seulement si différente de la précédente ou après un silence)
            Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, currentNote.frequency);

            buzzer1Player.currentIndex++;

            // Vérifier si nous avons atteint la fin de la mélodie
            if (buzzer1Player.currentIndex >= buzzer1Player.length) {
                if (!buzzer1Player.loopMode) {
                    // Marquer comme terminé
                    buzzer1Player.isPlaying = 0;
                } else {
                    // Boucler au début
                    buzzer1Player.currentIndex = 0;
                }
            }
        }
    }

    // Mise à jour du buzzer 2 (même logique)
    if (buzzer2Player.isPlaying) {
        if (buzzer2Player.currentIndex < buzzer2Player.length) {
            Note currentNote = buzzer2Player.melody[buzzer2Player.currentIndex];

            // Vérifier si la prochaine note est la même que l'actuelle
            uint8_t skipSilence = 0;
            if (buzzer2Player.currentIndex + 1 < buzzer2Player.length) {
                Note nextNote = buzzer2Player.melody[buzzer2Player.currentIndex + 1];
                if (nextNote.frequency == currentNote.frequency && currentNote.frequency != 0) {
                    skipSilence = 1;
                }
            }

            // Sauvegarder la fréquence actuelle
            buzzer2Player.currentFrequency = currentNote.frequency;
            buzzer2Player.continuePlaying = skipSilence;

            // Jouer la note
            Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, currentNote.frequency);

            buzzer2Player.currentIndex++;
            
            // Vérifier si nous avons atteint la fin de la mélodie
            if (buzzer2Player.currentIndex >= buzzer2Player.length) {
                if (!buzzer2Player.loopMode) {
                    buzzer2Player.isPlaying = 0;
                } else {
                    buzzer2Player.currentIndex = 0;
                }
            }
        }
    }

    // Mise à jour de la musique de fond (même logique)
    if (backgroundPlayer.isPlaying) {
        if (backgroundPlayer.currentIndex < backgroundPlayer.length) {
            Note currentNote = backgroundPlayer.melody[backgroundPlayer.currentIndex];

            // Vérifier si la prochaine note est la même que l'actuelle
            uint8_t skipSilence = 0;
            if (backgroundPlayer.currentIndex + 1 < backgroundPlayer.length) {
                Note nextNote = backgroundPlayer.melody[backgroundPlayer.currentIndex + 1];
                if (nextNote.frequency == currentNote.frequency && currentNote.frequency != 0) {
                    skipSilence = 1;
                }
            } else if (backgroundPlayer.loopMode && backgroundPlayer.length > 0) {
                // Si on est en mode boucle, vérifier également avec la première note
                Note nextNote = backgroundPlayer.melody[0];
                if (nextNote.frequency == currentNote.frequency && currentNote.frequency != 0) {
                    skipSilence = 1;
                }
            }

            backgroundPlayer.currentFrequency = currentNote.frequency;
            backgroundPlayer.continuePlaying = skipSilence;

            Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, currentNote.frequency);

            backgroundPlayer.currentIndex++;
            
            if (backgroundPlayer.currentIndex >= backgroundPlayer.length) {
                if (backgroundPlayer.loopMode) {
                    backgroundPlayer.currentIndex = 0;
                } else {
                    backgroundPlayer.isPlaying = 0;
                }
            }
        }
    }
}

/**
 * @brief Arrête les sons - appelée après Update_Sound.
 *
 * Cette fonction est appelée 25ms après Update_Sound pour
 * couper les sons si nécessaire (gestion de la durée des notes).
 */
void Stop_Sound() {
    // Ne couper le son que si on ne doit pas continuer à jouer la même note
    if (buzzer1Player.isPlaying) {
        if (!buzzer1Player.continuePlaying) {
            Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, 0);
        }
    } else if (buzzer1Player.currentIndex >= buzzer1Player.length && !buzzer1Player.loopMode) {
        // Si la mélodie vient de se terminer, s'assurer que le son est arrêté
        Set_Buzzer_Frequency(buzzer1Player.timer, buzzer1Player.channels, 0);
    }

    if (buzzer2Player.isPlaying) {
        if (!buzzer2Player.continuePlaying) {
            Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, 0);
        }
    } else if (buzzer2Player.currentIndex >= buzzer2Player.length && !buzzer2Player.loopMode) {
        Set_Buzzer_Frequency(buzzer2Player.timer, buzzer2Player.channels, 0);
    }

    if (backgroundPlayer.isPlaying) {
        if (!backgroundPlayer.continuePlaying) {
            Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, 0);
        }
    } else if (backgroundPlayer.currentIndex >= backgroundPlayer.length && !backgroundPlayer.loopMode) {
        Set_Buzzer_Frequency(backgroundPlayer.timer, backgroundPlayer.channels, 0);
    }
}

/**
 * @brief Démarre l'allumage d'une LED.
 *
 * @param player Pointeur vers la structure LEDPlayer
 * @param GPIOx Port GPIO
 * @param Pin Broche GPIO
 * @param duration Durée d'allumage en ticks
 * @param blinkMode Mode clignotement (1=actif, 0=inactif)
 * @param blinkPeriod Période de clignotement en ticks
 * @param infiniteMode Mode infini (1=actif, 0=inactif)
 */
void Start_LED(LEDPlayer *player, GPIO_TypeDef *GPIOx, uint32_t Pin,
               uint32_t duration, uint8_t blinkMode, uint32_t blinkPeriod,
               uint8_t infiniteMode) {
    // Configuration des paramètres du LEDPlayer
    player->gpio = GPIOx;
    player->pin = Pin;
    player->duration = duration;
    player->counter = 0;
    player->isActive = 1;
    player->blinkMode = blinkMode;
    player->blinkPeriod = blinkPeriod;
    player->blinkState = 1; // Commencer allumé
    player->infiniteMode = infiniteMode; // Nouveau paramètre

    // Allumer la LED immédiatement
    LL_GPIO_SetOutputPin(GPIOx, Pin);
}

/**
 * @brief Arrête l'allumage d'une LED.
 *
 * @param player Pointeur vers la structure LEDPlayer
 */
void Stop_LED(LEDPlayer *player) {
    if (player->isActive) {
        // Éteindre la LED
        LL_GPIO_ResetOutputPin(player->gpio, player->pin);
        player->isActive = 0;
    }
}

/**
 * @brief Met à jour l'état des LED - appelée périodiquement.
 *
 * Cette fonction est appelée toutes les 250ms par le timer système
 * pour mettre à jour l'état des LED.
 */
void Update_LEDs() {
    // Mise à jour de la LED du joueur 1
    if (led1Player.isActive) {
        led1Player.counter++;

        // Gestion du clignotement si activé
        if (led1Player.blinkMode) {
            if (led1Player.counter % led1Player.blinkPeriod == 0) {
                // Inverser l'état du clignotement
                led1Player.blinkState = !led1Player.blinkState;

                if (led1Player.blinkState) {
                    LL_GPIO_SetOutputPin(led1Player.gpio, led1Player.pin);
                } else {
                    LL_GPIO_ResetOutputPin(led1Player.gpio, led1Player.pin);
                }
            }
        }

        // Vérifier si la durée est écoulée (sauf en mode infini)
        if (!led1Player.infiniteMode && led1Player.counter >= led1Player.duration) {
            Stop_LED(&led1Player);
        }
    }

    // Mise à jour de la LED du joueur 2 (même logique)
    if (led2Player.isActive) {
        led2Player.counter++;

        // Gestion du clignotement si activé
        if (led2Player.blinkMode) {
            if (led2Player.counter % led2Player.blinkPeriod == 0) {
                // Inverser l'état du clignotement
                led2Player.blinkState = !led2Player.blinkState;

                if (led2Player.blinkState) {
                    LL_GPIO_SetOutputPin(led2Player.gpio, led2Player.pin);
                } else {
                    LL_GPIO_ResetOutputPin(led2Player.gpio, led2Player.pin);
                }
            }
        }

        // Vérifier si la durée est écoulée (sauf en mode infini)
        if (!led2Player.infiniteMode && led2Player.counter >= led2Player.duration) {
            Stop_LED(&led2Player);
        }
    }
}

/**
 * @brief Allume la LED de victoire du joueur 1.
 *
 * @param duration Durée d'allumage en ticks
 * @param blinkMode Mode clignotement (1=actif, 0=inactif)
 * @param blinkPeriod Période de clignotement en ticks
 * @param infiniteMode Mode infini (1=actif, 0=inactif)
 */
void Player1_Victory_LED(uint32_t duration, uint8_t blinkMode, uint32_t blinkPeriod, uint8_t infiniteMode) {
    Start_LED(&led1Player, LED_GPIO, LED_PIN_P1_VICTORY, duration, blinkMode, blinkPeriod, infiniteMode);
}

/**
 * @brief Allume la LED de victoire du joueur 2.
 *
 * @param duration Durée d'allumage en ticks
 * @param blinkMode Mode clignotement (1=actif, 0=inactif)
 * @param blinkPeriod Période de clignotement en ticks
 * @param infiniteMode Mode infini (1=actif, 0=inactif)
 */
void Player2_Victory_LED(uint32_t duration, uint8_t blinkMode, uint32_t blinkPeriod, uint8_t infiniteMode) {
    Start_LED(&led2Player, LED_GPIO, LED_PIN_P2_VICTORY, duration, blinkMode, blinkPeriod, infiniteMode);
}

/**
 * @brief Envoie toutes les données du jeu à l'IHM.
 *
 * Format: "game:all:status,grid_width,grid_height,ball_size,ball_x,ball_y,ball_dx,ball_dy,
 *         paddle_left_x,paddle_left_y,paddle_left_size,paddle_width,
 *         paddle_right_x,paddle_right_y,paddle_right_size,max_points,player1_points,player2_points,
 *         left_zone_width,right_zone_width"
 */
void Send_Game_All_Data() {
    printf("game:all:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        game.status, game.grid_width, game.grid_height, game.ball_size,
        game.ball_x, game.ball_y, game.ball_dx, game.ball_dy,
        game.paddle_left_x, game.paddle_left_y, game.paddle_left_size, game.paddle_width,
        game.paddle_right_x, game.paddle_right_y, game.paddle_right_size,
        game.max_points, game.player1_points, game.player2_points,
        game.left_zone_width, game.right_zone_width);
}

/**
 * @brief Envoie les données de refresh en cours de jeu à l'IHM.
 *
 * Format: "game:run:status,ball_x,ball_y,ball_dx,ball_dy,ball_size,
 *         paddle_left_x,paddle_left_y,paddle_left_size,paddle_width,
 *         paddle_right_x,paddle_right_y,paddle_right_size,p1points,p2points"
 */
void Send_Game_Run_Data() {
    printf("game:run:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        game.status, game.ball_x, game.ball_y, game.ball_dx, game.ball_dy, game.ball_size,
        game.paddle_left_x, game.paddle_left_y, game.paddle_left_size, game.paddle_width,
        game.paddle_right_x, game.paddle_right_y, game.paddle_right_size,
        game.player1_points, game.player2_points);
}

/**
 * @brief Limite la vitesse verticale de la balle dans une plage raisonnable.
 *
 * @param ball_dy Vitesse verticale actuelle de la balle
 * @return La vitesse verticale limitée
 */
int8_t Limit_Vertical_Speed(int8_t ball_dy) {
    if (ball_dy > MAX_VERTICAL_SPEED) {
        return MAX_VERTICAL_SPEED;
    } else if (ball_dy < -MAX_VERTICAL_SPEED) {
        return -MAX_VERTICAL_SPEED;
    }
    return ball_dy;
}

/**
 * @brief Démarre une partie.
 *
 * @param width Largeur de la grille
 * @param height Hauteur de la grille
 * @param max_points Points nécessaires pour gagner
 * @param ball_velocity Vitesse de la balle
 * @param ball_size Taille de la balle
 * @param paddle_velocity Vitesse des raquettes
 * @param paddle_size Taille des raquettes
 */
void Start_Game(uint16_t width, uint16_t height, uint8_t max_points, uint8_t ball_velocity, uint8_t ball_size, uint8_t paddle_velocity, uint8_t paddle_size) {
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

    // Stocker la vitesse initiale de la balle
    game.initial_ball_velocity = ball_velocity;

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

	// Limiter la vitesse verticale initiale
	game.ball_dy = Limit_Vertical_Speed(game.ball_dy);

	// Positionner les raquettes
	game.paddle_left_x = 20;  // Position X initiale de la raquette gauche
	game.paddle_left_y = (game.grid_height - game.paddle_left_size) / 2;  // Centrer verticalement

	game.paddle_right_x = game.grid_width - 20 - game.paddle_width;  // Position X initiale de la raquette droite
	game.paddle_right_y = (game.grid_height - game.paddle_right_size) / 2;  // Centrer verticalement

	// Initialiser les variables de boost
	game.player1_button_state = 0;
	game.player1_prev_button_state = 0;
	game.player1_boost_ready = 0;
	game.player1_boost_counter = 0;

	game.player2_button_state = 0;
	game.player2_prev_button_state = 0;
	game.player2_boost_ready = 0;
	game.player2_boost_counter = 0;

	game.boost_window = 10;  // 10 ticks de jeu (environ 300ms à 30Hz)
	game.boost_factor = 150; // 150% de la vitesse normale

	// Par défaut, définir le statut à "none"
	game.status = GAME_STATUS_NONE;

	// Tick
	game.tick = 0;

	// Envoyer les données du jeu à l'IHM
	Send_Game_All_Data();

	// Jouer la mélodie de démarrage
	Play_Sound(game_start_melody, game_start_length);

	// Définir le statut à "running"
	game.status = GAME_STATUS_RUNNING;

	// Allumer la LED du microcontrôleur
	LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

	// Envoyer les données du jeu à l'IHM avec le nouveau statut
	Send_Game_All_Data();

	// Démarrer la musique de fond en boucle
	Start_Music(background_melody, background_length);
}

/**
 * @brief Met le jeu en pause.
 */
void Pause_Game() {
    game.status = GAME_STATUS_PAUSED;
    LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
    Send_Game_All_Data();

    // Mettre en pause la musique de fond
    Stop_Music();

    // Jouer la mélodie de pause
    Play_Sound(pause_melody, pause_length);
}

/**
 * @brief Reprend le jeu après une pause.
 */
void Resume_Game() {
    // Jouer la mélodie de reprise
    Play_Sound(resume_melody, resume_length);

    game.status = GAME_STATUS_RUNNING;
    LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
    Send_Game_All_Data();

    // Reprendre la musique de fond
    Resume_Music();
}

/**
 * @brief Arrête le jeu.
 */
void Stop_Game() {
    // Réinitialiser la LED
    LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

    // Arrêter la musique de fond
    Stop_Music();

    // Changer le statut du jeu
    game.status = GAME_STATUS_FINISHED;

    // Envoyer les données à l'IHM pour actualiser l'affichage
    Send_Game_All_Data();

    // Changer le statut du jeu
    game.status = GAME_STATUS_NONE;
}

/**
 * @brief Met à jour le jeu - appelée périodiquement.
 *
 * Cette fonction est appelée toutes les 33ms environ (30Hz) par le timer système.
 * Elle met à jour la position de la balle, les raquettes, gère les collisions et les scores.
 */
void Update_Game() {
	// Si la partie est en cours
	if (game.status != GAME_STATUS_RUNNING) {
		return;
	}

	// Augmenter le tick
	game.tick++;

	// Si le tick est inférieur à 60, ne pas mettre à jour le jeu (délai de démarrage)
	if (game.tick < 60) {
		return;
	}

    // Mettre à jour l'état des boutons et du boost
    // Sauvegarder l'état précédent des boutons
    game.player1_prev_button_state = game.player1_button_state;
    game.player2_prev_button_state = game.player2_button_state;

    // Lire l'état actuel des boutons
    game.player1_button_state = !LL_GPIO_IsInputPinSet(BUTTON_GPIO, BUTTON_PIN_P1);
    game.player2_button_state = !LL_GPIO_IsInputPinSet(BUTTON_GPIO, BUTTON_PIN_P2);

    // Détecter les appuis sur les boutons (transition de non-appuyé à appuyé)
    if (game.player1_button_state && !game.player1_prev_button_state) {
        game.player1_boost_ready = 1;
        game.player1_boost_counter = 0;
        // Si le boost est prêt, allumer la LED du joueur 1
        Player1_Victory_LED(2, 1, 1, 0); // 2 ticks, clignotement rapide, non infini
    }

    if (game.player2_button_state && !game.player2_prev_button_state) {
        game.player2_boost_ready = 1;
        game.player2_boost_counter = 0;
        // Si le boost est prêt, allumer la LED du joueur 2
        Player2_Victory_LED(2, 1, 1, 0); // 2 ticks, clignotement rapide, non infini
    }

    // Incrémenter les compteurs et désactiver le boost s'il expire
    if (game.player1_boost_ready) {
        game.player1_boost_counter++;
        if (game.player1_boost_counter > game.boost_window) {
            game.player1_boost_ready = 0;
            // Éteindre la LED du joueur 1
            Stop_LED(&led1Player);
        }
    }

    if (game.player2_boost_ready) {
        game.player2_boost_counter++;
        if (game.player2_boost_counter > game.boost_window) {
            game.player2_boost_ready = 0;
            // Éteindre la LED du joueur 2
            Stop_LED(&led2Player);
        }
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
    
    // Augmenter la vitesse de la balle toutes les 10 secondes (environ 300 ticks à 30Hz)
    if (game.tick % 300 == 0) {
		if (game.ball_dx > 0) {
			game.ball_dx++;
		} else {
			game.ball_dx--;
		}
		if (game.ball_dy > 0) {
			game.ball_dy++;
		} else {
			game.ball_dy--;
		}
	}

    // Limiter la vitesse verticale après l'augmentation périodique
    game.ball_dy = Limit_Vertical_Speed(game.ball_dy);

    // Mise à jour de la position de la balle
    game.ball_x += game.ball_dx;
    game.ball_y += game.ball_dy;
    
    // Gestion des collisions avec les bords supérieur et inférieur
    if (game.ball_y <= 0 || game.ball_y + game.ball_size >= game.grid_height) {
        game.ball_dy = -game.ball_dy; // Inverser la direction verticale
        // Limiter la vitesse verticale après rebond
        game.ball_dy = Limit_Vertical_Speed(game.ball_dy);
        Play_Sound(hit_sound, hit_length);
    }
    
    // Gestion des collisions avec les raquettes
    
    // Collision avec la raquette gauche
    if (game.ball_dx < 0 // La balle se dirige vers la gauche
        && game.ball_x <= game.paddle_left_x + game.paddle_width
        && game.ball_x >= game.paddle_left_x
        && game.ball_y + game.ball_size >= game.paddle_left_y
        && game.ball_y <= game.paddle_left_y + game.paddle_left_size) {
        
        // Inverser la direction horizontale
        game.ball_dx = -game.ball_dx;

        // Appliquer le boost si activé
        if (game.player1_boost_ready) {
            // Calculer les nouvelles vitesses (augmenter de game.boost_factor %)
            game.ball_dx = (game.ball_dx * game.boost_factor) / 100;

            // S'assurer que la vitesse minimum est maintenue
            if (game.ball_dx > -2) game.ball_dx = -2;

            // Appliquer aussi au mouvement vertical
            game.ball_dy = (game.ball_dy * game.boost_factor) / 100;

            // Limiter la vitesse verticale après le boost
            game.ball_dy = Limit_Vertical_Speed(game.ball_dy);

            // S'assurer que la vitesse verticale minimum est maintenue
            if (game.ball_dy > 0 && game.ball_dy < 1) game.ball_dy = 1;
            if (game.ball_dy < 0 && game.ball_dy > -1) game.ball_dy = -1;

            // Réinitialiser le boost
            game.player1_boost_ready = 0;

            // Jouer le son de boost
            Play_Sound_P1(boost_sound, boost_sound_length);

            // Allumer la LED du joueur 1
            Player1_Victory_LED(4, 1, 1, 0); // 4 ticks, clignotement rapide, non infini
        } else {
            // Son normal de collision
            Play_Sound_P1(hit_sound, hit_length);
        }
        
        // Repositionner la balle juste après la raquette pour éviter les collisions multiples
        game.ball_x = game.paddle_left_x + game.paddle_width;
    }

    // Collision avec la raquette droite
    if (game.ball_dx > 0 // La balle se dirige vers la droite
        && game.ball_x + game.ball_size >= game.paddle_right_x
        && game.ball_x + game.ball_size <= game.paddle_right_x + game.paddle_width
        && game.ball_y + game.ball_size >= game.paddle_right_y
        && game.ball_y <= game.paddle_right_y + game.paddle_right_size) {
        
        // Inverser la direction horizontale
        game.ball_dx = -game.ball_dx;

        // Appliquer le boost si activé
        if (game.player2_boost_ready) {
            // Calculer les nouvelles vitesses (augmenter de game.boost_factor %)
            game.ball_dx = (game.ball_dx * game.boost_factor) / 100;

            // S'assurer que la vitesse minimum est maintenue
            if (game.ball_dx < 2) game.ball_dx = 2;

            // Appliquer aussi au mouvement vertical
            game.ball_dy = (game.ball_dy * game.boost_factor) / 100;

            // Limiter la vitesse verticale après le boost
            game.ball_dy = Limit_Vertical_Speed(game.ball_dy);

            // S'assurer que la vitesse verticale minimum est maintenue
            if (game.ball_dy > 0 && game.ball_dy < 1) game.ball_dy = 1;
            if (game.ball_dy < 0 && game.ball_dy > -1) game.ball_dy = -1;

            // Réinitialiser le boost
            game.player2_boost_ready = 0;

            // Jouer le son de boost
            Play_Sound_P2(boost_sound, boost_sound_length);

            // Allumer la LED du joueur 2
            Player2_Victory_LED(4, 1, 1, 0); // 4 ticks, clignotement rapide, non infini
        } else {
            // Son normal de collision
            Play_Sound_P2(hit_sound, hit_length);
        }
        
        // Repositionner la balle juste avant la raquette pour éviter les collisions multiples
        game.ball_x = game.paddle_right_x - game.ball_size;
    }
    
    // Gestion des conditions de score
    if (game.ball_x <= 0) {
        // Joueur 2 marque un point
        game.player2_points++;
        Play_Sound_P2(hit_sound, hit_length);
        
        Player2_Victory_LED(4, 0, 0, 0);

        // Vérifier si le joueur 2 a gagné la partie
        if (game.player2_points >= game.max_points) {
            // Fin de partie, le joueur 2 a gagné
            game.status = GAME_STATUS_FINISHED;

            // Jouer les mélodies de victoire/défaite en même temps sur les deux buzzers respectifs
            Play_Sound_P2(victory_melody, victory_length);
            Play_Sound_P1(defeat_melody, defeat_length);
            
            Player2_Victory_LED(0, 1, 2, 1); // Durée ignorée, clignotement avec période=2, mode infini

            // Envoyer les données du jeu pour actualiser l'affichage avant de l'arrêter
            Send_Game_Run_Data();
            
            // Arrêter le jeu sans réinitialisation
            Stop_Game();
            return;
        }
        
        // Repositionner la balle au centre
        game.ball_x = game.grid_width / 2;
        game.ball_y = game.grid_height / 2;

        // MODIFICATION : Réinitialiser la vitesse de la balle à sa valeur initiale
        // et la diriger vers le joueur 2 qui a marqué
        game.ball_dx = 2 + game.initial_ball_velocity;  // Vers la droite (joueur 2)

        // Réinitialiser aussi la composante verticale avec une direction aléatoire
        uint32_t r = get_random_number_range(0, 2);
        if (r == 0) {
            game.ball_dy = 1 + game.initial_ball_velocity;
        } else if (r == 1) {
            game.ball_dy = 2 + game.initial_ball_velocity;
        } else {
            game.ball_dy = -1 - game.initial_ball_velocity;
        }

        // Limiter la vitesse verticale après le score
        game.ball_dy = Limit_Vertical_Speed(game.ball_dy);
    }
    else if (game.ball_x + game.ball_size >= game.grid_width) {
        // Joueur 1 marque un point
        game.player1_points++;
        Play_Sound_P1(hit_sound, hit_length);
        
        Player1_Victory_LED(4, 0, 0, 0);

        // Vérifier si le joueur 1 a gagné la partie
        if (game.player1_points >= game.max_points) {
            // Fin de partie, le joueur 1 a gagné
            game.status = GAME_STATUS_FINISHED;

            // Jouer les mélodies de victoire/défaite en même temps sur les deux buzzers respectifs
            Play_Sound_P1(victory_melody, victory_length);
            Play_Sound_P2(defeat_melody, defeat_length);
            
            Player1_Victory_LED(0, 1, 2, 1); // Durée ignorée, clignotement avec période=2, mode infini

            // Envoyer les données du jeu pour actualiser l'affichage avant de l'arrêter
            Send_Game_Run_Data();
            
            // Arrêter le jeu sans réinitialisation
            Stop_Game();
            return;
        }    
        
        // Repositionner la balle au centre
        game.ball_x = game.grid_width / 2;
        game.ball_y = game.grid_height / 2;

        // MODIFICATION : Réinitialiser la vitesse de la balle à sa valeur initiale
        // et la diriger vers le joueur 1 qui a marqué
        game.ball_dx = -2 - game.initial_ball_velocity;  // Vers la gauche (joueur 1)

        // Réinitialiser aussi la composante verticale avec une direction aléatoire
        uint32_t r = get_random_number_range(0, 2);
        if (r == 0) {
            game.ball_dy = 1 + game.initial_ball_velocity;
        } else if (r == 1) {
            game.ball_dy = 2 + game.initial_ball_velocity;
        } else {
            game.ball_dy = -1 - game.initial_ball_velocity;
        }

        // Limiter la vitesse verticale après le score
        game.ball_dy = Limit_Vertical_Speed(game.ball_dy);
    }
    
    // Envoyer les données à l'IHM
    Send_Game_Run_Data();
}

/**
 * @brief Callback pour les données reçues sur l'UART.
 *
 * Cette fonction traite les commandes reçues sur l'UART:
 * - echo: Renvoie le message
 * - game:start: Démarre une partie
 * - game:pause: Met le jeu en pause
 * - game:resume: Reprend le jeu
 * - game:stop: Arrête le jeu
 *
 * @param msg Message reçu
 * @param length Longueur du message
 */
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
                                        Start_Game(grid_width, grid_height, max_points, ball_velocity, ball_size, paddle_velocity, paddle_size);
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

/**
 * @brief Callback pour le bouton bleu de la carte.
 *
 * - Si le jeu n'est pas démarré: démarre une partie
 * - Si le jeu est en cours: met en pause
 * - Si le jeu est en pause: reprend
 */
void Blue_Button_Callback() {
	// Si la partie n'est pas en cours, démarrer la partie
	if (game.status == GAME_STATUS_NONE) {
		Start_Game(400, 250, 5, 1, 3, 5, 6);
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
