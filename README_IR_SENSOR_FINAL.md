IR sensor final setup

Use the new LM393 IR obstacle sensor with these readings:
- No object: Serial = 1 / HIGH
- Object detected: Serial = 0 / LOW

Wiring for this project:
- Sensor OUT -> ESP32 GPIO32
- Sensor GND -> ESP32 GND
- Sensor VCC -> ESP32 3.3V

Important: do not use GPIO33 for IR in this project because GPIO33 is already used by PIN_LED.

Code changes:
- config.h: PIN_IR changed to 32
- config.h: IR_DETECTED_STATE = LOW
- config.h: IR_IDLE_STATE = HIGH
- alarm.cpp: IR checks now use these constants instead of hard-coded HIGH/LOW
