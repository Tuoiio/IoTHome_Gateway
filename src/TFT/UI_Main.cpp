#include "Task_TFT.h"

#define WIDTH 320
#define HEIGHT 240

extern lv_obj_t *timeLabel;
extern lv_obj_t *btn_open_popup_setting;
extern lv_obj_t *btn_open_popup_room;
extern lv_obj_t *btn_open_popup_gatehome;

void UI_Main(void) { //Done
    lv_obj_t *label;
    /*Create status bar*/
    lv_obj_t *statusBar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(statusBar,  WIDTH, 30);
    lv_obj_align(statusBar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_remove_style(statusBar, NULL, LV_PART_SCROLLBAR | LV_STATE_ANY);
    
    /*Create timeLabel, display time when connect wifi*/
    timeLabel = lv_label_create(statusBar);
    lv_obj_set_size(timeLabel,  WIDTH - 50, 30);
    lv_label_set_text(timeLabel, "WiFi Not Connected!  " LV_SYMBOL_CLOSE);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 8, 4);

    /*Create button open popup setting wifi*/
    btn_open_popup_setting = lv_btn_create(statusBar);
    lv_obj_set_size(btn_open_popup_setting, 30, 30);
    lv_obj_align(btn_open_popup_setting, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(btn_open_popup_setting, btn_event_display_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_open_popup_setting);      /*Add a label to the button*/
    lv_label_set_text(label, LV_SYMBOL_SETTINGS);       /*Set the labels text*/
    lv_obj_center(label);

    /*Create a button open popup room*/
    btn_open_popup_room = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_open_popup_room, 100, 50);
    lv_obj_align(btn_open_popup_room, LV_ALIGN_BOTTOM_LEFT, 20, -30);
    lv_obj_add_event_cb(btn_open_popup_room, btn_event_display_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_open_popup_room);
    lv_label_set_text(label, "Room  " LV_SYMBOL_HOME);
    lv_obj_center(label);

    /*Create a button open popup gate house*/
    btn_open_popup_gatehome = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_open_popup_gatehome, 100, 50);
    lv_obj_align(btn_open_popup_gatehome, LV_ALIGN_BOTTOM_RIGHT, -20, -30);
    lv_obj_add_event_cb(btn_open_popup_gatehome, btn_event_display_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_open_popup_gatehome);
    lv_label_set_text(label, "Gate");
    lv_obj_center(label);
}