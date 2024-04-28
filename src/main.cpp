#include <Arduino.h>
#include "Type_Data.h"

TaskHandle_t Task_MQTT_Handle;

QueueHandle_t xQueueRelayReceiveMQTT;
QueueHandle_t xQueueTemperature;
QueueHandle_t xQueueRelaySendMQTT;
QueueSetHandle_t xQueueReceiveNRF;

void TaskNRF(void *pvParameter);
void TaskMQTT(void *pvParameter);
void TaskTFT(void *pvParameter);
void tryPreviousNetwork(void);

void setup(){
  Serial.begin(115200);
  
  xQueueRelayReceiveMQTT = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueTemperature      = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));
  xQueueRelaySendMQTT    = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueReceiveNRF = xQueueCreateSet(2); // Queue Length Temperature + Queue Length Relay Send MQTT

  // Associate the semaphore and queue to the queue set handle
  xQueueAddToSet(xQueueTemperature, xQueueReceiveNRF);
  xQueueAddToSet(xQueueRelaySendMQTT, xQueueReceiveNRF);
  xTaskCreatePinnedToCore(TaskTFT, "Task_TFT", 8192, NULL, 1, NULL, 1);   // Core 1
  tryPreviousNetwork(); // Task Scan and Connect to WiFi
  xTaskCreatePinnedToCore(TaskMQTT, "Task_MQTT", 8192, NULL, 1, &Task_MQTT_Handle, 1);  // Core 0
  xTaskCreatePinnedToCore(TaskNRF, "Task_NRF", 4096, NULL,1, NULL, 1);    // Core 0
 //vTaskStartScheduler();
}

void loop() {
}



