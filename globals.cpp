#include "globals.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;
Servo gateServo;
Servo agitatorServo;
WebServer server(80);
Preferences prefs;

const bool stepSeq[8][4] = {
  {1,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,1,0},
  {0,0,1,0},{0,0,1,1},{0,0,0,1},{1,0,0,1}
};
int stepIndex = 0;
int currentSlot = 0;
int slotDoseType[8] = {0,0,0,0,0,0,0,0};

int alarmHour   = 8;
int alarmMinute = 0;
int testCycleCount      = 0;
int lastTriggeredMinute = -1;
int lastDispensedDay    = -1;
int lastManualDay       = -1;
bool debugMode          = false;
int dosesThisWeek       = 0;

const char* dayNames[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
DoseLog doseHistory[20];
int doseHistoryCount = 0;

State currentState = STANDBY;
bool ledState        = false;
bool irArmed         = false;
bool agitatorRunning = false;
bool gateClosed      = true;
bool systemArmed     = true;

unsigned long lastAlertMillis  = 0;
unsigned long lastFadeMillis   = 0;
unsigned long lastDispBuzzerMs = 0;
unsigned long dispenseStartMs  = 0;
unsigned long doneStartMs      = 0;
unsigned long lastRtcRead      = 0;
unsigned long lastWifiCheckMs  = 0;
unsigned long gateOpenedMs     = 0;
unsigned long agitatorStartMs  = 0;
unsigned long alarmStartMs     = 0;

int fadeValue     = 0;
int fadeDirection = 3;
const int dispPattern[]  = {100, 200, 100, 800};
const int dispPatternLen = 4;
int dispPatternIndex     = 0;

bool missedSent = false;
bool lastDispenseManual = false;
unsigned long tempMsgUntil = 0;
String tempMsgText = "";
