#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "config.h"

struct DoseLog {
  char entry[50];
  int  type;  // 0=system, 1=taken, 2=missed, 3=manual
};

enum State { STANDBY, ALARM_ACTIVE, DISPENSING, DONE, TEST_COMPLETE };

extern LiquidCrystal_I2C lcd;
extern RTC_DS1307 rtc;
extern Servo gateServo;
extern Servo agitatorServo;
extern WebServer server;
extern Preferences prefs;

extern const bool stepSeq[8][4];
extern int stepIndex;
extern int currentSlot;
extern int slotDoseType[8];

extern int alarmHour;
extern int alarmMinute;
extern int testCycleCount;
extern int lastTriggeredMinute;
extern int lastDispensedDay;
extern int lastManualDay;
extern bool debugMode;
extern int dosesThisWeek;

extern const char* dayNames[7];
extern DoseLog doseHistory[20];
extern int doseHistoryCount;

extern State currentState;
extern bool ledState;
extern bool irArmed;
extern bool agitatorRunning;
extern bool gateClosed;
extern bool systemArmed;
extern unsigned long lastAlertMillis;
extern unsigned long lastFadeMillis;
extern unsigned long lastDispBuzzerMs;
extern unsigned long dispenseStartMs;
extern unsigned long doneStartMs;
extern unsigned long lastRtcRead;
extern unsigned long lastWifiCheckMs;
extern unsigned long gateOpenedMs;
extern unsigned long agitatorStartMs;
extern unsigned long alarmStartMs;
extern int fadeValue;
extern int fadeDirection;
extern const int dispPattern[];
extern const int dispPatternLen;
extern int dispPatternIndex;
extern bool missedSent;
extern bool lastDispenseManual;
extern unsigned long tempMsgUntil;
extern String tempMsgText;
