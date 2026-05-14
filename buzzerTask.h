#ifndef BUZZER_TASK_H
#define BUZZER_TASK_H
#include <Arduino.h> // Wichtig, um Arduino-Befehle wie digitalWrite zu nutzen



// Task-Funktion für den Buzzer (läuft auf einem eigenen Kern/Thread)
void buzzerTask(void *pvParameters) {
  // Setup: Pin direkt mit PWM-Fähigkeit verbinden
  Serial.println("BuzzerTaskon");
 ////// ledcAttach(BUZZER_PIN, 4000, 8); 

 for (;;) {
    unsigned long currentTime = millis();
    //if (personDetected && isMoving){
     /* if (targetDistance < 100)  {
        ledcWriteTone(18, 4000); // 4kHz Ton an
        delay(100);
         ledcWriteTone(18, 0);    // Ton aus
         delay(100);
       }else if (targetDistance < 200 ){
         ledcWriteTone(18, 2000); // 4kHz Ton an
        delay(200);
         ledcWriteTone(18, 0);    // Ton aus
         delay(200);
       }else{
            ledcWriteTone(18, 1000); // 4kHz Ton an
        delay(400);
         ledcWriteTone(18, 0);    // Ton aus
         delay(400);
       } */

    //}
   ////// if (personDetected && isMoving) lastMoveTime = currentTime;

   /* if (currentTime - lastMoveTime < hysteresisDelay) {
      //int freq = map(targetDistance, 0, 800, 5000, 2000);
      //int pause = map(targetDistance, 0, 800, 50, 500);

     // ledcWriteTone(BUZZER_PIN, freq); // Frequenz auf Pin
     // vTaskDelay(60 / portTICK_PERIOD_MS);
     // ledcWriteTone(BUZZER_PIN, 0);    // Ton aus
     // vTaskDelay(constrain(pause, 50, 500) / portTICK_PERIOD_MS);
    } else {
     // ledcWriteTone(BUZZER_PIN, 0);
      //vTaskDelay(100 / portTICK_PERIOD_MS);
    } */
  }
}

// Buzzer Task setup

void buzzerTaskSetup() {
  //Serial.print("Buzzer Task Setup") ;
  // Erstelle den Buzzer-Thread (Task)
 xTaskCreatePinnedToCore(
    buzzerTask,    "BuzzerTask", 
    2048,          NULL, 
    1,             NULL, 
    0              // Läuft auf Kern 0
  );
}
#endif
