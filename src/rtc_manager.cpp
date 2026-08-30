#include "rtc_manager.h"
#include "config/config.h"
#include <Wire.h>
#include <RTClib.h>
#include <sys/time.h>

#ifndef NATIVE_TEST
static RTC_DS3231 rtc;
#endif

bool RtcManager::_available = false;

bool RtcManager::begin() {
#ifdef NATIVE_TEST
    return true;
#else
    if (!USE_DS3231_RTC) return false;

    Wire1.begin(RTC_SDA_PIN, RTC_SCL_PIN);
    
    if (!rtc.begin(&Wire1)) {
        Serial.println("[RTC] Couldn't find DS3231 on I2C.");
        _available = false;
        return false;
    }

    if (rtc.lostPower()) {
        Serial.println("[RTC] DS3231 lost power, needs sync!");
    }

    _available = true;
    Serial.printf("[RTC] DS3231 initialized on I2C (SDA: %d, SCL: %d)\n", RTC_SDA_PIN, RTC_SCL_PIN);
    return true;
#endif
}

bool RtcManager::syncToSystem() {
#ifndef NATIVE_TEST
    if (!_available) return false;
    
    DateTime now = rtc.now();
    if (now.year() < 2024) {
        Serial.println("[RTC] DS3231 time is invalid/uninitialized. Skipping sync to system.");
        return false;
    }
    
    struct tm timeinfo = {0};
    timeinfo.tm_year = now.year() - 1900;
    timeinfo.tm_mon = now.month() - 1;
    timeinfo.tm_mday = now.day();
    timeinfo.tm_hour = now.hour();
    timeinfo.tm_min = now.minute();
    timeinfo.tm_sec = now.second();

    struct timeval tv;
    tv.tv_sec = mktime(&timeinfo);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    
    Serial.println("[RTC] System time synchronized from DS3231.");
    return true;
#else
    return false;
#endif
}

bool RtcManager::syncFromSystem() {
#ifndef NATIVE_TEST
    if (!_available) return false;
    
    time_t now;
    time(&now);
    
    if (now < 100000) {
        return false;
    }

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    Serial.println("[RTC] DS3231 synchronized from System time.");
    return true;
#else
    return false;
#endif
}

bool RtcManager::isAvailable() {
    return _available;
}
