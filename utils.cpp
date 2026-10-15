#include "utils.h"

void ledWrite(int val) {
  ledcWrite(PIN_LED, val);
}

void stopAlerts() {
  noTone(PIN_BUZZER);
  ledWrite(0);
  ledState         = false;
  fadeValue        = 0;
  dispPatternIndex = 0;
}
