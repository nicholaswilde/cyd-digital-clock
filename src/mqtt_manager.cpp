#include "mqtt_manager.h"
#include "settings_manager.h"
#include "config/config.h"

extern SettingsManager settings;

MqttManager::MqttManager(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic)
    : _server(server), _port(port), _user(user), _password(password), _baseTopic(baseTopic), _reconnectTimer(nullptr), _reconnectBackoffMs(5000), _messageCallback(nullptr) {}

void MqttManager::updateConfig(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic) {
    _server = server;
    _port = port;
    _user = user;
    _password = password;
    _baseTopic = baseTopic;
    _willTopic = _baseTopic + "status";
    
    // If we're already connected, we should disconnect and let it reconnect with new settings, 
    // or just apply credentials for the next reconnect.
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
    _mqttClient.setWill(_willTopic.c_str(), 1, true, "offline");
}

void MqttManager::begin() {
    // 1. Create a FreeRTOS timer for non-blocking reconnects.
    _reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void*)this, onMqttReconnectTimer);

    // 2. Configure broker details
    _willTopic = _baseTopic + "status";
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
    _mqttClient.setWill(_willTopic.c_str(), 1, true, "offline");

    // 2.5 Configure unique Client ID
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    _clientId = "CYD-Weather-" + mac;
    _mqttClient.setClientId(_clientId.c_str());

    // 3. Register the asynchronous callbacks using C++ lambdas
    _mqttClient.onConnect([this](bool sessionPresent) {
        this->onMqttConnect(sessionPresent);
    });
    
    _mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
        this->onMqttDisconnect(reason);
    });

    _mqttClient.onPublish([](uint16_t packetId) {
        Serial.printf("[MQTT] Broker acknowledged publish (Packet ID: %d)\n", packetId);
    });

    _mqttClient.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        this->onMqttMessage(topic, payload, properties, len, index, total);
    });
}

void MqttManager::connectToMqtt() {
    Serial.printf("[MQTT] Connecting to broker at %s:%d...\n", _server, _port);
    _mqttClient.connect();
}

void MqttManager::onNetworkAvailable() {
    Serial.println("[MQTT] Network is up. Initiating broker connection...");
    connectToMqtt();
}

void MqttManager::onNetworkDisconnected() {
    Serial.println("[MQTT] Network is down. Halting reconnect timers...");
    if (_reconnectTimer) {
        xTimerStop(_reconnectTimer, 0);
    }
}


static void publishTask(void* pvParameters) {
    MqttManager* mqtt = static_cast<MqttManager*>(pvParameters);
    mqtt->publishHADiscovery();
    vTaskDelete(NULL);
}

void MqttManager::onMqttConnect(bool sessionPresent) {
    Serial.println("[MQTT] Connected to broker!");
    
    // Reset backoff on successful connection
    _reconnectBackoffMs = 5000;
    
    // Publish a boot message
    _mqttClient.publish((_baseTopic + "status").c_str(), 0, true, "online");
    
    // Publish HA Discovery configuration
    xTaskCreate(publishTask, "pubTask", 4096, this, 1, NULL);

    // Subscribe to commands
    subscribe("command/brightness", 0);
    subscribe("command/led_enabled", 0);
    subscribe("command/led_brightness", 0);
    subscribe("command/reboot", 0);
    subscribe("command/auto_brightness", 0);
    subscribe("command/use_24hr_format", 0);
    subscribe("command/show_seconds", 0);
    subscribe("command/screensaver", 0);
    subscribe("command/sleep_schedule", 0);
    subscribe("command/sleep_start", 0);
    subscribe("command/sleep_end", 0);
    subscribe("command/theme", 0);
    subscribe("command/screen_orientation", 0);
    subscribe("command/set_time", 0);
}

void MqttManager::publishHADiscovery() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String deviceId = "cyd_weather_" + mac;
    String deviceJson = "\"device\":{\"identifiers\":[\"" + deviceId + "\"],\"name\":\"CYD Weather Station " + mac.substring(mac.length() - 4) + "\",\"manufacturer\":\"Nicholas Wilde\",\"model\":\"CYD-28R/35C\"}";
    // Connection Status
    String connPayload = "{\"name\":\"Connection Status\",\"state_topic\":\"" + _baseTopic + "status\",\"payload_on\":\"online\",\"payload_off\":\"offline\",\"device_class\":\"connectivity\",\"unique_id\":\"" + deviceId + "_conn\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/binary_sensor/" + deviceId + "/connection/config").c_str(), 0, true, connPayload.c_str());

    // Brightness Control (Number)
    String brightPayload = "{\"name\":\"Brightness\",\"state_topic\":\"" + _baseTopic + "settings/brightness\",\"command_topic\":\"" + _baseTopic + "command/brightness\",\"min\":10,\"max\":100,\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_bright\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/number/" + deviceId + "/brightness/config").c_str(), 0, true, brightPayload.c_str());
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // LED Enabled (Switch)
    String ledEnabledPayload = "{\"name\":\"LED Enabled\",\"state_topic\":\"" + _baseTopic + "settings/led_enabled\",\"command_topic\":\"" + _baseTopic + "command/led_enabled\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_led_en\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/led_enabled/config").c_str(), 0, true, ledEnabledPayload.c_str());
    vTaskDelay(pdMS_TO_TICKS(50));

    // LED Brightness (Number)
    String ledBrightPayload = "{\"name\":\"LED Brightness\",\"state_topic\":\"" + _baseTopic + "settings/led_brightness\",\"command_topic\":\"" + _baseTopic + "command/led_brightness\",\"min\":10,\"max\":100,\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_ledbright\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/number/" + deviceId + "/led_brightness/config").c_str(), 0, true, ledBrightPayload.c_str());
    vTaskDelay(pdMS_TO_TICKS(50));

    // Reboot Control (Button)
    String rebootPayload = "{\"name\":\"Reboot\",\"command_topic\":\"" + _baseTopic + "command/reboot\",\"payload_press\":\"REBOOT\",\"device_class\":\"restart\",\"unique_id\":\"" + deviceId + "_reboot\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/button/" + deviceId + "/reboot/config").c_str(), 0, true, rebootPayload.c_str());

    // --- System Diagnostics ---
    // Uptime
    String uptimePayload = "{\"name\":\"Uptime\",\"state_topic\":\"" + _baseTopic + "system/uptime\",\"unit_of_measurement\":\"s\",\"device_class\":\"duration\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_uptime\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/uptime/config").c_str(), 0, true, uptimePayload.c_str());
    // Free Heap
    String heapPayload = "{\"name\":\"Free Memory\",\"state_topic\":\"" + _baseTopic + "system/free_heap\",\"unit_of_measurement\":\"B\",\"device_class\":\"data_size\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_heap\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/free_heap/config").c_str(), 0, true, heapPayload.c_str());
    // Wi-Fi RSSI
    String rssiPayload = "{\"name\":\"Wi-Fi Signal\",\"state_topic\":\"" + _baseTopic + "system/wifi_rssi\",\"unit_of_measurement\":\"dBm\",\"device_class\":\"signal_strength\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_rssi\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/wifi_rssi/config").c_str(), 0, true, rssiPayload.c_str());
    // IP Address
    String ipPayload = "{\"name\":\"IP Address\",\"state_topic\":\"" + _baseTopic + "system/ip\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_ip\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/ip/config").c_str(), 0, true, ipPayload.c_str());
    // Firmware Version
    String verPayload = "{\"name\":\"Firmware Version\",\"state_topic\":\"" + _baseTopic + "system/version\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_version\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/version/config").c_str(), 0, true, verPayload.c_str());
    vTaskDelay(pdMS_TO_TICKS(50));
    String macPayload = "{\"name\":\"MAC Address\",\"state_topic\":\"" + _baseTopic + "system/mac\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_mac\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/mac/config").c_str(), 0, true, macPayload.c_str());

    // --- Operational Settings ---
    // Auto Brightness (Switch)
    String autoBrPayload = "{\"name\":\"Auto Brightness\",\"state_topic\":\"" + _baseTopic + "settings/auto_brightness\",\"command_topic\":\"" + _baseTopic + "command/auto_brightness\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_autobright\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/auto_brightness/config").c_str(), 0, true, autoBrPayload.c_str());
    // 24 Hour Format (Switch)
    String hr24Payload = "{\"name\":\"24 Hour Format\",\"state_topic\":\"" + _baseTopic + "settings/use_24hr_format\",\"command_topic\":\"" + _baseTopic + "command/use_24hr_format\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_24hr_format\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/use_24hr_format/config").c_str(), 0, true, hr24Payload.c_str());

    // Show Seconds (Switch)
    String showSecPayload = "{\"name\":\"Show Seconds\",\"state_topic\":\"" + _baseTopic + "settings/show_seconds\",\"command_topic\":\"" + _baseTopic + "command/show_seconds\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_show_seconds\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/show_seconds/config").c_str(), 0, true, showSecPayload.c_str());

    // WiFi Enabled (Switch)
    String wifiEnPayload = "{\"name\":\"WiFi Enabled\",\"state_topic\":\"" + _baseTopic + "settings/wifi_enabled\",\"command_topic\":\"" + _baseTopic + "command/wifi_enabled\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_wifi_enabled\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/wifi_enabled/config").c_str(), 0, true, wifiEnPayload.c_str());

    String rtcEnPayload = "{\"name\":\"Hardware RTC Backup\",\"state_topic\":\"" + _baseTopic + "settings/use_rtc\",\"command_topic\":\"" + _baseTopic + "command/use_rtc\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_use_rtc\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/use_rtc/config").c_str(), 0, true, rtcEnPayload.c_str());

    // Screensaver (Switch)
    String ssPayload = "{\"name\":\"Screensaver\",\"state_topic\":\"" + _baseTopic + "settings/screensaver\",\"command_topic\":\"" + _baseTopic + "command/screensaver\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_screensaver\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/screensaver/config").c_str(), 0, true, ssPayload.c_str());

    String sleepPayload = "{\"name\":\"Sleep Schedule\",\"state_topic\":\"" + _baseTopic + "settings/sleep_schedule\",\"command_topic\":\"" + _baseTopic + "command/sleep_schedule\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_sleep_schedule\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/sleep_schedule/config").c_str(), 0, true, sleepPayload.c_str());

    String sleepStartPayload = "{\"name\":\"Sleep Start Time\",\"state_topic\":\"" + _baseTopic + "settings/sleep_start\",\"command_topic\":\"" + _baseTopic + "command/sleep_start\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_sleep_start\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/text/" + deviceId + "/sleep_start/config").c_str(), 0, true, sleepStartPayload.c_str());

    String sleepEndPayload = "{\"name\":\"Sleep End Time\",\"state_topic\":\"" + _baseTopic + "settings/sleep_end\",\"command_topic\":\"" + _baseTopic + "command/sleep_end\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_sleep_end\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/text/" + deviceId + "/sleep_end/config").c_str(), 0, true, sleepEndPayload.c_str());

    // Theme (Select)
    String themePayload = "{\"name\":\"Theme Flavor\",\"state_topic\":\"" + _baseTopic + "settings/theme\",\"command_topic\":\"" + _baseTopic + "command/theme\",\"options\":[\"Mocha\",\"Macchiato\",\"Frappe\",\"Latte\"],\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_theme\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/select/" + deviceId + "/theme/config").c_str(), 0, true, themePayload.c_str());
    // Screen Orientation (Select)
    String orientPayload = "{\"name\":\"Screen Orientation\",\"state_topic\":\"" + _baseTopic + "settings/screen_orientation\",\"command_topic\":\"" + _baseTopic + "command/screen_orientation\",\"options\":[\"Landscape\",\"Portrait\",\"Portrait Rev\",\"Landscape Rev\"],\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_orientation\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/select/" + deviceId + "/orientation/config").c_str(), 0, true, orientPayload.c_str());

    vTaskDelay(pdMS_TO_TICKS(50));
}

void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("[MQTT] Disconnected from broker! Reason code: %d\n", (int8_t)reason);
    
    if ((int8_t)reason == 4) {
        Serial.println("[MQTT] Hint: Reason 4 usually means Bad Username or Password.");
    }

    Serial.printf("[MQTT] Reconnecting in %lu seconds...\n", _reconnectBackoffMs / 1000);
    
    // Only start the reconnect timer if Wi-Fi is still connected
    if (WiFi.status() == WL_CONNECTED && _reconnectTimer) {
        xTimerChangePeriod(_reconnectTimer, pdMS_TO_TICKS(_reconnectBackoffMs), 0);
        xTimerStart(_reconnectTimer, 0);
        
        // Increase backoff for next time, capped at max limit (e.g., 2 minutes / 120000ms)
        _reconnectBackoffMs *= 2;
        if (_reconnectBackoffMs > 120000) {
            _reconnectBackoffMs = 120000;
        }
    }
}

void MqttManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    if (_messageCallback) {
        String payloadStr;
        for (size_t i = 0; i < len; i++) {
            payloadStr += (char)payload[i];
        }
        _messageCallback(String(topic), payloadStr);
    }
}

bool MqttManager::isConnected() {
    return _mqttClient.connected();
}

void MqttManager::publish(const char* topic, const char* payload, bool retain) {
    if (isConnected()) {
        String fullTopic = String(topic);
        if (!fullTopic.startsWith("homeassistant/")) {
            fullTopic = _baseTopic + fullTopic;
        }
        Serial.printf("[MQTT] Publishing -> Topic: '%s' | Retain: %d | Payload: '%s'\n", fullTopic.c_str(), retain, payload);
        uint16_t packetId = _mqttClient.publish(fullTopic.c_str(), 0, retain, payload);
        
        if (packetId == 0) {
            Serial.println("[MQTT] ERROR: Publish failed (buffer might be full)");
        }
    } else {
        Serial.printf("[MQTT] WARN: Cannot publish to '%s' - Not connected to broker.\n", topic);
    }
}

void MqttManager::subscribe(const char* topic, uint8_t qos) {
    if (isConnected()) {
        String fullTopic = String(topic);
        if (!fullTopic.startsWith("homeassistant/")) {
            fullTopic = _baseTopic + fullTopic;
        }
        _mqttClient.subscribe(fullTopic.c_str(), qos);
        Serial.printf("[MQTT] Subscribed to topic: %s\n", fullTopic.c_str());
    } else {
        Serial.printf("[MQTT] WARN: Cannot subscribe to '%s' - Not connected to broker.\n", topic);
    }
}

void MqttManager::onMessage(MqttMessageCallback cb) {
    _messageCallback = cb;
}

void MqttManager::disconnect() {
    Serial.println("[MQTT] Disconnecting from broker...");
    if (_reconnectTimer) {
        xTimerStop(_reconnectTimer, 0);
    }
    _mqttClient.disconnect();
}


void MqttManager::onMqttReconnectTimer(TimerHandle_t xTimer) {
    // Retrieve the class instance pointer from the timer ID
    MqttManager* instance = static_cast<MqttManager*>(pvTimerGetTimerID(xTimer));
    if (instance) {
        instance->connectToMqtt();
    }
}
