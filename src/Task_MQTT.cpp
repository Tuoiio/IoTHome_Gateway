#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Type_Data.h"


WiFiClientSecure espClient;
PubSubClient client(espClient);

extern QueueHandle_t xQueueRelaySendNRF;
extern QueueHandle_t xQueueAirSendMQTT;
extern QueueHandle_t xQueueRelaySendMQTT;
extern QueueSetHandle_t xQueueReceiveNRF;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
static Data_Relay_Room_t Data_Relay_Reci_MQTT;

// Sau khi mat dien tu dong gui lai thong tin
boolean updateState = 0;


void setup_mqtt(void);

void MQTT_Reconnect(void);
void MQTT_callback(char *topic, byte *payload, unsigned int lenth);
void MQTT_PublishMessage(const char *topic, String payload, boolean retained);


extern void TaskMQTT(void *pvParameter){
  vTaskSuspend(NULL);
  setup_mqtt();

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
    QueueSetMemberHandle_t Who_Unblocked = xQueueSelectFromSet(xQueueReceiveNRF, (TickType_t)10);
    if(Who_Unblocked == xQueueAirSendMQTT)
    {
      static Data_Air_Node_Room_t Data_Air_UpMQTT;
      if(xQueueReceive(xQueueAirSendMQTT, &Data_Air_UpMQTT, (TickType_t)0) == pdPASS)
      {
        DynamicJsonDocument doc(128);
        char mqtt_message[128];
        if (Data_Air_UpMQTT.temperature == -88.0f) {
          doc["teperature"] = "Sensor Error";
          doc["humidity"] = "Sensor Error";
        } else if (Data_Air_UpMQTT.temperature == -99.0f) {
          doc["temperature"] = "Disconect";
          doc["humidity"] = "Disconect";
        } else {
          doc["temperature"] = (float)Data_Air_UpMQTT.temperature;
          doc["humidity"] = (float)Data_Air_UpMQTT.humidity;
        }
        serializeJson(doc, mqtt_message);
        MQTT_PublishMessage("ESP32/Air", mqtt_message, true);
      }
      
    }
    else if(Who_Unblocked == xQueueRelaySendMQTT)
    {
      static Data_Relay_Room_t Data_Relay_Up_MQTT;
      if(xQueueReceive(xQueueRelaySendMQTT, &Data_Relay_Up_MQTT, (TickType_t)0) == pdPASS)
      {
        DynamicJsonDocument doc(128);
        doc["Relay1"] = Data_Relay_Up_MQTT.Relay1;
        doc["Relay2"] = Data_Relay_Up_MQTT.Relay2;
        char mqtt_message[50];
        serializeJson(doc, mqtt_message);
        MQTT_PublishMessage("ESP32/Relay", mqtt_message, true);
      }
    }
    
    /*************************************Receive From MQTT then Update Relay************************************************/
    if(updateState == 1) {
      Serial.printf("Relay 1: %d\n", Data_Relay_Reci_MQTT.Relay1);
      Serial.printf("Relay 2: %d\n", Data_Relay_Reci_MQTT.Relay2);

      DynamicJsonDocument doc(128);
      doc["Relay1"] = Data_Relay_Reci_MQTT.Relay1;
      doc["Relay2"] = Data_Relay_Reci_MQTT.Relay2;
      char mqtt_message[50];
      serializeJson(doc, mqtt_message);
      MQTT_PublishMessage("ESP32/Relay", mqtt_message, true);
      updateState = 0;
      xQueueSend(xQueueRelaySendNRF, &Data_Relay_Reci_MQTT, (TickType_t)0);
    } 
 }
}


void setup_mqtt(void){
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
  }
  if(obj.containsKey("Relay2")){
    boolean p = obj["Relay2"];
    Data_Relay_Reci_MQTT.Relay2 = p;
  }
  updateState = 1;
}

//-----Method for Publishing MQTT Messages---------
 void MQTT_PublishMessage(const char* topic, String payload, boolean retained){
  if(client.publish(topic,payload.c_str(),true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}
