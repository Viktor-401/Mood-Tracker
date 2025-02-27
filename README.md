# Mood Tracker

Este projeto é um mood tracker baseado em um display OLED SSD1306 e uma matriz de LEDs WS2812B, controlado por um joystick e botões. Ele permite que o usuário registre seu humor diário.

## Componentes

- **Display OLED SSD1306**
- **Matriz de LEDs WS2812B**
- **Joystick**
- **Botões**
- **Microcontrolador (ex: Raspberry Pi Pico)**

## Configuração do Hardware

### Pinos

- **BUTTON_A**: GPIO 5
- **BUTTON_B**: GPIO 6
- **MATRIZ_PIN**: GPIO 7
- **I2C_SDA**: GPIO 14
- **I2C_SCL**: GPIO 15
- **JOYSTICK_X_PIN**: GPIO 26
- **JOYSTICK_Y_PIN**: GPIO 27
- **JOYSTICK_BUTTON**: GPIO 22

### Conexões

- Conecte o display OLED SSD1306 aos pinos I2C_SDA e I2C_SCL.
- Conecte a matriz de LEDs WS2812B ao pino MATRIZ_PIN.
- Conecte o joystick aos pinos JOYSTICK_X_PIN, JOYSTICK_Y_PIN e JOYSTICK_BUTTON.
- Conecte os botões aos pinos BUTTON_A e BUTTON_B.

## Estrutura do Projeto

### Arquivos

- **mood_tracker.c**: Implementação principal do rastreador de humor.
- **mood_tracker.h**: Definições e declarações de variáveis e funções.
- **ssd1306.c**: Implementação da biblioteca para o display OLED SSD1306.
- **ssd1306.h**: Declarações de funções e definições para o display OLED SSD1306.

### Tipos e Estruturas

- **screens_enum**: Enumeração para as diferentes telas do menu.
- **mood_enum**: Enumeração para os diferentes estados de humor.
- **day_enum**: Enumeração para os dias da semana.
- **month_enum**: Enumeração para os meses do ano.
- **day**: Estrutura para armazenar o humor e a frase de um dia.
- **month**: Estrutura para armazenar os dias de um mês.
- **year**: Estrutura para armazenar os meses de um ano.

## Funcionalidades

- **Inicialização**: Inicializa o display OLED e a matriz de LEDs.
- **Leitura do Joystick**: Lê os valores do joystick e determina a ação correspondente.
- **Atualização da Tela**: Atualiza a tela OLED com base na seleção atual do usuário.
- **Atualização da Matriz de LEDs**: Atualiza a matriz de LEDs com base no humor selecionado.

## Como Compilar e Executar

1. **Clone o repositório**:
    ```sh
    git clone https://github.com/seu-usuario/mood_tracker.git
    cd mood_tracker
    ```

2. **Configure o SDK do Raspberry Pi Pico**:
    Siga as instruções no [Guia de Introdução ao Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) para configurar o SDK.

4. **Compile o código**:
    Utilizando a extensão para raspberry pico w é possível compilar o código.

5. **Carregue o código no Pico**:
    Utilizando a extensão para raspberry pico w é possível carregar o código na memória do Pico.

## Uso

- **Navegação**: Use o joystick para navegar pelos menus e selecionar a data.
- **Seleção**: Use os botões para desligar e ligar os leds, e mudar as páginas da matriz.
