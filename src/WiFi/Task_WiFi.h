#ifndef _TASK_WIFI_H_
#define _TASK_WIFI_H_

#include <WiFi.h>
#include <vector>
#include <time.h>
#include <Type_Data.h>

#include <EEPROM.h>
#define EEPROM_SIZE 128
#define EEPROM_ADDR_WIFI_FLAG 0
#define EEPROM_ADDR_WIFI_CREDENTIAL 12


extern void tryPreviousNetwork(void);

static void saveWiFiCredentialEEPROM(int flag, String ssidpw);

void networkScanner(void);

static void scanWiFiTask(void *pvParamter);

void networkConnector(void);

static void beginWiFiTask(void *pvParameter);

bool Notice_WiFi_Disconnect(void);

#endif