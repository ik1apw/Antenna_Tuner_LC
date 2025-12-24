# Accordatore LC con Joystick – IK1APW

## 🇮🇹 Italiano

Accordatore LC automatico e manuale basato su **Arduino Nano**, progettato per applicazioni radioamatoriali.  
Il sistema consente il controllo preciso di due motori (L e C) tramite **joystick analogico**, **memorie EEPROM** e **richiamo automatico (MOV)**, con risposta rapida e funzione di **STOP immediato**.

Progetto stabile, collaudato e realmente funzionante su hardware reale.

---

### ✨ Caratteristiche principali

- 🎮 **Controllo manuale tramite joystick**
  - Asse X → controllo P1 (L)
  - Asse Y → controllo P2 (C)
  - Risposta rapida (50 Hz), ideale per accordature fini

- 🛑 **STOP immediato durante MOV**
  - Il pulsante del joystick interrompe istantaneamente il movimento automatico

- 💾 **Memorie EEPROM**
  - Fino a **20 posizioni** memorizzabili
  - Richiamo automatico con movimento dei motori verso il target

- ⚙️ **Modalità automatica (MOV)**
  - Richiamo memoria con regolazione automatica
  - Timeout di sicurezza (30 s)

- 🔧 **Calibrazione**
  - Calibrazione automatica del centro joystick all’avvio
  - Calibrazione fine dei potenziometri di feedback
  - Dati salvati in EEPROM

- 📟 **Display LCD 16x2 I²C**
  - Visualizzazione valori correnti e target
  - Indicatori MOV / CAL / STP
  - Frecce direzionali durante il movimento

---

### 🧰 Hardware utilizzato

- Arduino Nano (ATmega328P)
- Display LCD 16x2 con interfaccia I²C (PCF8574)
- Joystick analogico a 2 assi con pulsante
- Encoder rotativo con pulsante
- Driver motori (L298P / VMA03 / H-bridge o relè)
- 2 potenziometri di feedback posizione
- Alimentazione separata per motori (consigliata)

---

### 🔌 Mappatura pin Arduino

| Funzione | Pin |
|--------|-----|
| LCD SDA | A4 |
| LCD SCL | A5 |
| Joystick X | A2 |
| Joystick Y | A3 |
| Joystick SW (STOP) | D10 |
| Encoder CLK | D3 |
| Encoder DT | D4 |
| Encoder SW | D5 |
| Motore P1 CW / CCW | D6 / D7 |
| Motore P2 CW / CCW | D8 / D9 |
| Feedback P1 | A0 |
| Feedback P2 | A1 |

---

### 🖥️ Funzioni del pulsante encoder

| Durata pressione | Funzione |
|------------------|----------|
| ~2 s | Avvio MOV (richiamo memoria) |
| ~6 s | Salvataggio memoria corrente |
| ~15 s | Reset completo memorie |
| ~30 s | Ingresso modalità calibrazione |

---

### 🎮 Funzioni joystick

- Movimento joystick → controllo diretto motori (manuale)
- Pulsante joystick → **STOP immediato MOV**
- Joystick disabilitato durante MOV automatico

---

### 📁 File presenti nel repository

- `Accordatore_LC_Joystick_FAST_STOP.ino` → firmware finale stabile
- `Accordatore_LC_Joystick_IK1APW_v2.pdf` → documentazione tecnica
- `README.md` → questo file

---

### 📜 Licenza

Questo progetto è rilasciato sotto licenza **MIT**.  
Una citazione dell’autore è gradita.

---

### 👤 Autore

**Emanuele Rossi – IK1APW**  
Radioamatore e sperimentatore – Italia 🇮🇹  

**73 de IK1APW**

---

---

## 🇬🇧 English

Automatic and manual **LC tuner** based on **Arduino Nano**, designed for amateur radio applications.  
The system provides precise control of two motors (L and C) using an **analog joystick**, **EEPROM memories**, and **automatic recall (MOV)**, with fast response and an **immediate STOP** function.

Stable project, fully tested on real hardware.

---

### ✨ Main features

- 🎮 **Manual control via joystick**
  - X axis → P1 (L) control
  - Y axis → P2 (C) control
  - Fast response (50 Hz), ideal for fine tuning

- 🛑 **Immediate STOP during MOV**
  - Joystick push button instantly stops automatic movement

- 💾 **EEPROM memories**
  - Up to **20 memory positions**
  - Automatic motor movement to target values

- ⚙️ **Automatic mode (MOV)**
  - Memory recall with automatic adjustment
  - Safety timeout (30 s)

- 🔧 **Calibration**
  - Automatic joystick center calibration at startup
  - Fine calibration of feedback potentiometers
  - Data stored in EEPROM

- 📟 **16x2 I²C LCD display**
  - Current and target value visualization
  - MOV / CAL / STP indicators
  - Direction arrows during movement
---
### 🧰 Hardware used

- Arduino Nano (ATmega328P)
- 16x2 LCD with I²C interface (PCF8574)
- 2-axis analog joystick with push button
- Rotary encoder with push button
- Motor driver (L298P / VMA03 / H-bridge or relays)
- 2 position feedback potentiometers
- Separate motor
