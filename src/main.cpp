#include <Arduino.h>
#include "config/config.h"
#include "include/settings_manager.h"
#include "include/wifi_manager.h"
#include "include/mqtt_manager.h"
#include "include/display.h"
#include "include/touch_manager.h"
#include "include/ui.h"
#include "include/button_manager.h"
#include "include/led_manager.h"
#include "include/backlight_manager.h"
#include "include/screensaver_manager.h"
#include "include/screenshot_manager.h"
#include "include/sd_card_manager.h"
#include <time.h>

SettingsManager settings;
WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
MqttManager mqtt("", 1883, "", "", "");

ButtonManager button(BOOT_BUTTON_PIN);
LedManager led(4, 16, 17);
BacklightManager backlight(TFT_BL);
ScreenSaverManager screensaver(backlight, 60000);

// Add missing variable referenced in original MQTT diagnostics
bool force_mqtt_publish = true;



#include <esp_sntp.h>

uint32_t _lastSyncMillis = 0;
struct timeval _lastSyncTime = {0};
bool _hasNtpSynced = false;

void timeSyncCallback(struct timeval *tv) {
    unsigned long currentMillis = millis();
    if (_lastSyncMillis != 0 && _lastSyncTime.tv_sec != 0) {
        unsigned long elapsedMillis = currentMillis - _lastSyncMillis;
        double elapsedSysSecs = elapsedMillis / 1000.0;
        double actualElapsedSecs = (tv->tv_sec - _lastSyncTime.tv_sec) + 
                                   (tv->tv_usec - _lastSyncTime.tv_usec) / 1000000.0;
        double driftSecs = actualElapsedSecs - elapsedSysSecs;
        if (elapsedSysSecs > 3000.0) {
            double driftPerDay = (driftSecs / elapsedSysSecs) * 86400.0;
            if (abs(driftPerDay) < 300.0) {
                settings.setRtcDrift(driftPerDay);
                settings.setChanged();
                Serial.printf("[System] NTP Sync Drift Calculated: %f sec/day\n", driftPerDay);
            }
        }
    }
    _lastSyncMillis = currentMillis;
    _lastSyncTime = *tv;
    _hasNtpSynced = true;
    Serial.println("[System] NTP Time Synced.");
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n[System] Booting Digital Clock...");

    settings.begin();

    // 1. Storage & Config
    SdCardManager::begin();
    
    // 2. Hardware Initialize
    button.begin();
    led.begin();
    led.setEnabled(settings.getLedEnabled());
    led.setBrightness(settings.getLedBrightness());

    // 3. Display & Touch
    initDisplayAndTouch();
    initLVGL();

    // Must call backlight.begin() AFTER initDisplayAndTouch so TFT library doesn't hijack the PWM pin
    backlight.begin();

    // 4. UI Initialize
    ui_init();
    ui_set_theme(settings.getThemeFlavor());

    // 5. Connect WiFi & Setup Time
    sntp_set_time_sync_notification_cb(timeSyncCallback);
    wifi.setCredentials(settings.getWifiSSID(), settings.getWifiPassword());
    if (settings.getWifiEnabled()) {
        wifi.begin();
    } else {
        wifi.stop();
    }
    
    // Sync time using NTP with POSIX Timezone
    configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());

    // Initialize to 12:00 locally if clock hasn't been set
    time_t now;
    time(&now);
    if (now < 100000) { 
        struct tm timeinfo = {0};
        timeinfo.tm_hour = 12;
        timeinfo.tm_mday = 1;
        timeinfo.tm_year = 124; // 2024
        struct timeval tv;
        tv.tv_sec = mktime(&timeinfo);
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
    }

    // 6. MQTT
    mqtt.onMessage([](String topic, String payload) {
        if (topic.endsWith("command/use_24hr_format")) {
            settings.setUse24HourFormat(payload == "ON");
            settings.setChanged();
        } else if (topic.endsWith("command/show_seconds")) {
            settings.setShowSeconds(payload == "ON");
            settings.setChanged();
        } else if (topic.endsWith("command/led_enabled")) {
            settings.setLedEnabled(payload == "ON");
            settings.setChanged();
        } else if (topic.endsWith("command/led_brightness")) {
            int pct = payload.toInt();
            if (pct < 10) pct = 10;
            if (pct > 100) pct = 100;
            settings.setLedBrightness((pct * 255) / 100);
            settings.setChanged();
                } else if (topic.endsWith("command/theme")) {
            int theme = 1;
            if (payload == "Macchiato") theme = 2;
            else if (payload == "Frappe") theme = 3;
            else if (payload == "Latte") theme = 4;
            settings.setThemeFlavor(theme);
            settings.setChanged();
        } else if (topic.endsWith("command/screen_orientation")) {
            int orient = 1;
            if (payload == "Portrait Rev") orient = 0;
            else if (payload == "Portrait") orient = 2;
            else if (payload == "Landscape Rev") orient = 3;
            settings.setScreenOrientation(orient);
            settings.setChanged();
        } else if (topic.endsWith("command/auto_brightness")) {
            settings.setAutoBrightness(payload == "ON");
            settings.setChanged();
        } else if (topic.endsWith("command/brightness")) {
            int pct = payload.toInt();
            if (pct < 10) pct = 10;
            if (pct > 100) pct = 100;
            settings.setBrightness(pct);
            settings.setAutoBrightness(false); // Manual override disables auto
            settings.setChanged();
        }
    });
    mqtt.begin();

    // Restore saved brightness on boot
    if (!settings.getAutoBrightness()) {
        backlight.setManualBrightness(settings.getBrightness());
    }

    Serial.println("[System] Setup Complete.");
}

void updateTimeUI() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        char timeStr[12]; // HH:MM:SS AM\0
        if (settings.getUse24HourFormat()) {
            if (settings.getShowSeconds()) {
                strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
            } else {
                strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
            }
        } else {
            if (settings.getShowSeconds()) {
                strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);
            } else {
                strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);
            }
        }
        ui_update_time(timeStr);
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Hardware updates
    button.update(currentMillis);
    led.update(currentMillis);

    // Display & UI updates
    lv_timer_handler();
    ui_update();
    screensaver.update(currentMillis);
    
    if (!screensaver.isActive() && settings.getAutoBrightness()) {
        static unsigned long lastBacklightUpdate = 0;
        if (currentMillis - lastBacklightUpdate >= 1000) {
            lastBacklightUpdate = currentMillis;
            uint16_t ldrRaw = analogRead(LDR_PIN);
            backlight.update(ldrRaw);
        }
    }
    // ScreenshotManager::update();

    // Network updates
    if (settings.getWifiEnabled()) {
        wifi.update();
    }
    
    ui_update_wifi_status(settings.getWifiEnabled() ? wifi.getState() : WIFI_STATE_DISCONNECTED);

    // mqtt.update();

    bool isAPMode = (wifi.getState() == WIFI_STATE_AP_MODE);
    static bool wasAPMode = false;
    if (isAPMode && !wasAPMode) {
        ui_show_ap_mode(wifi.getAPSSID().c_str());
        wasAPMode = true;
    } else if (!isAPMode && wasAPMode) {
        ui_hide_ap_mode();
        wasAPMode = false;
    }

    // Sync Settings changes from Web/API -> Device
    if (settings.hasChanged()) {
        Serial.println("[System] Applying updated settings...");
        
        ui_set_theme(settings.getThemeFlavor());
        if (!settings.getAutoBrightness()) {
            backlight.setManualBrightness(settings.getBrightness());
        }
        led.setEnabled(settings.getLedEnabled());
        led.setBrightness(settings.getLedBrightness());
        
        // Re-configure NTP timezone if it changed
        configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());
        
        if (settings.getWifiEnabled()) {
            if (WiFi.getMode() == WIFI_OFF) {
                wifi.setCredentials(settings.getWifiSSID(), settings.getWifiPassword());
                wifi.begin();
            }
        } else {
            if (WiFi.getMode() != WIFI_OFF) {
                wifi.stop();
            }
        }
        
        if (mqtt.isConnected()) {
            mqtt.publish("status", "Settings updated.");
        }
        
        settings.clearChanged();
    }
    
    // UI Interaction -> Settings changes (Bi-directional sync)
    ui_sync_toggles();

    // Time update loop (every 1 second)
    static unsigned long lastTimeUpdate = 0;
    if (currentMillis - lastTimeUpdate >= 1000) {
        lastTimeUpdate = currentMillis;
        updateTimeUI();
    }

    // Micro-tuning loop (every 1 hour)
    static unsigned long lastTuningUpdate = 0;
    if (lastTuningUpdate == 0) lastTuningUpdate = currentMillis;
    if (currentMillis - lastTuningUpdate >= 3600000) {
        lastTuningUpdate = currentMillis;
        if (currentMillis - _lastSyncMillis >= 3600000 || !_hasNtpSynced) {
            float driftPerDay = settings.getRtcDrift();
            if (abs(driftPerDay) > 0.01f) {
                float driftPerHour = driftPerDay / 24.0f;
                struct timeval tv;
                gettimeofday(&tv, NULL);
                double newTime = tv.tv_sec + (tv.tv_usec / 1000000.0) + driftPerHour;
                tv.tv_sec = (time_t)newTime;
                tv.tv_usec = (newTime - tv.tv_sec) * 1000000;
                settimeofday(&tv, NULL);
                Serial.printf("[System] Applied RTC Drift compensation: %f seconds\n", driftPerHour);
            }
        }
    }

    // Publish MQTT Diagnostics and Settings periodically
    static unsigned long lastMqttDiagMillis = 0;
    if (mqtt.isConnected() && (force_mqtt_publish || currentMillis - lastMqttDiagMillis >= 60000 || lastMqttDiagMillis == 0)) {
        force_mqtt_publish = false;
        lastMqttDiagMillis = currentMillis;
        
        mqtt.publish("system/uptime", String(currentMillis / 1000).c_str(), true);
        mqtt.publish("system/free_heap", String(ESP.getFreeHeap()).c_str(), true);
        mqtt.publish("system/wifi_rssi", String(WiFi.RSSI()).c_str(), true);
        mqtt.publish("system/ip", wifi.getIPAddress().c_str(), true);
        mqtt.publish("system/version", "v0.1.0", true);
        mqtt.publish("system/mac", WiFi.macAddress().c_str(), true);
        mqtt.publish("settings/use_24hr_format", settings.getUse24HourFormat() ? "ON" : "OFF", true);
        mqtt.publish("settings/show_seconds", settings.getShowSeconds() ? "ON" : "OFF", true);
        mqtt.publish("settings/led_enabled", settings.getLedEnabled() ? "ON" : "OFF", true);
        mqtt.publish("settings/led_brightness", String((settings.getLedBrightness() * 100) / 255).c_str(), true);
        mqtt.publish("settings/auto_brightness", settings.getAutoBrightness() ? "ON" : "OFF", true);
        mqtt.publish("settings/brightness", String(settings.getBrightness()).c_str(), true);
        
        String themeStr = "Mocha";
        switch (settings.getThemeFlavor()) {
            case 2: themeStr = "Macchiato"; break;
            case 3: themeStr = "Frappe"; break;
            case 4: themeStr = "Latte"; break;
        }
        mqtt.publish("settings/theme", themeStr.c_str(), true);

        String orientStr = "Landscape";
        switch (settings.getScreenOrientation()) {
            case 0: orientStr = "Portrait Rev"; break;
            case 2: orientStr = "Portrait"; break;
            case 3: orientStr = "Landscape Rev"; break;
        }
        mqtt.publish("settings/screen_orientation", orientStr.c_str(), true);
    }
}
