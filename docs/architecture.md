# Mask Machine Firmware Architecture

## Scope

This repository starts the STM32H743 mask dispensing machine firmware. Phase 1 is a bootable framework only: FreeRTOS tasks, replaceable service interfaces, BSP wrappers, driver stubs, and an LVGL dummy display port. Real motor, RFID, environment sensor, LCD, touch, and communications pin assignments are intentionally deferred.

## Layers

- `Core/`: STM32CubeMX generated startup, HAL init, GPIO, USART1, FreeRTOS glue, and clock setup.
- `User/BSP/`: small board support wrappers for LEDs, logging, time, and the LVGL dummy display.
- `User/Drivers/`: replaceable low-level driver interfaces for actuator, RFID, environment sensor, and communications.
- `User/Services/`: business-level mask dispensing, inventory, auth, environment, UI, and diagnostics services.
- `User/App/`: application bootstrap, CMSIS-RTOS v2 queue binding, and task entry functions.
- `Middlewares/LVGL/`: LVGL 8.2.0 core source with `lv_conf.h`; display output is still a null flush driver.

## Runtime Tasks

- `appControl`: consumes application events and calls the dispenser service state machine.
- `appPoll`: polls RFID and environment stubs; posts events when real drivers later provide data.
- `appUi`: updates a small LVGL label on the dummy display and runs `lv_timer_handler()`.
- `appDiag`: updates diagnostics and toggles the heartbeat LED.
- `defaultTask`: kept as a low-activity idle application task.

## Stable Phase 1 Interfaces

- Status: `app_status_t`, including `APP_OK`, `APP_ERR_NOT_READY`, `APP_ERR_NOT_IMPLEMENTED`, and `APP_ERR_TIMEOUT`.
- Dispenser: `Svc_Dispenser_RequestOne()`, `Svc_Dispenser_GetState()`, `Svc_Dispenser_GetSnapshot()`.
- Inventory: `Svc_Inventory_SetTotal()`, `Svc_Inventory_Decrement()`, `Svc_Inventory_GetSnapshot()`.
- Auth: `Svc_Auth_Poll()`, currently backed by an RFID stub.
- Environment: `Svc_Environment_Poll()`, currently backed by stub data.
- UI: `Svc_Ui_PostEvent()`, `Svc_Ui_SetSnapshot()`, currently using an LVGL dummy display.

## Regeneration

Open the `.ioc` as an existing CubeMX project. Do not use the CubeMX "New Project" flow for this file. If CubeMX regenerates the MDK project, run:

```powershell
powershell -ExecutionPolicy Bypass -File Firmware\MaskMachine_H743\tools\update_keil_project.ps1
```

That script re-adds the `User/` groups, LVGL source group, include paths, and `LV_CONF_INCLUDE_SIMPLE` define to the Keil project.
