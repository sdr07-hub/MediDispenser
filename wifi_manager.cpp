#include "wifi_manager.h"
#include "lcd.h"

bool connectWiFiBlocking() {
  lcdShow2("Connecting...", WIFI_SSID);
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("Open this in your browser: http://");
    Serial.println(WiFi.localIP());

    char ip[21];
    snprintf(ip, sizeof(ip), "%-20s", WiFi.localIP().toString().c_str());
    lcdShow2("** WiFi OK **", ip);
    delay(3000);
    return true;
  }

  Serial.println("\nWiFi failed — running without app");
  lcdShow2("!! No WiFi !!", "Offline Mode");
  delay(2000);
  return false;
}

void checkWiFiReconnect() {
  if (millis() - lastWifiCheckMs >= 5000) {
    lastWifiCheckMs = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(">> WiFi dropped, reconnecting...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }
}
