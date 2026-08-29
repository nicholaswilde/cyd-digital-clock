#include "include/ui.h"
#include "include/catppuccin.h"
#include "include/settings_manager.h"
extern SettingsManager settings;

lv_obj_t* ui_ScreenMain;
lv_obj_t* ui_LabelTime;
lv_obj_t* ui_ScreenSettings;

static lv_obj_t* ui_SwitchAutoBrightness = nullptr;
static lv_obj_t* ui_Switch24Hour = nullptr;
static lv_obj_t* ui_SwitchShowSeconds = nullptr;
static lv_obj_t* ui_BrightnessSlider = nullptr;
static lv_obj_t* ui_LedSwitch = nullptr;
static lv_obj_t* ui_LedBrightnessSlider = nullptr;

CatppuccinColors current_colors = getCatppuccinFlavor(CATPPUCCIN_MOCHA);

void ui_set_theme(int theme_id) {
    current_colors = getCatppuccinFlavor(theme_id);
    if (ui_ScreenMain) {
        lv_obj_set_style_bg_color(ui_ScreenMain, lv_color_hex(current_colors.mantle), LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_ScreenMain, lv_color_hex(current_colors.text), LV_PART_MAIN);
    }
}

static void main_screen_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
        lv_obj_clear_state(lv_event_get_target(e), LV_STATE_PRESSED);
        lv_scr_load_anim(ui_ScreenSettings, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}

static void settings_back_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_scr_load_anim(ui_ScreenMain, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}

static void switch_24hr_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings.setUse24HourFormat(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
        settings.setChanged();
    }
}

static void switch_show_seconds_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings.setShowSeconds(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
        settings.setChanged();
    }
}

static void switch_auto_bright_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings.setAutoBrightness(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
        settings.setChanged();
    }
}

static void brightness_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int val = lv_slider_get_value(slider);
        settings.setBrightness(val);
        // Force manual brightness (disable auto)
        settings.setAutoBrightness(false);
        if (ui_SwitchAutoBrightness) {
            lv_obj_clear_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
        }
        settings.setChanged();
        
        lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
        if (label) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Bright: %d%%", val);
            lv_label_set_text(label, buf);
        }
    }
}

static void led_switch_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        bool checked = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
        settings.setLedEnabled(checked);
        settings.setChanged();
        
        lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
        if (slider) {
            if (checked) lv_obj_clear_state(slider, LV_STATE_DISABLED);
            else lv_obj_add_state(slider, LV_STATE_DISABLED);
        }
    }
}

static void led_brightness_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int val = lv_slider_get_value(slider);
        int mapped = (val * 255) / 100;
        settings.setLedBrightness(mapped);
        settings.setChanged();
        
        lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
        if (label) {
            char buf[32];
            snprintf(buf, sizeof(buf), "LED: %d%%", val);
            lv_label_set_text(label, buf);
        }
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
    lv_obj_set_style_bg_color(ui_ScreenSettings, lv_color_hex(current_colors.crust), LV_PART_MAIN);
    
    // Title
    lv_obj_t* title = lv_label_create(ui_ScreenSettings);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(current_colors.text), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Create a flex container for settings items
    lv_obj_t* cont = lv_obj_create(ui_ScreenSettings);
    lv_obj_set_size(cont, lv_pct(100), 160); // Fits perfectly
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_opa(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 15, LV_PART_MAIN);

    // 1. 24 Hour Format
    lv_obj_t* row1 = lv_obj_create(cont);
    lv_obj_set_width(row1, lv_pct(90));
    lv_obj_set_height(row1, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row1, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_24hr = lv_label_create(row1);
    lv_label_set_text(label_24hr, "24 Hour Format");
    lv_obj_set_style_text_color(label_24hr, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_Switch24Hour = lv_switch_create(row1);
    if (settings.getUse24HourFormat()) lv_obj_add_state(ui_Switch24Hour, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_Switch24Hour, switch_24hr_event_cb, LV_EVENT_ALL, NULL);

    // 1.2 Show Seconds
    lv_obj_t* row1_2 = lv_obj_create(cont);
    lv_obj_set_width(row1_2, lv_pct(90));
    lv_obj_set_height(row1_2, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1_2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row1_2, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row1_2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1_2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_show_secs = lv_label_create(row1_2);
    lv_label_set_text(label_show_secs, "Show Seconds");
    lv_obj_set_style_text_color(label_show_secs, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_SwitchShowSeconds = lv_switch_create(row1_2);
    if (settings.getShowSeconds()) lv_obj_add_state(ui_SwitchShowSeconds, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchShowSeconds, switch_show_seconds_event_cb, LV_EVENT_ALL, NULL);

    // 1.5 Auto Brightness
    lv_obj_t* row1_5 = lv_obj_create(cont);
    lv_obj_set_width(row1_5, lv_pct(90));
    lv_obj_set_height(row1_5, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1_5, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row1_5, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row1_5, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1_5, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_auto = lv_label_create(row1_5);
    lv_label_set_text(label_auto, "Auto Brightness");
    lv_obj_set_style_text_color(label_auto, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_SwitchAutoBrightness = lv_switch_create(row1_5);
    if (settings.getAutoBrightness()) lv_obj_add_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchAutoBrightness, switch_auto_bright_event_cb, LV_EVENT_ALL, NULL);

    // 2. Screen Brightness
    lv_obj_t* row2 = lv_obj_create(cont);
    lv_obj_set_width(row2, lv_pct(90));
    lv_obj_set_height(row2, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row2, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row2, 10, LV_PART_MAIN);

    lv_obj_t* label_bright = lv_label_create(row2);
    char slider_buf[32];
    snprintf(slider_buf, sizeof(slider_buf), "Bright: %d%%", settings.getBrightness());
    lv_label_set_text(label_bright, slider_buf);
    lv_obj_set_style_text_color(label_bright, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_BrightnessSlider = lv_slider_create(row2);
    lv_slider_set_range(ui_BrightnessSlider, 10, 100);
    lv_obj_set_width(ui_BrightnessSlider, lv_pct(100));
    lv_slider_set_value(ui_BrightnessSlider, settings.getBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_BrightnessSlider, brightness_event_cb, LV_EVENT_VALUE_CHANGED, label_bright);

    // 3. Status LED Toggle
    lv_obj_t* row3 = lv_obj_create(cont);
    lv_obj_set_width(row3, lv_pct(90));
    lv_obj_set_height(row3, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row3, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row3, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row3, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_led = lv_label_create(row3);
    lv_label_set_text(label_led, "Status LED");
    lv_obj_set_style_text_color(label_led, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_LedSwitch = lv_switch_create(row3);
    if (settings.getLedEnabled()) lv_obj_add_state(ui_LedSwitch, LV_STATE_CHECKED);

    // 4. LED Brightness
    lv_obj_t* row4 = lv_obj_create(cont);
    lv_obj_set_width(row4, lv_pct(90));
    lv_obj_set_height(row4, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row4, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row4, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row4, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row4, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row4, 10, LV_PART_MAIN);

    lv_obj_t* label_led_bright = lv_label_create(row4);
    int led_pct = (settings.getLedBrightness() * 100) / 255;
    snprintf(slider_buf, sizeof(slider_buf), "LED: %d%%", led_pct);
    lv_label_set_text(label_led_bright, slider_buf);
    lv_obj_set_style_text_color(label_led_bright, lv_color_hex(current_colors.text), LV_PART_MAIN);

    ui_LedBrightnessSlider = lv_slider_create(row4);
    lv_slider_set_range(ui_LedBrightnessSlider, 10, 100);
    lv_obj_set_width(ui_LedBrightnessSlider, lv_pct(100));
    lv_slider_set_value(ui_LedBrightnessSlider, led_pct, LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_LedBrightnessSlider, led_brightness_event_cb, LV_EVENT_VALUE_CHANGED, label_led_bright);
    
    // Bind the switch event after slider creation so it can be passed as user_data
    lv_obj_add_event_cb(ui_LedSwitch, led_switch_event_cb, LV_EVENT_VALUE_CHANGED, ui_LedBrightnessSlider);
    if (!settings.getLedEnabled()) {
        lv_obj_add_state(ui_LedBrightnessSlider, LV_STATE_DISABLED);
    }

    // Back Button
    lv_obj_t* back_btn = lv_btn_create(ui_ScreenSettings);
    lv_obj_set_size(back_btn, 80, 25);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_add_event_cb(back_btn, settings_back_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    lv_scr_load(ui_ScreenMain);
}

void ui_update(void) {
}

void ui_sync_toggles(void) {
    if (ui_Switch24Hour) {
        bool ui_checked = lv_obj_has_state(ui_Switch24Hour, LV_STATE_CHECKED);
        if (ui_checked != settings.getUse24HourFormat()) {
            if (settings.getUse24HourFormat()) lv_obj_add_state(ui_Switch24Hour, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_Switch24Hour, LV_STATE_CHECKED);
        }
    }
    if (ui_SwitchShowSeconds) {
        bool ui_checked = lv_obj_has_state(ui_SwitchShowSeconds, LV_STATE_CHECKED);
        if (ui_checked != settings.getShowSeconds()) {
            if (settings.getShowSeconds()) lv_obj_add_state(ui_SwitchShowSeconds, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_SwitchShowSeconds, LV_STATE_CHECKED);
        }
    }
    if (ui_SwitchAutoBrightness) {
        bool ui_checked = lv_obj_has_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
        if (ui_checked != settings.getAutoBrightness()) {
            if (settings.getAutoBrightness()) lv_obj_add_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
        }
    }
    if (ui_BrightnessSlider) {
        if (lv_slider_get_value(ui_BrightnessSlider) != settings.getBrightness()) {
            lv_slider_set_value(ui_BrightnessSlider, settings.getBrightness(), LV_ANIM_OFF);
        }
    }
    if (ui_LedSwitch) {
        bool ui_checked = lv_obj_has_state(ui_LedSwitch, LV_STATE_CHECKED);
        bool set_en = settings.getLedEnabled();
        if (ui_checked != set_en) {
            if (set_en) lv_obj_add_state(ui_LedSwitch, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_LedSwitch, LV_STATE_CHECKED);
        }
        if (ui_LedBrightnessSlider) {
            if (set_en) lv_obj_clear_state(ui_LedBrightnessSlider, LV_STATE_DISABLED);
            else lv_obj_add_state(ui_LedBrightnessSlider, LV_STATE_DISABLED);
        }
    }
    if (ui_LedBrightnessSlider) {
        int set_val = (settings.getLedBrightness() * 100) / 255;
        if (lv_slider_get_value(ui_LedBrightnessSlider) != set_val) {
            lv_slider_set_value(ui_LedBrightnessSlider, set_val, LV_ANIM_OFF);
        }
    }
}

void ui_update_time(const char* time_str) {
    if (ui_LabelTime) lv_label_set_text(ui_LabelTime, time_str);
}

void showScreenSaver(void) {}
void hideScreenSaver(void) {}

static lv_obj_t* ui_ScreenAP = nullptr;
static lv_obj_t* ui_LabelAPSSID = nullptr;

static void ap_restart_btn_event_cb(lv_event_t * e) {
    ESP.restart();
}

void ui_show_ap_mode(const char* apSSID) {
    if (!ui_ScreenAP) {
        ui_ScreenAP = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(ui_ScreenAP, lv_color_hex(current_colors.base), LV_PART_MAIN);

        // Title label
        lv_obj_t * lbl_title = lv_label_create(ui_ScreenAP);
        lv_label_set_text(lbl_title, "CYD Digital Clock");
        lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(lbl_title, lv_color_hex(current_colors.text), 0);
        lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);

        // Status label
        lv_obj_t * lbl_status = lv_label_create(ui_ScreenAP);
        lv_label_set_text(lbl_status, "Captive Portal Active");
        lv_obj_set_style_text_color(lbl_status, lv_color_hex(current_colors.mauve), 0);
        lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 50);

        // Info details
        ui_LabelAPSSID = lv_label_create(ui_ScreenAP);
        lv_obj_set_style_text_align(ui_LabelAPSSID, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(ui_LabelAPSSID, lv_color_hex(current_colors.text), 0);
        lv_obj_align(ui_LabelAPSSID, LV_ALIGN_CENTER, 0, 10);

        // Restart Button
        lv_obj_t * btn_restart = lv_btn_create(ui_ScreenAP);
        lv_obj_set_size(btn_restart, 200, 50);
        lv_obj_align(btn_restart, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(btn_restart, lv_color_hex(current_colors.red), 0);
        lv_obj_add_event_cb(btn_restart, ap_restart_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * lbl_btn = lv_label_create(btn_restart);
        lv_label_set_text(lbl_btn, "Restart Device");
        lv_obj_set_style_text_color(lbl_btn, lv_color_hex(current_colors.base), 0);
        lv_obj_center(lbl_btn);
    }
    
    if (ui_LabelAPSSID) {
        char infoBuf[256];
        snprintf(infoBuf, sizeof(infoBuf), 
                 "SSID: %s\nIP: 192.168.4.1\n\nConnect your device to setup WiFi", 
                 apSSID);
        lv_label_set_text(ui_LabelAPSSID, infoBuf);
    }

    lv_scr_load(ui_ScreenAP);
}

void ui_hide_ap_mode() {
    lv_scr_load(ui_ScreenMain);
}
