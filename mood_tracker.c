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
#include "ws2818b_interface.c"
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

int main()
{
    init();

    year years[100];
    selected selected = {0, 0, 0, 0};
    screens_enum screen = MENU;
    volatile bool execute_action = false;
    volatile uint joystick_x, joystick_y = 0;
    volatile action_enum action = NOONE;

    printf("Iniciando...\n");
    ssd1306_screen_update(&screen, &selected, years);
    printf("Tela inicializada\n");
    // reset_usb_boot(0, 0);

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
        printf("X: %d, Y: %d\n", joystick_x, joystick_y);
        printf("Execute action: %d\n", execute_action);
        printf("Screen: %d\n", screen);
        printf("Action: %d\n", action);

        do_action(&screen, action, years, &selected);
        ssd1306_screen_update(&screen, &selected, years);
        sleep_ms(200);
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
            case BUTTON_A:
                break;
            case BUTTON_B:
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
    pwm_set_gpio_level(pin, 0);            // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);          // Habilita o PWM no slice correspondente
}

void ssd1306_screen_update(screens_enum *screen, selected *selected, year years[MAX_YEARS])
{
    char text[9] = "8D";
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
        printf("Mood: %s\n", text);
        printf("Selected mood: %d\n", selected->mood);
        break;
    default:
        break;
    }

    ssd1306_draw_string(&display, text, 53, 28);
    ssd1306_send_data(&display);
}

void do_action(screens_enum *screen, action_enum action, year years[MAX_YEARS], selected *selected)
{  
    if (action == RIGHT && *screen < MENU_YEAR_MONTH_DAY_MOOD)
    {
        *screen = *screen + 1;
        printf("Screen em do action: %d\n", *screen);
    }
    else if (action == LEFT && *screen > MENU)
    {
        *screen = *screen - 1;;
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
            break;
        default:
            break;
        }
    }
}

// void ssd1306_update_blinking_arrows()
// {

// }