#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "pio_matrix.pio.h"

#define PIN_RGB 11
#define PIN_WS2812 7
#define PIN_BUTTON_A 5
#define PIN_BUTTON_B 6
#define INT 0.5

#define QUANT_FRAMES 10
#define LARG 5
#define ALTU 5

volatile uint frame_index = 0;
volatile bool atualizou = false;
static volatile uint32_t temp_led = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile uint32_t temp_button = 0;

const float frames[QUANT_FRAMES][LARG][ALTU] = {
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, 0.0, INT, 0.0}, // 0
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, 0.0, INT, 0.0, 0.0},
        {0.0, INT, INT, 0.0, 0.0},
        {0.0, 0.0, INT, 0.0, 0.0}, // 1
        {0.0, 0.0, INT, 0.0, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 2
        {0.0, INT, 0.0, 0.0, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, 0.0, INT, INT, 0.0}, // 3
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 4
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, INT, 0.0, 0.0, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 5
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, INT, 0.0, 0.0, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 6
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0}, // 7
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 8
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}
    },
    {
        {0.0, INT, INT, INT, 0.0},
        {0.0, INT, 0.0, INT, 0.0},
        {0.0, INT, INT, INT, 0.0}, // 9
        {0.0, 0.0, 0.0, INT, 0.0},
        {0.0, 0.0, 0.0, INT, 0.0}
    }
};

// Protótipos das funções
void init_pins();
uint32_t matrix_rgb();
void exibirFrame(const float frame[ALTU][LARG], PIO pio, uint sm);
void button_irq_callback(uint gpio, uint32_t events); // Função de callback da interrupção
void blinkLed();

int main() {
    stdio_init_all();
    init_pins();



    PIO pio = pio0;
    bool sys_clock = set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, PIN_WS2812);

    exibirFrame(frames[frame_index], pio, sm);
    while (true) {
        if (atualizou) {exibirFrame(frames[frame_index], pio, sm);}
        blinkLed();
    }
}

// Inicialização dos pinos e configuração da interrupção
void init_pins() {
    gpio_init(PIN_BUTTON_A);
    gpio_set_dir(PIN_BUTTON_A, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_A);
    gpio_put(PIN_BUTTON_A, true);
    
    gpio_init(PIN_BUTTON_B);
    gpio_set_dir(PIN_BUTTON_B, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_B);
    gpio_put(PIN_BUTTON_B, true);


    gpio_init(PIN_RGB);
    gpio_set_dir(PIN_RGB, GPIO_OUT);

    gpio_init(PIN_WS2812);
    gpio_set_dir(PIN_WS2812, GPIO_OUT);

    // Configura interrupções para borda de descida (pressionamento do botão)
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_irq_callback);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_irq_callback);
}

// Callback de interrupção dos botões
void button_irq_callback(uint gpio, uint32_t events) {
    printf("Entrei no callback");
    if (events & GPIO_IRQ_EDGE_FALL) {

        uint32_t current_time = to_us_since_boot(get_absolute_time());

        // Debouncing
        if (current_time - temp_button> 150000) 
        {
            temp_button = current_time;

            if (gpio == PIN_BUTTON_A) {
                frame_index = (frame_index == 0) ? QUANT_FRAMES - 1 : frame_index - 1;
            } 
            else if (gpio == PIN_BUTTON_B) {
                frame_index = (frame_index < QUANT_FRAMES - 1) ? frame_index + 1 : 0;
            }
        atualizou = true;
        }
    }
    printf("\nNUMERO: %i\n", frame_index);
}

// Função para definir a cor do LED RGB
uint32_t matrix_rgb() {
    unsigned char R = 0.5 * 255;
    unsigned char G = 0.7 * 255;
    unsigned char B = 0.5 * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Exibe um frame na matriz
void exibirFrame(const float frame[ALTU][LARG], PIO pio, uint sm) {
    uint32_t cor_led = matrix_rgb();

    for (uint linha = 0; linha < ALTU; linha++) {
        for (uint coluna = 0; coluna < LARG; coluna++) {
            uint32_t valor_cor = (linha > 0 && linha % 2 != 0) ? cor_led * frame[ALTU - linha - 1][coluna] : cor_led * frame[ALTU - linha - 1][LARG - coluna - 1];
            pio_sm_put_blocking(pio, sm, valor_cor);
        }
    }

    atualizou = false;
}

void blinkLed() {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - temp_led > 100000)
    {
        temp_led = current_time;
        gpio_put(PIN_RGB, !(gpio_get(PIN_RGB)));
                       
    }
}
