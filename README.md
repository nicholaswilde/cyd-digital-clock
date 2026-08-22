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
- **12 / 24-Hour Time Format**:
  - Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes.
- **Catppuccin Color Schemes**:
  - Full support for Catppuccin flavors: **Mocha**, **Macchiato**, **Frappé**, and **Latte**.
  - Dynamically redraws the entire user interface on theme change.
- **Long-Press Settings Navigation**:
  - Long-press anywhere on the clock display for 1.5 seconds to open the full-screen Settings screen.
- **Interactive On-Device Settings**:
  - **Time Format**: Switch between 12-hour and 24-hour display format.
  - **Auto Brightness**: Toggle automatic backlight dimming driven by the onboard LDR light sensor (GPIO 34).
  - **Manual Brightness**: Slider to set fixed screen backlight brightness when auto brightness is off.
  - **Theme Flavor**: Switch between Mocha, Macchiato, Frappé, and Latte palettes.
  - **Screen Orientation**: Support for Landscape and Portrait screen orientations.
  - **Screensaver & Sleep Schedule**: Inactivity screensaver and configurable sleep schedule (e.g. 22:00 to 07:00) to protect display longevity.
  - **Status LED**: Enable/disable onboard RGB LED feedback and adjust brightness.
  - **MQTT**: Toggle MQTT connection and telemetry publishing.
- **Auto-Brightness Control**:
  - Uses the LDR photoresistor (GPIO 34) with an Exponential Moving Average (EMA) filter driving LEDC PWM backlight control (GPIO 21).
- **RGB LED Status Indicator**:
  - Onboard RGB LED (GPIO 4/16/17) provides visual feedback (blinking blue when connecting, solid green when connected, fast red when disconnected, slow purple blink in AP setup mode).
- **Web Dashboard & Settings Portal**:
  - Access `http://<DEVICE_IP>/` in any web browser for a central Catppuccin-themed dashboard.
  - Direct navigation links to Device Settings (`/settings`), Firmware Updates (`/update`), Live Screenshots (`/screenshot`), and Factory Reset (`/reset`).
- **Wi-Fi AP Captive Portal Fallback**:
  - Automatically hosts an open Soft AP (`cyd-digital-clock-<mac_short>`) if Wi-Fi connection fails or times out.
  - Runs a captive portal configuration web server on `192.168.4.1` for selecting SSIDs and entering credentials.
  - On-screen setup guide displaying SSID and IP address while in AP mode.
- **MQTT Integration & Home Assistant Auto-Discovery**:
  - Exposes device controls, configuration switches, and telemetry to Home Assistant automatically.
  - Bi-directional synchronization: changes made via MQTT, Web UI, or touchscreen update across all endpoints in real time.
- **Remote Screenshot Streaming**:
  - `GET /screenshot` streams a 24-bit BMP capture directly over HTTP.
  - Physical BOOT button (GPIO 0) press saves screenshots to microSD card when inserted.
- **Wireless OTA Firmware Updates**:
  - Update firmware wirelessly via `/update` in your browser.

## :hammer_and_wrench: Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**:
  - **CYD 2.8" (Resistive)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
  - **CYD 3.5" (Capacitive)**: ESP32-3248S035C — 3.5″ 480×320 ST7796 LCD with GT911/CST820 capacitive touch.
- **Onboard Hardware**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17), BOOT button (GPIO 0).
- **Storage**: MicroSD card slot (optional, for screenshot storage and logs).
- Micro-USB / USB-C cable for power and programming.

## :floppy_disk: MicroSD Card Support

The device supports optional microSD cards for local logging and screenshot storage.

> [!WARNING]
> Ensure any inserted card is formatted as **FAT32** with a **Master Boot Record (MBR)** partition scheme (exFAT and GUID/GPT partition tables are not supported).

## :globe_with_meridians: Web Dashboard & REST API

> [!NOTE]
> The Settings Web UI, Dashboard, and Configuration API are accessible while the device is connected to Wi-Fi. The device IP is printed to serial on boot: `[WiFi] Connected! IP address: <IP>`.

**Web Dashboard:**
Navigate to `http://<DEVICE_IP>/` in any browser to access the central dashboard:
- **⚙️ Device Settings (`/settings`)**: Configure clock parameters at runtime (Time format, Theme, Brightness, Timezone, Sleep schedule, MQTT, etc.).
- **🔄 Firmware Update (`/update`)**: Flash new firmware binaries wirelessly.
- **📸 View Screenshot (`/screenshot`)**: Stream a real-time capture of the current display.
- **⚠️ Factory Reset (`/reset`)**: Erase saved settings and Wi-Fi credentials to reboot into AP Setup mode.

**Capture Screenshot via HTTP:**
```bash
# Save to file
curl http://<DEVICE_IP>/screenshot -o screenshot.bmp

# View inline (if ImageMagick is installed)
curl -s http://<DEVICE_IP>/screenshot | display -
```

**Get Configuration Settings:**
Retrieve current settings in JSON format:
```bash
curl http://<DEVICE_IP>/api/config
```

Example JSON response:
```json
{
  "use_24hr_format": true,
  "brightness": 75,
  "auto_brightness": false,
  "timezone": "PST8PDT,M3.2.0,M11.1.0",
  "theme_flavor": 1,
  "screenshot_server_enabled": true,
  "api_server_enabled": true,
  "screen_orientation": 1,
  "led_enabled": true,
  "led_brightness": 60,
  "mqtt_enabled": true,
  "mqtt_server": "192.168.1.88",
  "mqtt_port": 1883,
  "mqtt_user": "user",
  "mqtt_password": "password",
  "mqtt_base_topic": "cyd/clock/",
  "wifi_ssid": "Your_SSID",
  "wifi_password": "Your_Password",
  "screensaver_enabled": false,
  "screensaver_timeout": 300000,
  "sleep_schedule_enabled": false,
  "sleep_start_time": "22:00",
  "sleep_end_time": "07:00",
  "static_ip_enabled": false,
  "static_ip": "192.168.1.50",
  "static_gateway": "192.168.1.1",
  "static_subnet": "255.255.255.0",
  "static_dns": "1.1.1.1",
  "ntp_server": "pool.ntp.org"
}
```

**Update Configuration Settings:**
Update any subset of device settings dynamically by posting a JSON payload:
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"brightness": 80, "theme_flavor": 2, "use_24hr_format": false}' \
  http://<DEVICE_IP>/api/config
```

Response:
```json
{"status":"ok"}
```

---

## :rocket: Getting Started

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

   // (Optional) Secure the configuration AP with a password (at least 8 chars).
   // Leave blank ("") or comment out to run an open Access Point.
   #define AP_PASSWORD ""
   ```

#### Configuration

Default settings live in [`config/config.h`](config/config.h). Runtime user preferences are changed via the on-device Settings screen, Web UI, or MQTT.

**Timezone & NTP Server:**
```cpp
#define TIMEZONE_DEFAULT "UTC0"
#define NTP_SERVER_DEFAULT "pool.ntp.org"
```

**Screensaver & Sleep Schedule:**
```cpp
#define SCREENSAVER_ENABLED     true
#define SCREENSAVER_TIMEOUT_MS  300000 // 5 minutes

#define DEFAULT_SLEEP_SCHEDULE_ENABLED false
#define DEFAULT_SLEEP_START_TIME       "22:00"
#define DEFAULT_SLEEP_END_TIME         "07:00"
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

Settings can be adjusted via touchscreen, Web UI (`/settings`), REST API, or MQTT:

| Setting | Description |
| :--- | :--- |
| **Time Format** | Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes. |
| **Auto Brightness** | Enable/disable ambient light-based automatic backlight dimming. |
| **Brightness** | Fixed backlight brightness slider (active when Auto Brightness is disabled). |
| **Theme** | Catppuccin flavor selector: Mocha, Macchiato, Frappé, Latte. |
| **Timezone** | POSIX timezone string for local time and automatic DST calculation. |
| **Orientation** | Display orientation: Landscape, Portrait, Landscape Rev, Portrait Rev. |
| **Status LED** | Enable/disable onboard RGB status LED. |
| **LED Brightness** | RGB status LED brightness level slider. |
| **Screensaver** | Enable/disable inactivity screensaver mode. |
| **Sleep Schedule** | Enable/disable scheduled screen turn-off during sleep hours. |
| **MQTT** | Enable/disable MQTT broker connection and telemetry. |

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

### :satellite: MQTT Topics & Home Assistant Integration

When MQTT is enabled, the clock connects to your MQTT broker and exposes telemetry, diagnostics, and bidirectional controls.

#### Telemetry & State Topics

| Topic | Description | Example / Values |
| :--- | :--- | :--- |
| `<base_topic>status` | Connection availability (LWT) | `online` / `offline` |
| `<base_topic>system/uptime` | Device uptime (seconds) | `3600` |
| `<base_topic>system/free_heap` | Free heap memory (bytes) | `184320` |
| `<base_topic>system/wifi_rssi` | Wi-Fi signal strength (dBm) | `-58` |
| `<base_topic>system/ip` | Device IP address | `192.168.1.150` |
| `<base_topic>system/version` | Firmware version string | `v0.1.0` |
| `<base_topic>system/mac` | Device MAC address | `B0:CB:D8:DA:77:5C` |
| `<base_topic>settings/use_24hr_format` | 24-hour format switch state | `ON` / `OFF` |
| `<base_topic>settings/brightness` | Screen brightness percentage | `0`–`100` |
| `<base_topic>settings/led_brightness` | Status LED brightness percentage | `0`–`100` |
| `<base_topic>settings/auto_brightness` | Auto brightness switch state | `ON` / `OFF` |
| `<base_topic>settings/screensaver` | Screensaver switch state | `ON` / `OFF` |
| `<base_topic>settings/theme` | Catppuccin theme flavor | `Mocha` / `Macchiato` / `Frappe` / `Latte` |
| `<base_topic>settings/screen_orientation`| Display orientation | `Landscape` / `Portrait` / `Landscape Rev` / `Portrait Rev` |
| `<base_topic>settings/screensaver_timeout`| Screensaver timeout (mins) | `5` |
| `<base_topic>settings/sleep_schedule` | Sleep schedule switch state | `ON` / `OFF` |
| `<base_topic>settings/sleep_start` | Sleep schedule start time | `22:00` |
| `<base_topic>settings/sleep_end` | Sleep schedule end time | `07:00` |
| `<base_topic>settings/led` | Status LED enabled switch state | `ON` / `OFF` |

#### Remote Control Topics

| Topic | Payload | Action |
| :--- | :--- | :--- |
| `<base_topic>command/use_24hr_format` | `ON` / `OFF` / `1` / `0` | Switches between 12h and 24h clock mode. |
| `<base_topic>command/brightness` | `0`–`100` | Adjusts the screen backlight brightness percentage. |
| `<base_topic>command/led_brightness` | `0`–`100` | Adjusts the status LED brightness percentage (0-100%). |
| `<base_topic>command/auto_brightness`| `ON` / `OFF` / `1` / `0` | Enables or disables ambient light-based automatic brightness. |
| `<base_topic>command/screensaver` | `ON` / `OFF` / `1` / `0` | Enables or disables the screensaver. |
| `<base_topic>command/sleep_schedule` | `ON` / `OFF` / `1` / `0` | Enables or disables the sleep schedule. |
| `<base_topic>command/sleep_start` | `HH:MM` | Sets the sleep schedule start time (24-hour format). |
| `<base_topic>command/sleep_end` | `HH:MM` | Sets the sleep schedule end time (24-hour format). |
| `<base_topic>command/theme` | `Mocha` / `Macchiato` / `Frappe` / `Latte` | Changes the active Catppuccin theme flavor. |
| `<base_topic>command/screen_orientation`| `Landscape` / `Portrait` / `Landscape Rev` / `Portrait Rev` | Changes display orientation dynamically. |
| `<base_topic>command/screensaver_timeout`| `1`–`60` | Sets screensaver activation inactivity timeout in minutes. |
| `<base_topic>command/led` | `ON` / `OFF` / `1` / `0` | Enables or disables the onboard status RGB LED. |
| `<base_topic>command/reboot` | `REBOOT` / `1` / `true` / `ON` | Reboots the ESP32 digital clock. |

#### Home Assistant MQTT Discovery
On connection, the device registers itself as `CYD Digital Clock <short_mac>` with full device hierarchy and exposes configuration entities and diagnostics to Home Assistant automatically.

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
| **API Tests** | `task test:api` | — | Runs live JSON API integration tests against the device. |
| **Web Health Check** | `task test:web` | — | Verifies all HTTP endpoints respond properly. |
| **MQTT Tests** | `task test:mqtt` | — | Verifies MQTT broker connectivity and command handling. |
| **OTA Update** | `task update:ota` | — | Compiles and flashes firmware wirelessly over the network. |
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
