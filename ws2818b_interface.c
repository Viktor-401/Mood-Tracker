#include "pico/stdlib.h"
#include "hardware/pio.h"

#define LED_COUNT 25 //Quantidade de leds na matriz

#ifndef ws2818b_interface_c
#define ws2818b_interface_c
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
#endif
