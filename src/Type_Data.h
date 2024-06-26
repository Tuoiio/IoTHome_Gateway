#ifndef TYPE_DATA_H_
#define TYPE_DATA_H_


typedef struct {
  char ID;
  float temperature;
  float humidity;
  float CO2;
}Data_Air_Node_Room_t;


typedef struct {
  char ID;
  unsigned char Relay1;
  unsigned char Relay2;
}Data_Relay_Room_t;

typedef struct {
  char ID;
  unsigned char Status;
  unsigned char count;
}Data_GateHouse_Node_t;


typedef enum {
    NONE,
    NETWORK_SEARCHING,
    NETWORK_CONNECTED_POPUP,
    NETWORK_CONNECTED,
    NETWORK_CONNECT_FAILED
}Network_Status_t;

#endif