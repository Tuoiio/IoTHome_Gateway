#include <Arduino.h>
#include "Type_Data.h"

QueueHandle_t xQueueRelayReceiveMQTT;
QueueHandle_t xQueueTemperature;
QueueHandle_t xQueueRelaySendMQTT;
QueueSetHandle_t xQueueReceiveNRF;

void TaskNRF(void *pvParameter);
void TaskWiFi(void *pvParameter);

 
 
void setup(){
  Serial.begin(115200);
  xQueueRelayReceiveMQTT = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueTemperature      = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));
  xQueueRelaySendMQTT    = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueReceiveNRF = xQueueCreateSet(2); // Queue Length Temperature + Queue Length Relay Send MQTT

  // Associate the semaphore and queue to the queue set handle
  xQueueAddToSet(xQueueTemperature, xQueueReceiveNRF);
  xQueueAddToSet(xQueueRelaySendMQTT, xQueueReceiveNRF);

  xTaskCreatePinnedToCore(TaskWiFi, "Task_WiFi", 4096, NULL, 2, NULL,1);  // Core 1
  //xTaskCreatePinnedToCore(TaskMQTT, "Task_MQTT", 256, NULL, 1, NULL, 1);  // Core 1
  xTaskCreate(TaskNRF, "Task_NRF", 4096, NULL,2, NULL);    // Core 0

 //vTaskStartScheduler();
}

void loop() {
  delay(2000);
}



