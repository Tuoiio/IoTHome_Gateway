#ifndef _TASK_TFT_H_
#define _TASK_TFT_H_

//#include <TFT_eSPI.h>
#include <Arduino.h>
#include <lvgl.h>
#include <Ticker.h>
#include <vector>
#include <Type_Data.h>

#define LVGL_TICK_PERIOD 60



extern void networkConnector(void);
extern void networkScanner(void);


/*Display flushing*/
static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

/*Read the touchpad*/
static void my_touchpad_read (lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

/*Tick Handler LVGL*/
static void lv_tick_handler(void);

extern void btn_event_display_cb(lv_event_t *e);

/*Event Button*/
extern void btn_event_cb(lv_event_t *e);

/*Event List*/
extern void list_event_handler(lv_event_t *e);

/*Event text input*/
extern void text_input_event_cb(lv_event_t *e);

/*Setting style for display*/
static void setStyle(void);

/*Setting user interface*/
void UI_Main(void);

/*Setting displays configuration information*/
void UI_Wifi(void);

/*Setting box*/
void UI_Wifi_Enter(void);

/*Setting popupBox*/
void UI_Wifi_Notification(String title, String msg);

/*Setting UI_Room*/
 void UI_Room(void);

/*Setting UI_Gate House*/
 void UI_GateHouse(void);

 /*Setting Keyboard*/
static void makeKeyboard(void);

static void timerForNetwork(lv_timer_t *timer);

static void showingFoundWiFiList(void) ;

static void updateLocalTime(void);

extern bool Notice_WiFi_Disconnect(void);

void Notice_WiFi_Connected(void);
#endif