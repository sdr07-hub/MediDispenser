#include "history.h"

void addLog(const char* msg, int type) {
  if (doseHistoryCount < 20) {
    strncpy(doseHistory[doseHistoryCount].entry, msg, 49);
    doseHistory[doseHistoryCount].entry[49] = '\0';
    doseHistory[doseHistoryCount].type = type;
    doseHistoryCount++;
  } else {
    for (int i = 0; i < 19; i++) doseHistory[i] = doseHistory[i+1];
    strncpy(doseHistory[19].entry, msg, 49);
    doseHistory[19].entry[49] = '\0';
    doseHistory[19].type = type;
  }
}
