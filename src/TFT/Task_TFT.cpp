#include <TFT_eSPI.h>
#include "Task_TFT.h"

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /*TFT instance*/

Ticker tick;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 60 * 60; // Set your timezone here
const int daylightOffset_sec = 1;

String SSID_Name, SSID_Password;    // Name and Password Wifi
extern std::vector<String> foundWifiList;
int foundNetworks = 0;


extern TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;    
extern SemaphoreHandle_t xBinaryNoticeReconectWifi; 
extern QueueSetHandle_t xQueueUnlockTaskTFT;
extern QueueHandle_t xQueueGateHouseSendTFT;
extern QueueHandle_t xQueueGateHouseSendNRF;
extern QueueHandle_t xQueueAirSendTFT;  
extern QueueHandle_t xQueueRelaySendTFT;    
extern QueueHandle_t xQueueRelaySendNRF;
extern Network_Status_t networkStatus;  

Data_Relay_Room_t Value_Relay_Recive_NRF;
Data_GateHouse_Node_t Data_GateHouse_Receive_NRF;


/*Display flushing*/
static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors( (uint16_t *)&color_p->full, w *h, true);
    tft.endWrite();

    lv_disp_flush_ready( disp_drv );
}

/*Read the touchpad*/
static void my_touchpad_read (lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if ( !touched ) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        
        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

static void lv_tick_handler(void) {
    lv_tick_inc(LVGL_TICK_PERIOD);
}


 lv_style_t border_style;
 lv_style_t popupBox_style;
 lv_style_t btn_style_off;
 lv_style_t btn_style_on;
 lv_obj_t *timeLabel;
 lv_obj_t *keyboard;
 lv_obj_t *popupSettings;
 lv_obj_t *btn_open_popup_setting;
 lv_obj_t *btn_close_popup_setting;
 lv_obj_t *settingWiFiSwitch;
 lv_obj_t *WiFiList;
 lv_obj_t *popup_wifi_enter;
lv_obj_t *btn_connect_wifi;
 lv_obj_t *btn_close_popup_wifi_enter;
 lv_obj_t *mBoxTitle;
 lv_obj_t *mBoxPassword;

 lv_obj_t *popupBox;
 lv_obj_t *popupBoxCloseBtn;
 lv_timer_t *timer;

 lv_obj_t *btn_open_popup_room;
 lv_obj_t *popup_room;
 lv_obj_t *btn_close_popup_room;
 lv_obj_t *data_temperature;
 lv_obj_t *data_humidity;
 lv_obj_t *data_co2;
 lv_obj_t *btn_ctrl1;
 lv_obj_t *state_btn_ctrl1;
 lv_obj_t *btn_ctrl2;
 lv_obj_t *state_btn_ctrl2;

 lv_obj_t *btn_open_popup_gatehome;
 lv_obj_t *GateHome;
 lv_obj_t *Status_GateHome;
 lv_obj_t *btn_OpenGate;
 lv_obj_t *btn_PauseGate;
 lv_obj_t *btn_CloseGate;
 lv_obj_t *btn_GateHomeClose;

void updateLocalTime(void) { // Done
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  char hourMin[6];
  strftime(hourMin, 6, "%H:%M", &timeinfo);
  String hourMinWithSymbol = String(hourMin);
  hourMinWithSymbol += "   ";
  hourMinWithSymbol += LV_SYMBOL_WIFI;
  lv_label_set_text(timeLabel, hourMinWithSymbol.c_str());
}

static void showingFoundWiFiList(void) { // Done
  if (foundWifiList.size() == 0 || foundNetworks == foundWifiList.size())
    return;

  lv_obj_clean(WiFiList);
  lv_list_add_text(WiFiList, foundWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");

  for (std::vector<String>::iterator item = foundWifiList.begin(); item != foundWifiList.end(); ++item) {
    lv_obj_t *btn = lv_list_add_btn(WiFiList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, list_event_handler, LV_EVENT_CLICKED, NULL);
    delay(1);
  }

  foundNetworks = foundWifiList.size();
}

static void timerForNetwork(lv_timer_t *timer) { // Done
    LV_UNUSED(timer);

    switch (networkStatus) {
    case NETWORK_SEARCHING:
        showingFoundWiFiList();
        break;
    
    case NETWORK_CONNECTED_POPUP:
        UI_Wifi_Notification("WiFi Connected!", "Now you'll get the current time soon.");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        networkStatus = NETWORK_CONNECTED;
        break;
    
    case NETWORK_CONNECTED:
        showingFoundWiFiList();
        updateLocalTime();
        break;
    
    case NETWORK_CONNECT_FAILED:
        networkStatus = NETWORK_SEARCHING;
        UI_Wifi_Notification("Oops!", "Please check your wifi password and try again.");
        break;
    default:
        break;
    }
}

extern void btn_event_display_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        /*Event bar*/
        if (btn == btn_open_popup_setting) {
            lv_obj_clear_flag(popupSettings, LV_OBJ_FLAG_HIDDEN);
            if(ntScanTaskHandler != NULL) vTaskResume(ntScanTaskHandler); 
        
        } else if (btn == btn_close_popup_setting) {
            lv_obj_add_flag(popupSettings, LV_OBJ_FLAG_HIDDEN);
        
        } else if (btn == btn_close_popup_wifi_enter) {
            lv_obj_move_background(popup_wifi_enter);
        
        } else if (btn == popupBoxCloseBtn) {
            lv_obj_move_background(popupBox);
        
        } else if (btn == btn_open_popup_room) {    /*Event Room*/
            lv_obj_clear_flag(popup_room, LV_OBJ_FLAG_HIDDEN);
        
        } else if (btn == btn_close_popup_room) {
            lv_obj_add_flag(popup_room, LV_OBJ_FLAG_HIDDEN);
        
        } else if(btn == btn_open_popup_gatehome) {
            lv_obj_clear_flag(GateHome, LV_OBJ_FLAG_HIDDEN);
        
        } else if(btn == btn_GateHomeClose) {
            lv_obj_add_flag(GateHome, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

extern void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    static Data_Relay_Room_t Value_Relay_Send_NRF;
    static Data_GateHouse_Node_t Data_GateHouse_Send_NRF;

    if (code == LV_EVENT_CLICKED) {
        Value_Relay_Send_NRF = Value_Relay_Recive_NRF;
        Data_GateHouse_Send_NRF = Data_GateHouse_Receive_NRF;

        if (btn == btn_connect_wifi) { // Connect to Wifi
            SSID_Password = String(lv_textarea_get_text(mBoxPassword));
            networkConnector();
            lv_obj_move_background(popup_wifi_enter);
            UI_Wifi_Notification("Connecting!", "Attempting to connect the selected network.");
        
        } else if (btn == btn_ctrl1) {
            if(Value_Relay_Send_NRF.Relay1 == 1) Value_Relay_Send_NRF.Relay1 = 0;
            else                                Value_Relay_Send_NRF.Relay1 = 1;
            xQueueSend(xQueueRelaySendNRF, &Value_Relay_Send_NRF, (TickType_t)0);
        
        } else if (btn == btn_ctrl2) {
            if(Value_Relay_Send_NRF.Relay2 == 1) Value_Relay_Send_NRF.Relay2 = 0;
            else                                Value_Relay_Send_NRF.Relay2 = 1;
            xQueueSend(xQueueRelaySendNRF, &Value_Relay_Send_NRF, (TickType_t)0);
        
        } else if (btn == btn_OpenGate) {
            //Serial.print("Buttton Open Click \n");
            if(Data_GateHouse_Send_NRF.Status != 2 || Data_GateHouse_Send_NRF.count != 20) { // the gate house was all open
                Data_GateHouse_Send_NRF.Status = 1;
                xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Send_NRF, (TickType_t)0);
            }
        
        } else if (btn == btn_PauseGate) {
            //Serial.print("Buttton Pause Click \n");
            if(Data_GateHouse_Send_NRF.Status == 1){
                Data_GateHouse_Send_NRF.Status = 2;
                xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Send_NRF, (TickType_t)0);
            } else if(Data_GateHouse_Send_NRF.Status == 3){
                Data_GateHouse_Send_NRF.Status = 4;
                xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Send_NRF, (TickType_t)0);
            }
        } else if (btn == btn_CloseGate) {
           
            if(Data_GateHouse_Send_NRF.count != 0 || Data_GateHouse_Send_NRF.Status != 4) { // the gate house was all close
                Data_GateHouse_Send_NRF.Status = 3;
                xQueueSend(xQueueGateHouseSendNRF, &Data_GateHouse_Send_NRF, (TickType_t)0);
            }
        }

    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (btn == settingWiFiSwitch) {
            if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                if(ntScanTaskHandler == NULL) {
                    networkStatus = NETWORK_SEARCHING;
                    networkScanner();
                    timer = lv_timer_create(timerForNetwork, 1000, WiFiList);
                    lv_list_add_text(WiFiList, "WiFi: Looking for networks...");
                }
            } else {
                if (ntScanTaskHandler != NULL) {
                    networkStatus = NONE;
                    vTaskDelete(ntScanTaskHandler);
                    ntScanTaskHandler = NULL;
                    lv_timer_del(timer);
                    lv_obj_clean(WiFiList);
                }
                if (Notice_WiFi_Disconnect() == true) {
                    lv_label_set_text(timeLabel, "WiFi Not Connected!   " LV_SYMBOL_CLOSE);
                }
            }
        }
    }
}

extern void list_event_handler(lv_event_t *e) { // Done
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    if (code == LV_EVENT_CLICKED) {
        String selectedItem = String(lv_list_get_btn_text(WiFiList, obj));
        for (int i = 0; i < selectedItem.length() - 1; i++) {
            if (selectedItem.substring(i, i + 2) == " (") {
                SSID_Name = selectedItem.substring(0, i);
                lv_label_set_text_fmt(mBoxTitle, "Selected WiFi SSID: %s", SSID_Name);
                lv_obj_move_foreground(popup_wifi_enter);
            }
        }
    }
}

extern void text_input_event_cb(lv_event_t *e) { // Done
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED) {
        lv_obj_move_foreground(keyboard);
        lv_keyboard_set_textarea(keyboard, ta);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void setStyle(void) { // Done
    lv_style_init(&border_style);
    lv_style_set_border_width(&border_style, 2);
    lv_style_set_border_color(&border_style, lv_color_black());

    lv_style_init(&popupBox_style);
    lv_style_set_radius(&popupBox_style, 10);
    lv_style_set_bg_opa(&popupBox_style, LV_OPA_COVER);
    lv_style_set_border_color(&popupBox_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&popupBox_style, 5);

    lv_style_init(&btn_style_off);
    lv_style_set_bg_color(&btn_style_off, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_opa(&btn_style_off, LV_OPA_COVER);

    lv_style_init(&btn_style_on);
    lv_style_set_bg_color(&btn_style_on, lv_color_make(255, 0, 0));
    lv_style_set_bg_opa(&btn_style_on, LV_OPA_COVER);
}


static void makeKeyboard(void) { // Done
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}


void setup_TFT(void){
    /*TFT init and Landsacpe orientation, flipped*/
    tft.begin();
    tft.setRotation(3);

    /*TFT calibrate Touch*/
    uint16_t calData[5] = {249, 3330, 385, 3317, 7 };
    tft.setTouch(calData);
    
    /*LVGL init*/
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[ screenWidth * screenHeight / 10];
    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10);

    tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
    
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    LV_IMG_DECLARE(img_usr_map);
    lv_obj_t *img_src = lv_img_create(lv_scr_act());
    lv_img_set_src(img_src, &img_usr_map);
    lv_obj_align(img_src, LV_ALIGN_CENTER, 0, 0);
  
}

extern void TaskTFT(void *pvParameter) {
    setup_TFT();
    setStyle();
    UI_Main();
    UI_Room();
    UI_GateHouse();
    UI_Wifi();
    UI_Wifi_Enter();
    makeKeyboard();
    Data_Air_Node_Room_t Value_Air; 
    for(;;){
        lv_task_handler(); //Let the GUI do its work
        if (xSemaphoreTake(xBinaryNoticeReconectWifi, (TickType_t)0)  == pdPASS) {
            lv_obj_add_state(settingWiFiSwitch, LV_STATE_CHECKED);
            lv_event_send(settingWiFiSwitch, LV_EVENT_VALUE_CHANGED, NULL);
            UI_Wifi_Notification("Welcome Back!", "Attempts to reconnect to the previously connected network.");
        }
        
        QueueSetMemberHandle_t Who_Unlock = xQueueSelectFromSet(xQueueUnlockTaskTFT, (TickType_t)10);
        if(Who_Unlock == xQueueAirSendTFT) {
            if (xQueueReceive(xQueueAirSendTFT, &Value_Air,(TickType_t)0) == pdPASS) {
                if(Value_Air.temperature == -88.0f) {
                    lv_label_set_text(data_temperature, "Sensor Er");
                    lv_label_set_text(data_humidity,    "Sensor Er");
                    lv_label_set_text(data_co2,         "Sensor Er");
                } else if(Value_Air.temperature == -99.0f) {
                    lv_label_set_text(data_temperature, "Disconect");
                    lv_label_set_text(data_humidity,    "Disconect");
                    lv_label_set_text(data_co2,         "Disconect");
                } else {
                    lv_label_set_text_fmt(data_temperature, "%.1f °C", Value_Air.temperature);
                    lv_label_set_text_fmt(data_humidity,    "%.1f %%", Value_Air.humidity);
                    lv_label_set_text_fmt(data_co2,         "%.1f ", Value_Air.CO2);
                }
            }
        
        } else if (Who_Unlock == xQueueRelaySendTFT) {
            if (xQueueReceive(xQueueRelaySendTFT, &Value_Relay_Recive_NRF, (TickType_t)0) == pdPASS) {
                /*Relay 1*/
                if(Value_Relay_Recive_NRF.Relay1 == 1) {
                    lv_label_set_text(state_btn_ctrl1, "On");
                    lv_obj_add_style(btn_ctrl1, &btn_style_on, 0);
                } else {
                    lv_label_set_text(state_btn_ctrl1, "Off");
                    lv_obj_add_style(btn_ctrl1, &btn_style_off, 0);
                }
                /*Relay 2*/
                if (Value_Relay_Recive_NRF.Relay2 == 1) {
                    lv_label_set_text(state_btn_ctrl2, "On");
                    lv_obj_add_style(btn_ctrl2, &btn_style_on, 0);
                } else {
                    lv_label_set_text(state_btn_ctrl2, "Off");
                    lv_obj_add_style(btn_ctrl2, &btn_style_off, 0);
                }
            }

        } else if (Who_Unlock == xQueueGateHouseSendTFT) {
            if(xQueueReceive(xQueueGateHouseSendTFT, &Data_GateHouse_Receive_NRF, (TickType_t)0) == pdPASS) {
                if(Data_GateHouse_Receive_NRF.Status == 1 && Data_GateHouse_Receive_NRF.count  > 0 && Data_GateHouse_Receive_NRF.count < 20) {
                    lv_label_set_text(Status_GateHome, "Opening Gate");
            
                } else if (Data_GateHouse_Receive_NRF.Status == 2 || Data_GateHouse_Receive_NRF.Status == 4) {
                    if(Data_GateHouse_Receive_NRF.count == 0 && Data_GateHouse_Receive_NRF.Status == 4) {
                    lv_label_set_text(Status_GateHome, "Closed");
                
                    } else if (Data_GateHouse_Receive_NRF.Status == 2 && Data_GateHouse_Receive_NRF.count == 20) {
                        lv_label_set_text(Status_GateHome, "Opened");
                
                    } else {
                        lv_label_set_text(Status_GateHome, "Pause Gate");
                    }

                } else if (Data_GateHouse_Receive_NRF.Status == 3) {
                    if(Data_GateHouse_Receive_NRF.count > 0 && Data_GateHouse_Receive_NRF.count < 20) {
                        lv_label_set_text(Status_GateHome, "Closing Gate");
                    }
                } 
            }
        }
    }
}