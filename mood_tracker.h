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
#define MAX_YEARS 100
#define MAX_MONTHS 12
#define MAX_DAYS 31
#define MAX_MOOD 5

char labels[6][6] = {"Menu", "Ano", "Mes", "Dia", "Humor", "Frase"};
char mood_labels[5][9] = {"Feliz", "Triste", "Neutro", "Cansado", "Produtivo"};

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
    FELIZ,
    TRISTE,
    NEUTRO,
    DESANIMADO,
    PRODUTIVO,
}mood_enum;

typedef enum day_enum
{
    SEGUNDA,
    TERCA,
    QUARTA,
    QUINTA,
    SEXTA,
    SABADO,
    DOMINGO,
}day_enum;

typedef enum month_enum
{
    JANEIRO,
    FEVEREIRO,
    MARCO,
    ABRIL,
    MAIO,
    JUNHO,
    JULHO,
    AGOSTO,
    SETEMBRO,
    OUTUBRO,
    NOVEMBRO,
    DEZEMBRO
}month_enum;

typedef struct day
{
    day_enum dayName;
    mood_enum mood;
    char phrase[32];
}day;

typedef struct month
{
    month_enum monthName;
    day days[31];
}month;

typedef struct year
{
    uint yearNumber;
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