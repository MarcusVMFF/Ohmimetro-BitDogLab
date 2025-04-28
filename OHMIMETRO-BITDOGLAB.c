#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include <math.h>
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define BUTTON_A 5  // GPIO para botão A
#define BUTTON_B 6
#define LED_R 13
#define LED_G 11
#define LED_B 12
int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
char* cores[] = {
  "Black ", " Brown", " Red", "Orange",
  "Yellow", "Green ", "Blue", "Violet",
  "Grey", "White"
};
volatile int cores_resistor[3] = {0};
volatile bool botao_pressionado = false;

// Função para configurar o LED RGB
void setup_rgb_led() {
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
}

// Função para acender o LED com uma cor específica
void acender_led(int cor) {
    // Desliga todas as cores primeiro
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
    
    switch(cor) {
        case 0: // Black (desligado)
            break;
        case 1: // Brown (usaremos branco como substituto)
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            gpio_put(LED_B, 1);
            break;
        case 2: // Red
            gpio_put(LED_R, 1);
            break;
        case 3: // Orange (vermelho + verde)
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            break;
        case 4: // Yellow (vermelho + verde)
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            break;
        case 5: // Green
            gpio_put(LED_G, 1);
            break;
        case 6: // Blue
            gpio_put(LED_B, 1);
            break;
        case 7: // Violet (vermelho + azul)
            gpio_put(LED_R, 1);
            gpio_put(LED_B, 1);
            break;
        case 8: // Grey (usaremos branco como substituto)
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            gpio_put(LED_B, 1);
            break;
        case 9: // White
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            gpio_put(LED_B, 1);
            break;
    }
}

void obter_cores_resistor(int valor, char* resultado) {
  if (valor < 10 || valor > 990000000) {
      sprintf(resultado, "---");
      return;
  }

  int dig1 = 0, dig2 = 0, multiplicador = 0;
  int temp = valor;

  while (temp >= 100) {
      temp /= 10;
      multiplicador++;
  }

  dig1 = temp / 10;
  dig2 = temp % 10;

  if (dig1 < 0 || dig1 > 9 || dig2 < 0 || dig2 > 9 || multiplicador > 9) {
      sprintf(resultado, "---");
      return;
  }

  // Armazena as cores para uso na interrupção
  cores_resistor[0] = dig1;
  cores_resistor[1] = dig2;
  cores_resistor[2] = multiplicador;

  sprintf(resultado, "%s %s %s", cores[dig1], cores[dig2], cores[multiplicador]);
}
void piscar_cores_resistor() {
  for (int i = 0; i < 3; i++) {
      acender_led(cores_resistor[i]);
      sleep_ms(1000);
      acender_led(0); // Desliga o LED
      sleep_ms(200);  // Pequena pausa entre as cores
  }
}

#include "pico/bootrom.h"

uint32_t last_time_btn = 0;
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A && (current_time - last_time_btn > 300000)) {
        last_time_btn = current_time;
        botao_pressionado = true;
    }
    if (gpio == BUTTON_B) {
        last_time_btn = current_time;
        reset_usb_boot(0, 0);
    }
}
float E24_base[] = {
  10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30,
  33, 36, 39, 43, 47, 51, 56, 62, 68, 75, 82, 91
};

// Encontra o valor comercial mais próximo
int encontrar_valor_comercial(float valor) {
  float menor_dif = 999999;
  float valor_mais_proximo = 0;

  // Explora potências de 10 de 1Ω até 1MΩ
  for (int exp = 0; exp <= 6; exp++) {
    for (int i = 0; i < 24; i++) {
      float candidato = E24_base[i] * powf(10, exp);
      float dif = fabsf(candidato - valor);
      if (dif < menor_dif) {
        menor_dif = dif;
        valor_mais_proximo = candidato;
      }
    }
  }
  return (int)(valor_mais_proximo + 0.5f); // Arredonda para inteiro
}
int main() {
  stdio_init_all();
  
  // Configuração do botão B (BOOTSEL)
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_B);
  gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
  // Configuração do botão A
  gpio_init(BUTTON_A);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_pull_up(BUTTON_A);
  gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_callback(gpio_irq_handler);
  irq_set_enabled(IO_IRQ_BANK0, true);
  
  // Configuração do LED RGB
  setup_rgb_led();
  
  // Configuração do I2C e display (existente)
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  
  ssd1306_t ssd;
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
  ssd1306_config(&ssd);
  ssd1306_send_data(&ssd);
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);
  
  adc_init();
  adc_gpio_init(ADC_PIN);
  
  char str_cores[32];
  char str_x[5];
  char str_y[5];
  
  bool cor = true;
  
  while (true) {
      adc_select_input(2);
      
      float soma = 0.0f;
      for (int i = 0; i < 500; i++) {
          soma += adc_read();
          sleep_ms(1);
      }
      float media = soma / 500.0f;
      
      if (media < 200 || media > 3900) {
          R_x = 0;
          sprintf(str_y, "---");
          sprintf(str_x, "---");
          sprintf(str_cores, "---");
      } else {
          R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
          float comercial = encontrar_valor_comercial(R_x);
          sprintf(str_y, "%1.0f", comercial);
          sprintf(str_x, "%1.0f", R_x);
          obter_cores_resistor((int)comercial, str_cores);
      }
      
      // Verifica se o botão A foi pressionado
      if (botao_pressionado) {
          botao_pressionado = false;
          piscar_cores_resistor();
      }
      
      // Atualização do display (existente)
      ssd1306_fill(&ssd, !cor);
      ssd1306_line(&ssd, 3, 25, 123, 25, cor);
      ssd1306_line(&ssd, 3, 37, 123, 37, cor);
      ssd1306_draw_string(&ssd, str_cores, 1, 5);
      ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28);
      ssd1306_draw_string(&ssd, "REAL", 5, 41);
      ssd1306_draw_string(&ssd, "COMERCIAL", 50, 41);
      ssd1306_line(&ssd, 44, 37, 44, 60, cor);
      ssd1306_draw_string(&ssd, str_x, 8, 52);
      ssd1306_draw_string(&ssd, str_y, 59, 52);
      ssd1306_send_data(&ssd);
      
      sleep_ms(700);
  }
}
