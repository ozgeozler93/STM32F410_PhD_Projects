cat <<EOF > README.md
# STM32F410 PhD Projects 🚀

This repository contains my embedded systems journey using the **NUCLEO-F410RB** development board.

## Current Progress
- [x] **Project 1: LED & Button Interfacing**
  - Configured GPIOs for LED (PA5) and Blue Button (PC13).
  - Implemented logic using **External Interrupts (EXTI)** with Rising/Falling edge detection.
- [ ] **Project 2: UART Communication** (In Progress)
  - Real-time serial messaging between MCU and PC.

## Tools Used
- **IDE:** STM32CubeIDE 2.1.1
- **Configuration:** STM32CubeMX (Standalone)
- **Hardware:** STM32F410RB Nucleo-64
EOF

git add README.md
git commit -m "Add README.md with project overview"
git push origin main
