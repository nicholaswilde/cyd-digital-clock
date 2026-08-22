#include "include/ui.h"
#include "include/catppuccin.h"

lv_obj_t* ui_ScreenMain;
lv_obj_t* ui_LabelTime;

CatppuccinColors current_colors = getCatppuccinFlavor(CATPPUCCIN_MOCHA);

void ui_set_theme(int theme_id) {
    current_colors = getCatppuccinFlavor(theme_id);
    if (ui_ScreenMain) {
        lv_obj_set_style_bg_color(ui_ScreenMain, lv_color_hex(current_colors.mantle), LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_ScreenMain, lv_color_hex(current_colors.text), LV_PART_MAIN);
    }
}

lv_obj_t* ui_ScreenSettings;

static void main_screen_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_LONG_PRESSED) {
        lv_scr_load_anim(ui_ScreenSettings, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}

static void settings_back_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_scr_load_anim(ui_ScreenMain, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}

#include "include/settings_manager.h"
extern SettingsManager settings;

lv_obj_t* ui_Switch24Hour;

static void switch_24hr_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * obj = lv_event_get_target(e);
        bool is_checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
        settings.setUse24HourFormat(is_checked);
    }
}

void ui_init(void) {
    // --- Main Screen ---
    ui_ScreenMain = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ui_ScreenMain, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(ui_ScreenMain, lv_color_hex(current_colors.mantle), LV_PART_MAIN);
    lv_obj_add_event_cb(ui_ScreenMain, main_screen_event_cb, LV_EVENT_ALL, NULL);
    
    ui_LabelTime = lv_label_create(ui_ScreenMain);
    lv_label_set_text(ui_LabelTime, "--:--:--");
    lv_obj_set_style_text_font(ui_LabelTime, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(ui_LabelTime, LV_ALIGN_CENTER, 0, 0);
    
    // --- Settings Screen ---
    ui_ScreenSettings = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenSettings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_ScreenSettings, lv_color_hex(current_colors.crust), LV_PART_MAIN);
    
    lv_obj_t* title = lv_label_create(ui_ScreenSettings);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(current_colors.text), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 24 Hour Format Toggle
    lv_obj_t* label_24hr = lv_label_create(ui_ScreenSettings);
    lv_label_set_text(label_24hr, "24 Hour Format");
    lv_obj_set_style_text_color(label_24hr, lv_color_hex(current_colors.text), LV_PART_MAIN);
    lv_obj_align(label_24hr, LV_ALIGN_LEFT_MID, 20, -30);

    ui_Switch24Hour = lv_switch_create(ui_ScreenSettings);
    lv_obj_align(ui_Switch24Hour, LV_ALIGN_RIGHT_MID, -20, -30);
    if (settings.getUse24HourFormat()) {
        lv_obj_add_state(ui_Switch24Hour, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_Switch24Hour, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_Switch24Hour, switch_24hr_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t* back_btn = lv_btn_create(ui_ScreenSettings);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(back_btn, settings_back_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");

    lv_scr_load(ui_ScreenMain);
}

void ui_update(void) {
    // any manual LVGL ticks if needed, usually handled by display_update()
}

void ui_sync_toggles(void) {
    if (ui_Switch24Hour) {
        bool ui_checked = lv_obj_has_state(ui_Switch24Hour, LV_STATE_CHECKED);
        bool settings_checked = settings.getUse24HourFormat();
        if (ui_checked != settings_checked) {
            if (settings_checked) lv_obj_add_state(ui_Switch24Hour, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_Switch24Hour, LV_STATE_CHECKED);
        }
    }
}

void ui_update_time(const char* time_str) {
    if (ui_LabelTime) {
        lv_label_set_text(ui_LabelTime, time_str);
    }
}

void showScreenSaver(void) {
    // Stub
}

void hideScreenSaver(void) {
    // Stub
}
