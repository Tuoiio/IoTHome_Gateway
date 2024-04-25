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

#endif