#include "stepper.h"
#include "storage.h"

static void applyStepperPhase() {
  digitalWrite(PIN_IN1, stepSeq[stepIndex][0]);
  digitalWrite(PIN_IN2, stepSeq[stepIndex][1]);
  digitalWrite(PIN_IN3, stepSeq[stepIndex][2]);
  digitalWrite(PIN_IN4, stepSeq[stepIndex][3]);
}

void releaseStepper() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void rotateOneSlot() {
  Serial.printf(">> Stepper: moving from slot %d by %d half-steps\n", currentSlot, STEPS_PER_SLOT);

  for (int i = 0; i < STEPS_PER_SLOT; i++) {
    applyStepperPhase();
    stepIndex = (stepIndex + 1) % 8;

    int rampDelay = STEP_DELAY_MS;
    if (i < STEP_RAMP_STEPS) {
      rampDelay += STEP_RAMP_EXTRA_MS;
    } else if (i >= STEPS_PER_SLOT - STEP_RAMP_STEPS) {
      rampDelay += STEP_RAMP_EXTRA_MS;
    }
    delay(rampDelay);
  }

  // Hold briefly at the target phase so the loaded carousel settles before release.
  applyStepperPhase();
  delay(STEPPER_HOLD_MS);
  releaseStepper();

  currentSlot = (currentSlot + 1) % CAROUSEL_POSITIONS;
  saveCarouselState();

  Serial.printf(">> Stepper: target reached -> slot %d, stepIndex=%d\n", currentSlot, stepIndex);
}
