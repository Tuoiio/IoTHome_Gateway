#include <RF24.h>
#include "Type_Data.h"

// Variable config CS, CE for NRF24
RF24 radio(21, 22);
 
 // Variable address node from the room
 const uint64_t Address_NodeGate = 0x1122334400;
 uint8_t Gate_On_Pipe_1 = 1;

// Variable address node from the gate home
const uint64_t Address_NodeRoom = 0x1122334411;
uint8_t Room_On_Pipe_2 = 2;

// Data Air, data relay and data gate house recive from task NRF send task TFT
extern QueueHandle_t xQueueAirSendTFT;
extern QueueHandle_t xQueueRelaySendTFT;
extern QueueHandle_t xQueueGateHouseSendTFT; 

// Data air, data relay and data gate house receive from task NRF send task MQTT
extern QueueHandle_t xQueueAirSendMQTT;
extern QueueHandle_t xQueueRelaySendMQTT;
extern QueueHandle_t xQueueGateHouseSendMQTT; 

// Data relay and data gate house from task TFT or MQTT send task NRF
extern QueueHandle_t xQueueRelaySendNRF;
extern QueueHandle_t xQueueGateHouseSendNRF; 

//extern QueueSetHandle_t xQueueUnlockTaskNRF;

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

void reconnect_nrf24(void) {
  radio.stopListening();
  radio.openWritingPipe(Address_NodeRoom);
  uint8_t data[] = "Reconnect";
  radio.write(&data, sizeof(data));
  delay(5);
  radio.openWritingPipe(Address_NodeGate);
  radio.write(&data, sizeof(data));
  radio.startListening();
}

extern void TaskNRF(void *pvParameter){
  setup_nrf24();
  reconnect_nrf24();
  radio.printDetails();
  Data_GateHouse_Node_t Data_GateHouse;
  Data_Relay_Room_t Data_Relay; 
  for(;;){
    /********************************Recive data from node then upadate mqtt and tft*************************/ 
    if(radio.available()) {
      uint8_t Length_NRF = radio.getDynamicPayloadSize();
      char *myRxData = (char *)calloc(Length_NRF, sizeof(char));
      
      if (myRxData != NULL) {
        radio.read(myRxData, Length_NRF);
        //Serial.println("Length Data: "); Serial.println(Length_NRF);
        if (myRxData[0] == 'A') {           // Node Gate House    
            //Serial.print("Node Gate House \n");
            Data_GateHouse_Node_t Data_GateHouse;
            memcpy(&Data_GateHouse, myRxData, sizeof(Data_GateHouse)); 
            //Serial.printf("ID: %d \n",Data_GateHouse.ID);
            //Serial.printf("Status: %d \n",Data_GateHouse.Status);
            //Serial.printf("Count: %d \n", Data_GateHouse.count);
            xQueueSend(xQueueGateHouseSendTFT, &Data_GateHouse, (TickType_t)0);
            xQueueSend(xQueueGateHouseSendMQTT, &Data_GateHouse, (TickType_t)0); 
        
        } else if (myRxData[0] == 'B') {    // Data Air Node Room
            Data_Air_Node_Room_t Value_Node_Room;
            memcpy(&Value_Node_Room, myRxData, sizeof(Value_Node_Room));
            xQueueSend(xQueueAirSendTFT, &Value_Node_Room, (TickType_t)0);
            xQueueSend(xQueueAirSendMQTT, &Value_Node_Room, (TickType_t)0);
            //Serial.printf("Nhiet do: %.1f\n",Value_Node_Room.temperature);
            //Serial.printf("Do am: %.1f\n",Value_Node_Room.humidity);
            //Serial.printf("CO2: %.1f\n",Value_Node_Room.CO2);
        
        } else if (myRxData[0] == 'D') {    // Data Relay Node Room
            Data_Relay_Room_t Data_Relay;
            memcpy(&Data_Relay, myRxData, sizeof(Data_Relay));
            xQueueSend(xQueueRelaySendTFT, &Data_Relay, (TickType_t)0);
            xQueueSend(xQueueRelaySendMQTT, &Data_Relay, (TickType_t)0);
            //Serial.printf("Relay 1: %d\n", Data_Relay.Relay1);
            //Serial.printf("Relay 2: %d\n", Data_Relay.Relay2);
        }
      }
      free(myRxData);
    }

    /*********************************Receive Data Relay From MQTT then upload Node Relay*****************************************/
    //QueueSetMemberHandle_t Who_Unlocked = xQueueSelectFromSet(xQueueUnlockTaskNRF, (TickType_t)0);
    static bool Retransmit = 0;
    //if(Who_Unlocked == xQueueRelaySendNRF) {
      if (xQueueReceive(xQueueRelaySendNRF, &Data_Relay, (TickType_t)0) == pdPASS) {
        radio.stopListening();
        radio.openWritingPipe(Address_NodeRoom);
        Data_Relay.ID = 'D';
        Retransmit = radio.write(&Data_Relay, sizeof(Data_Relay));
        if(Retransmit == 0){
          radio.write(&Data_Relay, sizeof(Data_Relay));
        }
        radio.startListening();
      }
      //} else if(Who_Unlocked == xQueueGateHouseSendNRF) {
        if (xQueueReceive(xQueueGateHouseSendNRF, &Data_GateHouse, (TickType_t)0) == pdPASS) {
          radio.stopListening();
          radio.openWritingPipe(Address_NodeGate);
          Data_GateHouse.ID = 'A';
          Retransmit = radio.write(&Data_GateHouse, sizeof(Data_GateHouse));
          if(Retransmit == 0){
            radio.write(&Data_GateHouse, sizeof(Data_GateHouse));
          }
          radio.startListening();
        }
    //}
  }
}