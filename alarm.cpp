#include "alarm.h"
#include "utils.h"
#include "lcd.h"
#include "dispenser.h"
#include "history.h"

void handleAlert(unsigned long ms) {
  if (ms - lastAlertMillis >= ALERT_INTERVAL) {
    lastAlertMillis = ms;
    ledState = !ledState;
    if (ledState) { ledWrite(255); tone(PIN_BUZZER, 1000, 80); }
    else           { ledWrite(0);  noTone(PIN_BUZZER); }
  }
}

void handleStandby(DateTime& t) {
  stopAlerts();

  if (!systemArmed) {
    lcdCenterSingle("SYSTEM OFFLINE");
    return;   // skip clock/alarm display + alarm checks while disarmed
  }

  if (millis() < tempMsgUntil) {
    lcdCenterSingle(tempMsgText.c_str());
    return;   // briefly show confirmation (e.g. "SYSTEM ONLINE") before resuming clock
  }

  char line0[17];
  snprintf(line0, sizeof(line0), "%s %02d:%02d:%02d",
           dayNames[t.dayOfTheWeek()], t.hour(), t.minute(), t.second());

  char line1[17];
  if (TEST_MODE)
    snprintf(line1, sizeof(line1), "Test %d/%d ->:%02d",
             testCycleCount, TEST_CYCLES, (t.minute()+1)%60);
  else
    snprintf(line1, sizeof(line1), "Next Dose %02d:%02d", alarmHour, alarmMinute);

  lcdShow2(line0, line1);

  bool timeMatch;
  if (TEST_MODE) {
    timeMatch = (t.minute() != lastTriggeredMinute) &&
                (t.second() < 5) &&
                (testCycleCount < TEST_CYCLES);
  } else {
    timeMatch = (t.hour()   == alarmHour    &&
                 t.minute() == alarmMinute  &&
                 t.second() < 8            &&
                 (debugMode || lastDispensedDay != t.dayOfTheWeek()));
  }

  if (timeMatch && systemArmed) {
    if (!debugMode && dosesThisWeek >= PILL_SLOTS) {
      addLog("Dose blocked - refill carousel", 0);
      tempMsgText = "REFILL CAROUSEL";
      tempMsgUntil = millis() + 3000;
      Serial.println(">> ALARM BLOCKED: carousel empty/loading bay");
      return;
    }

    if (TEST_MODE) {
      lastTriggeredMinute = t.minute();
      testCycleCount++;
    }
    currentState    = ALARM_ACTIVE;
    lastAlertMillis = millis();
    alarmStartMs    = millis();
    missedSent      = false;
    irArmed         = false;
    ledState        = false;
    lcd.clear();

    char logMsg[40];
    snprintf(logMsg, sizeof(logMsg), "Alarm fired - %02d:%02d", t.hour(), t.minute());
    addLog(logMsg, 0);
    Serial.println(">> STATE: ALARM_ACTIVE");
  }

  if (TEST_MODE && testCycleCount >= TEST_CYCLES && currentState == STANDBY) {
    currentState = TEST_COMPLETE;
    lcd.clear();
  }
}

void handleAlarmActive(DateTime& t) {
  unsigned long ms = millis();

  char line0[17];
  if (TEST_MODE)
    snprintf(line0, sizeof(line0), "ALARM! %d/%d", testCycleCount, TEST_CYCLES);
  else
    strcpy(line0, "* TAKE MEDS! *");
  lcdShow2(line0, ">> Place Hand <<");

  handleAlert(ms);

  // Missed dose after 30 mins
  if (!missedSent && ms - alarmStartMs >= MISSED_TIMEOUT) {
    missedSent = true;
    char logMsg[40];
    snprintf(logMsg, sizeof(logMsg), "%s - Dose MISSED", dayNames[t.dayOfTheWeek()]);
    addLog(logMsg, 2);
    Serial.println(">> MISSED DOSE logged");
  }

  if (!irArmed) {
    if (digitalRead(PIN_IR) == IR_IDLE_STATE) { irArmed = true; }
    return;
  }

  if (digitalRead(PIN_IR) == IR_DETECTED_STATE) {
    Serial.println(">> HAND DETECTED");
    lastDispenseManual = false;
    triggerDispense(t);
  }
}

void handleDispensing(DateTime& t) {
  unsigned long ms = millis();

  handleFade(ms);
  handleDispenseBuzzer(ms);

  if (agitatorRunning && ms - agitatorStartMs >= AGITATOR_DURATION) stopAgitator();
  if (ms - gateOpenedMs >= GATE_OPEN_DURATION) closeGate();

  if (ms - dispenseStartMs >= 6000) {
    stopAlerts();
    noTone(PIN_BUZZER);
    stopAgitator();
    closeGate();

    // Only an AUTOMATIC (scheduled) dose should mark today's alarm as satisfied.
    // A manual app dose is an extra/bonus dose and must NOT block today's scheduled alarm.
    if (!lastDispenseManual) {
      lastDispensedDay = t.dayOfTheWeek();
    }

    char logMsg[50];
    if (lastDispenseManual) {
      snprintf(logMsg, sizeof(logMsg), "%s %02d:%02d - Manual dose (app)",
               dayNames[t.dayOfTheWeek()], t.hour(), t.minute());
      addLog(logMsg, 3);   // type 3 = manual
    } else {
      snprintf(logMsg, sizeof(logMsg), "%s %02d:%02d - Scheduled dose taken",
               dayNames[t.dayOfTheWeek()], t.hour(), t.minute());
      addLog(logMsg, 1);   // type 1 = automatic/scheduled
    }

    if (TEST_MODE) {
      char line0[17];
      snprintf(line0, sizeof(line0), "Done! %d/%d", testCycleCount, TEST_CYCLES);
      lcdShow2(line0, "Stay healthy :)");
    } else {
      lcdShow2("~ Dose Ready ~", "Stay healthy :)");
    }

    currentState = DONE;
    doneStartMs  = millis();
    Serial.println(">> STATE: DONE");
  }
}

void handleDone() {
  if (millis() - doneStartMs >= 2000) {
    lcd.clear();
    currentState = STANDBY;
    Serial.println(">> STATE: STANDBY");
  }
}

void handleTestComplete() {
  lcdShow2("** ALL DONE **", "Cycles OK");
  static bool beeped = false;
  if (!beeped) {
    for (int i = 0; i < 3; i++) {
      ledWrite(255); tone(PIN_BUZZER, 1800, 100); delay(150);
      ledWrite(0);   noTone(PIN_BUZZER);           delay(100);
    }
    beeped = true;
    Serial.println("ALL TESTS PASSED!");
  }
}

// ==========================================
// LOOP
