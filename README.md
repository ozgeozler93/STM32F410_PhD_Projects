# STM32F410 PhD Projects 🚀

This repository documents my embedded systems research and development journey using the **NUCLEO-F410RB** (ARM Cortex-M4) development board. The goal is to master low-level hardware interaction, real-time constraints, and peripheral interfacing.

## 🛠 Tech Stack
- **Hardware:** STM32F410RB (Nucleo-64)
- **IDE:** STM32CubeIDE v2.1.1
- **Configuration:** STM32CubeMX
- **Language:** C (HAL Library)

---
<<<<<<< HEAD

## 📈 Project Milestones

### 1. External Interrupts (EXTI) & GPIO
- **Objective:** Efficiently handle user input without polling the CPU.
- **Implementation:** Configured the Blue User Button (PC13) with **Rising/Falling edge detection**.
- **Outcome:** Real-time LED (PA5) control and interrupt-driven logic.

### 2. UART Serial Communication
- **Objective:** Establish a data link between the MCU and PC for debugging and data logging.
- **Configuration:** USART2 @ 115200 Baud Rate.
- **Outcome:** Real-time status updates ("Button Pressed / Released") sent to the serial terminal (MacBook Air).

### 3. Software PWM (Breathing LED)
- **Objective:** Control LED brightness on a pin (PA5) that lacks hardware PWM support.
- **Implementation:** - Created a custom microsecond delay function (`HAL_Delay_us`) using **TIM1**.
  - Implemented a **Software PWM** algorithm by manually toggling the GPIO within a high-frequency loop.
  - Added a "Breathing" effect by dynamically adjusting the duty cycle.
- **Outcome:** Smooth analog-like brightness transitions on a digital-only pin.

---

## 🚀 How to Run
1. Clone the repository.
2. Open the project in **STM32CubeIDE**.
3. Connect your **Nucleo-F410RB** via USB.
4. Build the project (Hammer icon) and Run (Play icon).
5. Open a Serial Terminal (e.g., `screen` or IDE Console) at **115200 baud** to see live logs.

## 📬 Contact
**Özge Özler** - [GitHub](https://github.com/ozgeozler93)
EOF
=======
>>>>>>> 1e2456d (Success: ADC Continuous Conversion implemented. Verified with manual A0/GND testing.)

## 📈 Project Milestones

### 1. External Interrupts (EXTI) & GPIO
- **Objective:** Efficiently handle user input without polling the CPU.
- **Implementation:** Configured the Blue User Button (PC13) with **Rising/Falling edge detection**.
- **Outcome:** Real-time LED (PA5) control and interrupt-driven logic.

### 2. UART Serial Communication
- **Objective:** Establish a data link between the MCU and PC for debugging and data logging.
- **Configuration:** USART2 @ 115200 Baud Rate.
- **Outcome:** Real-time status updates ("Button Pressed / Released") sent to the serial terminal (MacBook Air).

### 3. Software PWM (Breathing LED)
- **Objective:** Control LED brightness on a pin (PA5) that lacks hardware PWM support.
- **Implementation:** - Created a custom microsecond delay function () using **TIM1**.
  - Implemented a **Software PWM** algorithm by manually toggling the GPIO within a high-frequency loop.
  - Added a "Breathing" effect by dynamically adjusting the duty cycle.
- **Outcome:** Smooth analog-like brightness transitions on a digital-only pin.

---

## 🚀 How to Run
1. Clone the repository.
2. Open the project in **STM32CubeIDE**.
3. Connect your **Nucleo-F410RB** via USB.
4. Build the project (Hammer icon) and Run (Play icon).
5. Open a Serial Terminal (e.g., $TERM too long - sorry. or IDE Console) at **115200 baud** to see live logs.

## 📬 Contact
**Özge Özler** - [GitHub](https://github.com/ozgeozler93) EOF
