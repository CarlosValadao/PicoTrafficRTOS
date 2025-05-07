# TrafficLightRTOS [Vídeo de Apresentação]

Um sistema de semáforo de trânsito com funcionalidades de acessibilidade para pessoas com deficiência visual, desenvolvido para a plataforma RP2040 utilizando FreeRTOS, C e Pico SDK 2.1.0.

## 📋 Descrição do Projeto

TrafficLightRTOS é um software que simula um semáforo de trânsito completo com dois modos de operação: diurno e noturno. O sistema gerencia 5 periféricos através de tarefas concorrentes do FreeRTOS:

- Matriz de LEDs (WS2812B)
- LED RGB (SMD 5050)
- Buzzer passivo (MLT 8530)
- Botão
- Display OLED 128x64 (SSD1306)

O projeto implementa uma máquina de estados finita para controlar o fluxo de operação do semáforo, com recursos visuais e sonoros para garantir acessibilidade.

## 🚦 Funcionamento

### Modo Diurno

O semáforo segue uma sequência cíclica de três estados:

1. **Verde** (9 segundos)
   - Display: "Siga"
   - Buzzer: Beep de 250ms uma vez por segundo
   - Contagem regressiva em verde

2. **Amarelo** (3 segundos)
   - Display: "Atenção"
   - Buzzer: Beeps intermitentes de 250ms
   - Contagem regressiva em amarelo

3. **Vermelho** (6 segundos)
   - Display: "Pare"
   - Buzzer: Beep de 500ms a cada 1,5 segundos
   - Contagem regressiva em vermelho

### Modo Noturno

- LED e matriz: Cor amarela intermitente
- Buzzer: Beeps ininterruptos

## 🧩 Arquitetura do Sistema

O sistema é baseado em uma arquitetura de tarefas concorrentes gerenciadas pelo FreeRTOS. Cada periférico é controlado por uma tarefa dedicada:

| Tarefa          | Função                                | Prioridade               |
|-----------------|---------------------------------------|--------------------------|
| vBlinkTask      | Gerencia a matriz de LEDs e o contador | tskIDLE_PRIORITY + 3    |
| vLedColorTask   | Controla o LED RGB                    | tskIDLE_PRIORITY + 2     |
| vDisplayTask    | Atualiza as mensagens no OLED         | tskIDLE_PRIORITY + 1     |
| vBuzzerTask     | Produz os alertas sonoros             | tskIDLE_PRIORITY         |
| Botão           | Alterna entre modos diurno e noturno  | (Interrupção)            |

### Variáveis de Controle Global

- `g_semaphore_state`: Estado atual do semáforo
- `g_semaphore_led_color`: Cor atual do LED RGB
- `g_semaphore_counter`: Valor da contagem regressiva
- `g_semaphore_mode`: Modo atual do semáforo (diurno/noturno)

## 🔌 Hardware Utilizado

- **Microcontrolador:** RP2040 (Raspberry Pi Pico) rodando a 128MHz
- **Display OLED:** SSD1306 128x64 (I2C1, pinos 14 e 15, endereço 0x3C)
- **Matriz de LEDs:** WS2812B controlada pelo PIO0
- **LED RGB:** Conectado aos pinos 13 (vermelho), 11 (verde) e 12 (azul)
- **Buzzer Passivo:** Conectado ao pino 10
- **Botão:** Configurado para alternar entre modos e ativar modo BOOTSEL

## 💻 Detalhes de Implementação

### Inicialização do Sistema

O sistema configura todos os periféricos e cria as tarefas do FreeRTOS antes de iniciar o escalonador. Durante a inicialização, o display OLED mostra uma tela de apresentação com "CEPEDI TIC37", "TrafficLightRTOS" e "FreeRTOS".

### Controle de Estados

A função `update_semaphore_counter()` é responsável por decrementar o contador e gerenciar as transições entre os estados do semáforo quando o contador chega a zero.

### Acessibilidade

O sistema implementa feedback sonoro para pessoas com deficiência visual, com padrões distintos para cada estado do semáforo:

- Verde: Beep curto regular
- Amarelo: Beeps intermitentes rápidos
- Vermelho: Beeps longos espaçados

## ⚙️ Requisitos

- Pico SDK 2.1.0
- FreeRTOS
- Toolchain GCC para ARM
- CMake
- Placa BitDogLab

## 📝 Notas de Desenvolvimento

A troca de informações entre as tarefas é feita por meio de variáveis globais voláteis, sem necessidade de mecanismos adicionais de sincronização como filas, mutex ou semáforos.

O botão também pode ativar o modo BOOTSEL através da função `reset_usb_boot()`, permitindo reprogramar o dispositivo quando necessário.

## Como Executar o Projeto 🚀

1. **Conectar a Placa BitDogLab** via USB ao seu computador.
2. **Instalar o Pico SDK 2.1.0** em seu ambiente de desenvolvimento. Se necessário, siga as instruções de instalação do SDK [aqui](https://github.com/raspberrypi/pico-sdk).
3. **Carregar o código** no dispositivo usando uma IDE compatível com o Pico SDK (ex: VS Code ou CLion).
4. **Compilar e carregar o projeto** para o dispositivo. No terminal, use o seguinte comando para compilar o código:

   ```bash
   mkdir build && cd build
   cmake -G ninja && ninja```
