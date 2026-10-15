#include "lcd.h"

void lcdRow(int row, const char* text) {
  char buf[LCD_COLS + 1];
  strncpy(buf, text, LCD_COLS);
  buf[LCD_COLS] = '\0';
  int start = 0;
  while (buf[start] == ' ') start++;
  int end = strlen(buf) - 1;
  while (end >= start && buf[end] == ' ') end--;
  int len = (end >= start) ? (end - start + 1) : 0;
  char trimmed[LCD_COLS + 1];
  if (len > 0) { memcpy(trimmed, buf + start, len); }
  trimmed[len] = '\0';

  int pad = (LCD_COLS - len) / 2;
  if (pad < 0) pad = 0;

  lcd.setCursor(0, row);
  for (int i = 0; i < pad; i++) lcd.print(' ');
  lcd.print(trimmed);
  for (int i = pad + len; i < LCD_COLS; i++) lcd.print(' ');
}

// Shows a single line centered both horizontally AND vertically on the
// 20x4 screen (lands on row 1, the visual middle of a 4-row display).
void lcdCenterSingle(const char* text) {
  lcdRow(0, "");
  lcdRow(1, text);
  lcdRow(2, "");
  lcdRow(3, "");
}

// Shows two lines centered as a vertically-centered pair (rows 1 & 2),
// each line also horizontally centered. Used for all normal two-line
// status messages so nothing ever sits jammed in a corner.
void lcdShow2(const char* line0, const char* line1) {
  lcdRow(0, "");
  lcdRow(1, line0);
  lcdRow(2, line1);
  lcdRow(3, "");
}

// Backward-compatible alias kept for any single-row updates (e.g. ticking clock).
void lcdLine(int row, const char* text) {
  lcdRow(row, text);
}
