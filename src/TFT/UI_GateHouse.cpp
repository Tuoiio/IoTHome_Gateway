
#include "Task_TFT.h"

#define WIDTH 320
#define HEIGHT 240

extern lv_style_t border_style;
extern lv_obj_t *btn_GateHome;
extern lv_obj_t *GateHome;
extern lv_obj_t *btn_GateHomeClose;

 void UI_GateHouse(void) {
    lv_obj_t *label;
    /*Popup Gate House*/
    GateHome = lv_obj_create(lv_scr_act());
    lv_obj_add_style(GateHome, &border_style, 0);
    lv_obj_set_size(GateHome, WIDTH, HEIGHT - 40);
    lv_obj_align(GateHome, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_add_flag(GateHome, LV_OBJ_FLAG_HIDDEN);
    label = lv_label_create(GateHome);
    lv_label_set_text(label, "Gate House");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    /*Create button close UI Gate House*/
    btn_GateHomeClose = lv_btn_create(GateHome);
    lv_obj_set_size(btn_GateHomeClose, 30, 30);
    lv_obj_align(btn_GateHomeClose, LV_ALIGN_TOP_RIGHT, 0, -10);
    lv_obj_add_event_cb(btn_GateHomeClose, btn_event_display_cb, LV_EVENT_ALL, NULL);
    label = lv_label_create(btn_GateHomeClose);
    lv_label_set_text(label, LV_SYMBOL_CLOSE);
    lv_obj_center(label);

    /*Create a popup status gate inside popup Gate house*/
    lv_obj_t *scr_status_gate = lv_obj_create(GateHome);
    lv_obj_add_style(scr_status_gate, &border_style, 0);
    lv_obj_set_size(scr_status_gate, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_status_gate, LV_ALIGN_TOP_LEFT, 0, 25);
    label = lv_label_create(scr_status_gate);
    lv_label_set_text(label, "Status Gate");

    /*Create a popup control open/close gate inside popup gate house*/

    lv_obj_t *scr_ctrl_gate = lv_obj_create(GateHome);
    lv_obj_add_style(scr_ctrl_gate, &border_style, 0);
    lv_obj_set_size(scr_ctrl_gate, WIDTH - 180, HEIGHT - 120);
    lv_obj_align(scr_ctrl_gate, LV_ALIGN_TOP_RIGHT, 0, 25);
    label = lv_label_create(scr_ctrl_gate);
    lv_label_set_text(label, "Control Gate");
}
