#  Geração de números em uma matriz de LEDs 5x5

Este projeto implementa a funcionalidade de controle de uma matriz de LEDs coloridos 5x5 (WS2812) utilizando o **Raspberry Pi Pico W** através do pressionamento de 2 botões para passagem dos Frames, além disso, também é realizado o Debouncing dos botões e o Blink contínuo de um led RBG. O código é desenvolvido em **C** para sistemas embarcados e foi implementado na placa voltada a aprendizagem BitDogLab.

## Estrutura do Projeto

- **CMakeLists.txt**: Definição dos processos de compilação e requisitos necessários.
- **pico_sdk_import.cmake**: Arquivo com as configurações de importação do Pico SDK.
- **atv_interrupcoes.c**: Arquivo com o código C para o Firmware.

## Funcionalidades

- **Botão A**: Faz o movimento de decremento ao número exibido.  
- **Botão B**: Faz o movimento de incremento ao número exibido .  
- **Led RBG**: Pisca 5 vezes por segundo.  
- **Matriz de Leds**: Exibe o número.  

## Requisitos

- **Hardware**:
  - Computador pessoal.
  - Raspberry Pi Pico W (RP2040).
  - 2 Botões.
  - 1 Led RGB.
  - No mínimo 3 resistores.
  - Matriz de LEDs Coloridos 5x5 WS2812.
  - Cabo micro-USB para USB-A.

- **Software**:
  - Visual Studio Code
  - Pico SDK
  - Compilador ARM GCC


## Vídeo demonstrativo 
[Assista aqui](https://drive.google.com/file/d/11QTtBCxPk06YGa7v4cQOCkaB4slWW50g/view?usp=sharing)

