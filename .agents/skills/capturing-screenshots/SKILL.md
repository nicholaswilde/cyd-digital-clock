---
name: capturing-screenshots
description: Automates taking screen captures of all screens (Main, Settings) in both orientations (Landscape, Portrait) from the CYD Digital Clock device.
---

# Capturing Screenshots Skill

This skill provides the instructions and guidelines for an agent to capture, organize, and update screenshots of the CYD Digital Clock.

## Prerequisites

- The Cheap Yellow Device (CYD) must be powered on, running the latest firmware, and connected to the same local network as the host.
- The API server (`API Srv` toggle in the Settings tab) must be running and accessible on the device.
- The utility script [capture-screenshots.sh](file:///home/nicholas/git/nicholaswilde/cyd-digital-clock/scripts/capture-screenshots.sh) must be executable.
- Python 3 and the `uv` tool must be installed on the host.

## How to capture screenshots

Run the automation script with the device's IP address:

```bash
./scripts/capture-screenshots.sh <DEVICE_IP>
```

This script will automatically transition the screen across orientations and screens to generate:
- `screenshots/landscape_main.png`
- `screenshots/landscape_settings.png`
- `screenshots/portrait_main.png`
- `screenshots/portrait_settings.png`

## Updating README.md

After capturing new screenshots, reference them in the [README.md](file:///home/nicholas/git/nicholaswilde/cyd-digital-clock/README.md) file inside the screenshots section using HTML table or standard markdown image links.
