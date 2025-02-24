#ifndef mood_tracker_h 
#define mood_tracker_h

#include "ssd1306.h"
#define BUTTON_A 5
#define BUTTON_B 6
#define MATRIZ_PIN 7
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON 22

absolute_time_t last_press = {0};

ssd1306_t display;

const PIO pio = pio0;
const uint sm = 0;

#define PERIOD 2000   // Período do PWM (valor máximo do contador)
#define DIVIDER_PWM 16 // Divisor fracional do clock para o PWM

#define CURRENTLY_YEAR 2024
#define MAX_YEARS 1
#define MAX_MONTHS 12
#define MAX_DAYS 31
#define MAX_MOOD 6

#define LED_COUNT 25 //Quantidade de leds na matriz

typedef struct color
{
    uint r;
    uint g;
    uint b;
}color;

//Como a fita de led utilizada na matriz é indexada linearmente, traduzi cada index para uma coordenada em uma matriz 5X5
//Não é necessário, mas facilita a visualização da matriz no código, e ter certeza de como ela aparecerá na matriz de leds
const int translated_indexes[LED_COUNT][2] = {
    {4,4},
    {4,3},
    {4,2},
    {4,1},
    {4,0},
    {3,0},
    {3,1},
    {3,2},
    {3,3},
    {3,4},
    {2,4},
    {2,3},
    {2,2},
    {2,1},
    {2,0},
    {1,0},
    {1,1},
    {1,2},
    {1,3},
    {1,4},
    {0,4},
    {0,3},
    {0,2},
    {0,1},
    {0,0}
    };

char labels[6][6] = {"Menu", "Ano", "Mes", "Dia", "Humor", "Frase"};
char mood_labels[6][9] = {" --- ", "Feliz", "Triste", "Neutro", "Cansado", "Produtivo"};
color mood_colors[6] = {{0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}, {0, 0, 1}, {0, 1, 1}};

typedef enum action_enum
{
    NOONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
}action_enum;

typedef enum screens_enum
{
    MENU,
    MENU_YEAR,
    MENU_YEAR_MONTH,
    MENU_YEAR_MONTH_DAY,
    MENU_YEAR_MONTH_DAY_MOOD,
    MENU_YEAR_MONTH_DAY_MOOD_PHRASE,
}screens_enum;

typedef enum mood_enum
{
    NOT_SELECTED,
    FELIZ,
    TRISTE,
    NEUTRO,
    DESANIMADO,
    PRODUTIVO,
}mood_enum;

typedef struct day
{
    mood_enum mood;
    char phrase[32];
}day;

typedef struct month
{
    day days[31];
}month;

typedef struct year
{
    month months[12];
} year;

typedef struct selected
{
    uint year;
    uint month;
    uint day;
    uint mood;
} selected;

#endif // !defined( mood_tracker_h )