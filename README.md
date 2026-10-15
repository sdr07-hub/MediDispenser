# MediDispenser Modular Refactor

This project was refactored from `MediDispenser_StepperFix.ino` without intentionally changing behavior.

## Dependency map

- `config.h`: hardware pins, timing constants, Wi-Fi credentials, LCD dimensions.
- `globals.h/.cpp`: shared hardware objects and persistent runtime state used across modules.
- `utils.h/.cpp`: LEDC wrapper and common alert stop helper.
- `lcd.h/.cpp`: all LCD row/centering helpers.
- `stepper.h/.cpp`: 28BYJ-48 carousel movement only.
- `dispenser.h/.cpp`: gate servo, agitator servo, dispense sequence, fade and dispensing buzzer pattern.
- `alarm.h/.cpp`: state-machine handlers, RTC schedule checks, alarm activation, missed-dose logic, done/test states.
- `webserver.h/.cpp`: HTML UI and REST endpoints.
- `wifi_manager.h/.cpp`: first Wi-Fi connection and non-blocking reconnect watchdog.
- `storage.h/.cpp`: NVS/Preferences startup loading and optional save helpers. Existing API handlers still write directly through `prefs` to preserve behavior.
- `history.h/.cpp`: dose history ring buffer.

## Why some state remains global

Arduino hardware objects (`lcd`, `rtc`, `server`, `Servo`, `Preferences`) and the main state-machine variables are shared by interrupt-free loop code, web API handlers, dispensing logic, and display code. They remain declared as `extern` in `globals.h` and defined once in `globals.cpp` so every module can access the same hardware/session state without duplicate instances.

## Compile notes

Open the `MediDispenser` folder in Arduino IDE. The folder name and `.ino` name match, so Arduino will compile all `.cpp` files in the same sketch folder as one project.


## Stepper debug fixes added

Hardware target: ULN2003 driver + 28BYJ-48 stepper motor.

Changes in this debug build:
- Uses 2048 half-steps/revolution and 256 steps per carousel slot for 8 positions.
- Slows the motor down and adds a small start/end ramp to reduce missed steps under load.
- Holds the coil briefly at the end of each move before releasing to reduce bounce/backlash.
- Saves carousel position, step phase, weekly dose count, and UI slot history in ESP32 Preferences/NVS.
- Refill now resets carousel software state to Sunday with 7/7 pills remaining.
- After 7 doses the app reports Loading Bay / refill needed.
- UI now separates physical motor slot from display slot so the app can show Sunday after refill and Loading Bay when empty.

Important hardware note: without a physical home sensor/limit switch, software cannot prove that the 28BYJ-48 actually moved if the motor stalls. If it still misses a slot, increase STEP_DELAY_MS in config.h from 4 to 5 or 6 and use a separate 5V supply with common GND.
