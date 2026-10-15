#include "storage.h"

void beginStorage() {
  prefs.begin("medidisp", false);
}

void loadSettings() {
  alarmHour   = prefs.getInt("alarmHour", alarmHour);
  alarmMinute = prefs.getInt("alarmMin",  alarmMinute);
  systemArmed = prefs.getBool("armed",    systemArmed);

  currentSlot   = prefs.getInt("curSlot", currentSlot);
  stepIndex     = prefs.getInt("stepIdx", stepIndex);
  dosesThisWeek = prefs.getInt("dosesWeek", dosesThisWeek);

  // Defensive cleanup if NVS contains old/corrupted values.
  if (currentSlot < 0 || currentSlot >= CAROUSEL_POSITIONS) currentSlot = 0;
  if (stepIndex < 0 || stepIndex >= 8) stepIndex = 0;
  if (dosesThisWeek < 0) dosesThisWeek = 0;
  if (dosesThisWeek > PILL_SLOTS) dosesThisWeek = PILL_SLOTS;

  for (int i = 0; i < CAROUSEL_POSITIONS; i++) {
    char key[10];
    snprintf(key, sizeof(key), "slotT%d", i);
    slotDoseType[i] = prefs.getInt(key, 0);
    if (slotDoseType[i] < 0 || slotDoseType[i] > 2) slotDoseType[i] = 0;
  }

  Serial.printf(">> Restored alarm %02d:%02d, armed=%d, carouselSlot=%d, stepIndex=%d, doses=%d/%d\n",
                alarmHour, alarmMinute, systemArmed, currentSlot, stepIndex, dosesThisWeek, PILL_SLOTS);
}

void saveAlarmSettings() {
  prefs.putInt("alarmHour", alarmHour);
  prefs.putInt("alarmMin",  alarmMinute);
}

void saveArmedState() {
  prefs.putBool("armed", systemArmed);
}

void saveCarouselState() {
  prefs.putInt("curSlot", currentSlot);
  prefs.putInt("stepIdx", stepIndex);
  prefs.putInt("dosesWeek", dosesThisWeek);
  for (int i = 0; i < CAROUSEL_POSITIONS; i++) {
    char key[10];
    snprintf(key, sizeof(key), "slotT%d", i);
    prefs.putInt(key, slotDoseType[i]);
  }
}

void resetCarouselState() {
  currentSlot = 0;
  stepIndex = 0;
  dosesThisWeek = 0;
  for (int i = 0; i < CAROUSEL_POSITIONS; i++) slotDoseType[i] = 0;
  saveCarouselState();
}
