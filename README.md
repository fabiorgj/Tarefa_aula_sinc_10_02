
Embarcatech    

Tarefa da aula síncrona do dia 10/02/2025  

Aluno: **Fábio Rocha Gomes Jardim**    

E-mail: **fabiogomesj@gmail.com**

Matrícula: **TIC370100723**    


# Projeto Interativo com Joystick, PWM e Display OLED na BitdogLab

## Vídeo

[Link para o Drive]()    

## Introdução

Este projeto foi desenvolvido para a placa **BitdogLab**, que utiliza o **Raspberry Pi Pico W** como microcontrolador. O código demonstra a integração de diversas funcionalidades da placa, tais como:

- **Leitura do Joystick**: Utiliza o ADC para capturar as posições X e Y do joystick.
- **Controle de LED via PWM**: Ajusta o brilho dos LEDs vermelho e azul conforme a posição do joystick.
- **Interação com Botões**: Dois botões (BOT_A e BOT_JOY) permitem alternar estados do sistema, como habilitar/desabilitar o PWM, ligar e desligar o LED verde e alterar o padrão visual do display.
- **Display OLED via I2C**: O display SSD1306 é configurado para exibir elementos gráficos que reagem à interação com o joystick e os botões.

## Descrição e Funcionalidades do Projeto

### Controle de Brilho dos LEDs via PWM

- **Leitura Analógica**: O joystick, conectado aos canais ADC dos pinos designados, fornece leituras dos eixos X e Y.
- **Cálculo do Nível de Brilho**: A diferença absoluta entre a leitura e o valor central (2048) é utilizada para definir o nível de PWM.
- **Ajuste de PWM**: Função `set_pwm` configura o brilho dos LEDs vermelho (LED_R) e azul (LED_B) de acordo com as leituras do joystick. A frequência do PWM é definida em 1 kHz com resolução de 12 bits.

### Display OLED via I2C

- **Inicialização e Configuração**: A função `config_display` configura a comunicação I2C (utilizando o I2C1) e inicializa o display SSD1306.
- **Exibição Dinâmica**: No loop principal, o display é atualizado para:
  - Limpar a tela.
  - Desenhar um quadrado de 8x8 pixels cuja posição varia conforme as leituras dos eixos do joystick.
  - Exibir diferentes bordas (três estilos distintos) que alternam a cada acionamento do botão BOT_JOY.

### Interação com Botões

- **BOT_A**:
  - **Função**: Alterna a habilitação do PWM nos LEDs vermelho e azul.
  - **Mecanismo**: Ao pressionar, a função de callback `call_back_dos_botoes` inverte o estado de ativação do PWM, permitindo que os LEDs sejam ligados ou desligados.
  
- **BOT_JOY**:
  - **Função**: Alterna o estado do LED verde (LED_G) e modifica o estilo da borda desenhada no display.
  - **Mecanismo**: Cada pressionamento muda o estado do LED verde e incrementa um contador (`cont_bot_joy`), que define qual dos três estilos de borda será exibido.

### Comunicação e Processamento

- **ADC e PWM**: O código utiliza os módulos ADC e PWM do Raspberry Pi Pico para capturar a posição do joystick e ajustar os níveis de brilho dos LEDs.
- **I2C para o Display**: A comunicação com o display SSD1306 é realizada via I2C, permitindo o envio de comandos e dados gráficos para atualizar a tela em tempo real.
- **Interrupções e Debounce**: As interrupções dos botões são gerenciadas com um mecanismo de debounce (250 ms) para evitar leituras incorretas devido ao “bounce” dos contatos.

## Funções que Compõem o Código

- ### `config_display(void)`
  **Descrição**:  
  Inicializa e configura o display OLED SSD1306 utilizando a interface I2C.  
  **Funcionamento**:
  - Inicializa o I2C1 com 400 kHz e configura os pinos SDA e SCL.
  - Configura e envia os dados iniciais para o display, limpando-o e preparando-o para receber comandos gráficos.

- ### `config_pwm(uint gpio_pin)`
  **Descrição**:  
  Configura um pino específico para operação em PWM, definindo a frequência e resolução necessárias para o controle do brilho dos LEDs.  
  **Funcionamento**:
  - Define o pino para a função PWM.
  - Calcula o divisor de clock para atingir a frequência de 1 kHz.
  - Inicializa a “slice” e o canal do PWM com wrap de 4095 (12 bits) e nível inicial de brilho zero.

- ### `init(void)`
  **Descrição**:  
  Realiza as configurações iniciais dos GPIOs, ADC e PWM para os botões, joystick e LEDs.  
  **Funcionamento**:
  - Configura os pinos dos botões (BOT_JOY e BOT_A) com resistores de pull-up.
  - Inicializa os pinos do joystick como entradas ADC.
  - Configura o LED verde (LED_G) como saída digital.
  - Chama `config_pwm` para configurar os LEDs vermelho e azul.

- ### `set_pwm(uint gpio_pin, uint16_t level)`
  **Descrição**:  
  Ajusta o nível de brilho de um LED através da modulação PWM.  
  **Funcionamento**:
  - Determina a “slice” e o canal do pino.
  - Define o nível do PWM com base na leitura do joystick, atualizando o brilho do LED.

- ### `call_back_dos_botoes(uint gpio, uint32_t events)`
  **Descrição**:  
  Função de callback para tratar as interrupções dos botões.  
  **Funcionamento**:
  - Implementa um mecanismo de debounce (250 ms).
  - Se o botão acionado for o **BOT_A**, alterna a habilitação do PWM nos LEDs vermelho e azul.
  - Se o botão for o **BOT_JOY**, alterna o estado do LED verde e incrementa o contador que controla o estilo da borda do display.

- ### `main(void)`
  **Descrição**:  
  Função principal que integra todas as funcionalidades do projeto.  
  **Fluxo de Execução**:
  1. **Inicializações**:
     - Inicializa a comunicação padrão (stdio), o ADC e os GPIOs.
     - Configura o display OLED e os canais PWM.
  2. **Configuração de Interrupções**:
     - Define callbacks para os botões **BOT_A** e **BOT_JOY**.
  3. **Loop Principal**:
     - Realiza a leitura dos eixos do joystick via ADC.
     - Calcula os níveis de PWM para os LEDs vermelho e azul com base na diferença em relação ao valor central.
     - Atualiza os níveis de brilho dos LEDs.
     - Atualiza o display OLED:
       - Limpa a tela.
       - Desenha um quadrado que se move conforme a posição do joystick.
       - Desenha uma das três bordas, de acordo com o contador alterado pelo botão **BOT_JOY**.
     - Aguarda brevemente (100 ms) antes de repetir o ciclo.

## Considerações Finais

Este código demonstra como integrar leituras analógicas, controle de PWM e comunicação via I2C em um ambiente interativo na BitdogLab. Através do joystick e dos botões, o usuário pode controlar dinamicamente os níveis de brilho dos LEDs e interagir com o display OLED, tornando o sistema uma excelente plataforma para aprendizado e experimentação com microcontroladores e interfaces gráficas.

