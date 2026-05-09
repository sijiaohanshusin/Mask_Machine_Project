# Mask Machine Project

This repository tracks only the new STM32H743 mask machine firmware and lightweight project notes.

Large local reference material, the previous `NECCS` acoustic imaging project, generated OCR context, and vendor PDF/ZIP bundles are intentionally ignored by Git. Keep them in this workspace as references, but do not commit them into this project history.

## Firmware Entry

- `Firmware/MaskMachine_H743/`: STM32H743IITx mask machine firmware skeleton.
- `docs/architecture.md`: phase 1 layering, task model, public interfaces, and regeneration notes.
- Toolchain target: STM32CubeMX + MDK-ARM/Keil.
- Phase 1 scope: FreeRTOS task framework, middleware/service interfaces, driver stubs, LVGL dummy integration.

## Hardware Policy

Only board resources that are already unambiguous are configured in CubeMX for phase 1:

- HSE 25 MHz
- SYSCLK 480 MHz
- SWD debug
- TIM6 HAL timebase
- USART1 debug log
- PB0/PB1 LEDs

Motor, RFID, environmental sensor, communication module, real display, and touch pins must not be hard-coded until the wiring is confirmed.
