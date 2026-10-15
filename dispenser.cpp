#include "dispenser.h"
#include "utils.h"
#include "lcd.h"
#include "stepper.h"
#include "storage.h"

void stopAgitator() {
  if (agitatorRunning) {
    agitatorServo.write(AGITATOR_STOP);
    delay(200);
    agitatorServo.detach();
    agitatorRunning = false;
  }
}

void closeGate() {
  if (!gateClosed) {
    gateServo.write(GATE_CLOSED);
    gateClosed = true;
  }
}

void openGate() {
  if (gateClosed) {
    gateServo.write(GATE_OPEN);
    gateClosed   = false;
    gateOpenedMs = millis();
  }
}

void handleFade(unsigned long ms) {
  if (ms - lastFadeMillis >= FADE_INTERVAL) {
    lastFadeMillis = ms;
    fadeValue += fadeDirection;
    if (fadeValue >= 255) { fadeValue = 255; fadeDirection = -3; }
    else if (fadeValue <= 0) { fadeValue = 0; fadeDirection = 3; }
    ledWrite(fadeValue);
  }
}

void handleDispenseBuzzer(unsigned long ms) {
  if (ms - lastDispBuzzerMs >= dispPattern[dispPatternIndex]) {
    lastDispBuzzerMs = ms;
    dispPatternIndex = (dispPatternIndex + 1) % dispPatternLen;
    if (dispPatternIndex % 2 == 0) tone(PIN_BUZZER, 1200, dispPattern[dispPatternIndex]);
    else noTone(PIN_BUZZER);
  }
}

void triggerDispense(DateTime& t) {
  stopAlerts();
  lcdShow2("~ Dispensing ~", "Please wait...");

  int doseSlotForUi = dosesThisWeek;
  if (doseSlotForUi < 0) doseSlotForUi = 0;
  if (doseSlotForUi >= PILL_SLOTS) doseSlotForUi = PILL_SLOTS - 1;

  rotateOneSlot();

  // UI/history tracking: slots 0-6 represent Sun-Sat doses, slot 7 is the loading bay.
  slotDoseType[doseSlotForUi] = lastDispenseManual ? 2 : 1;   // 2=manual(cyan), 1=auto(green)

  if (dosesThisWeek < PILL_SLOTS) dosesThisWeek++;
  saveCarouselState();
  if (lastDispenseManual) lastManualDay = t.dayOfTheWeek();

  agitatorServo.attach(PIN_AGITATOR, 500, 2400);
  agitatorServo.write(AGITATOR_SPIN);
  agitatorRunning = true;
  agitatorStartMs = millis();

  openGate();

  dispPatternIndex = 0;
  lastDispBuzzerMs = millis();
  fadeValue        = 0;
  fadeDirection    = 3;
  lastFadeMillis   = millis();

  currentState    = DISPENSING;
  dispenseStartMs = millis();
}

// ==========================================
