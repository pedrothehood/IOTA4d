#ifndef SERVER_INIT_H
#define SERVER_INIT_H
#include <Arduino.h> // Wichtig, um Arduino-Befehle wie digitalWrite zu nutzen

// Init Server
void serverInit(AsyncWebServer &server, AsyncWebSocket &ws) {
  Serial.print("Server-Handling/Add Handler/Server.on/server.begin");
  delay(500);  // Dem System Zeit geben, den Stack zu stabilisieren
  //Serial.println("Add handler");
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  server.begin();
}
#endif