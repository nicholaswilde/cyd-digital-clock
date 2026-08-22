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
BacklightManager backlight(21);
ScreenSaverManager screensaver(backlight, 60000);

// Add missing variable referenced in original MQTT diagnostics
bool force_mqtt_publish = true;

void setup() {
    Serial.begin(115200);
    Serial.println("\n[System] Booting Digital Clock...");

    settings.begin();

    // 1. Storage & Config
    SdCardManager::begin();
    
    // 2. Hardware Initialize
    button.begin();
    led.begin();
    backlight.begin();

    // 3. Display & Touch
    initDisplayAndTouch();
    initLVGL();

    // 4. UI Initialize
    ui_init();
    ui_set_theme(settings.getThemeFlavor());

    // 5. Connect WiFi & Setup Time
    wifi.begin();
    
    // Sync time using NTP with POSIX Timezone
    configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());

    // 6. MQTT
    mqtt.onMessage([](String topic, String payload) {
        if (topic.endsWith("command/use_24hr_format")) {
            settings.setUse24HourFormat(payload == "ON");
            settings.setChanged();
        }
    });
    mqtt.begin();

    Serial.println("[System] Setup Complete.");
}

void updateTimeUI() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        char timeStr[12]; // HH:MM:SS AM\0
        if (settings.getUse24HourFormat()) {
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        } else {
            strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);
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
    // ScreenshotManager::update();

    // Network updates
    wifi.update();
    // mqtt.update();

    // Sync Settings changes from Web/API -> Device
    if (settings.hasChanged()) {
        Serial.println("[System] Applying updated settings...");
        
        ui_set_theme(settings.getThemeFlavor());
        backlight.setManualBrightness(settings.getBrightness());
        
        // Re-configure NTP timezone if it changed
        configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());
        
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
    }
}
