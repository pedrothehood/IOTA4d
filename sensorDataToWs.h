#ifndef SENSOR_DATA_TO_WS_H
#define SENSOR_DATA_TO_WS_H
#include <Arduino.h> // Wichtig, um Arduino-Befehle wie digitalWrite zu nutzen

// Sensor-Daten an Websocket senden
void sensorDataToWs(AsyncWebSocket &ws,RD03D &radar, volatile bool &personDetected, volatile float &targetDistance,volatile bool &isMoving) {
  //Serial.print("Sensor-Daten an Websocket senden");
  ws.cleanupClients();
  radar.tasks();
  //static uint32_t lastMsg = 0;
  unsigned long now = millis();
  static unsigned long lastWSUpdate = 0;
  if (now - lastWSUpdate > 100) {
    lastWSUpdate = now;
    if (ws.count() > 0 && ws.availableForWriteAll()) {
      bool found = false;
      // Prüfe ob ein Ziel erkannt wurde, bevor du sendest
      if (RD03D::MAX_TARGETS > 0) {
        String json = "{\"targets\":[";
        bool first = true;
        for (int i = 0; i < RD03D::MAX_TARGETS; i++) {
          TargetData *t = radar.getTarget(i);
          //Serial.println("After TargetData");
          if (t && t->isValid()) {
            if (i == 0) {
              personDetected = true;
              float rawDist  = t->distance / 10;  // in cm ???
              targetDistance = (targetDistance * 0.8) + (rawDist * 0.2); 
                                                 // Nutze die Geschwindigkeit (speed) als Indikator für Bewegung
                               isMoving = (abs(t->speed) > 0.1);
            }
            if (!first) json += ",";
            json += "{\"x\":" + String(t->x)
                    + ",\"y\":" + String(t->y)
                    + ",\"s\":" + String(t->speed)
                    + ",\"id\":" + String(i) + "}";
            first = false;
          } else { 
            personDetected = false;
            isMoving = false;
          }
        }
        json += "]}";
        if (!first) {
          ws.textAll(json);
        }
      }
    }
    // delay(500);  // Dem System Zeit geben, den Stack zu stabilisieren TTTTTTTTTTTTTTT
  }
}
#endif