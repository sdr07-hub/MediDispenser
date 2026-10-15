#pragma once
#include "globals.h"
void stopAgitator();
void closeGate();
void openGate();
void handleFade(unsigned long ms);
void handleDispenseBuzzer(unsigned long ms);
void triggerDispense(DateTime& t);
