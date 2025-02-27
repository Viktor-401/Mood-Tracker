#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"
#include "mood_tracker.h"
#include "ssd1306.c"
#include "pico/bootrom.h"

// Inicialização dos periféricos
void init();
// Função debounce, para retirar ruidos ao pressionar o botão
bool debounce();
// Callback da interrupção do botão
void button_press(uint gpio, uint32_t events); 
// Configura um pino para ser utilizado com PWM
void setup_pwm(int pin);
void ssd1306_screen_update(screens_enum *screen, selected *selected, year years[MAX_YEARS]);
void do_action(screens_enum *screen, action_enum action, year years[MAX_YEARS], selected *selected);
void ws2818b_update(year year[MAX_YEARS], selected selected);
void ws2818b_update_frame(PIO pio, uint sm, color matriz[5][5]);
void ws2818b_clear(PIO pio, uint sm);

bool matrix_on = true;
int matrix_page = 0;

int main()
{
    init();
    day days[MAX_DAYS];
    month months[MAX_MONTHS];
    year years[MAX_YEARS];
    int i;
    for(i=0; i<MAX_DAYS; i++)
    {
        days[i].mood = NOT_SELECTED;
        strcpy(days[i].phrase, "");
    }
    for(i=0; i<MAX_MONTHS; i++)
    {
        memcpy(months[i].days, days, sizeof(days));
    }
    memcpy(years->months, months, sizeof(months));

    selected selected = {0, 0, 0, 0};
    screens_enum screen = MENU;
    volatile uint joystick_x, joystick_y = 0;
    volatile action_enum action = NOONE;

    ssd1306_screen_update(&screen, &selected, years);

    while (true) {
        adc_select_input(0);
        joystick_y = adc_read();
        adc_select_input(1);
        joystick_x = adc_read();
        if (joystick_x > 3800)
        {
            action = RIGHT;
        }
        else if (joystick_x < 300)
        {
            action = LEFT;
        }
        else if (joystick_y > 3800)
        {
            action = UP;
        }
        else if (joystick_y < 300)
        {
            action = DOWN;
        }
        else
        {
            action = NOONE;
        }

        do_action(&screen, action, years, &selected);
        ssd1306_screen_update(&screen, &selected, years);
        if (matrix_on)
        {
            ws2818b_update(years, selected);
        }
        else
        {
            ws2818b_clear(pio, sm);
        }
        sleep_ms(100);
    }
}

void init()
{
    stdio_init_all();
    // Inicialização do I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_init(&display, 128, 64, false, 0x3C, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_send_data(&display);

    // Limpa completamente a tela
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Inicialização dos botões e leds
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_init(JOYSTICK_BUTTON);

    // ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    //PWM
    setup_pwm(MATRIZ_PIN);

    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);

    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
    gpio_pull_up(JOYSTICK_BUTTON);

    //Inicialização do programa da matriz de leds
    uint offset_ws = pio_add_program(pio0, &ws2818b_program);
    ws2818b_program_init(pio, sm, offset_ws, MATRIZ_PIN, 800000.f);

    // Habilitação das interrupções acionadas pelos botões, ativam em borda de descida
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_press);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_press);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_press);
}

//Compara o tempo atual com o tempo em que o botão foi pressionado pela última vez e determina se o sinal deve ser aceito
bool debounce()
{
    absolute_time_t time = get_absolute_time();

    if (absolute_time_diff_us(last_press, time) >= 200000) 
    {
        last_press = time;
        return true;
    }
    return false;
}

//Callback da interrupção do botão
void button_press(uint gpio, uint32_t events)
{
    if (debounce())
    {
        switch (gpio)
        {
            case BUTTON_A:                /* code */
                matrix_on = !matrix_on;
                break;
            case BUTTON_B:
                matrix_page = matrix_page == 1 ? 0 : 1;
                break;
            case JOYSTICK_BUTTON:
                break;
        }
    }
}

void setup_pwm(int pin)
{
    uint slice;
    gpio_set_function(pin, GPIO_FUNC_PWM); // Configura o pino do pin para função PWM
    slice = pwm_gpio_to_slice_num(pin);    // Obtém o slice do PWM associado ao pino do pin
    pwm_set_clkdiv(slice, DIVIDER_PWM);    // Define o divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);           // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(pin, 100);            // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);          // Habilita o PWM no slice correspondente
}

void ssd1306_draw_arrow(ssd1306_t *ssd, uint8_t x0, uint8_t y0, bool invert, bool up_arrow)
{  
    int offset = invert ? -6 : 6;
    if (up_arrow)
    {
        ssd1306_line(ssd, x0, y0, x0+3, y0+offset, true);
        ssd1306_line(ssd, x0+6, y0, x0+3, y0+offset, true);
        return;
    }
    else
    {
        ssd1306_line(ssd, x0, y0, x0 + offset, y0 + 3, true);
        ssd1306_line(ssd, x0, y0+6, x0 + offset, y0 + 3, true);
    }
}

void ssd1306_screen_update(screens_enum *screen, selected *selected, year years[MAX_YEARS])
{
    char text[9] = "";
    ssd1306_fill(&display, false);

    ssd1306_draw_string(&display, labels[*screen], 0, 28);

    ssd1306_rect(&display, 26, 50, 76, 11, true, false);

    switch (*screen)
    {
    case MENU_YEAR:
        itoa(selected->year+CURRENTLY_YEAR, text, 10);
        break;
    case MENU_YEAR_MONTH:
        itoa(selected->month+1,text, 10);
        break;
    case MENU_YEAR_MONTH_DAY:
        itoa(selected->day+1, text, 10);
        break;
    case MENU_YEAR_MONTH_DAY_MOOD:
        strcpy(text, mood_labels[selected->mood]);
        break;
    default:
        break;
    }
    ssd1306_draw_string(&display, text, 53, 28);

    ssd1306_draw_arrow(&display, 118, 43, false, false);
    ssd1306_draw_arrow(&display, 7, 43, true, false);

    ssd1306_draw_arrow(&display, 85, 23, true, true);
    ssd1306_draw_arrow(&display, 85, 39, false, true);
    if(matrix_on)
        ssd1306_draw_string(&display, "MatrizON", 0, 55);
    else
    {
        ssd1306_draw_string(&display, "MatrizOFF", 0, 55);
    }
    char page[1];
    itoa(matrix_page, page, 10);
    ssd1306_draw_string(&display, "Page ", 79, 55);
    ssd1306_draw_string(&display, page, 79+(8*5), 55);
    ssd1306_send_data(&display);
}

void do_action(screens_enum *screen, action_enum action, year years[MAX_YEARS], selected *selected)
{  
    if (action == RIGHT && *screen < MENU_YEAR_MONTH_DAY_MOOD)
    {
        *screen = *screen + 1;
    }
    else if (action == LEFT && *screen > MENU)
    {
        *screen = *screen - 1;
    }
    else if (action == UP && *screen > MENU)
    {
        switch (*screen)
        {
        case MENU_YEAR:
            selected->year = selected->year > 0 ? selected->year - 1 : 0;
            break;
        case MENU_YEAR_MONTH:
            selected->month = selected->month > 0 ? selected->month - 1 : 0;
            break;
        case MENU_YEAR_MONTH_DAY:
            selected->day = selected->day > 0 ? selected->day - 1 : 0;
            break;
        case MENU_YEAR_MONTH_DAY_MOOD:
            selected->mood = selected->mood > 0 ? selected->mood - 1 : 0;
            years[selected->year].months[selected->month].days[selected->day].mood = selected->mood;
            break;
        default:
            break;
        }
    }
    else if (action == DOWN && *screen > MENU)
    {
        switch (*screen)
        {
        case MENU_YEAR:
            selected->year = selected->year < MAX_YEARS-1 ? selected->year + 1 : MAX_YEARS-1;
            break;
        case MENU_YEAR_MONTH:
            selected->month = selected->month < MAX_MONTHS-1 ? selected->month + 1 : MAX_MONTHS-1;
            break;
        case MENU_YEAR_MONTH_DAY:
            selected->day = selected->day < MAX_DAYS-1 ? selected->day + 1 : MAX_DAYS-1;
            break;
        case MENU_YEAR_MONTH_DAY_MOOD:
            selected->mood = selected->mood < MAX_MOOD-1 ? selected->mood + 1 : MAX_MOOD-1;
            years[selected->year].months[selected->month].days[selected->day].mood = selected->mood;
            break;
        default:
            break;
        }
    }
}

void ws2818b_update(year year[MAX_YEARS], selected selected)
{
    uint index;
    color matrix[5][5];
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            index = (i*5) + j + (matrix_page*25);
            if (index < 31)
            {
                matrix[i][j] = mood_colors[year[selected.year].months[selected.month].days[index].mood];
            }
            else
            {
                matrix[i][j] = mood_colors[NOT_SELECTED];
            }
        }
    }
    ws2818b_update_frame(pio, sm, matrix);
}

//Recebe uma matriz 5x5 composta por struct color, e mostra um desenho na matriz de leds
void ws2818b_update_frame(PIO pio, uint sm, color matriz[5][5])
{
    for (int k = 0; k < LED_COUNT; k++)
    {
        int i = translated_indexes[k][0];
        int j = translated_indexes[k][1];
        pio_sm_put_blocking(pio, sm, matriz[i][j].g);
        pio_sm_put_blocking(pio, sm, matriz[i][j].r);
        pio_sm_put_blocking(pio, sm, matriz[i][j].b);
    }
}

void ws2818b_clear(PIO pio, uint sm)
{
    for (int k = 0; k < LED_COUNT; k++)
    {
        int i = translated_indexes[k][0];
        int j = translated_indexes[k][1];
        pio_sm_put_blocking(pio, sm, 0);
        pio_sm_put_blocking(pio, sm, 0);
        pio_sm_put_blocking(pio, sm, 0);
    }
}