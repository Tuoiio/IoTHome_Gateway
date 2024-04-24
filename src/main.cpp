#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "RF24.h"


WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
// Sau khi mat dien tu dong gui lai thong tin
boolean updateState = 1;

void setup_wifi(void);
void setup_nrf24(void);
void setup_mqtt(void);

void MQTT_Reconnect(void);
void MQTT_callback(char *topic, byte *payload, unsigned int lenth);
void MQTT_PublishMessage(const char *topic, String payload, boolean retained);

// Variable config CS, CE for NRF24
RF24 radio(21, 22);
 
 // Variable address node from the room
 const uint64_t Address_NodeGate = 0x1122334400;
 uint8_t Gate_On_Pipe_1 = 1;

// Variable address node from the gate home
const uint64_t Address_NodeRoom = 0x1122334411;
uint8_t Room_On_Pipe_2 = 2;

char myTxData[32];
char myRxData[32];


typedef struct {
  char ID;
  float temperature;
  float humidity;
  float CO2;
}Data_Air_Node_Room_t;

typedef struct {
  char ID;
  uint8_t Relay1;
  uint8_t Relay2;
}Data_Relay_Room_t;

Data_Relay_Room_t Data_Relay;

uint32_t lastTime;
  const char *mqtt_server = "971474d396034f83b2f2e2d608193178.s1.eu.hivemq.cloud";
  const int mqtt_port = 8883;

void setup(){
  Serial.begin(115200);

  setup_wifi();
  setup_nrf24();
  setup_mqtt();


  lastTime = millis();
}

void loop() {
  // ---------------------------- Connect MQTT------------------------------------------//
  
  if(!client.connected()){
    static uint32_t Time_Reconnect;
   if(millis() - Time_Reconnect > 5000){
     MQTT_Reconnect();
     Time_Reconnect = millis();
    }
  } else {
    client.loop();
  }
  
  // NRF24 Transiver 
  
  if( (uint32_t)(millis() - lastTime) > 900)
  {
    // Time1 - 250: Node Gate, Time 2 - 500: Node Room, Time3 - 750: Node Gate, Time4 - 1000: Node Room
    static bool b_Node_Active = 1;

    radio.stopListening();
    if( b_Node_Active == 0)   // Node Gate
    {
      radio.openWritingPipe(Address_NodeGate);
      myTxData[0] = 'A';
      radio.write(myTxData, 1);
    
    } else {                // Node Room
      radio.openWritingPipe(Address_NodeRoom);
      static uint8_t count = 0;
      myTxData[0] = 'B';
      radio.write(myTxData, 1);
    }
    b_Node_Active = !b_Node_Active;
    lastTime = millis();
    radio.startListening();
  }
  

  // NRF24 Receive
  
  if(radio.available())
  {
    
    uint8_t Length_NRF = radio.getDynamicPayloadSize();
    radio.read(myRxData, Length_NRF);
    Serial.println("Length Data: ");
    Serial.println(Length_NRF);
    // Node Gate House
    if(myRxData[0] == 'A')  
    {
      Serial.print("Node Gate House");
    }
    
    // Data Air Node Room
    if(myRxData[0] == 'B')
    {
      Data_Air_Node_Room_t Value_Node_Room;
      memcpy(&Value_Node_Room, myRxData, sizeof(Value_Node_Room));
      Serial.print("Nhiet do: ");
      Serial.println(Value_Node_Room.temperature);
      Serial.print("Do am: ");
      Serial.println(Value_Node_Room.humidity);
      Serial.print("CO2: ");
      Serial.println(Value_Node_Room.CO2);
      Serial.println("\n");

      DynamicJsonDocument doc(128);
      doc["temperature"] = (float)Value_Node_Room.temperature;
      doc["humidity"] = (float)Value_Node_Room.humidity;
      char mqtt_message[50];
      serializeJson(doc, mqtt_message);
      MQTT_PublishMessage("ESP32/Air", mqtt_message, true);
    }
    
    
    // Data Relay Node Room
    if((myRxData[0] == 'D')){
      memcpy(&Data_Relay, myRxData, sizeof(Data_Relay));
      Serial.print("Relay 1: ");
      Serial.println(Data_Relay.Relay1);
      Serial.print("Relay 2: ");
      Serial.println(Data_Relay.Relay2);

    DynamicJsonDocument doc(128);
    doc["Relay1"] = Data_Relay.Relay1;
    doc["Relay2"] = Data_Relay.Relay2;
    char mqtt_message[50];
    serializeJson(doc, mqtt_message);
    MQTT_PublishMessage("ESP32/Relay", mqtt_message, true);
    }
    
  }
  
  if(updateState == 1) {
    Serial.print("Relay 1: ");
    Serial.println(Data_Relay.Relay1);
    Serial.print("Relay 2:");
    Serial.println(Data_Relay.Relay2);

    DynamicJsonDocument doc(128);
    doc["Relay1"] = Data_Relay.Relay1;
    doc["Relay2"] = Data_Relay.Relay2;
    char mqtt_message[50];
    serializeJson(doc, mqtt_message);
    MQTT_PublishMessage("ESP32/Relay", mqtt_message, true);
    updateState = 0; 

    radio.stopListening();
    radio.openWritingPipe(Address_NodeRoom);
    Data_Relay.ID = 'D';
    static bool Retransmit = 0;
    Retransmit = radio.write(&Data_Relay, sizeof(Data_Relay));
    if(Retransmit == 0){
      radio.write(&Data_Relay, sizeof(Data_Relay));
    }
    radio.startListening();
  }
}


void setup_wifi(void){
  const char *ssid = "DuongTuoi";
  const char *password = "17072002";

  delay(10);
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}





void setup_mqtt(void){
  const char *mqtt_server = "971474d396034f83b2f2e2d608193178.s1.eu.hivemq.cloud";
  static int mqtt_port = 8883;
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(MQTT_callback);
}





void setup_nrf24(void){
  // Setup NRF24L01
  radio.begin();
  radio.setChannel(52);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(10,15);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.openReadingPipe(Gate_On_Pipe_1, Address_NodeGate);
  radio.openReadingPipe(Room_On_Pipe_2, Address_NodeRoom);
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

  DynamicJsonDocument doc(100);
  DeserializationError error = deserializeJson(doc, incomingMessage);
  if(error){
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  JsonObject obj = doc.as<JsonObject>();
  if(obj.containsKey("Relay1")){
    boolean p = obj["Relay1"];
    Data_Relay.Relay1 = p;
  }
  if(obj.containsKey("Relay2")){
    boolean p = obj["Relay2"];
    Data_Relay.Relay2 = p;
  }
  updateState = 1;
}

//-----Method for Publishing MQTT Messages---------
 void MQTT_PublishMessage(const char* topic, String payload, boolean retained){
  if(client.publish(topic,payload.c_str(),true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}

