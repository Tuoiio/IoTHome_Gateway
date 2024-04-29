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


extern TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;    //x
extern SemaphoreHandle_t xBinaryNoticeReconectWifi; //x
extern QueueHandle_t xQueueAirSendTFT;  //x
extern QueueHandle_t xQueueRelaySendTFT;    //x
extern QueueHandle_t xQueueRelaySendNRF;
extern Network_Status_t networkStatus;  //x

Data_Relay_Room_t Value_Relay;


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

static lv_style_t border_style;
static lv_style_t popupBox_style;
static lv_style_t btn_style_off;
static lv_style_t btn_style_on;
static lv_obj_t *timeLabel;
static lv_obj_t *keyboard;
static lv_obj_t *settings;
static lv_obj_t *settingLabel;
static lv_obj_t *settingBtn;
static lv_obj_t *settingCloseBtn;
static lv_obj_t *settingWiFiSwitch;
static lv_obj_t *WiFiList;
static lv_obj_t *mBoxConnect;
static lv_obj_t *mBoxTitle;
static lv_obj_t *mBoxPassword;
static lv_obj_t *mBoxConnectBtn;
static lv_obj_t *mBoxCloseBtn;
static lv_obj_t *popupBox;
static lv_obj_t *popupBoxCloseBtn;
static lv_timer_t *timer;

static lv_obj_t *btn_Room;
static lv_obj_t *Room;
static lv_obj_t *Room_CloseBtn;
static lv_obj_t *data_temperature;
static lv_obj_t *data_humidity;
static lv_obj_t *data_co2;
static lv_obj_t *btn_ctrl1;
static lv_obj_t *state_btn_ctrl1;
static lv_obj_t *btn_ctrl2;
static lv_obj_t *state_btn_ctrl2;

void updateLocalTime(void) {
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

static void showingFoundWiFiList(void) {
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

static void timerForNetwork(lv_timer_t *timer) {
    LV_UNUSED(timer);

    switch (networkStatus) {
    case NETWORK_SEARCHING:
        showingFoundWiFiList();
        break;
    
    case NETWORK_CONNECTED_POPUP:
        popupMsgBox("WiFi Connected!", "Now you'll get the current time soon.");
        networkStatus = NETWORK_CONNECTED;
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        break;
    
    case NETWORK_CONNECTED:
        showingFoundWiFiList();
        updateLocalTime();
        break;
    
    case NETWORK_CONNECT_FAILED:
        networkStatus = NETWORK_SEARCHING;
        popupMsgBox("Oops!", "Please check your wifi password and try again.");
        break;
    default:
        break;
    }
}

static void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    static Data_Relay_Room_t Temp_Value_Relay;
    if (code == LV_EVENT_CLICKED) {
        Temp_Value_Relay = Value_Relay;
        /*Event bar*/
        if (btn == settingBtn) {
            lv_obj_clear_flag(settings, LV_OBJ_FLAG_HIDDEN);
            if(ntScanTaskHandler != NULL) vTaskResume(ntScanTaskHandler);
        } else if (btn == settingCloseBtn) {
            lv_obj_add_flag(settings, LV_OBJ_FLAG_HIDDEN);
        } else if (btn == mBoxConnectBtn) { // Connect to Wifi
            SSID_Password = String(lv_textarea_get_text(mBoxPassword));
            networkConnector();
            lv_obj_move_background(mBoxConnect);
            popupMsgBox("Connecting!", "Attempting to connect the selected network.");
        } else if (btn == mBoxCloseBtn) {
            lv_obj_move_background(mBoxConnect);
        } else if (btn == popupBoxCloseBtn) {
            lv_obj_move_background(popupBox);
        /*Event Room*/
        } else if (btn == btn_Room) {
            lv_obj_clear_flag(Room, LV_OBJ_FLAG_HIDDEN);
        } else if (btn == Room_CloseBtn) {
            lv_obj_add_flag(Room, LV_OBJ_FLAG_HIDDEN);
        } else if (btn == btn_ctrl1) {
            if(Temp_Value_Relay.Relay1 == 1) Temp_Value_Relay.Relay1 = 0;
            else                             Temp_Value_Relay.Relay1 = 1;
            xQueueSend(xQueueRelaySendNRF, &Temp_Value_Relay, (TickType_t)0);
        } else if (btn == btn_ctrl2) {
            if(Temp_Value_Relay.Relay2 == 1) Temp_Value_Relay.Relay2 = 0;
            else                             Temp_Value_Relay.Relay2 = 1;
            xQueueSend(xQueueRelaySendNRF, &Temp_Value_Relay, (TickType_t)0);
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

static void list_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    
    if (code == LV_EVENT_CLICKED) {
        String selectedItem = String(lv_list_get_btn_text(WiFiList, obj));
        for (int i = 0; i < selectedItem.length() - 1; i++) {
            if (selectedItem.substring(i, i + 2) == " (") {
                SSID_Name = selectedItem.substring(0, i);
                lv_label_set_text_fmt(mBoxTitle, "Selected WiFi SSID: %s", SSID_Name);
                lv_obj_move_foreground(mBoxConnect);
            }
        }
    }
}

static void text_input_event_cb(lv_event_t *e) {
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

static void setStyle(void) {
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


static void buildStatusBar(void) {
    lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xC5C5C5));
    lv_style_set_bg_opa(&style_btn, LV_OPA_50);

    lv_obj_t *statusBar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(statusBar, tft.width(), 30);
    lv_obj_align(statusBar, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_remove_style(statusBar, NULL, LV_PART_SCROLLBAR | LV_STATE_ANY);
    
    timeLabel = lv_label_create(statusBar);
    lv_obj_set_size(timeLabel, tft.width() - 50, 30);

    lv_label_set_text(timeLabel, "WiFi Not Connected!  " LV_SYMBOL_CLOSE);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 8, 4);

    settingBtn = lv_btn_create(statusBar);
    lv_obj_set_size(settingBtn, 30, 30);
    lv_obj_align(settingBtn, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_add_event_cb(settingBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *label = lv_label_create(settingBtn);      /*Add a label to the button*/
    lv_label_set_text(label, LV_SYMBOL_SETTINGS);       /*Set the labels text*/
    lv_obj_center(label);
}

static void buildSettings(void) {
    settings = lv_obj_create(lv_scr_act());
    lv_obj_add_style(settings, &border_style, 0);
    lv_obj_set_size(settings, tft.width() - 50, tft.height() - 40);
    lv_obj_align(settings, LV_ALIGN_TOP_MID, 0, 35);

    settingLabel = lv_label_create(settings);
    lv_label_set_text(settingLabel, "Settings " LV_SYMBOL_SETTINGS);
    lv_obj_align(settingLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    settingCloseBtn = lv_btn_create(settings);
    lv_obj_set_size(settingCloseBtn, 30, 30);
    lv_obj_align(settingCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
    lv_obj_add_event_cb(settingCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *btnSymbol = lv_label_create(settingCloseBtn);
    lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
    lv_obj_center(btnSymbol);

    settingWiFiSwitch = lv_switch_create(settings);
    lv_obj_add_event_cb(settingWiFiSwitch, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align_to(settingWiFiSwitch, settingLabel, LV_ALIGN_TOP_RIGHT, 60, -10);
    lv_obj_add_flag(settings, LV_OBJ_FLAG_HIDDEN);

    WiFiList = lv_list_create(settings);
    lv_obj_set_size(WiFiList, tft.width() - 80, 140);
    lv_obj_align_to(WiFiList, settingLabel, LV_ALIGN_TOP_LEFT, 0, 30);
}

static void buildPWMsgBox(void) {
    mBoxConnect = lv_obj_create(lv_scr_act());
    lv_obj_add_style(mBoxConnect, &border_style, 0);
    lv_obj_set_size(mBoxConnect, tft.width() * 3 / 4, tft.height() - 80);
    lv_obj_center(mBoxConnect);
    lv_obj_move_background(mBoxConnect);

    mBoxTitle = lv_label_create(mBoxConnect);
    lv_label_set_text(mBoxTitle, "Select WiFi SSID: DuongTuoi");
    lv_obj_align(mBoxTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    mBoxPassword = lv_textarea_create(mBoxConnect);
    lv_obj_set_size(mBoxPassword, tft.width() / 2, 40);
    lv_obj_align_to(mBoxPassword, mBoxTitle, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_textarea_set_placeholder_text(mBoxPassword, "Password?");
    lv_obj_add_event_cb(mBoxPassword, text_input_event_cb, LV_EVENT_ALL, keyboard);

    mBoxConnectBtn = lv_btn_create(mBoxConnect);
    lv_obj_add_event_cb(mBoxConnectBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mBoxConnectBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *btnLabel = lv_label_create(mBoxConnectBtn);
    lv_label_set_text(btnLabel, "Connect");
    lv_obj_center(btnLabel);

    mBoxCloseBtn = lv_btn_create(mBoxConnect);
    lv_obj_add_event_cb(mBoxCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mBoxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *btnLabel2 = lv_label_create(mBoxCloseBtn);
    lv_label_set_text(btnLabel2, "Cancel");
    lv_obj_center(btnLabel2);
}

static void popupMsgBox(String title, String msg) {
    if(popupBox != NULL) {
        lv_obj_del(popupBox);
    }
    popupBox = lv_obj_create(lv_scr_act());
    lv_obj_add_style(popupBox, &popupBox_style, 0);
    lv_obj_set_size(popupBox, tft.width() * 3 / 4, tft.height() - 80);
    lv_obj_center(popupBox);

    lv_obj_t *popupTitle = lv_label_create(popupBox);
    lv_label_set_text(popupTitle, title.c_str());
    lv_obj_set_width(popupTitle, tft.width() * 2 / 3 - 50);
    lv_obj_align(popupTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *popupMSG = lv_label_create(popupBox);
    lv_obj_set_width(popupMSG, tft.width() * 2 / 3 - 50);
    lv_label_set_text(popupMSG, msg.c_str());
    lv_obj_align(popupMSG, LV_ALIGN_TOP_LEFT, 0, 40);

    popupBoxCloseBtn = lv_btn_create(popupBox);
    lv_obj_add_event_cb(popupBoxCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(popupBoxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *btnLabel = lv_label_create(popupBoxCloseBtn);
    lv_label_set_text(btnLabel, "Okay");
    lv_obj_center(btnLabel);
}

static void makeKeyboard(void) {
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}


static void UI_Room(void) {
    lv_obj_t *label; 
    btn_Room = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_Room, 100, 50);
    lv_obj_align(btn_Room, LV_ALIGN_BOTTOM_LEFT, 20, -30);
    lv_obj_add_event_cb(btn_Room, btn_event_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_Room);
    lv_label_set_text(label, "Room  " LV_SYMBOL_HOME);
    lv_obj_center(label);


    Room = lv_obj_create(lv_scr_act());
    lv_obj_add_style(Room, &border_style, 0);
    lv_obj_set_size(Room, tft.width(), tft.height() - 40);
    lv_obj_align(Room, LV_ALIGN_TOP_MID, 0, 35);
    label = lv_label_create(Room);
    lv_label_set_text(label, "Room " LV_SYMBOL_HOME);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(Room, LV_OBJ_FLAG_HIDDEN);

    /*Temperature*/
    lv_obj_t *scr_Temp = lv_obj_create(Room);
    lv_obj_add_style(scr_Temp, &border_style, 0);
    lv_obj_set_size(scr_Temp, tft.width() - 180, tft.height() - 120);
    lv_obj_align(scr_Temp, LV_ALIGN_TOP_LEFT, 0, 25);
    label = lv_label_create(scr_Temp);
    lv_label_set_text(label, "Temperature");
    
    LV_IMG_DECLARE(temperature_control);
    lv_obj_t *thememeter_img = lv_img_create(scr_Temp);
    lv_img_set_src(thememeter_img, &temperature_control);
    lv_obj_set_size(thememeter_img, 70, 50);
    lv_obj_align(thememeter_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);

    data_temperature = lv_label_create(scr_Temp);
    lv_label_set_text(data_temperature, "NULL");
    lv_obj_align(data_temperature, LV_ALIGN_BOTTOM_RIGHT,0, -15);
    
    /*Humidity*/
    lv_obj_t *scr_Humi = lv_obj_create(Room);
    lv_obj_add_style(scr_Humi, &border_style, 0);
    lv_obj_set_size(scr_Humi, tft.width() - 180, tft.height() - 120);
    lv_obj_align(scr_Humi, LV_ALIGN_TOP_RIGHT, 0, 25);
    label = lv_label_create(scr_Humi);
    lv_label_set_text(label, "Humidity");
    
    LV_IMG_DECLARE(humidity_control);
    lv_obj_t *Humi_img = lv_img_create(scr_Humi);
    lv_img_set_src(Humi_img, &humidity_control);
    lv_obj_set_size(Humi_img, 70, 50);
    lv_obj_align(Humi_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);

    data_humidity = lv_label_create(scr_Humi);
    lv_label_set_text(data_humidity, "NULL");
    lv_obj_align(data_humidity, LV_ALIGN_BOTTOM_RIGHT,0, -15);

    /*Co2*/
    lv_obj_t *scr_Co2 = lv_obj_create(Room);
    lv_obj_add_style(scr_Co2, &border_style, 0);
    lv_obj_set_size(scr_Co2, tft.width() - 180, tft.height() - 120);
    lv_obj_align(scr_Co2, LV_ALIGN_BOTTOM_LEFT, 0, 105);
    label = lv_label_create(scr_Co2);
    lv_label_set_text(label, "CO2");
    
    LV_IMG_DECLARE(co2_control);
    lv_obj_t *Co2_img = lv_img_create(scr_Co2);
    lv_img_set_src(Co2_img, &co2_control);
    lv_obj_set_size(Co2_img, 70, 50);
    lv_obj_align(Co2_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);


    data_co2 = lv_label_create(scr_Co2);
    lv_label_set_text(data_co2, "NULL");
    lv_obj_align(data_co2, LV_ALIGN_BOTTOM_RIGHT,0, -15);

    /*Control relay*/
    lv_obj_t *scr_btn_ctrl = lv_obj_create(Room);
    lv_obj_add_style(scr_btn_ctrl, &border_style, 0);
    lv_obj_set_size(scr_btn_ctrl, tft.width() - 180, tft.height() - 120);
    lv_obj_align(scr_btn_ctrl, LV_ALIGN_BOTTOM_RIGHT, 0, 105);
    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Control");
    
    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Device 1");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, -7, -10);

    btn_ctrl1 = lv_btn_create(scr_btn_ctrl);
    lv_obj_set_size(btn_ctrl1, 60, 30);
    lv_obj_align(btn_ctrl1, LV_ALIGN_RIGHT_MID, 3, -10);
    lv_obj_add_event_cb(btn_ctrl1, btn_event_cb, LV_EVENT_ALL, NULL);
    state_btn_ctrl1 = lv_label_create(btn_ctrl1);
    lv_label_set_text(state_btn_ctrl1, "Off");
    lv_obj_center(state_btn_ctrl1);

    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Device 2");
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, -7, 0);

    btn_ctrl2 = lv_btn_create(scr_btn_ctrl);
    lv_obj_set_size(btn_ctrl2, 60, 30);
    lv_obj_align(btn_ctrl2, LV_ALIGN_BOTTOM_RIGHT, 3, 0);
    lv_obj_add_event_cb(btn_ctrl2, btn_event_cb, LV_EVENT_ALL, NULL);
    state_btn_ctrl2 = lv_label_create(btn_ctrl2);
    lv_label_set_text(state_btn_ctrl2, "Off");
    lv_obj_center(state_btn_ctrl2);

    /*Close object room*/
    Room_CloseBtn = lv_btn_create(Room);
    lv_obj_set_size(Room_CloseBtn, 30, 30);
    lv_obj_align(Room_CloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
    lv_obj_add_event_cb(Room_CloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *btnSymbol = lv_label_create(Room_CloseBtn);
    lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
    lv_obj_center(btnSymbol);
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
    UI_Room();
    setStyle();
    makeKeyboard();
    buildStatusBar();
    buildPWMsgBox();
    buildSettings();
    Data_Air_Node_Room_t Value_Air; 
    for(;;){
        lv_task_handler(); //Let the GUI do its work
        vTaskDelay(5);
        if (xSemaphoreTake(xBinaryNoticeReconectWifi, (TickType_t)0)  == pdPASS) {
            lv_obj_add_state(settingWiFiSwitch, LV_STATE_CHECKED);
            lv_event_send(settingWiFiSwitch, LV_EVENT_VALUE_CHANGED, NULL);
            popupMsgBox("Welcome Back!", "Attempts to reconnect to the previously connected network.");
        }

        if (xQueueReceive(xQueueAirSendTFT, &Value_Air,(TickType_t)0) == pdPASS) {
            lv_label_set_text_fmt(data_temperature, "%.1f °C", Value_Air.temperature);
            lv_label_set_text_fmt(data_humidity,    "%.1f %%", Value_Air.humidity);
            lv_label_set_text_fmt(data_co2,         "%.1f ", Value_Air.CO2);
        }

        if (xQueueReceive(xQueueRelaySendTFT, &Value_Relay, (TickType_t)0) == pdPASS) {
            /*Relay 1*/
            if(Value_Relay.Relay1 == 1) {
                lv_label_set_text(state_btn_ctrl1, "On");
                lv_obj_add_style(btn_ctrl1, &btn_style_on, 0);
            } else {
                lv_label_set_text(state_btn_ctrl1, "Off");
                lv_obj_add_style(btn_ctrl1, &btn_style_off, 0);
            }
            /*Relay 2*/
            if (Value_Relay.Relay2 == 1) {
                lv_label_set_text(state_btn_ctrl2, "On");
                lv_obj_add_style(btn_ctrl2, &btn_style_on, 0);
            } else {
                lv_label_set_text(state_btn_ctrl2, "Off");
                lv_obj_add_style(btn_ctrl2, &btn_style_off, 0);
             }
        }

    }
}