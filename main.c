#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "ssd1306.h"

#define BUFFER_SIZE 256

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDRESS 0x3C
#define MIC_PIN 28

ssd1306_t ssd;

void setup_mic() {
    gpio_set_function(MIC_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MIC_PIN);
    pwm_set_enabled(slice_num, true);
}

uint16_t capture_audio() {
    uint slice_num = pwm_gpio_to_slice_num(MIC_PIN);
    return pwm_get_counter(slice_num);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Aguarda inicialização

    setup_mic();
    printf("Microfone pronto, aguardando som...\n");

    // Inicializa I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED corretamente
    ssd1306_init(&ssd, 128, 64, false, OLED_ADDRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, 0); // Limpa a tela
    ssd1306_draw_string(&ssd, "Aguardando texto...", 0, 0); // Corrigido
    ssd1306_send_data(&ssd); // Corrigido

    char buffer[BUFFER_SIZE];
    int index = 0;

    printf("📡 Sistema pronto! Aguardando dados pela UART...\n");

    while (1) {

        uint16_t sample = capture_audio();
        putchar((char)(sample & 0xFF));
        putchar((char)(sample >> 8) & 0xFF);

        sleep_ms(50);



        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || index >= BUFFER_SIZE - 1) { 
                buffer[index] = '\0'; // Finaliza a string
                printf("📩 Recebido: %s\n", buffer); 

                // Atualiza o OLED corretamente
                ssd1306_fill(&ssd, 0);
                ssd1306_draw_string(&ssd, buffer, 0, 0); // Corrigido
                ssd1306_send_data(&ssd); // Corrigido

                index = 0;
            } else {
                buffer[index++] = (char)c;
            }
        }
    }
    return 0;
}
