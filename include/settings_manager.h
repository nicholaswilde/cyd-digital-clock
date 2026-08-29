#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#ifndef NATIVE_TEST
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

class SettingsManager {
private:
    bool _settingsChanged = false;
    int _brightness;
    bool _autoBrightness;
    String _timezone;
    int _themeFlavor;
    bool _screenshotServerEnabled;
    bool _apiServerEnabled;
    int _screensaverTimeout;
    bool _staticIpEnabled;
    String _staticIp;
    String _staticGateway;
    String _staticSubnet;
    String _staticDns;
    String _apPassword;
    int _screenOrientation;
    bool _ledEnabled;
    int _ledBrightness;
    bool _mqttEnabled;
    String _mqttServer;
    int _mqttPort;
    String _mqttUser;
    String _mqttPassword;
    String _mqttBaseTopic;
    String _wifiSSID;
    String _wifiPassword;
    bool _screensaverEnabled;
    bool _sleepScheduleEnabled;
    String _sleepStartTime;
    String _sleepEndTime;
    String _ntpServer;
    bool _use24HourFormat;
    bool _showSeconds;

public:
    SettingsManager();
    void begin();
    
    bool getUse24HourFormat() const;
    void setUse24HourFormat(bool use24Hour);
    
    bool getShowSeconds() const;
    void setShowSeconds(bool showSeconds);
    
    
    int getBrightness() const;
    void setBrightness(int brightness);
    
    bool getAutoBrightness() const;
    void setAutoBrightness(bool autoBrightness);
    
    const String& getTimezone() const;
    void setTimezone(const String& timezone);
    int getThemeFlavor() const;
    void setThemeFlavor(int flavor);


    bool getScreenshotServerEnabled() const;
    void setScreenshotServerEnabled(bool enabled);

    bool getApiServerEnabled() const;
    void setApiServerEnabled(bool enabled);

    int getScreenOrientation() const;
    void setScreenOrientation(int orientation);

    bool getLedEnabled() const;
    void setLedEnabled(bool enabled);

    int getLedBrightness() const;
    void setLedBrightness(int brightness);

    bool getMqttEnabled() const;
    void setMqttEnabled(bool enabled);

    const String& getMqttServer() const;
    void setMqttServer(const String& server);

    int getMqttPort() const;
    void setMqttPort(int mqttPort);
    const String& getMqttUser() const;
    void setMqttUser(const String& mqttUser);
    const String& getMqttPassword() const;
    void setMqttPassword(const String& mqttPassword);
    const String& getMqttBaseTopic() const;
    void setMqttBaseTopic(const String& mqttBaseTopic);

    const String& getWifiSSID() const;
    void setWifiSSID(const String& ssid);

    const String& getWifiPassword() const;
    void setWifiPassword(const String& password);


    bool getScreensaverEnabled() const;
    void setScreensaverEnabled(bool enabled);

    int getScreensaverTimeout() const;
    void setScreensaverTimeout(int timeout);

    bool getSleepScheduleEnabled() const;
    void setSleepScheduleEnabled(bool enabled);
    const String& getSleepStartTime() const;
    void setSleepStartTime(const String& startTime);
    const String& getSleepEndTime() const;
    void setSleepEndTime(const String& endTime);

    bool getStaticIpEnabled() const;
    void setStaticIpEnabled(bool enabled);
    const String& getStaticIp() const;
    void setStaticIp(const String& ip);
    const String& getStaticGateway() const;
    void setStaticGateway(const String& gateway);
    const String& getStaticSubnet() const;
    void setStaticSubnet(const String& subnet);
    const String& getStaticDns() const;
    void setStaticDns(const String& dns);

    const String& getApPassword() const;
    void setApPassword(const String& password);

    void factoryReset();






    const String& getNtpServer() const;
    void setNtpServer(const String& ntpServer);






    bool hasChanged() const { return _settingsChanged; }
    void clearChanged() { _settingsChanged = false; }
    void setChanged() { _settingsChanged = true; }
};

#endif // SETTINGS_MANAGER_H
