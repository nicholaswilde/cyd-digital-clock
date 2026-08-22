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

void ui_init(void) {
    ui_ScreenMain = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_ScreenMain, lv_color_hex(current_colors.mantle), LV_PART_MAIN);
    
    ui_LabelTime = lv_label_create(ui_ScreenMain);
    lv_label_set_text(ui_LabelTime, "--:--:--");
    lv_obj_set_style_text_font(ui_LabelTime, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(ui_LabelTime, LV_ALIGN_CENTER, 0, 0);
    
    lv_scr_load(ui_ScreenMain);
}

void ui_update(void) {
    // any manual LVGL ticks if needed, usually handled by display_update()
}

void ui_sync_toggles(void) {
    // Synchronize hardware/software toggles when interacting with UI
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
