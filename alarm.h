#pragma once
#include "globals.h"
void handleAlert(unsigned long ms);
void handleStandby(DateTime& t);
void handleAlarmActive(DateTime& t);
void handleDispensing(DateTime& t);
void handleDone();
void handleTestComplete();
