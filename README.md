# :alarm_clock: CYD Digital Clock :pager:
[![Coveralls](https://img.shields.io/badge/dynamic/xml?url=https%3A%2F%2Fcoveralls.io%2Frepos%2Fgithub%2Fnicholaswilde%2Fcyd-digital-clock%2Fbadge.svg%3Fbranch%3Dmain&query=%2F%2F*%5Blocal-name()%3D'text'%5D%5Blast()%5D&label=Coveralls&style=for-the-badge&logo=coveralls)](https://coveralls.io/github/nicholaswilde/cyd-digital-clock?branch=main)
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![test](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-digital-clock/test.yaml?label=test&style=for-the-badge&branch=main&logo=github-actions)](https://github.com/nicholaswilde/cyd-digital-clock/actions/workflows/test.yaml)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/cyd-digital-clock/ci.yaml?label=ci&style=for-the-badge&logo=github-actions)](https://github.com/nicholaswilde/cyd-digital-clock/actions/workflows/ci.yaml)

A beautiful, configurable real-time digital clock built for the **ESP32 Cheap Yellow Device (CYD)** (board model ESP32-2432S028R / ESP32-3248S035C) utilizing the **LVGL v8** graphics library and the **Catppuccin Color Theme**.

> [!WARNING]
> This project is currently in a `v0.X.X` development stage. Features and configurations are subject to change, and breaking changes may be introduced at any time.

## :star: Features

- **Real-Time Clock Display**:
  - Accurate, synchronized time display updated every second.
  - Automatic time synchronization via Network Time Protocol (NTP).
  - Native POSIX timezone string support with automatic Daylight Saving Time (DST) calculations.
  - Manual fallback time setting capability via the device UI or API.
  - Built-in RTC Drift Compensation software to maintain accuracy when offline.
  - **Hardware RTC Backup**: Full integration with I2C DS3231 modules for zero-network timekeeping.
- **Offline & True Standby**:
  - Entirely functional without an internet connection.
  - Toggle Wi-Fi radio fully off directly from device settings.
- **12 / 24-Hour Time Format & Seconds Toggle**:
  - Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes and toggle seconds on/off via the touchscreen interface.
- **Catppuccin Color Themes**:
  - Full dynamic theme selection across 4 Catppuccin flavors (**Mocha**, **Macchiato**, **Frappé**, **Latte**) with instant live UI restyling.
  - Configurable compile-time default via `DEFAULT_THEME_FLAVOR` in `config/config.h`.
- **On-Device Settings & Interactive Network Modal**:
  - Long-press anywhere on the clock display for 1.5 seconds to open the full-screen Settings screen.
  - Interactive Wi-Fi status icon colored dynamically by connection state (Green=Connected, Yellow=Connecting, Red=Disconnected, Mauve=AP Mode).
  - Tapping the Wi-Fi icon displays a network information modal with live details (SSID, IP address, Hostname, MAC address, RSSI).
- **Backlight & Auto-Brightness Control**:
  - Manual backlight level slider (10%–100%) or automatic brightness driven by onboard LDR photoresistor (GPIO 34) with LEDC PWM (GPIO 21).
- **RGB LED Status Indicator**:
  - Onboard RGB LED (GPIO 4/16/17) provides connectivity status feedback with configurable enabled toggle and brightness slider (10%–100%).
- **Web Dashboard & REST API**:
  - Catppuccin-themed web settings portal at `http://<DEVICE_IP>/` and dynamic JSON configuration endpoints (`/api/config`).
- **MQTT & Home Assistant Integration**:
  - Auto-discovery for Home Assistant, real-time telemetry, and bidirectional remote control.

## :hammer_and_wrench: Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**:
  - **CYD 2.8" (Resistive)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
  - **CYD 3.5" (Capacitive)**: ESP32-3248S035C — 3.5″ 480×320 ST7796 LCD with GT911/CST820 capacitive touch.
- **Onboard Hardware**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17), BOOT button (GPIO 0).
- Micro-USB / USB-C cable for power and programming.

## :electric_plug: Hardware Setup / RTC Wiring

To use the **Hardware RTC Backup** feature, wire a DS3231 I2C Real-Time Clock module to the **CN1** breakout port. Do not use connector P3 near the SD card slot as it lacks a 3.3V power supply.

- **VCC**: 3V3
- **GND**: GND
- **SDA**: IO27 (on `cyd_28r`) or IO21 (on `cyd_35c`)
- **SCL**: IO22

> [!NOTE]
> On the `cyd_28r` board, the CN1 connector breaks out **IO27**. IO27 must be used for SDA to prevent conflicts with the display backlight on IO21.
> On the `cyd_35c` board, the CN1 connector breaks out **IO21**. IO21 must be used for SDA to prevent conflicts with the display backlight on IO27.

```text
+------------------------------+
|                              |
|                          +------+
|                          |  P3  |
|                          +------+
|                              |
|                       +---------+
|                       |   CN1   |      +--------------------+
|                       |  GND[X] |------| [X]GND             |
|                       | IO22[X] |------| [X]SCL    .---.    |   
|                       | IO27[X] |------| [X]SDA  /       \  |
|                       | 3.3V[X] |------| [X]VCC | CR 2032 | |
|                       +---------+      | [ ]SQW  \       /  |
|                              |         | [ ]32K    '---'    |
|                          +------+      +--------------------+
|                          |  SD  |              DS3231
|                          | CARD |
|                          +------+
|                              |
| +--+ +-------+ +-----------+ |
+-|  |-| USB C |-| MICRO USB |-+
  +--+ +-------+ +-----------+ 
         ESP32-2432S028R  
```

```text
+------------------------------+
|                              |
|                          +------+
|                          |  P3  |
|                          +------+
|                              |
|                       +---------+
|                       |   CN1   |      +--------------------+
|                       |  GND[X] |------| [X]GND             |
|                       | IO22[X] |------| [X]SCL    .---.    |   
|                       | IO21[X] |------| [X]SDA  /       \  |
|                       | 3.3V[X] |------| [X]VCC | CR 2032 | |
|                       +---------+      | [ ]SQW  \       /  |
|                              |         | [ ]32K    '---'    |
|                          +------+      +--------------------+
|                          |  SD  |              DS3231
|                          | CARD |
|                          +------+
|                              |
| +--+ +-------+ +-----------+ |
+-|  |-| USB C |-| MICRO USB |-+
  +--+ +-------+ +-----------+ 
         ESP32-3248S035C    
```

---

## :rocket: Getting Started

### Zero-Configuration Wi-Fi Setup (Improv Wi-Fi)

This project supports **Improv Wi-Fi** over Serial. You can provision Wi-Fi credentials to the device directly from a modern web browser (Chrome, Edge, Opera) via WebSerial without needing to hardcode secrets or join an access point.

1. Connect your CYD to your computer via USB.
2. Go to [Improv Wi-Fi Web](https://www.improv-wifi.com/).
3. Click "Connect Device" and select the CYD serial port.
4. Enter your Wi-Fi credentials when prompted.
5. The device will save them, connect, and report its IP address back to the browser.

### 1. Build from Source

#### Secrets Setup

Wi-Fi credentials and local secrets live in a Git-ignored secrets file to prevent accidental commits.

1. Copy the template:
   ```bash
   task init
   ```
   *(Or manually: `cp config/secrets.h.example config/secrets.h`)*

2. Edit `config/secrets.h`:
   ```cpp
   #define WIFI_SSID     "Your_WiFi_Network"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   ```

#### Configuration

Default settings live in [`config/config.h`](config/config.h). 

**Theme Flavor Default:**
```cpp
#define DEFAULT_THEME_FLAVOR CATPPUCCIN_MOCHA
```

**Timezone & NTP Server:**
```cpp
#define TIMEZONE_DEFAULT "UTC0"
#define NTP_SERVER "pool.ntp.org"
```

**Display Performance Tuning:**
```cpp
#define DISPLAY_DRAW_BUF_ROWS 30
#define DISPLAY_REFR_PERIOD_MS 20
#define DISPLAY_INDEV_READ_PERIOD_MS 10
```

#### Build & Upload

```bash
task build    # Compile firmware
task upload   # Flash to the connected CYD board
task monitor  # Open serial monitor (115200 baud)
```

---

## :gear: Settings Reference

Settings can be adjusted via the touchscreen interface:

| Setting | Description |
| :--- | :--- |
| **Enable WiFi** | Toggle the Wi-Fi radio on or off for true offline capability. |
| **Hardware RTC Backup** | Toggle integration with an external DS3231 I2C module. |
| **Theme (Catppuccin)** | Select UI color theme flavor (**Mocha**, **Macchiato**, **Frappé**, **Latte**). |
| **Time Format** | Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes. |
| **Show Seconds** | Toggle displaying seconds on the clock face. |
| **Auto Brightness** | Toggle automatic brightness adjustment driven by ambient light sensor (LDR). |
| **Screen Brightness** | Adjust the display backlight brightness (10% to 100%). |
| **Status LED** | Enable or disable the onboard RGB LED status indicator. |
| **LED Brightness** | Adjust the brightness of the RGB LED (10% to 100%). |

---

### Remote Web Settings & Configuration API

When connected to Wi-Fi, the device hosts a web configuration portal at `http://<DEVICE_IP>/` and a REST API for dynamic remote configuration.

**Get Configuration Settings:**
```bash
curl http://<DEVICE_IP>/api/config
```

Example JSON response:
```json
{
  "use_24hr_format": false,
  "use_rtc": true,
  "show_seconds": true,
  "brightness": 80,
  "auto_brightness": true,
  "timezone": "UTC0",
  "theme_flavor": 1,
  "screenshot_server_enabled": false,
  "api_server_enabled": true,
  "screen_orientation": 1,
  "led_enabled": true,
  "led_brightness": 100,
  "mqtt_enabled": false,
  "mqtt_server": "",
  "mqtt_port": 1883,
  "mqtt_user": "",
  "mqtt_password": "",
  "mqtt_base_topic": "cyd/",
  "wifi_enabled": true,
  "wifi_ssid": "Your_SSID",
  "wifi_password": "Your_Password",
  "screensaver_enabled": false,
  "screensaver_timeout": 300000,
  "sleep_schedule_enabled": false,
  "sleep_start_time": "22:00",
  "sleep_end_time": "07:00",
  "static_ip_enabled": false,
  "static_ip": "",
  "static_gateway": "",
  "static_subnet": "255.255.255.0",
  "static_dns": "1.1.1.1",
  "ap_password": "",
  "ntp_server": "pool.ntp.org",
  "rtc_drift": 0.0
}
```

**Update Configuration Settings:**
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"brightness": 80, "led_enabled": true, "led_brightness": 50, "rtc_drift": 1.5}' \
  http://<DEVICE_IP>/api/config
```

**Set Device Time:**
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"timestamp": 1700000000}' \
  http://<DEVICE_IP>/api/time
```
*(Or use `{"time": "2023-01-01T12:00:00"}`)*

---

### :satellite: MQTT & Home Assistant Integration

When MQTT is enabled in settings, the device connects to your MQTT broker and exposes entities for Home Assistant auto-discovery:

#### State & Telemetry Topics

| Topic | Description | Values / Example |
| :--- | :--- | :--- |
| `<base_topic>status` | Connection availability (LWT) | `online` / `offline` |
| `<base_topic>system/uptime` | System uptime in seconds | `3600` |
| `<base_topic>system/free_heap` | Free heap memory (bytes) | `184320` |
| `<base_topic>system/wifi_rssi` | Wi-Fi RSSI (dBm) | `-55` |
| `<base_topic>system/ip` | Device IP address | `192.168.1.100` |
| `<base_topic>system/version` | Firmware version | `v0.1.0` |
| `<base_topic>system/mac` | MAC address | `AA:BB:CC:DD:EE:FF` |
| `<base_topic>settings/theme` | Current theme flavor | `Mocha` / `Macchiato` / `Frappe` / `Latte` |
| `<base_topic>settings/screen_orientation` | Screen orientation | `Landscape` / `Portrait` / `Portrait Rev` / `Landscape Rev` |
| `<base_topic>settings/use_24hr_format` | 24-hour format switch | `ON` / `OFF` |
| `<base_topic>settings/show_seconds` | Show seconds switch | `ON` / `OFF` |
| `<base_topic>settings/auto_brightness` | Auto-brightness switch | `ON` / `OFF` |
| `<base_topic>settings/brightness` | Screen brightness percentage | `10`–`100` |
| `<base_topic>settings/wifi_enabled` | Wi-Fi radio switch | `ON` / `OFF` |
| `<base_topic>settings/use_rtc` | Hardware RTC Backup switch | `ON` / `OFF` |
| `<base_topic>settings/led_enabled` | Status LED switch | `ON` / `OFF` |
| `<base_topic>settings/led_brightness` | Status LED brightness percentage | `10`–`100` |

#### Remote Command Topics

| Topic | Payload | Description |
| :--- | :--- | :--- |
| `<base_topic>command/theme` | `Mocha` / `Macchiato` / `Frappe` / `Latte` | Changes the UI Catppuccin theme flavor. |
| `<base_topic>command/screen_orientation` | `Landscape` / `Portrait` / `Portrait Rev` / `Landscape Rev` | Rotates display orientation. |
| `<base_topic>command/use_24hr_format` | `ON` / `OFF` / `1` / `0` | Toggles 12/24-hour display format. |
| `<base_topic>command/show_seconds` | `ON` / `OFF` / `1` / `0` | Toggles showing seconds on clock display. |
| `<base_topic>command/auto_brightness` | `ON` / `OFF` / `1` / `0` | Toggles LDR-driven automatic brightness. |
| `<base_topic>command/brightness` | `10`–`100` | Adjusts the display backlight brightness percentage. |
| `<base_topic>command/wifi_enabled` | `ON` / `OFF` / `1` / `0` | Enables or disables the Wi-Fi radio. |
| `<base_topic>command/use_rtc` | `ON` / `OFF` / `1` / `0` | Enables or disables the hardware RTC backup integration. |
| `<base_topic>command/led_enabled` | `ON` / `OFF` / `1` / `0` | Enables or disables the status RGB LED. |
| `<base_topic>command/led_brightness` | `10`–`100` | Adjusts the status RGB LED brightness percentage. |
| `<base_topic>command/set_time` | `1700000000` | Sets the device time manually using a Unix timestamp. |

---

### POSIX Timezone Configuration

The CYD Digital Clock uses standard POSIX timezone strings to natively calculate Daylight Saving Time (DST) transitions without requiring bulky offline timezone databases.

Common POSIX timezone examples:

| Region | Description | POSIX String |
| :--- | :--- | :--- |
| **UTC** | Coordinated Universal Time | `UTC0` |
| **London** | GMT / British Summer Time | `GMT0BST,M3.5.0/1,M10.5.0` |
| **Central Europe** | CET / CEST | `CET-1CEST,M3.5.0,M10.5.0/3` |
| **Eastern Europe** | EET / EEST | `EET-2EEST,M3.5.0/3,M10.5.0/4` |
| **US Eastern** | Eastern Time | `EST5EDT,M3.2.0,M11.1.0` |
| **US Central** | Central Time | `CST6CDT,M3.2.0,M11.1.0` |
| **US Mountain** | Mountain Time | `MST7MDT,M3.2.0,M11.1.0` |
| **US Pacific** | Pacific Time | `PST8PDT,M3.2.0,M11.1.0` |
| **US Alaska** | Alaska Time | `AKST9AKDT,M3.2.0,M11.1.0` |
| **US Hawaii** | Hawaii Standard Time | `HST10` |
| **AU Eastern** | Sydney, Melbourne | `AEST-10AEDT,M10.1.0,M4.1.0/3` |
| **AU Central** | Adelaide, Darwin | `ACST-9:30ACDT,M10.1.0,M4.1.0/3` |
| **AU Western** | Perth | `AWST-8` |

---

## :computer: Development

This project is built with **PlatformIO** and supports both ESP32 hardware builds and native desktop unit testing via CMock/Unity.

### Command Reference

| Action | Task Command | PlatformIO Equivalent | Description |
| :--- | :--- | :--- | :--- |
| **Initialize** | `task init` | `cp config/secrets.h.example config/secrets.h` | Copies the secrets template. |
| **Build** | `task build` | `pio run` | Compiles the ESP32 firmware. |
| **Upload** | `task upload` | `pio run --target upload` | Flashes firmware to the CYD board. |
| **Monitor** | `task monitor` | `pio device monitor` | Opens the serial monitor at 115200 baud. |
| **Native Tests** | `task test` | `pio test -e native` | Runs mock unit tests on the host. |
| **Pre-Flight Check** | `task test:preflight` | — | Builds all environments and runs unit tests. |
| **Lint Check** | `task check` | `pio check` | Runs `cppcheck` static analysis. |
| **Clean** | `task clean` | `pio run --target clean` | Removes build output and temp files. |

## :wrench: Troubleshooting

If you encounter any issues with your screen or the software, please review the solutions below or [create an issue](https://github.com/nicholaswilde/cyd-digital-clock/issues) on GitHub.

### Inverted Colors
If the colors on your display appear inverted, this is a common hardware variation with some CYD TFT panels. You can resolve it by:
- Flashing the pre-compiled `_inv` releases (e.g. `cyd-digital-clock-cyd_28r_inv.zip`).
- Or, when building from source, using the `cyd_28r_inv` PlatformIO environment which sets `-D TFT_INVERSION_ON=1`.

### RGB / BGR Swap
If your display has red and blue colors swapped, the panel expects BGR color order. Fix it by appending `-D TFT_RGB_ORDER=TFT_BGR` to your environment's `build_flags` in `platformio.ini`.

### Touch Calibration (Resistive Screens)
The 2.8" version (`cyd_28r`) uses a resistive touch layer which can sometimes be misaligned or mapped to inverted coordinates depending on the manufacturer batch.

### Backlight Pin and Polarity Differences
If your screen remains black while the board is booting, check your board's backlight pin mapping (GPIO 21 vs 27) and polarity (`TFT_BACKLIGHT_ON=HIGH` vs `LOW`).

### Power Delivery Glitches
Intermittent touch issues, screen flickering, or spontaneous brownout resets are usually caused by inadequate USB power supplies. Ensure you are using a reliable USB power source and cable capable of supplying stable current to the ESP32 and LCD backlight simultaneously.

## :balance_scale: License

[Apache License 2.0](LICENSE)

## :writing_hand: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).
