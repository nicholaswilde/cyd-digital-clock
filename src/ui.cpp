#define lv_color_16(c) ((lv_color_t){.full = (c)})
#include "include/ui.h"
#include "include/screensaver_manager.h"
#include "include/rtc_manager.h"
#include "include/catppuccin.h"
#include "include/settings_manager.h"
#include "include/wifi_manager.h"
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>

extern SettingsManager settings;
extern WifiManager wifi;

lv_obj_t* ui_ScreenMain;
lv_obj_t* ui_LabelTime;
lv_obj_t* ui_ScreenSettings;

static lv_obj_t* ui_TimezoneDropdown = nullptr;
static lv_obj_t* ui_SwitchAutoBrightness = nullptr;
static lv_obj_t* ui_Switch24Hour = nullptr;
static lv_obj_t* ui_SwitchShowSeconds = nullptr;
static lv_obj_t* ui_BrightnessSlider = nullptr;
static lv_obj_t* ui_LedSwitch = nullptr;
static lv_obj_t* ui_LedBrightnessSlider = nullptr;
static lv_obj_t* ui_WifiIcon = nullptr;
static lv_obj_t* ui_WifiModal = nullptr;
static lv_obj_t* ui_SwitchWifiEnabled = nullptr;
static lv_obj_t* ui_SwitchMqttEnabled = nullptr;
static lv_obj_t* ui_SwitchRtc = nullptr;

static lv_obj_t* ui_SliderHour = nullptr;
static lv_obj_t* ui_LabelHourVal = nullptr;
static lv_obj_t* ui_BtnHourDec = nullptr;
static lv_obj_t* ui_BtnHourInc = nullptr;
static lv_obj_t* ui_SliderMinute = nullptr;
static lv_obj_t* ui_LabelMinuteVal = nullptr;
static lv_obj_t* ui_BtnMinDec = nullptr;
static lv_obj_t* ui_BtnMinInc = nullptr;

CatppuccinColors current_colors = getCatppuccinFlavor(CATPPUCCIN_MOCHA);


static void theme_dropdown_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * dropdown = lv_event_get_target(e);
        uint16_t opt = lv_dropdown_get_selected(dropdown);
        settings.setThemeFlavor(opt + 1); // 1 = Mocha, 2 = Macchiato, 3 = Frappe, 4 = Latte
        settings.setChanged();
    }
}

static void timezone_dropdown_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * dropdown = lv_event_get_target(e);
        uint16_t opt = lv_dropdown_get_selected(dropdown);
        const char* tz_opts[] = {"UTC", "America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles", "America/Anchorage", "Pacific/Honolulu", "Europe/London", "Europe/Paris", "Asia/Tokyo", "Asia/Shanghai", "Australia/Sydney"};
        if (opt < 12) {
            settings.setTimezone(tz_opts[opt]);
            settings.setChanged();
        }
    }
}

void ui_set_theme(int theme_id) {
    current_colors = getCatppuccinFlavor(theme_id);
    
    lv_obj_t* old_main = ui_ScreenMain;
    lv_obj_t* old_settings = ui_ScreenSettings;
    
    bool on_settings = false;
    if (lv_scr_act() == old_settings && old_settings != nullptr) {
        on_settings = true;
    }

    // Backup old pointers to avoid dangling usage before they are initialized
    ui_ScreenMain = nullptr;
    ui_ScreenSettings = nullptr;

    ui_WifiIcon = nullptr;
    ui_WifiModal = nullptr;
    ui_LabelTime = nullptr;

    ui_TimezoneDropdown = nullptr;
    ui_SwitchAutoBrightness = nullptr;
    ui_Switch24Hour = nullptr;
    ui_SwitchShowSeconds = nullptr;
    ui_BrightnessSlider = nullptr;
    ui_LedSwitch = nullptr;
    ui_LedBrightnessSlider = nullptr;

    ui_init(); 
    ui_sync_toggles();

    if (on_settings) {
        lv_scr_load_anim(ui_ScreenSettings, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
    
    if (old_main) lv_obj_del(old_main);
    if (old_settings) lv_obj_del(old_settings);
}

static void main_screen_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
        lv_obj_clear_state(lv_event_get_target(e), LV_STATE_PRESSED);
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 100)) {
            if (ui_SliderHour && ui_LabelHourVal) {
                lv_slider_set_value(ui_SliderHour, timeinfo.tm_hour, LV_ANIM_OFF);
                char hbuf[16];
                snprintf(hbuf, sizeof(hbuf), "Hour: %02d", timeinfo.tm_hour);
                lv_label_set_text(ui_LabelHourVal, hbuf);
            }
            if (ui_SliderMinute && ui_LabelMinuteVal) {
                lv_slider_set_value(ui_SliderMinute, timeinfo.tm_min, LV_ANIM_OFF);
                char mbuf[16];
                snprintf(mbuf, sizeof(mbuf), "Minute: %02d", timeinfo.tm_min);
                lv_label_set_text(ui_LabelMinuteVal, mbuf);
            }
        }
        
        lv_scr_load_anim(ui_ScreenSettings, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}


static void close_wifi_info_cb(lv_event_t * e) {
    if (ui_WifiModal != nullptr) {
        lv_obj_del(ui_WifiModal);
        ui_WifiModal = nullptr;
    }
}

static void wifi_icon_click_cb(lv_event_t * e) {
    if (ui_WifiModal != nullptr) return;

    ui_WifiModal = lv_obj_create(ui_ScreenSettings);
    lv_obj_set_size(ui_WifiModal, 320, 240);
    lv_obj_align(ui_WifiModal, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ui_WifiModal, lv_color_16(current_colors.base), 0);
    lv_obj_set_style_border_width(ui_WifiModal, 0, 0);
    lv_obj_clear_flag(ui_WifiModal, LV_OBJ_FLAG_SCROLLABLE);

    // Title label
    lv_obj_t * lbl_title = lv_label_create(ui_WifiModal);
    lv_label_set_text(lbl_title, "WiFi Info");
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_16(current_colors.text), 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 5);

    // Status label
    lv_obj_t * lbl_status = lv_label_create(ui_WifiModal);
    WifiState state = wifi.getState();
    const char* statusStr = "Unknown";
    uint16_t statusColor = current_colors.text;
    if (state == WIFI_STATE_CONNECTED) {
        statusStr = "Connected";
        statusColor = current_colors.green;
    } else if (state == WIFI_STATE_CONNECTING) {
        statusStr = "Connecting";
        statusColor = current_colors.yellow;
    } else if (state == WIFI_STATE_AP_MODE) {
        statusStr = "AP Mode";
        statusColor = current_colors.mauve;
    } else {
        statusStr = "Disconnected";
        statusColor = current_colors.red;
    }
    
    lv_label_set_text(lbl_status, statusStr);
    lv_obj_set_style_text_color(lbl_status, lv_color_16(statusColor), 0);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 30);

    // Info details
    lv_obj_t * lbl_info = lv_label_create(ui_WifiModal);
#ifndef NATIVE_TEST
    char infoBuf[256];
    snprintf(infoBuf, sizeof(infoBuf), "SSID: %s\nIP: %s\nHost: %s\nMAC: %s\nRSSI: %d dBm", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.getHostname(), WiFi.macAddress().c_str(), WiFi.RSSI());
    lv_label_set_text(lbl_info, infoBuf);
#else
    lv_label_set_text(lbl_info, "SSID: Test\nIP: 192.168.1.2\nHost: ESP32\nMAC: 00:00:00\nRSSI: -50 dBm");
#endif
    lv_obj_set_style_text_align(lbl_info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lbl_info, lv_color_16(current_colors.text), 0);
    lv_obj_align(lbl_info, LV_ALIGN_TOP_MID, 0, 60);

    // Close Button
    lv_obj_t * btn_close = lv_btn_create(ui_WifiModal);
    lv_obj_set_size(btn_close, 100, 32);
    lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn_close, lv_color_16(current_colors.overlay0), 0);
    lv_obj_add_event_cb(btn_close, close_wifi_info_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_close = lv_label_create(btn_close);
    lv_label_set_text(lbl_close, "Close");
    lv_obj_set_style_text_color(lbl_close, lv_color_16(current_colors.crust), 0);
    lv_obj_center(lbl_close);
}
static void settings_back_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_scr_load_anim(ui_ScreenMain, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    }
}


static void switch_mqtt_enabled_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    settings.setMqttEnabled(lv_obj_has_state(sw, LV_STATE_CHECKED));
    settings.setChanged();
}

static void switch_wifi_enabled_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings.setWifiEnabled(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
        settings.setChanged();
    }
}

static void switch_rtc_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings.setUseRtc(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
        settings.setChanged();
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

void update_system_time(int hour, int min) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        if (hour >= 0) timeinfo.tm_hour = hour;
        if (min >= 0) timeinfo.tm_min = min;
        
        struct timeval tv;
        tv.tv_sec = mktime(&timeinfo);
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
#ifndef NATIVE_TEST
        RtcManager::syncFromSystem();
#endif
    }
}

static void hour_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int val = lv_slider_get_value(slider);
        char buf[16];
        snprintf(buf, sizeof(buf), "Hour: %02d", val);
        lv_label_set_text(ui_LabelHourVal, buf);
        update_system_time(val, -1);
    }
}

static void hour_dec_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        int val = lv_slider_get_value(ui_SliderHour);
        if (val > 0) val--;
        else val = 23;
        lv_slider_set_value(ui_SliderHour, val, LV_ANIM_OFF);
        lv_event_send(ui_SliderHour, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void hour_inc_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        int val = lv_slider_get_value(ui_SliderHour);
        if (val < 23) val++;
        else val = 0;
        lv_slider_set_value(ui_SliderHour, val, LV_ANIM_OFF);
        lv_event_send(ui_SliderHour, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void minute_event_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = lv_event_get_target(e);
        int val = lv_slider_get_value(slider);
        char buf[16];
        snprintf(buf, sizeof(buf), "Minute: %02d", val);
        lv_label_set_text(ui_LabelMinuteVal, buf);
        update_system_time(-1, val);
    }
}

static void min_dec_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        int val = lv_slider_get_value(ui_SliderMinute);
        if (val > 0) val--;
        else val = 59;
        lv_slider_set_value(ui_SliderMinute, val, LV_ANIM_OFF);
        lv_event_send(ui_SliderMinute, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void min_inc_cb(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        int val = lv_slider_get_value(ui_SliderMinute);
        if (val < 59) val++;
        else val = 0;
        lv_slider_set_value(ui_SliderMinute, val, LV_ANIM_OFF);
        lv_event_send(ui_SliderMinute, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void ui_init(void) {
    // --- Main Screen ---
    ui_ScreenMain = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ui_ScreenMain, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(ui_ScreenMain, lv_color_16(current_colors.mantle), LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_ScreenMain, lv_color_16(current_colors.text), LV_PART_MAIN);
    lv_obj_add_event_cb(ui_ScreenMain, main_screen_event_cb, LV_EVENT_ALL, NULL);
    
    ui_LabelTime = lv_label_create(ui_ScreenMain);
    lv_label_set_text(ui_LabelTime, "--:--:--");
    lv_obj_set_style_text_font(ui_LabelTime, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(ui_LabelTime, LV_ALIGN_CENTER, 0, 0);
    
    // --- Settings Screen ---
    ui_ScreenSettings = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_ScreenSettings, lv_color_16(current_colors.crust), LV_PART_MAIN);
    
    // Title
    lv_obj_t* title = lv_label_create(ui_ScreenSettings);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_16(current_colors.text), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    // Wifi Icon
    ui_WifiIcon = lv_label_create(ui_ScreenSettings);
    lv_label_set_text(ui_WifiIcon, LV_SYMBOL_WIFI);
    lv_obj_align(ui_WifiIcon, LV_ALIGN_TOP_RIGHT, -15, 15);
    lv_obj_add_flag(ui_WifiIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(ui_WifiIcon, 15);
    lv_obj_add_event_cb(ui_WifiIcon, wifi_icon_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_color(ui_WifiIcon, lv_color_16(current_colors.red), 0);


    // Create a flex container for settings items
    lv_obj_t* cont = lv_obj_create(ui_ScreenSettings);
    lv_obj_set_size(cont, lv_pct(100), 160); // Fits perfectly
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_opa(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 8, LV_PART_MAIN);

    // 0b. Theme Flavor
    lv_obj_t* row0 = lv_obj_create(cont);
    lv_obj_set_width(row0, lv_pct(90));
    lv_obj_set_height(row0, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row0, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row0, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row0, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row0, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_theme = lv_label_create(row0);
    lv_label_set_text(label_theme, "Theme");
    lv_obj_set_style_text_color(label_theme, lv_color_16(current_colors.text), LV_PART_MAIN);

    lv_obj_t* theme_dropdown = lv_dropdown_create(row0);
    lv_dropdown_set_options(theme_dropdown, "Mocha\nMacchiato\nFrappe\nLatte");
    lv_dropdown_set_selected(theme_dropdown, settings.getThemeFlavor() - 1);
    lv_obj_add_event_cb(theme_dropdown, theme_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_width(theme_dropdown, 130);
    lv_obj_set_style_bg_color(theme_dropdown, lv_color_16(current_colors.crust), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(theme_dropdown, lv_color_16(current_colors.text), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(theme_dropdown, lv_color_16(current_colors.overlay0), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 0c. Timezone
    lv_obj_t* row0c = lv_obj_create(cont);
    lv_obj_set_width(row0c, lv_pct(90));
    lv_obj_set_height(row0c, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row0c, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row0c, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row0c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row0c, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_tz = lv_label_create(row0c);
    lv_label_set_text(label_tz, "Timezone");
    lv_obj_set_style_text_color(label_tz, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_TimezoneDropdown = lv_dropdown_create(row0c);
    lv_dropdown_set_options(ui_TimezoneDropdown, "UTC\nAmerica/New_York\nAmerica/Chicago\nAmerica/Denver\nAmerica/Los_Angeles\nAmerica/Anchorage\nPacific/Honolulu\nEurope/London\nEurope/Paris\nAsia/Tokyo\nAsia/Shanghai\nAustralia/Sydney");
    
    // Find matching timezone index or default to 0
    uint16_t tz_idx = 0;
    const char* cur_tz = settings.getTimezone().c_str();
    const char* tz_opts[] = {"UTC", "America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles", "America/Anchorage", "Pacific/Honolulu", "Europe/London", "Europe/Paris", "Asia/Tokyo", "Asia/Shanghai", "Australia/Sydney"};
    for (int i=0; i<12; i++) {
        if (strcmp(cur_tz, tz_opts[i]) == 0) {
            tz_idx = i;
            break;
        }
    }
    lv_dropdown_set_selected(ui_TimezoneDropdown, tz_idx);
    lv_obj_add_event_cb(ui_TimezoneDropdown, timezone_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_width(ui_TimezoneDropdown, 160);
    lv_obj_set_style_bg_color(ui_TimezoneDropdown, lv_color_16(current_colors.crust), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_TimezoneDropdown, lv_color_16(current_colors.text), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_TimezoneDropdown, lv_color_16(current_colors.overlay0), LV_PART_MAIN | LV_STATE_DEFAULT);

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
    lv_obj_set_style_text_color(label_24hr, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_Switch24Hour = lv_switch_create(row1);
    if (settings.getUse24HourFormat()) lv_obj_add_state(ui_Switch24Hour, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_Switch24Hour, switch_24hr_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

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
    lv_obj_set_style_text_color(label_show_secs, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_SwitchShowSeconds = lv_switch_create(row1_2);
    if (settings.getShowSeconds()) lv_obj_add_state(ui_SwitchShowSeconds, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchShowSeconds, switch_show_seconds_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

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
    lv_obj_set_style_text_color(label_auto, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_SwitchAutoBrightness = lv_switch_create(row1_5);
    if (settings.getAutoBrightness()) lv_obj_add_state(ui_SwitchAutoBrightness, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchAutoBrightness, switch_auto_bright_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 0. Enable WiFi
    lv_obj_t* row_wifi_en = lv_obj_create(cont);
    lv_obj_set_width(row_wifi_en, lv_pct(90));
    lv_obj_set_height(row_wifi_en, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_wifi_en, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row_wifi_en, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row_wifi_en, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_wifi_en, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_wifi_en = lv_label_create(row_wifi_en);
    lv_label_set_text(label_wifi_en, "Enable WiFi");
    lv_obj_set_style_text_color(label_wifi_en, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_SwitchWifiEnabled = lv_switch_create(row_wifi_en);
    if (settings.getWifiEnabled()) lv_obj_add_state(ui_SwitchWifiEnabled, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchWifiEnabled, switch_wifi_enabled_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 0c. Enable MQTT
    lv_obj_t* row_mqtt_en = lv_obj_create(cont);
    lv_obj_set_width(row_mqtt_en, lv_pct(90));
    lv_obj_set_height(row_mqtt_en, LV_SIZE_CONTENT);
    lv_obj_clear_flag(row_mqtt_en, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(row_mqtt_en, 5, 0);
    lv_obj_set_style_bg_opa(row_mqtt_en, 0, 0);
    lv_obj_set_style_border_width(row_mqtt_en, 0, 0);
    lv_obj_set_layout(row_mqtt_en, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row_mqtt_en, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_mqtt_en, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* label_mqtt_en = lv_label_create(row_mqtt_en);
    lv_label_set_text(label_mqtt_en, "Enable MQTT");
    lv_obj_set_style_text_color(label_mqtt_en, lv_color_hex(current_colors.text), 0);
    
    ui_SwitchMqttEnabled = lv_switch_create(row_mqtt_en);
    if (settings.getMqttEnabled()) lv_obj_add_state(ui_SwitchMqttEnabled, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchMqttEnabled, switch_mqtt_enabled_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 0d. Enable Hardware RTC
    lv_obj_t* row_rtc_en = lv_obj_create(cont);
    lv_obj_set_width(row_rtc_en, lv_pct(90));
    lv_obj_set_height(row_rtc_en, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_rtc_en, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row_rtc_en, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row_rtc_en, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_rtc_en, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label_rtc_en = lv_label_create(row_rtc_en);
    lv_label_set_text(label_rtc_en, "Enable Hardware RTC");
    lv_obj_set_style_text_color(label_rtc_en, lv_color_16(current_colors.text), LV_PART_MAIN);

    ui_SwitchRtc = lv_switch_create(row_rtc_en);
    if (settings.getUseRtc()) lv_obj_add_state(ui_SwitchRtc, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_SwitchRtc, switch_rtc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 1.6 Set Hour
    lv_obj_t* row1_6 = lv_obj_create(cont);
    lv_obj_set_width(row1_6, lv_pct(90));
    lv_obj_set_height(row1_6, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1_6, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row1_6, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row1_6, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row1_6, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row1_6, 5, LV_PART_MAIN);

    ui_LabelHourVal = lv_label_create(row1_6);
    lv_label_set_text(ui_LabelHourVal, "Hour: 12");
    lv_obj_set_style_text_color(ui_LabelHourVal, lv_color_16(current_colors.text), LV_PART_MAIN);

    lv_obj_t* hour_ctrl = lv_obj_create(row1_6);
    lv_obj_set_width(hour_ctrl, lv_pct(100));
    lv_obj_set_height(hour_ctrl, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(hour_ctrl, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(hour_ctrl, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(hour_ctrl, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hour_ctrl, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(hour_ctrl, 0, LV_PART_MAIN);

    ui_BtnHourDec = lv_btn_create(hour_ctrl);
    lv_obj_set_size(ui_BtnHourDec, 30, 30);
    lv_obj_t* lbl_h_dec = lv_label_create(ui_BtnHourDec);
    lv_label_set_text(lbl_h_dec, "<");
    lv_obj_center(lbl_h_dec);
    lv_obj_add_event_cb(ui_BtnHourDec, hour_dec_cb, LV_EVENT_CLICKED, NULL);

    ui_SliderHour = lv_slider_create(hour_ctrl);
    lv_slider_set_range(ui_SliderHour, 0, 23);
    lv_obj_set_width(ui_SliderHour, lv_pct(60));
    lv_obj_add_event_cb(ui_SliderHour, hour_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    ui_BtnHourInc = lv_btn_create(hour_ctrl);
    lv_obj_set_size(ui_BtnHourInc, 30, 30);
    lv_obj_t* lbl_h_inc = lv_label_create(ui_BtnHourInc);
    lv_label_set_text(lbl_h_inc, ">");
    lv_obj_center(lbl_h_inc);
    lv_obj_add_event_cb(ui_BtnHourInc, hour_inc_cb, LV_EVENT_CLICKED, NULL);

    // 1.7 Set Minute
    lv_obj_t* row1_7 = lv_obj_create(cont);
    lv_obj_set_width(row1_7, lv_pct(90));
    lv_obj_set_height(row1_7, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1_7, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row1_7, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row1_7, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row1_7, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row1_7, 5, LV_PART_MAIN);

    ui_LabelMinuteVal = lv_label_create(row1_7);
    lv_label_set_text(ui_LabelMinuteVal, "Minute: 00");
    lv_obj_set_style_text_color(ui_LabelMinuteVal, lv_color_16(current_colors.text), LV_PART_MAIN);

    lv_obj_t* min_ctrl = lv_obj_create(row1_7);
    lv_obj_set_width(min_ctrl, lv_pct(100));
    lv_obj_set_height(min_ctrl, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(min_ctrl, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(min_ctrl, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(min_ctrl, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(min_ctrl, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(min_ctrl, 0, LV_PART_MAIN);

    ui_BtnMinDec = lv_btn_create(min_ctrl);
    lv_obj_set_size(ui_BtnMinDec, 30, 30);
    lv_obj_t* lbl_m_dec = lv_label_create(ui_BtnMinDec);
    lv_label_set_text(lbl_m_dec, "<");
    lv_obj_center(lbl_m_dec);
    lv_obj_add_event_cb(ui_BtnMinDec, min_dec_cb, LV_EVENT_CLICKED, NULL);

    ui_SliderMinute = lv_slider_create(min_ctrl);
    lv_slider_set_range(ui_SliderMinute, 0, 59);
    lv_obj_set_width(ui_SliderMinute, lv_pct(60));
    lv_obj_add_event_cb(ui_SliderMinute, minute_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    ui_BtnMinInc = lv_btn_create(min_ctrl);
    lv_obj_set_size(ui_BtnMinInc, 30, 30);
    lv_obj_t* lbl_m_inc = lv_label_create(ui_BtnMinInc);
    lv_label_set_text(lbl_m_inc, ">");
    lv_obj_center(lbl_m_inc);
    lv_obj_add_event_cb(ui_BtnMinInc, min_inc_cb, LV_EVENT_CLICKED, NULL);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        lv_slider_set_value(ui_SliderHour, timeinfo.tm_hour, LV_ANIM_OFF);
        char hbuf[16];
        snprintf(hbuf, sizeof(hbuf), "Hour: %02d", timeinfo.tm_hour);
        lv_label_set_text(ui_LabelHourVal, hbuf);

        lv_slider_set_value(ui_SliderMinute, timeinfo.tm_min, LV_ANIM_OFF);
        char mbuf[16];
        snprintf(mbuf, sizeof(mbuf), "Minute: %02d", timeinfo.tm_min);
        lv_label_set_text(ui_LabelMinuteVal, mbuf);
    } else {
        lv_slider_set_value(ui_SliderHour, 12, LV_ANIM_OFF);
        lv_slider_set_value(ui_SliderMinute, 0, LV_ANIM_OFF);
    }

    if (settings.getWifiEnabled() && settings.getWifiSSID().length() > 0 && settings.getNtpServer().length() > 0) {
        lv_obj_add_state(ui_SliderHour, LV_STATE_DISABLED);
        lv_obj_add_state(ui_BtnHourDec, LV_STATE_DISABLED);
        lv_obj_add_state(ui_BtnHourInc, LV_STATE_DISABLED);
        lv_obj_add_state(ui_SliderMinute, LV_STATE_DISABLED);
        lv_obj_add_state(ui_BtnMinDec, LV_STATE_DISABLED);
        lv_obj_add_state(ui_BtnMinInc, LV_STATE_DISABLED);
    }

    // 2. Screen Brightness
    lv_obj_t* row2 = lv_obj_create(cont);
    lv_obj_set_width(row2, lv_pct(90));
    lv_obj_set_height(row2, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row2, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row2, 5, LV_PART_MAIN);

    lv_obj_t* label_bright = lv_label_create(row2);
    char slider_buf[32];
    snprintf(slider_buf, sizeof(slider_buf), "Bright: %d%%", settings.getBrightness());
    lv_label_set_text(label_bright, slider_buf);
    lv_obj_set_style_text_color(label_bright, lv_color_16(current_colors.text), LV_PART_MAIN);

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
    lv_obj_set_style_text_color(label_led, lv_color_16(current_colors.text), LV_PART_MAIN);

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
    lv_obj_set_style_pad_row(row4, 5, LV_PART_MAIN);

    lv_obj_t* label_led_bright = lv_label_create(row4);
    int led_pct = (settings.getLedBrightness() * 100) / 255;
    snprintf(slider_buf, sizeof(slider_buf), "LED: %d%%", led_pct);
    lv_label_set_text(label_led_bright, slider_buf);
    lv_obj_set_style_text_color(label_led_bright, lv_color_16(current_colors.text), LV_PART_MAIN);

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
    if (ui_TimezoneDropdown) {
        uint16_t tz_idx = 0;
        const char* cur_tz = settings.getTimezone().c_str();
        const char* tz_opts[] = {"UTC", "America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles", "America/Anchorage", "Pacific/Honolulu", "Europe/London", "Europe/Paris", "Asia/Tokyo", "Asia/Shanghai", "Australia/Sydney"};
        for (int i=0; i<12; i++) {
            if (strcmp(cur_tz, tz_opts[i]) == 0) {
                tz_idx = i;
                break;
            }
        }
        if (lv_dropdown_get_selected(ui_TimezoneDropdown) != tz_idx) {
            lv_dropdown_set_selected(ui_TimezoneDropdown, tz_idx);
        }
    }
    if (ui_SwitchRtc) {
        bool ui_checked = lv_obj_has_state(ui_SwitchRtc, LV_STATE_CHECKED);
        if (ui_checked != settings.getUseRtc()) {
            if (settings.getUseRtc()) lv_obj_add_state(ui_SwitchRtc, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_SwitchRtc, LV_STATE_CHECKED);
        }
    }
    if (ui_SwitchMqttEnabled) {
        bool ui_checked = lv_obj_has_state(ui_SwitchMqttEnabled, LV_STATE_CHECKED);
        if (ui_checked != settings.getMqttEnabled()) {
            if (settings.getMqttEnabled()) lv_obj_add_state(ui_SwitchMqttEnabled, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_SwitchMqttEnabled, LV_STATE_CHECKED);
        }
    }
    if (ui_SwitchWifiEnabled) {
        bool ui_checked = lv_obj_has_state(ui_SwitchWifiEnabled, LV_STATE_CHECKED);
        if (ui_checked != settings.getWifiEnabled()) {
            if (settings.getWifiEnabled()) lv_obj_add_state(ui_SwitchWifiEnabled, LV_STATE_CHECKED);
            else lv_obj_clear_state(ui_SwitchWifiEnabled, LV_STATE_CHECKED);
        }
    }
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

    if (ui_SliderHour) {
        bool disable_time = (settings.getWifiEnabled() && settings.getWifiSSID().length() > 0 && settings.getNtpServer().length() > 0);
        if (disable_time) {
            lv_obj_add_state(ui_SliderHour, LV_STATE_DISABLED);
            lv_obj_add_state(ui_BtnHourDec, LV_STATE_DISABLED);
            lv_obj_add_state(ui_BtnHourInc, LV_STATE_DISABLED);
            lv_obj_add_state(ui_SliderMinute, LV_STATE_DISABLED);
            lv_obj_add_state(ui_BtnMinDec, LV_STATE_DISABLED);
            lv_obj_add_state(ui_BtnMinInc, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(ui_SliderHour, LV_STATE_DISABLED);
            lv_obj_clear_state(ui_BtnHourDec, LV_STATE_DISABLED);
            lv_obj_clear_state(ui_BtnHourInc, LV_STATE_DISABLED);
            lv_obj_clear_state(ui_SliderMinute, LV_STATE_DISABLED);
            lv_obj_clear_state(ui_BtnMinDec, LV_STATE_DISABLED);
            lv_obj_clear_state(ui_BtnMinInc, LV_STATE_DISABLED);
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

static void ap_disable_wifi_btn_event_cb(lv_event_t * e) {
    settings.setWifiEnabled(false);
    settings.setChanged();
    ui_hide_ap_mode();
}

void ui_show_ap_mode(const char* apSSID) {
    if (!ui_ScreenAP) {
        ui_ScreenAP = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(ui_ScreenAP, lv_color_16(current_colors.base), LV_PART_MAIN);

        // Title label
        lv_obj_t * lbl_title = lv_label_create(ui_ScreenAP);
        lv_label_set_text(lbl_title, "CYD Digital Clock");
        lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(lbl_title, lv_color_16(current_colors.text), 0);
        lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);

        // Status label
        lv_obj_t * lbl_status = lv_label_create(ui_ScreenAP);
        lv_label_set_text(lbl_status, "Captive Portal Active");
        lv_obj_set_style_text_color(lbl_status, lv_color_16(current_colors.mauve), 0);
        lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 50);

        // Info details
        ui_LabelAPSSID = lv_label_create(ui_ScreenAP);
        lv_obj_set_style_text_align(ui_LabelAPSSID, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(ui_LabelAPSSID, lv_color_16(current_colors.text), 0);
        lv_obj_align(ui_LabelAPSSID, LV_ALIGN_CENTER, 0, 10);

        // Restart Button
        lv_obj_t * btn_restart = lv_btn_create(ui_ScreenAP);
        lv_obj_set_size(btn_restart, 200, 50);
        lv_obj_align(btn_restart, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(btn_restart, lv_color_16(current_colors.red), 0);
        lv_obj_add_event_cb(btn_restart, ap_disable_wifi_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * lbl_btn = lv_label_create(btn_restart);
        lv_label_set_text(lbl_btn, "Disable WiFi");
        lv_obj_set_style_text_color(lbl_btn, lv_color_16(current_colors.base), 0);
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

void ui_update_wifi_status(int state) {
    if (!ui_WifiIcon) return;
    
    uint16_t c = current_colors.red;
    if (state == WIFI_STATE_CONNECTED) c = current_colors.green;
    else if (state == WIFI_STATE_CONNECTING) c = current_colors.yellow;
    else if (state == WIFI_STATE_AP_MODE) c = current_colors.mauve;
    
    lv_obj_set_style_text_color(ui_WifiIcon, lv_color_16(c), 0);
}

void setUIActiveTab(int idx) {
    if (idx == 0) {
        lv_scr_load_anim(ui_ScreenMain, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    } else if (idx == 1) {
        lv_scr_load_anim(ui_ScreenSettings, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
}

void setUIOrientation(int rotation) {
    if (rotation >= 0 && rotation < 4) {
        settings.setScreenOrientation(rotation);
        settings.setChanged();
    }
}
