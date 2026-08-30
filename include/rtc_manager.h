#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>

class RtcManager {
public:
    static bool begin();
    static bool syncToSystem();
    static bool syncFromSystem();
    static bool isAvailable();
private:
    static bool _available;
};

#endif
