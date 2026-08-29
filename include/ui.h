#ifndef UI_H
#define UI_H

#include <lvgl.h>

extern lv_obj_t* ui_ScreenMain;
extern lv_obj_t* ui_LabelTime;

extern lv_obj_t* ui_ScreenSettings;

void ui_init(void);
void ui_update(void);
void ui_set_theme(int theme_id);
void ui_sync_toggles(void);
void ui_update_time(const char* time_str);
void showScreenSaver(void);
void hideScreenSaver(void);

void ui_show_ap_mode(const char* apSSID);
void ui_hide_ap_mode(void);

#endif // UI_H
