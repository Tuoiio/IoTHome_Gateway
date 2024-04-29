#include <Arduino.h>
#include "Type_Data.h"

TaskHandle_t Task_MQTT_Handle;

QueueHandle_t xQueueRelaySendNRF; 
QueueHandle_t xQueueRelaySendTFT; //x
QueueHandle_t xQueueAirSendMQTT;  //x
QueueHandle_t xQueueAirSendTFT;   //x
QueueHandle_t xQueueRelaySendMQTT;  //x
QueueSetHandle_t xQueueReceiveNRF;  //x

void TaskNRF(void *pvParameter);
void TaskMQTT(void *pvParameter);
void TaskTFT(void *pvParameter);
void tryPreviousNetwork(void);

void setup(){
  Serial.begin(115200);
  
  xQueueRelaySendNRF     = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueRelaySendTFT     = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueAirSendTFT       = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));
  xQueueAirSendMQTT      = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));
  xQueueRelaySendMQTT    = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueReceiveNRF = xQueueCreateSet(2); // Queue Length Air + Queue Length Relay Send MQTT
  // Associate the semaphore and queue to the queue set handle
  xQueueAddToSet(xQueueAirSendMQTT, xQueueReceiveNRF);
  xQueueAddToSet(xQueueRelaySendMQTT, xQueueReceiveNRF);

  xTaskCreatePinnedToCore(TaskNRF, "Task_NRF", 4096, NULL, 2, NULL, 0);    // Core 0
  xTaskCreatePinnedToCore(TaskTFT, "Task_TFT", 10000, NULL, 2, NULL, 1);   // Core 1
  tryPreviousNetwork(); // Task Scan and Connect to WiFi
  xTaskCreatePinnedToCore(TaskMQTT, "Task_MQTT", 10000, NULL, 1, &Task_MQTT_Handle, 1);  // Core 0
}

void loop() {
}



