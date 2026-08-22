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
  - Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes via the touch screen.
- **Catppuccin Color Schemes**:
  - Built with the beautiful Catppuccin color palette (Mocha flavor by default).
- **Long-Press Settings Navigation**:
  - Long-press anywhere on the clock display for 1.5 seconds to open the full-screen Settings screen.
- **Auto-Brightness Control**:
  - Uses the onboard LDR photoresistor (GPIO 34) with an Exponential Moving Average (EMA) filter driving LEDC PWM backlight control (GPIO 21).
- **RGB LED Status Indicator**:
  - Onboard RGB LED (GPIO 4/16/17) provides connectivity status feedback.

## :hammer_and_wrench: Hardware Requirements

- **ESP32 Cheap Yellow Device (CYD)**:
  - **CYD 2.8" (Resistive)**: ESP32-2432S028R — 2.8″ 320×240 ILI9341 LCD with XPT2046 resistive touch.
  - **CYD 3.5" (Capacitive)**: ESP32-3248S035C — 3.5″ 480×320 ST7796 LCD with GT911/CST820 capacitive touch.
- **Onboard Hardware**: LDR photoresistor (GPIO 34), Backlight PWM (GPIO 21), RGB LED (GPIO 4/16/17), BOOT button (GPIO 0).
- Micro-USB / USB-C cable for power and programming.

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
   ```

#### Configuration

Default settings live in [`config/config.h`](config/config.h). 

**Timezone & NTP Server:**
```cpp
#define TIMEZONE_DEFAULT "UTC0"
#define NTP_SERVER_DEFAULT "pool.ntp.org"
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
| **Time Format** | Toggle between 12-Hour (`HH:MM:SS AM/PM`) and 24-Hour (`HH:MM:SS`) modes. |

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
