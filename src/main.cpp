#include <Arduino.h>
#include "Type_Data.h"

TaskHandle_t Task_MQTT_Handle;

QueueHandle_t xQueueGateHouseSendNRF;
QueueHandle_t xQueueGateHouseSendTFT;
QueueHandle_t xQueueGateHouseSendMQTT;

QueueHandle_t xQueueRelaySendNRF; 
QueueHandle_t xQueueRelaySendTFT; 
QueueHandle_t xQueueRelaySendMQTT;  

QueueHandle_t xQueueAirSendMQTT;  
QueueHandle_t xQueueAirSendTFT;   
 
QueueSetHandle_t xQueueUnlockTaskMQTT;
QueueSetHandle_t xQueueUnlockTaskTFT;

void TaskNRF(void *pvParameter);
void TaskMQTT(void *pvParameter);
void TaskTFT(void *pvParameter);
void tryPreviousNetwork(void);

void setup(){
  Serial.begin(115200);
  
  xQueueRelaySendNRF      = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueGateHouseSendNRF  = xQueueCreate(1, sizeof(Data_GateHouse_Node_t));

  xQueueGateHouseSendTFT  = xQueueCreate(1, sizeof(Data_GateHouse_Node_t));
  xQueueGateHouseSendMQTT = xQueueCreate(1, sizeof(Data_GateHouse_Node_t));

  xQueueRelaySendTFT      = xQueueCreate(1, sizeof(Data_Relay_Room_t));
  xQueueAirSendTFT        = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));

  xQueueAirSendMQTT       = xQueueCreate(1, sizeof(Data_Air_Node_Room_t));
  xQueueRelaySendMQTT     = xQueueCreate(1, sizeof(Data_Relay_Room_t));

  xQueueUnlockTaskMQTT = xQueueCreateSet(3); // Queue Length Air + Queue Length Relay + Queue Length Gate House Send MQTT
  // Associate the semaphore and queue to the queue set handle
  xQueueAddToSet(xQueueAirSendMQTT, xQueueUnlockTaskMQTT);
  xQueueAddToSet(xQueueRelaySendMQTT, xQueueUnlockTaskMQTT);
  xQueueAddToSet(xQueueGateHouseSendMQTT, xQueueUnlockTaskMQTT);

  xQueueUnlockTaskTFT = xQueueCreateSet(3); // Queue Length Relay + Queue Length Gate House send NRF
  // Associate the semaphore and queue to the queue set handle
  xQueueAddToSet(xQueueAirSendTFT, xQueueUnlockTaskTFT);
  xQueueAddToSet(xQueueRelaySendTFT, xQueueUnlockTaskTFT);
  xQueueAddToSet(xQueueGateHouseSendTFT, xQueueUnlockTaskTFT);

  xTaskCreatePinnedToCore(TaskNRF, "Task_NRF", 4096, NULL, 2, NULL, 0);    // Core 0
  xTaskCreatePinnedToCore(TaskTFT, "Task_TFT", 4096, NULL, 2, NULL, 1);   // Core 1
  tryPreviousNetwork(); // Task Scan and Connect to WiFi
  xTaskCreatePinnedToCore(TaskMQTT, "Task_MQTT", 8192, NULL, 1, &Task_MQTT_Handle, 1);  // Core 0
}

void loop() {
}



