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

char myTxData[32];

extern QueueHandle_t xQueueTemperature;
extern QueueHandle_t xQueueRelaySendMQTT;
extern QueueHandle_t xQueueRelayReceiveMQTT;


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

extern void TaskNRF(void *pvParameter){
  setup_nrf24();
  TickType_t lastTime = xTaskGetTickCount();
  Data_Relay_Room_t Data_Relay;
  for(;;){
    /*********************************************NRF Transimit*************************************************************/ 
    if((TickType_t)xTaskGetTickCount() - lastTime > 500) 
    {
      static bool b_Node_Active = 1;
      radio.stopListening();
      if( b_Node_Active == 0) {   // Node Gate
        radio.openWritingPipe(Address_NodeGate);
        myTxData[0] = 'A';
        radio.write(myTxData, 1);
         
      } else {   // Node Room
        radio.openWritingPipe(Address_NodeRoom);
        static uint8_t count = 0;
        myTxData[0] = 'B';
        radio.write(myTxData, 1);
      }
      b_Node_Active = !b_Node_Active;
      lastTime = xTaskGetTickCount();
      radio.startListening();
    }

    /*********************************************NRF Receive*************************************************************/ 
    if(radio.available()) 
    {
      uint8_t Length_NRF = radio.getDynamicPayloadSize();
      char *myRxData = (char *)calloc(Length_NRF, sizeof(char));
      
      if (myRxData != NULL) 
      {
        radio.read(myRxData, Length_NRF);
        //Serial.println("Length Data: "); Serial.println(Length_NRF);

        if (myRxData[0] == 'A') // Node Gate House
        {   
          Serial.print("Node Gate House");
        }

        if (myRxData[0] == 'B') // Data Air Node Room
        {     
          Data_Air_Node_Room_t Value_Node_Room;
          memcpy(&Value_Node_Room, myRxData, sizeof(Value_Node_Room));
          Serial.printf("Nhiet do: %.2f\n",Value_Node_Room.temperature);
          Serial.printf("Do am: %.2f\n",Value_Node_Room.humidity);
          Serial.printf("CO2: %.2f\n",Value_Node_Room.CO2);

          xQueueSend(xQueueTemperature, &Value_Node_Room, (TickType_t)0);
        }
  
        if (myRxData[0] == 'D') // Data Relay Node Room
        {
          
          memcpy(&Data_Relay, myRxData, sizeof(Data_Relay));
          Serial.printf("Relay 1: %d\n", Data_Relay.Relay1);
          Serial.printf("Relay 2: %d\n", Data_Relay.Relay2);

          xQueueSend(xQueueRelaySendMQTT, &Data_Relay, (TickType_t)0);
        }
      }
      free(myRxData);
    }
    /*********************************Receive Data Relay From MQTT then upload Node Relay*****************************************/
    if (xQueueReceive(xQueueRelayReceiveMQTT, &Data_Relay, (TickType_t)10) == pdPASS)
    {
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
}