#include "Task_TFT.h"

#define WIDTH 320
#define HEIGHT 240

extern lv_style_t border_style;
extern lv_style_t popupBox_style;

extern lv_obj_t *keyboard;
extern lv_obj_t *popupSettings;
extern lv_obj_t *btn_close_popup_setting;
extern lv_obj_t *settingWiFiSwitch;
extern lv_obj_t *WiFiList;

extern lv_obj_t *popup_wifi_enter;
extern lv_obj_t *mBoxTitle;
extern lv_obj_t *mBoxPassword;

extern lv_obj_t *btn_connect_wifi;
extern lv_obj_t *btn_close_popup_wifi_enter;
extern lv_obj_t *popupBox;
extern lv_obj_t *popupBoxCloseBtn;

void UI_Wifi(void) { // Done
    lv_obj_t *label;
    lv_obj_t *labelSetting;
    popupSettings = lv_obj_create(lv_scr_act());
    lv_obj_add_style(popupSettings, &border_style, 0);
    lv_obj_set_size(popupSettings, WIDTH - 50, HEIGHT - 40);
    lv_obj_align(popupSettings, LV_ALIGN_TOP_MID, 0, 35);

    labelSetting = lv_label_create(popupSettings);
    lv_label_set_text(labelSetting, "Settings " LV_SYMBOL_SETTINGS);
    lv_obj_align(labelSetting, LV_ALIGN_TOP_LEFT, 0, 0);

    settingWiFiSwitch = lv_switch_create(popupSettings);
    lv_obj_add_event_cb(settingWiFiSwitch, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align_to(settingWiFiSwitch, labelSetting, LV_ALIGN_TOP_RIGHT, 60, -10);
    lv_obj_add_flag(popupSettings, LV_OBJ_FLAG_HIDDEN);

    WiFiList = lv_list_create(popupSettings);
    lv_obj_set_size(WiFiList, WIDTH - 80, 140);
    lv_obj_align_to(WiFiList, labelSetting, LV_ALIGN_TOP_LEFT, 0, 30);

    btn_close_popup_setting = lv_btn_create(popupSettings);
    lv_obj_set_size(btn_close_popup_setting, 30, 30);
    lv_obj_align(btn_close_popup_setting, LV_ALIGN_TOP_RIGHT, 0, -10);
    lv_obj_add_event_cb(btn_close_popup_setting, btn_event_display_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_close_popup_setting);
    lv_label_set_text(label, LV_SYMBOL_CLOSE);
    lv_obj_center(label);


}

void UI_Wifi_Enter(void) { // Done
    lv_obj_t *label;

    popup_wifi_enter = lv_obj_create(lv_scr_act());
    lv_obj_add_style(popup_wifi_enter, &border_style, 0);
    lv_obj_set_size(popup_wifi_enter, WIDTH * 3 / 4, HEIGHT - 80);
    lv_obj_center(popup_wifi_enter);
    lv_obj_move_background(popup_wifi_enter);

    mBoxTitle = lv_label_create(popup_wifi_enter);
    lv_label_set_text(mBoxTitle, "Select WiFi SSID: DuongTuoi");
    lv_obj_align(mBoxTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    mBoxPassword = lv_textarea_create(popup_wifi_enter);
    lv_obj_set_size(mBoxPassword, WIDTH / 2, 40);
    lv_obj_align_to(mBoxPassword, mBoxTitle, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_textarea_set_placeholder_text(mBoxPassword, "Password?");
    lv_obj_add_event_cb(mBoxPassword, text_input_event_cb, LV_EVENT_ALL, keyboard);

    btn_connect_wifi = lv_btn_create(popup_wifi_enter);
    lv_obj_add_event_cb(btn_connect_wifi, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_connect_wifi, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    label = lv_label_create(btn_connect_wifi);
    lv_label_set_text(label, "Connect");
    lv_obj_center(label);

    btn_close_popup_wifi_enter = lv_btn_create(popup_wifi_enter);
    lv_obj_add_event_cb(btn_close_popup_wifi_enter, btn_event_display_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_close_popup_wifi_enter, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    label = lv_label_create(btn_close_popup_wifi_enter);
    lv_label_set_text(label, "Cancel");
    lv_obj_center(label);
}

void UI_Wifi_Notification(String title, String msg) { //Done
    if(popupBox != NULL) {
        lv_obj_del(popupBox);
    }
    popupBox = lv_obj_create(lv_scr_act());
    lv_obj_add_style(popupBox, &popupBox_style, 0);
    lv_obj_set_size(popupBox, WIDTH * 3 / 4, HEIGHT - 80);
    lv_obj_center(popupBox);

    lv_obj_t *popupTitle = lv_label_create(popupBox);
    lv_label_set_text(popupTitle, title.c_str());
    lv_obj_set_width(popupTitle, WIDTH * 2 / 3 - 50);
    lv_obj_align(popupTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *popupMSG = lv_label_create(popupBox);
    lv_obj_set_width(popupMSG, WIDTH * 2 / 3 - 50);
    lv_label_set_text(popupMSG, msg.c_str());
    lv_obj_align(popupMSG, LV_ALIGN_TOP_LEFT, 0, 40);

    popupBoxCloseBtn = lv_btn_create(popupBox);
    lv_obj_add_event_cb(popupBoxCloseBtn, btn_event_display_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(popupBoxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *btnLabel = lv_label_create(popupBoxCloseBtn);
    lv_label_set_text(btnLabel, "Okay");
    lv_obj_center(btnLabel);
}