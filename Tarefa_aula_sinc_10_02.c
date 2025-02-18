#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#define JoyX 27
#define JoyY 26
#define BOT_JOY 22
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define BOT_A 5
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C // Endereço I2C do display
#define PWM_FREQ 1000  // Frequência do PWM (1 kHz)
#define CLOCK_PWM 125000000  // Clock base do PWM na Pico W (125 MHz)

//Variáveis globais
const uint32_t wrap = 4095;                     // Resolução de 12 bits
static volatile uint32_t tempo_anterior = 0;    // Timer para debounce
bool flag_bot_A = 1;                            // Flag para habilitar ou desabilitar o PWM
int cont_bot_joy = 0;                           // Contador para alternar a borda do display
ssd1306_t ssd;                                  // Estrutura do display
bool cor = true;                                // Cor de fundo do display

void config_display(void) {

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void config_pwm(uint gpio_pin) {
    // Configurar o pino para PWM
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

    // Identificar "slice" e "channel" associados ao pino
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    uint channel = pwm_gpio_to_channel(gpio_pin);

    // Calcular divisor do clock para atingir a frequência desejada
    float divider = CLOCK_PWM / (PWM_FREQ * (wrap + 1));

    // Configurar o PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, wrap);
    pwm_config_set_clkdiv(&config, divider);
    pwm_init(slice_num, &config, true);

    // Inicializar PWM com brilho 0
    pwm_set_chan_level(slice_num, channel, 0);
}

// Função de inicialização
void init(void) {
    gpio_init(BOT_JOY);
    gpio_set_dir(BOT_JOY, GPIO_IN);
    gpio_pull_up(BOT_JOY);

    gpio_init(BOT_A);
    gpio_set_dir(BOT_A, GPIO_IN);
    gpio_pull_up(BOT_A);

    adc_gpio_init(JoyX);  // Configura o pino como entrada ADC
    adc_gpio_init(JoyY);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 0);

    config_pwm(LED_R);
    config_pwm(LED_B);

}

// Função que define o brilho dos LEDs através do PWM
void set_pwm(uint gpio_pin, uint16_t level) {
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    uint channel = pwm_gpio_to_channel(gpio_pin);
    pwm_set_chan_level(slice_num, channel, level);
    printf("Brilho do LED: %d\n", level);
    printf("Wrap: %d\n", wrap);
}

void call_back_dos_botoes(uint gpio, uint32_t events){
    uint32_t tempo_agora = to_ms_since_boot(get_absolute_time()); // Pega o tempo atual em milissegundos para debounce
    if (tempo_agora - tempo_anterior > 250){ // Verifica se o botão foi pressionado por mais de 250ms para evitar bounce
        tempo_anterior = tempo_agora; // Atualiza o tempo anterior
        if (gpio == BOT_A){
            // Habilita ou desabilita o PWM dependendo do estado da flag
            pwm_set_enabled(pwm_gpio_to_slice_num(LED_R), !flag_bot_A);
            pwm_set_enabled(pwm_gpio_to_slice_num(LED_B), !flag_bot_A);
            flag_bot_A = !flag_bot_A;
        }else{
            gpio_put(LED_G, !gpio_get(LED_G)); // Inverte o estado do LED verde
            cont_bot_joy++; // Incrementa o contador, usado para alternar a borda do display
            if(cont_bot_joy == 3){
                cont_bot_joy = 0;
            }
        }
    }
}

int main(){

    //Inicializações e configurações
    stdio_init_all();
    adc_init();
    init();
    config_display();

    //Configura as interrupções dos botoes
    gpio_set_irq_enabled_with_callback(BOT_A, GPIO_IRQ_EDGE_FALL, true, &call_back_dos_botoes);
    gpio_set_irq_enabled_with_callback(BOT_JOY, GPIO_IRQ_EDGE_FALL, true, &call_back_dos_botoes);

    //Variáveis auxiliiares no cálculo dos valores do joystick
    int level_pwm = 0;
    int level_pwm_2 = 0;
    int leitura_joyX = 0;
    int leitura_joyY = 0;

    while (1) {

        adc_select_input(1); //Seleciona a entrada ADC do X, para fazer a medição
        leitura_joyX = adc_read();
        printf("X: %d\n", leitura_joyX);
        level_pwm = abs(leitura_joyX-2048)*2;        

        adc_select_input(0); //Seleciona a entrada ADC do Y, para fazer a medição
        leitura_joyY = adc_read();
        printf("Y: %d\n", leitura_joyY);
        level_pwm_2 = abs(leitura_joyY-2048)*2;        

        //Chama a função para setar o brilho dos LEDs de acordo com a leitura do joystick
        set_pwm(LED_R, level_pwm);
        set_pwm(LED_B, level_pwm_2);
        sleep_ms(100);

        ssd1306_fill(&ssd, !cor); // Limpa o display

        //Desenha um quadrado 8x8, cujo a posição depende da leitura do joystick
        ssd1306_rect(&ssd, 28-((leitura_joyY-2048)*0.0105), 60+((leitura_joyX-2048)*0.0265), 8, 8, cor, cor);

        if (cont_bot_joy==0){ //Desenha a primeira borda do display
            ssd1306_rect(&ssd, 1, 1, 127, 63, 1, 0);
            ssd1306_rect(&ssd, 5, 5, 118, 55, 0, 0);
        }
        if(cont_bot_joy==1){ //Desenha a segunda borda do display
            ssd1306_rect(&ssd, 1, 1, 127, 63, 0, 0);
            ssd1306_rect(&ssd, 5, 5, 118, 55, 1, 0);
        }
        if(cont_bot_joy==2){ //Desenha a terceira borda do display
            ssd1306_rect(&ssd, 1, 1, 127, 63, 1, 0);
            ssd1306_rect(&ssd, 5, 5, 118, 55, 1, 0);
        }

        ssd1306_send_data(&ssd); // Atualiza o display
        
    }
    return 0;
}
