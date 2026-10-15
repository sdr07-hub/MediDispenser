/*
  ============================================================
  MediDispenser — ESP32 Final Code with Built-in Web UI
  Refactored into modules from MediDispenser_StepperFix.ino
  ============================================================
*/

#include "config.h"
#include "globals.h"
#include "utils.h"
#include "lcd.h"
#include "app_webserver.h"
#include "wifi_manager.h"
#include "storage.h"
#include "alarm.h"
#include "history.h"

void setup() {
  Serial.begin(115200);

  pinMode(PIN_IR, INPUT);  // IR module OUT/DO -> GPIO32, active LOW
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT); pinMode(PIN_IN4, OUTPUT);
  digitalWrite(PIN_IN1,0); digitalWrite(PIN_IN2,0);
  digitalWrite(PIN_IN3,0); digitalWrite(PIN_IN4,0);

  ledcAttach(PIN_LED, LED_FREQ, LED_RES);
  ledWrite(0);
  noTone(PIN_BUZZER);

  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  gateServo.setPeriodHertz(50);
  agitatorServo.setPeriodHertz(50);
  gateServo.attach(PIN_GATE, 500, 2400);
  gateServo.write(GATE_CLOSED);
  agitatorServo.attach(PIN_AGITATOR, 500, 2400);
  agitatorServo.write(AGITATOR_STOP);
  delay(500);
  agitatorServo.detach();

  Wire.begin(21, 23);
  lcd.init();
  lcd.backlight();

  beginStorage();
  loadSettings();

  if (!rtc.begin()) {
    lcdShow2("!! RTC ERROR !!", "Check Wiring");
    while (1);
  }

  // Uncomment once to set time, then comment out
  // rtc.adjust(DateTime(2026, 6, 29, 8, 0, 0));

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  connectWiFiBlocking();
  setupWebServer();

  Serial.println("Web server started");
  addLog("SYSTEM BOOT - Carousel at slot 0", 0);

  DateTime now = rtc.now();
  lastTriggeredMinute = now.minute();

  lcdShow2("~ MedDispenser ~", TEST_MODE ? "** TEST MODE **" : "Ready");
  ledWrite(255);
  tone(PIN_BUZZER, 1500, 200);
  delay(300);
  ledWrite(0);
  noTone(PIN_BUZZER);
  delay(1200);
  lcd.clear();

  Serial.println("=============================");
  Serial.println("  MEDIDISPENSER READY       ");
  Serial.println("=============================");
}

void loop() {
  server.handleClient();
  checkWiFiReconnect();

  static DateTime now = rtc.now();
  if (millis() - lastRtcRead >= 500) {
    now = rtc.now();
    lastRtcRead = millis();
  }

  switch (currentState) {
    case STANDBY:       handleStandby(now);      break;
    case ALARM_ACTIVE:  handleAlarmActive(now);  break;
    case DISPENSING:    handleDispensing(now);   break;
    case DONE:          handleDone();            break;
    case TEST_COMPLETE: handleTestComplete();    break;
  }

  delay(10);
}
