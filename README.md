# 📊 **Ohmimetro na placa BitDogLab**

Projeto com a função de criar um ohmimetro capaz de medir resistores de 510 a 100kΩ, exibindo seu valor no display SSD1306 via comunicação I2C, e também uma representação visual de suas cores pelo LED GPIO.

---

## 🎥 **Demonstração**

[Ver Vídeo do Projeto](https://drive.google.com/file/d/1fqjvWtCGfokam-fgx0nKVfiGjMkEqMlz/view?usp=drive_link)

---

## 🛠️ **Tecnologias Utilizadas**

- **Linguagem de Programação:** C / CMake
- **Placas Microcontroladoras:**
  - BitDogLab
  - Pico W
- **Periféricos:**
  - Protoboard
  - Jumpers
  - Resistores
---

## 📖 **Como Utilizar**

- O circuito compõe 3 jumpers conectados a placa e a protoboard, na protoboard, um resistor de 10kΩ e o outro é o que será testado
- Conecte o resistor que você quer testar, no display, irá aparecer o código de cores, seu valor e seu valor comercial
- Apertando o botão A, os LEDs GPIO irão piscar o código de cores do resistor que está sendo testado.

---
