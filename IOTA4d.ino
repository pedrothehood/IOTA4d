#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <RD03D.h>
#include <websiteRadar.h>
#include "buzzerTask.h"      // Lokale Header-Datei laden
#include "serverInit.h"      // Lokale Header-Datei laden
#include "sensorDataToWs.h"  // Lokale Header-Datei laden
#include "serverInit.h"      // Lokale Header-Datei laden
#include "wifiInit.h"        // Lokale Header-Datei laden


const char *ssid = "TP-Link_2.4GHz_0494CA";
const char *password = "pedrothehood007";

#define SENSOR_RX 16
#define SENSOR_TX 17
// Pins definieren
// Buzzer - Pin
#define BUZZER_PIN 18
// Globaler Status für den Buzzer
volatile float targetDistance = 800.0;  // füttern
volatile bool personDetected = false;   // füttern
volatile bool isMoving = false;         // füttern
unsigned long lastMoveTime = 0;
const unsigned long hysteresisDelay = 1000;

// RD03D radar(Serial1);
RD03D radar(SENSOR_RX, SENSOR_TX, 256000);  // RX, TX, Baudrate
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);
  delay(1000);
  //buzzerTaskSetup();
  wifiInit(ssid, password, radar);
  serverInit(server, ws);
  // static TargetData*  ptrTarget ; //= radar.getTarget();    // get pointer to first target ( SINGLE DETECTION )
  // static bool detected = false;
  //sensorDataToWs(ws,radar,personDetected,targetDistance,isMoving);
}
void loop() {

sensorDataToWs(ws,radar, personDetected, targetDistance,isMoving);
}
