#include "settings_manager.h"
#include <Preferences.h>
#include "config/config.h"

#ifndef MQTT_SERVER
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#endif

SettingsManager::SettingsManager() {
    _brightness = 80;
    _autoBrightness = USE_LDR_AUTO_BACKLIGHT;
    _timezone = TIMEZONE_DEFAULT;
    _themeFlavor = DEFAULT_THEME_FLAVOR;
    _screenshotServerEnabled = false; // Default to false
    _apiServerEnabled = API_SERVER_ENABLED;
    _screenOrientation = 1;
    _ledEnabled = LED_ENABLED;
    _ledBrightness = LED_BRIGHTNESS;
    _mqttEnabled = MQTT_ENABLED;
    _mqttServer = MQTT_SERVER;
    _mqttPort = MQTT_PORT;
    _mqttUser = MQTT_USER;
    _mqttPassword = MQTT_PASSWORD;
    _mqttBaseTopic = "cyd/";
    _wifiEnabled = WIFI_ENABLED;
    _wifiSSID = WIFI_SSID;
    _wifiPassword = WIFI_PASSWORD;
    _screensaverEnabled = SCREENSAVER_ENABLED;
    _screensaverTimeout = SCREENSAVER_TIMEOUT_MS;
    _staticIpEnabled = false;
    _staticIp = "";
    _staticGateway = "";
    _staticSubnet = "255.255.255.0";
    _staticDns = "1.1.1.1";
#ifdef AP_PASSWORD
    _apPassword = AP_PASSWORD;
#else
    _apPassword = "";
#endif
    _ntpServer = NTP_SERVER;
    _use24HourFormat = DEFAULT_USE_24HOUR_FORMAT;
    _showSeconds = DEFAULT_SHOW_SECONDS;
    _sleepScheduleEnabled = DEFAULT_SLEEP_SCHEDULE_ENABLED;
    _sleepStartTime = DEFAULT_SLEEP_START_TIME;
    _sleepEndTime = DEFAULT_SLEEP_END_TIME;
    _rtcDrift = 0.0f;
}

void SettingsManager::begin() {
    Preferences prefs;
    prefs.begin("settings", false);
    
    _brightness = prefs.getInt("bright", 80);
    _autoBrightness = prefs.getBool("auto_bright", USE_LDR_AUTO_BACKLIGHT);
    _timezone = prefs.getString("tz", TIMEZONE_DEFAULT);
    _themeFlavor = prefs.getInt("theme", DEFAULT_THEME_FLAVOR);
    _screenshotServerEnabled = prefs.getBool("scr_srv", false);
    _apiServerEnabled = prefs.getBool("api_srv", API_SERVER_ENABLED);
    _screenOrientation = prefs.getInt("screen_rot", 1);
    _ledEnabled = prefs.getBool("led_en", LED_ENABLED);
    _ledBrightness = prefs.getInt("led_bright", LED_BRIGHTNESS);
    _mqttEnabled = prefs.getBool("mqtt_en", MQTT_ENABLED);
    _mqttServer = prefs.getString("mqtt_srv", MQTT_SERVER);
    _mqttPort = prefs.getInt("mqtt_prt", MQTT_PORT);
    _mqttUser = prefs.getString("mqtt_usr", MQTT_USER);
    _mqttPassword = prefs.getString("mqtt_pwd", MQTT_PASSWORD);
    _mqttBaseTopic = prefs.getString("mqtt_base", "cyd/");
    _wifiEnabled = prefs.getBool("wifi_en", WIFI_ENABLED);
    _wifiSSID = prefs.getString("wifi_ssid", WIFI_SSID);
    _wifiPassword = prefs.getString("wifi_pass", WIFI_PASSWORD);
    _screensaverEnabled = prefs.getBool("scr_enabled", SCREENSAVER_ENABLED);
    _screensaverTimeout = prefs.getInt("scr_timeout", SCREENSAVER_TIMEOUT_MS);
    _staticIpEnabled = prefs.getBool("static_en", false);
    _staticIp = prefs.getString("static_ip", "");
    _staticGateway = prefs.getString("static_gw", "");
    _staticSubnet = prefs.getString("static_sn", "255.255.255.0");
    _staticDns = prefs.getString("static_dns", "1.1.1.1");
#ifdef AP_PASSWORD
    _apPassword = prefs.getString("ap_pass", AP_PASSWORD);
#else
    _apPassword = prefs.getString("ap_pass", "");
#endif
    _ntpServer = prefs.getString("ntp_srv", NTP_SERVER);
    _use24HourFormat = prefs.getBool("use_24hr", DEFAULT_USE_24HOUR_FORMAT);
    _showSeconds = prefs.getBool("show_secs", DEFAULT_SHOW_SECONDS);
    _sleepScheduleEnabled = prefs.getBool("sleep_sched", DEFAULT_SLEEP_SCHEDULE_ENABLED);
    _sleepStartTime = prefs.getString("sleep_start", DEFAULT_SLEEP_START_TIME);
    _sleepEndTime = prefs.getString("sleep_end", DEFAULT_SLEEP_END_TIME);
    _rtcDrift = prefs.getFloat("rtc_drift", 0.0f);
    
    prefs.end();
}



int SettingsManager::getBrightness() const {
    return _brightness;
}

void SettingsManager::setBrightness(int brightness) {
    if (brightness < 10) brightness = 10;
    if (brightness > 100) brightness = 100;
    
    if (_brightness != brightness) {
        _brightness = brightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("bright", _brightness);
        prefs.end();
    }
}

bool SettingsManager::getAutoBrightness() const {
    return _autoBrightness;
}

void SettingsManager::setAutoBrightness(bool autoBrightness) {
    if (_autoBrightness != autoBrightness) {
        _autoBrightness = autoBrightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("auto_bright", _autoBrightness);
        prefs.end();
    }
}

const String& SettingsManager::getTimezone() const {
    return _timezone;
}

void SettingsManager::setTimezone(const String& timezone) {
    if (_timezone != timezone) {
        _timezone = timezone;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("tz", _timezone);
        prefs.end();
    }
}

int SettingsManager::getThemeFlavor() const {
    return _themeFlavor;
}

void SettingsManager::setThemeFlavor(int flavor) {
    if (flavor < 1) flavor = 1;
    if (flavor > 4) flavor = 4;

    if (_themeFlavor != flavor) {
        _themeFlavor = flavor;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("theme", _themeFlavor);
        prefs.end();
    }
}

bool SettingsManager::getWifiEnabled() const {
    return _wifiEnabled;
}

void SettingsManager::setWifiEnabled(bool enabled) {
    if (_wifiEnabled != enabled) {
        _wifiEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("wifi_en", _wifiEnabled);
        prefs.end();
    }
}

const String& SettingsManager::getWifiSSID() const {
    return _wifiSSID;
}

void SettingsManager::setWifiSSID(const String& ssid) {
    if (_wifiSSID != ssid) {
        _wifiSSID = ssid;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("wifi_ssid", _wifiSSID);
        prefs.end();
    }
}

const String& SettingsManager::getWifiPassword() const {
    return _wifiPassword;
}

void SettingsManager::setWifiPassword(const String& password) {
    if (_wifiPassword != password) {
        _wifiPassword = password;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("wifi_pass", _wifiPassword);
        prefs.end();
    }
}



bool SettingsManager::getScreenshotServerEnabled() const {
    return _screenshotServerEnabled;
}

void SettingsManager::setScreenshotServerEnabled(bool enabled) {
    if (_screenshotServerEnabled != enabled) {
        _screenshotServerEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("scr_srv", _screenshotServerEnabled);
        prefs.end();
    }
}

bool SettingsManager::getApiServerEnabled() const {
    return _apiServerEnabled;
}

void SettingsManager::setApiServerEnabled(bool enabled) {
    if (_apiServerEnabled != enabled) {
        _apiServerEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("api_srv", _apiServerEnabled);
        prefs.end();
    }
}

int SettingsManager::getScreenOrientation() const {
    return _screenOrientation;
}

void SettingsManager::setScreenOrientation(int orientation) {
    if (orientation < 0) orientation = 0;
    if (orientation > 3) orientation = 3;

    if (_screenOrientation != orientation) {
        _screenOrientation = orientation;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("screen_rot", _screenOrientation);
        prefs.end();
    }
}

bool SettingsManager::getLedEnabled() const {
    return _ledEnabled;
}

void SettingsManager::setLedEnabled(bool enabled) {
    if (_ledEnabled != enabled) {
        _ledEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("led_en", _ledEnabled);
        prefs.end();
    }
}

int SettingsManager::getLedBrightness() const {
    return _ledBrightness;
}

void SettingsManager::setLedBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    if (_ledBrightness != brightness) {
        _ledBrightness = brightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("led_bright", _ledBrightness);
        prefs.end();
    }
}

bool SettingsManager::getMqttEnabled() const {
    return _mqttEnabled;
}

void SettingsManager::setMqttEnabled(bool enabled) {
    if (_mqttEnabled != enabled) {
        _mqttEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("mqtt_en", _mqttEnabled);
        prefs.end();
    }
}



bool SettingsManager::getScreensaverEnabled() const {
    return _screensaverEnabled;
}

void SettingsManager::setScreensaverEnabled(bool enabled) {
    if (_screensaverEnabled != enabled) {
        _screensaverEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("scr_enabled", _screensaverEnabled);
        prefs.end();
    }
}

int SettingsManager::getScreensaverTimeout() const {
    return _screensaverTimeout;
}

void SettingsManager::setScreensaverTimeout(int timeout) {
    if (_screensaverTimeout != timeout) {
        _screensaverTimeout = timeout;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("scr_timeout", _screensaverTimeout);
        prefs.end();
    }
}











const String& SettingsManager::getNtpServer() const {
    return _ntpServer;
}

void SettingsManager::setNtpServer(const String& ntpServer) {
    if (_ntpServer != ntpServer) {
        _ntpServer = ntpServer;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("ntp_srv", _ntpServer);
        prefs.end();
    }
}

const String& SettingsManager::getMqttServer() const { return _mqttServer; }
void SettingsManager::setMqttServer(const String& server) {
    if (_mqttServer != server) {
        _mqttServer = server;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_srv", _mqttServer); prefs.end();
    }
}

int SettingsManager::getMqttPort() const { return _mqttPort; }
void SettingsManager::setMqttPort(int port) {
    if (_mqttPort != port) {
        _mqttPort = port;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putInt("mqtt_prt", _mqttPort); prefs.end();
    }
}

const String& SettingsManager::getMqttUser() const { return _mqttUser; }
void SettingsManager::setMqttUser(const String& user) {
    if (_mqttUser != user) {
        _mqttUser = user;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_usr", _mqttUser); prefs.end();
    }
}

const String& SettingsManager::getMqttPassword() const { return _mqttPassword; }
void SettingsManager::setMqttPassword(const String& password) {
    if (_mqttPassword != password) {
        _mqttPassword = password;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_pwd", _mqttPassword); prefs.end();
    }
}

const String& SettingsManager::getMqttBaseTopic() const {
    return _mqttBaseTopic;
}

void SettingsManager::setMqttBaseTopic(const String& baseTopic) {
    if (_mqttBaseTopic != baseTopic) {
        _mqttBaseTopic = baseTopic;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_base", _mqttBaseTopic); prefs.end();
    }
}



bool SettingsManager::getStaticIpEnabled() const { return _staticIpEnabled; }
void SettingsManager::setStaticIpEnabled(bool enabled) {
    if (_staticIpEnabled != enabled) {
        _staticIpEnabled = enabled;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putBool("static_en", _staticIpEnabled); prefs.end();
    }
}
const String& SettingsManager::getStaticIp() const { return _staticIp; }
void SettingsManager::setStaticIp(const String& ip) {
    if (_staticIp != ip) {
        _staticIp = ip;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_ip", _staticIp); prefs.end();
    }
}
const String& SettingsManager::getStaticGateway() const { return _staticGateway; }
void SettingsManager::setStaticGateway(const String& gateway) {
    if (_staticGateway != gateway) {
        _staticGateway = gateway;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_gw", _staticGateway); prefs.end();
    }
}
const String& SettingsManager::getStaticSubnet() const { return _staticSubnet; }
void SettingsManager::setStaticSubnet(const String& subnet) {
    if (_staticSubnet != subnet) {
        _staticSubnet = subnet;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_sn", _staticSubnet); prefs.end();
    }
}
const String& SettingsManager::getStaticDns() const { return _staticDns; }
void SettingsManager::setStaticDns(const String& dns) {
    if (_staticDns != dns) {
        _staticDns = dns;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_dns", _staticDns); prefs.end();
    }
}
const String& SettingsManager::getApPassword() const { return _apPassword; }
void SettingsManager::setApPassword(const String& password) {
    if (_apPassword != password) {
        _apPassword = password;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("ap_pass", _apPassword); prefs.end();
    }
}

void SettingsManager::factoryReset() {
    Preferences prefs;
    prefs.begin("settings", false);
    prefs.clear();
    prefs.end();
}












bool SettingsManager::getSleepScheduleEnabled() const { return _sleepScheduleEnabled; }
void SettingsManager::setSleepScheduleEnabled(bool enabled) { 
    if (_sleepScheduleEnabled != enabled) {
        _sleepScheduleEnabled = enabled; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("sleep_sched", _sleepScheduleEnabled);
        prefs.end();
    }
}

const String& SettingsManager::getSleepStartTime() const { return _sleepStartTime; }
void SettingsManager::setSleepStartTime(const String& startTime) { 
    if (_sleepStartTime != startTime) {
        _sleepStartTime = startTime; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("sleep_start", _sleepStartTime);
        prefs.end();
    }
}

const String& SettingsManager::getSleepEndTime() const { return _sleepEndTime; }
void SettingsManager::setSleepEndTime(const String& endTime) { 
    if (_sleepEndTime != endTime) {
        _sleepEndTime = endTime; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("sleep_end", _sleepEndTime);
        prefs.end();
    }
}

bool SettingsManager::getUse24HourFormat() const { return _use24HourFormat; }
void SettingsManager::setUse24HourFormat(bool use24Hour) {
    if (_use24HourFormat != use24Hour) {
        _use24HourFormat = use24Hour;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("use_24hr", _use24HourFormat);
        prefs.end();
    }
}

bool SettingsManager::getShowSeconds() const { return _showSeconds; }
void SettingsManager::setShowSeconds(bool showSeconds) {
    if (_showSeconds != showSeconds) {
        _showSeconds = showSeconds;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("show_secs", _showSeconds);
        prefs.end();
    }
}

float SettingsManager::getRtcDrift() const { return _rtcDrift; }
void SettingsManager::setRtcDrift(float rtcDrift) {
    if (_rtcDrift != rtcDrift) {
        _rtcDrift = rtcDrift;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putFloat("rtc_drift", _rtcDrift);
        prefs.end();
    }
}
