#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Type_Data.h"


WiFiClientSecure espClient;
PubSubClient client(espClient);
extern QueueHandle_t xQueueGateHouseSendNRF;
extern QueueHandle_t xQueueGateHouseSendMQTT;
extern QueueHandle_t xQueueRelaySendNRF;
extern QueueHandle_t xQueueAirSendMQTT;
extern QueueHandle_t xQueueRelaySendMQTT;
extern QueueSetHandle_t xQueueUnlockTaskMQTT;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
static Data_Relay_Room_t Data_Relay_Reci_MQTT;
static Data_GateHouse_Node_t Data_GateHouse_Up_MQTT;


void MQTT_Setup(void);
void MQTT_Reconnect(void);
void MQTT_callback(char *topic, byte *payload, unsigned int lenth);
void MQTT_PublishMessage(const char *topic, String payload, boolean retained);


extern void TaskMQTT(void *pvParameter){
  vTaskSuspend(NULL);
  MQTT_Setup();

  for(;;){
    /*********************************************Reconnect to MQTT***************************************************************/
    if(!client.connected()){
      static TickType_t Time_Reconnect = xTaskGetTickCount();
      if ((TickType_t)xTaskGetTickCount() - Time_Reconnect > 5000){
        MQTT_Reconnect();
        Time_Reconnect = xTaskGetTickCount();
      }
    } else {
      client.loop();
    }
    
    /************************************************Publish data to coulud******************************************************/
    QueueSetMemberHandle_t Who_Unblocked = xQueueSelectFromSet(xQueueUnlockTaskMQTT, (TickType_t)10);
    if(Who_Unblocked == xQueueAirSendMQTT)
    {
      static Data_Air_Node_Room_t Data_Air_UpMQTT;
      if(xQueueReceive(xQueueAirSendMQTT, &Data_Air_UpMQTT, (TickType_t)0) == pdPASS) {
        DynamicJsonDocument doc(128);
        char mqtt_message[128];
        if (Data_Air_UpMQTT.temperature == -88.0f) {
          doc["teperature"] = "Sensor Error";
          doc["humidity"] = "Sensor Error";
          doc["Co2"] = "Sensor Error";
        } else if (Data_Air_UpMQTT.temperature == -99.0f) {
          doc["temperature"] = "Disconnect";
          doc["humidity"] = "Disconnect";
          doc["Co2"] = "Disconnect";
        } else {
          doc["temperature"] = (float)Data_Air_UpMQTT.temperature;
          doc["humidity"] = (float)Data_Air_UpMQTT.humidity;
          doc["Co2"] = (float)Data_Air_UpMQTT.CO2;
          //Serial.printf("Temperature %.1f\n", Data_Air_UpMQTT.temperature);
          //Serial.printf("Humidity %.1f\n", Data_Air_UpMQTT.humidity);
          //Serial.printf("Co2 %.1f\n", Data_Air_UpMQTT.CO2);
        }
        serializeJson(doc, mqtt_message);
        MQTT_PublishMessage("ESP32/Air", mqtt_message, true);
      }
      
    } else if(Who_Unblocked == xQueueRelaySendMQTT) {
      static Data_Relay_Room_t Data_Relay_Up_MQTT;
      if(xQueueReceive(xQueueRelaySendMQTT, &Data_Relay_Up_MQTT, (TickType_t)0) == pdPASS) {
        DynamicJsonDocument doc(128);
        char mqtt_message[50];
        doc["Relay1"] = Data_Relay_Up_MQTT.Relay1;
        doc["Relay2"] = Data_Relay_Up_MQTT.Relay2;
        serializeJson(doc, mqtt_message);
        MQTT_PublishMessage("ESP32/Relay", mqtt_message, true);
      }
    
    } else if (Who_Unblocked == xQueueGateHouseSendMQTT) {
      if(xQueueReceive(xQueueGateHouseSendMQTT, &Data_GateHouse_Up_MQTT, (TickType_t)0) == pdPASS) {
        DynamicJsonDocument doc(128);
        char mqtt_message[128];
        if(Data_GateHouse_Up_MQTT.Status == 1 && Data_GateHouse_Up_MQTT.count  > 0 && Data_GateHouse_Up_MQTT.count < 20) {
          doc["GateStatus"] = 1; //"Opening Gate";
        
        } else if (Data_GateHouse_Up_MQTT.Status == 2 || Data_GateHouse_Up_MQTT.Status == 4) {
          if(Data_GateHouse_Up_MQTT.count == 0 && Data_GateHouse_Up_MQTT.Status == 4) {
            doc["GateStatus"] = 2;//"Closed";
                
          } else if (Data_GateHouse_Up_MQTT.Status == 2 && Data_GateHouse_Up_MQTT.count == 20) {
            doc["GateStatus"] = 3;//"Opened";
                
          } else {
            doc["GateStatus"] = 4;//"Pause";
          }
        } else if (Data_GateHouse_Up_MQTT.Status == 3) {
          if(Data_GateHouse_Up_MQTT.count > 0 && Data_GateHouse_Up_MQTT.count < 20) {
            doc["GateStatus"] = 5;//"Closing Gate";
          }
        }
        serializeJson(doc, mqtt_message);
        MQTT_PublishMessage("ESP32/GateHouse", mqtt_message, true);
      }
    }
  }
}


void MQTT_Setup(void){
  const char *mqtt_server = "971474d396034f83b2f2e2d608193178.s1.eu.hivemq.cloud";
  static int mqtt_port = 8883;
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(MQTT_callback);
}

//-------------------------------- Connect to MQTT Broker -------------------------------//
void MQTT_Reconnect(void){
  const char *mqtt_username = "DuongTuoi";
  const char *mqtt_password = "Automation!2";

  if(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    String clientID = "ESPClient-";
    clientID += String(random(0xffff), HEX);
    if(client.connect(clientID.c_str(), mqtt_username, mqtt_password)){
      Serial.println("connected");
      client.subscribe("ESP32/client");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
    }
  }
}

//--------------------------------- Call back Method for Receiving MQTT message -------------------//
void MQTT_callback(char *topic, byte *payload, unsigned int length){
  String incomingMessage = "";
  for(int i = 0; i < length; i++) incomingMessage += (char)payload[i];
  Serial.println("Message arived [" + String(topic) + "]" + incomingMessage);

  DynamicJsonDocument doc(128);
  DeserializationError error = deserializeJson(doc, incomingMessage);
  if(error){
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  JsonObject obj = doc.as<JsonObject>();
  if(obj.containsKey("Relay1")){
    boolean p = obj["Relay1"];
    Data_Relay_Reci_MQTT.Relay1 = p;
    xQueueSend(xQueueRelaySendNRF, &Data_Relay_Reci_MQTT, (TickType_t)0);
  }
  if(obj.containsKey("Relay2")){
    boolean p = obj["Relay2"];
    Data_Relay_Reci_MQTT.Relay2 = p;
    xQueueSend(xQueueRelaySendNRF, &Data_Relay_Reci_MQTT, (TickType_t)0);
  }
  if(obj.containsKey("GateHouse")) {
    static Data_GateHouse_Node_t Data_GateHouse_Reci_MQTT_And_Tran_NRF;
    
    Data_GateHouse_Reci_MQTT_And_Tran_NRF  = Data_GateHouse_Up_MQTT; // Update data synchronization
    unsigned char status_click = obj["GateHouse"];

    if (status_click == 1)   // if variable status_click = 1 that mean: click open gate
   {
      if (Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status != 2 || Data_GateHouse_Reci_MQTT_And_Tran_NRF.count != 20) 
      {
        Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status = 1;
        xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Reci_MQTT_And_Tran_NRF, (TickType_t)0);
      }
    
    } else if (status_click == 2) // if variable status_click = 2, that mean: click pause gate
    { 
      if(Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status == 1) 
      {
        Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status = 2;
        xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Reci_MQTT_And_Tran_NRF, (TickType_t)0);

      } else if(Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status == 3) 
      {
        Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status = 4;
        xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Reci_MQTT_And_Tran_NRF, (TickType_t)0);
      }
    } else if (status_click == 3) // if variable status_click = 3,, that mean: click pause stop
    { 
      if(Data_GateHouse_Reci_MQTT_And_Tran_NRF.count != 0 || Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status != 4) 
      {
        Data_GateHouse_Reci_MQTT_And_Tran_NRF.Status = 3;
        xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Reci_MQTT_And_Tran_NRF, (TickType_t)0);
      }
    }
  }
}

//-----Method for Publishing MQTT Messages---------
 void MQTT_PublishMessage(const char* topic, String payload, boolean retained){
  if(client.publish(topic,payload.c_str(),true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}
