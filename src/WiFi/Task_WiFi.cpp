#include "Task_WiFi.h"

extern TaskHandle_t Task_MQTT_Handle;

SemaphoreHandle_t xBinaryNoticeReconectWifi;

Network_Status_t networkStatus = NONE;

extern String SSID_Name, SSID_Password;

TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
std::vector<String> foundWifiList;


static void beginWiFiTask(void *pvParameter) {
  unsigned long startingTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(100);

  WiFi.begin(SSID_Name.c_str(), SSID_Password.c_str());
  while (WiFi.status() != WL_CONNECTED && (millis() - startingTime) < 10000) {
    vTaskDelay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    networkStatus = NETWORK_CONNECTED_POPUP;
    saveWiFiCredentialEEPROM(1, SSID_Name + " " + SSID_Password);
    vTaskResume(Task_MQTT_Handle);
  } else {
    networkStatus = NETWORK_CONNECT_FAILED;
    saveWiFiCredentialEEPROM(0, "");
  }

  vTaskDelete(NULL);
}

static void scanWiFiTask(void *pvParamter) {
  for(;;) {
    foundWifiList.clear();
    int n = WiFi.scanNetworks();  // scan wifi gay mat ket noi mang
    vTaskDelay(10);
    for (int i = 0; i < n; ++i) {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      foundWifiList.push_back(item);
      vTaskDelay(5);
    }
    vTaskSuspend(NULL);
  }
}

void networkConnector(void) {
  xTaskCreatePinnedToCore(beginWiFiTask, "beginWiFiTask", 4096, NULL, 3, &ntConnectTaskHandler,0);
}

void networkScanner(void) {
  xTaskCreatePinnedToCore(scanWiFiTask, "scanWiFiTask", 4096, NULL, 2, &ntScanTaskHandler,0);
}

static void saveWiFiCredentialEEPROM(int flag, String ssidpw) {
  EEPROM.writeInt(EEPROM_ADDR_WIFI_FLAG, flag);
  EEPROM.writeString(EEPROM_ADDR_WIFI_CREDENTIAL, flag == 1 ? ssidpw : "");
  EEPROM.commit();
}

extern void tryPreviousNetwork(void){
  xBinaryNoticeReconectWifi = xSemaphoreCreateBinary();
  if (!EEPROM.begin(EEPROM_SIZE)) {
    vTaskDelay(1000);
    ESP.restart();
  }
  // Load Wifi Credenntial EEPROM
  int wifiFlag = EEPROM.readInt(EEPROM_ADDR_WIFI_FLAG);
  String wifiCredential = EEPROM.readString(EEPROM_ADDR_WIFI_CREDENTIAL);
  Serial.println(wifiCredential);
  if (wifiFlag == 1 && wifiCredential.length() != 0 && wifiCredential.indexOf(" ") != -1) {
    char preSSIDName[30], preSSIDPassword[30];
    
    if (sscanf(wifiCredential.c_str(), "%s %s", preSSIDName, preSSIDPassword) == 2) {
      xSemaphoreGive(xBinaryNoticeReconectWifi);
      vTaskDelay(5);    // Waiting dislay update and scan wifi before reconnect.
      SSID_Name = String(preSSIDName);
      SSID_Password = String(preSSIDPassword);
      networkConnector();
    } else {
      saveWiFiCredentialEEPROM(0, "");
    }
    
  }
}

bool Notice_WiFi_Disconnect(void) {
  bool Result = false;
  if(WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect(true);
    vTaskSuspend(Task_MQTT_Handle);
    Result = true;
  }
  return Result;
}
