# EOD-Milsim-Game-Controller-v2

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Arduino](https://img.shields.io/badge/Platform-Arduino-00979D.svg)](https://www.arduino.cc/)

An advanced electronic game controller designed for tactical simulation sports (MILSIM/Airsoft/Paintball) and automated tabletop mechanics. This project showcases structured C++ logic, hardware-level random seeding via analog noise, and dynamic status telemetry over Serial interfaces.

---

## ⚠️ LEGAL DISCLAIMER & ETHICAL COMPLIANCE

**ENGLISH:** This project is strictly an educational piece of software and a recreational electronic countdown device designed exclusively for regulated sports simulation (such as Airsoft, Paintball, and MILSIM activities) and board games. 
* It **DOES NOT** contain instructions, schematics, or guidelines for creating hazardous, illegal, or destructive real-world devices. 
* It **DOES NOT** interact with or control any harmful substances or pyrotechnics. 
* The underlying code manages internal state variables (time modifiers, game states, LCD prints, and pull-up button readings) for recreational game design purposes only. The authors hold no responsibility for any misuse of this software.

**PORTUGUÊS:** Este projeto é estritamente um software educacional e um dispositivo eletrônico recreativo de contagem regressiva projetado exclusivamente para simulação de esportes regulamentados (como Airsoft, Paintball e atividades MILSIM) e jogos de tabuleiro.
* Ele **NÃO** contém instruções, esquemas ou diretrizes para a criação de dispositivos perigosos, ilegais ou destrutivos do mundo real.
* Ele **NÃO** interage ou controla quaisquer substâncias nocivas ou pirotecnia.
* O código gerencia variáveis de estado interno (modificadores de tempo, estados de jogo, exibições no LCD e leitura de botões pull-up) apenas para fins de design de jogos recreativos. Os autores não assumem qualquer responsabilidade pelo uso indevido deste software.

---

## 🛠️ Hardware Stack & Architecture

- **Microcontroller:** ATmega328P (Arduino Nano/Uno architecture)
- **Display:** 16x2 LCD Interface via I2C protocol (PCF8574)
- **Input Channels:** 8 Digital IO lines configured with internal `INPUT_PULLUP` resistors (simulating circuit isolation vectors).
- **Audio Feedback:** Piezoelectric active buzzer for state transition alerts.
- **Actuator Link:** Low-voltage isolated relay interface for perimeter alarm triggering.

## 🧠 Advanced Embedded Features (Software Highlights)

1. **True-Random Cryptographic Shuffling:** Combating pseudo-random limits in microcontrollers by gathering cumulative electrical ambient noise from an un-terminated analog pin (`A7`) over 16 iterations to seed the random engine.
2. **Fisher-Yates Shuffling Algorithm:** Implements a strict, unbiased programmatic distribution to dynamically assign the game functions to the hardware conductors at boot.
3. **SRAM Optimization (F() Macro):** Critical memory management reducing dynamic SRAM footprints by moving static telemetry strings directly into Program Memory (Flash Storage), ensuring deployment stability.
4. **State-Machine Logic:** Robust execution flow distinguishing initialization states, runtime countdown modulations, and absolute win/loss state locks.

---

## 📄 Documentation & Manuals

The game logic relies on detailed procedural manuals. You can find the rules in Portuguese here:

- [Manual do Engenheiro Solo (Solo Mode Manual)](docs/manual_solo.md) - Features the custom D6 table mechanics.
- [Manual de Esquadrão (Team Operations Manual)](docs/manual_esquadrao.md) - Standard operational procedures.
