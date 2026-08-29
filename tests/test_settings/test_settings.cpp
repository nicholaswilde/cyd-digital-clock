#include <unity.h>
#include "../mocks/mocks.cpp"

void setUp(void) {
    // any setup
}

void tearDown(void) {
    // any cleanup
}

void test_settings_default_values(void) {
    SettingsManager settings;
    settings.begin();
    
    // Default values expected:
    TEST_ASSERT_EQUAL(80, settings.getBrightness());
    TEST_ASSERT_EQUAL(USE_LDR_AUTO_BACKLIGHT, settings.getAutoBrightness());
    TEST_ASSERT_EQUAL_STRING(TIMEZONE_DEFAULT, settings.getTimezone().c_str());
    TEST_ASSERT_EQUAL(false, settings.getScreenshotServerEnabled());
    TEST_ASSERT_EQUAL(1, settings.getScreenOrientation());
    TEST_ASSERT_EQUAL(MQTT_ENABLED, settings.getMqttEnabled());
    TEST_ASSERT_EQUAL(SCREENSAVER_ENABLED, settings.getScreensaverEnabled());
    TEST_ASSERT_EQUAL(SCREENSAVER_TIMEOUT_MS, settings.getScreensaverTimeout());
    TEST_ASSERT_EQUAL(API_SERVER_ENABLED, settings.getApiServerEnabled());
    TEST_ASSERT_EQUAL(false, settings.getStaticIpEnabled());
    TEST_ASSERT_EQUAL_STRING("", settings.getStaticIp().c_str());
    TEST_ASSERT_EQUAL_STRING("", settings.getStaticGateway().c_str());
    TEST_ASSERT_EQUAL_STRING("255.255.255.0", settings.getStaticSubnet().c_str());
    TEST_ASSERT_EQUAL_STRING("1.1.1.1", settings.getStaticDns().c_str());
#ifdef AP_PASSWORD
    TEST_ASSERT_EQUAL_STRING(AP_PASSWORD, settings.getApPassword().c_str());
#else
    TEST_ASSERT_EQUAL_STRING("", settings.getApPassword().c_str());
#endif
    TEST_ASSERT_EQUAL(DEFAULT_USE_24HOUR_FORMAT, settings.getUse24HourFormat());
    TEST_ASSERT_EQUAL(DEFAULT_SHOW_SECONDS, settings.getShowSeconds());
}

void test_settings_save_and_load(void) {
    SettingsManager settings;
    settings.begin();
    
    // Modify settings
    settings.setUse24HourFormat(false);
    settings.setShowSeconds(false);
    settings.setBrightness(50);
    settings.setAutoBrightness(true);
    settings.setTimezone("EST5EDT,M3.2.0,M11.1.0");
    settings.setScreenshotServerEnabled(false);
    settings.setScreenOrientation(2);
    settings.setMqttEnabled(false);
    settings.setScreensaverEnabled(!SCREENSAVER_ENABLED);
    settings.setScreensaverTimeout(600000);
    settings.setApiServerEnabled(!API_SERVER_ENABLED);
    settings.setStaticIpEnabled(true);
    settings.setStaticIp("192.168.1.100");
    settings.setStaticGateway("192.168.1.1");
    settings.setStaticSubnet("255.255.0.0");
    settings.setStaticDns("8.8.8.8");
    settings.setApPassword("new_ap_pass");
    
    // Create new instance to simulate re-reading from preferences
    SettingsManager settings_new;
    settings_new.begin();
    
    TEST_ASSERT_EQUAL(false, settings_new.getUse24HourFormat());
    TEST_ASSERT_EQUAL(false, settings_new.getShowSeconds());
    TEST_ASSERT_EQUAL(50, settings_new.getBrightness());
    TEST_ASSERT_EQUAL(true, settings_new.getAutoBrightness());
    TEST_ASSERT_EQUAL_STRING("EST5EDT,M3.2.0,M11.1.0", settings_new.getTimezone().c_str());
    TEST_ASSERT_EQUAL(false, settings_new.getScreenshotServerEnabled());
    TEST_ASSERT_EQUAL(2, settings_new.getScreenOrientation());
    TEST_ASSERT_EQUAL(false, settings_new.getMqttEnabled());
    TEST_ASSERT_EQUAL(!SCREENSAVER_ENABLED, settings_new.getScreensaverEnabled());
    TEST_ASSERT_EQUAL(600000, settings_new.getScreensaverTimeout());
    TEST_ASSERT_EQUAL(!API_SERVER_ENABLED, settings_new.getApiServerEnabled());
    TEST_ASSERT_EQUAL(true, settings_new.getStaticIpEnabled());
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", settings_new.getStaticIp().c_str());
    TEST_ASSERT_EQUAL_STRING("192.168.1.1", settings_new.getStaticGateway().c_str());
    TEST_ASSERT_EQUAL_STRING("255.255.0.0", settings_new.getStaticSubnet().c_str());
    TEST_ASSERT_EQUAL_STRING("8.8.8.8", settings_new.getStaticDns().c_str());
    TEST_ASSERT_EQUAL_STRING("new_ap_pass", settings_new.getApPassword().c_str());
}

void test_settings_wifi_credentials(void) {
    SettingsManager settings;
    settings.begin();

    // Verify default values fallback to secrets.h macros
    TEST_ASSERT_EQUAL_STRING(WIFI_SSID, settings.getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING(WIFI_PASSWORD, settings.getWifiPassword().c_str());

    // Modify WiFi credentials
    settings.setWifiSSID("New_SSID");
    settings.setWifiPassword("New_Password");

    // Re-instantiate to simulate reboot
    SettingsManager settings_new;
    settings_new.begin();

    TEST_ASSERT_EQUAL_STRING("New_SSID", settings_new.getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING("New_Password", settings_new.getWifiPassword().c_str());
}
void test_settings_location_data(void) {
    SettingsManager settings;
    settings.begin();

    // Verify default values fallback to config.h/secrets.h macros

    // Modify location data

    // Re-instantiate to simulate reboot
    SettingsManager settings_new;
    settings_new.begin();

}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_settings_default_values);
    RUN_TEST(test_settings_save_and_load);
    RUN_TEST(test_settings_wifi_credentials);
    RUN_TEST(test_settings_location_data);
    return UNITY_END();
}
