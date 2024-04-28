#ifndef _TASK_TFT_H_
#define _TASK_TFT_H_

#include <TFT_eSPI.h>
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

/*Event Button*/
static void btn_event_cb(lv_event_t *e);

/*Event List*/
static void list_event_handler(lv_event_t *e);

/*Event text input*/
static void text_input_event_cb(lv_event_t *e);

/*Setting style for display*/
static void setStyle(void);

/*Setting status bar*/
static void buildStatusBar(void);

/*Setting displays configuration information*/
static void buildSettings(void);

/*Setting box*/
static void buildPWMsgBox(void);

/*Setting popupBox*/
static void popupMsgBox(String title, String msg);

/*Setting Keyboard*/
static void makeKeyboard(void);

static void timerForNetwork(lv_timer_t *timer);

static void showingFoundWiFiList(void) ;

static void updateLocalTime(void);

extern bool Notice_WiFi_Disconnect(void);

void Notice_WiFi_Connected(void);
#endif