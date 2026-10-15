#pragma once

// Wi-Fi credentials
#define WIFI_SSID   "MediDemo"
#define WIFI_PASS   "12345678"

// Pinout
#define PIN_IR          32
#define IR_DETECTED_STATE LOW
#define IR_IDLE_STATE     HIGH
#define PIN_LED         33
#define PIN_BUZZER      15
#define PIN_IN1         26
#define PIN_IN2         27
#define PIN_IN3         14
#define PIN_IN4         25
#define PIN_GATE        13
#define PIN_AGITATOR    12

// LEDC — ESP32 v3.x API
#define LED_FREQ    5000
#define LED_RES     8

// Stepper — 28BYJ-48 + ULN2003
// Calibrated for this 28BYJ-48 + ULN2003 project.
// Previous 2048/8 = 256 made the carousel move about half the required distance.
// 4096/8 = 512 gives one full 45-degree carousel slot per dose.
#define STEPS_PER_REV   4096
#define CAROUSEL_POSITIONS 8
#define PILL_SLOTS 7
#define LOADING_BAY_SLOT 7
#define STEPS_PER_SLOT  (STEPS_PER_REV / CAROUSEL_POSITIONS)

// Slow stepping is more reliable for 28BYJ-48 + ULN2003 under load.
// If the motor misses steps, increase these delays slightly.
#define STEP_DELAY_MS          4
#define STEP_RAMP_EXTRA_MS     3
#define STEP_RAMP_STEPS        32
#define STEPPER_HOLD_MS        120

// Servo configuration
#define GATE_CLOSED           0
#define GATE_OPEN             90
#define GATE_OPEN_DURATION    5000
#define AGITATOR_STOP         92
#define AGITATOR_SPIN         150
#define AGITATOR_DURATION     1500

// Alarm / test configuration
#define TEST_MODE         false
#define TEST_CYCLES       3
#define ALERT_INTERVAL    500
#define FADE_INTERVAL     10
#define MISSED_TIMEOUT    1800000

// LCD
#define LCD_COLS 20
#define LCD_ROWS 4
