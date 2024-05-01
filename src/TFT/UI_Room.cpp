#include "Task_TFT.h"

#define WIDTH 320
#define HEIGHT 240

extern lv_style_t border_style;
extern lv_obj_t *popup_room;
extern lv_obj_t *btn_close_popup_room;
extern lv_obj_t *data_temperature;
extern lv_obj_t *data_humidity;
extern lv_obj_t *data_co2;
extern lv_obj_t *btn_ctrl1;
extern lv_obj_t *state_btn_ctrl1;
extern lv_obj_t *btn_ctrl2;
extern lv_obj_t *state_btn_ctrl2;

void UI_Room(void) { // Done
    lv_obj_t *label; 
    /*Create object Room*/
    popup_room = lv_obj_create(lv_scr_act());
    lv_obj_add_style(popup_room, &border_style, 0);
    lv_obj_set_size(popup_room, WIDTH, HEIGHT - 40);
    lv_obj_align(popup_room, LV_ALIGN_TOP_MID, 0, 35);
    label = lv_label_create(popup_room);
    lv_label_set_text(label, "Room " LV_SYMBOL_HOME);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(popup_room, LV_OBJ_FLAG_HIDDEN);

    /*Create a popup temperature inside popup room*/
    lv_obj_t *scr_Temp = lv_obj_create(popup_room);
    lv_obj_add_style(scr_Temp, &border_style, 0);
    lv_obj_set_size(scr_Temp, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_Temp, LV_ALIGN_TOP_LEFT, 0, 25);
    label = lv_label_create(scr_Temp);
    lv_label_set_text(label, "Temperature");
    
    /*Create a image inside poup temperature*/
    LV_IMG_DECLARE(temperature_control);
    lv_obj_t *thememeter_img = lv_img_create(scr_Temp);
    lv_img_set_src(thememeter_img, &temperature_control);
    lv_obj_set_size(thememeter_img, 70, 50);
    lv_obj_align(thememeter_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);

    /*Create a data inside popup temperature*/
    data_temperature = lv_label_create(scr_Temp);
    lv_label_set_text(data_temperature, "NULL");
    lv_obj_align(data_temperature, LV_ALIGN_BOTTOM_RIGHT,0, -15);
    
    /*Create a popup humidity inside popup Room*/
    lv_obj_t *scr_Humi = lv_obj_create(popup_room);
    lv_obj_add_style(scr_Humi, &border_style, 0);
    lv_obj_set_size(scr_Humi, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_Humi, LV_ALIGN_TOP_RIGHT, 0, 25);
    label = lv_label_create(scr_Humi);
    lv_label_set_text(label, "Humidity");
    
    /*Create a image inside popup humidity*/
    LV_IMG_DECLARE(humidity_control);
    lv_obj_t *Humi_img = lv_img_create(scr_Humi);
    lv_img_set_src(Humi_img, &humidity_control);
    lv_obj_set_size(Humi_img, 70, 50);
    lv_obj_align(Humi_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);

    /*Create a data inside popup humidity*/
    data_humidity = lv_label_create(scr_Humi);
    lv_label_set_text(data_humidity, "NULL");
    lv_obj_align(data_humidity, LV_ALIGN_BOTTOM_RIGHT,0, -15);

    /*Create a popup CO2 inside popup popup_room*/
    lv_obj_t *scr_Co2 = lv_obj_create(popup_room);
    lv_obj_add_style(scr_Co2, &border_style, 0);
    lv_obj_set_size(scr_Co2, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_Co2, LV_ALIGN_BOTTOM_LEFT, 0, 105);
    label = lv_label_create(scr_Co2);
    lv_label_set_text(label, "CO2");
    
    /*Create a image inside popup Humidity*/
    LV_IMG_DECLARE(co2_control);
    lv_obj_t *Co2_img = lv_img_create(scr_Co2);
    lv_img_set_src(Co2_img, &co2_control);
    lv_obj_set_size(Co2_img, 70, 50);
    lv_obj_align(Co2_img, LV_ALIGN_BOTTOM_LEFT, -10, 0);

    /*Create a data inside popup Co2*/
    data_co2 = lv_label_create(scr_Co2);
    lv_label_set_text(data_co2, "NULL");
    lv_obj_align(data_co2, LV_ALIGN_BOTTOM_RIGHT,0, -15);

    /*Create a popup cotrol realay inside popup popup_room*/
    lv_obj_t *scr_btn_ctrl = lv_obj_create(popup_room);
    lv_obj_add_style(scr_btn_ctrl, &border_style, 0);
    lv_obj_set_size(scr_btn_ctrl, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_btn_ctrl, LV_ALIGN_BOTTOM_RIGHT, 0, 105);
    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Control");
    
    /*Create a label text "Device 1" inside popup control relay*/
    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Device 1");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, -10, -10);

    /*Create a button control relay 1 inside popup control realay*/
    btn_ctrl1 = lv_btn_create(scr_btn_ctrl);
    lv_obj_set_size(btn_ctrl1, 60, 30);
    lv_obj_align(btn_ctrl1, LV_ALIGN_RIGHT_MID, 0, -10);
    lv_obj_add_event_cb(btn_ctrl1, btn_event_cb, LV_EVENT_ALL, NULL);
    state_btn_ctrl1 = lv_label_create(btn_ctrl1);
    lv_label_set_text(state_btn_ctrl1, "Off");
    lv_obj_center(state_btn_ctrl1);

    /*Create a label text "Device 2" inside popup control relay*/
    label = lv_label_create(scr_btn_ctrl);
    lv_label_set_text(label, "Device 2");
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, -11, -5);

    /*Create a button control relay 2 inside popup control realay*/
    btn_ctrl2 = lv_btn_create(scr_btn_ctrl);
    lv_obj_set_size(btn_ctrl2, 60, 30);
    lv_obj_align(btn_ctrl2, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_add_event_cb(btn_ctrl2, btn_event_cb, LV_EVENT_ALL, NULL);
    state_btn_ctrl2 = lv_label_create(btn_ctrl2);
    lv_label_set_text(state_btn_ctrl2, "Off");
    lv_obj_center(state_btn_ctrl2);

    /*Close object popup_room*/
    btn_close_popup_room = lv_btn_create(popup_room);
    lv_obj_set_size(btn_close_popup_room, 30, 30);
    lv_obj_align(btn_close_popup_room, LV_ALIGN_TOP_RIGHT, 0, -10);
    lv_obj_add_event_cb(btn_close_popup_room, btn_event_display_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *btnSymbol = lv_label_create(btn_close_popup_room);
    lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
    lv_obj_center(btnSymbol);
}