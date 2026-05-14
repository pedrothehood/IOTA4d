#ifndef WIFI_INIT_H
#define WIFI_INIT_H
#include <Arduino.h> // Wichtig, um Arduino-Befehle wie digitalWrite zu nutzen

// init wifi
void wifiInit(const char* &ssid, const char* &password,RD03D &radar) {
  Serial.print("Wifi verbinden");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());
  Serial.print("Wifi verbunden, initialisiere Sensor...");

  if (radar.initialize(RD03D::RD03DMode::MULTI_TARGET)) {
    Serial.println("Sensor erfolgreich initialisiert!");
  } else {
    Serial.println("Initialisierung fehlgeschlagen! Check RX/TX & Power.");
    while (1) {
      Serial.print(".");
      delay(1000);
    }
  }
}
#endif