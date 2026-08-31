#!/usr/bin/env bash
set -e

# Screen map: 0=main, 1=settings

SCREEN=$1
if [ -z "$SCREEN" ]; then
    echo "Usage: $0 <main|settings>"
    exit 1
fi

case "$SCREEN" in
    main) SCREEN_IDX=0 ;;
    settings) SCREEN_IDX=1 ;;
    *) echo "Invalid screen: $SCREEN. Must be main or settings."; exit 1 ;;
esac

# Source .env for CYD_DEVICE_IP
if [ -f .env ]; then
    export $(grep -v '^#' .env | xargs)
else
    echo "Error: .env file not found."
    exit 1
fi

if [ -z "$CYD_DEVICE_IP" ]; then
    echo "Error: CYD_DEVICE_IP not set in .env."
    exit 1
fi

echo "Switching screen to $SCREEN..."
curl -sS -m 5 -d "index=${SCREEN_IDX}" "http://${CYD_DEVICE_IP}/api/screen" > /dev/null || true

sleep 2

mkdir -p screenshots
OUT_FILE="screenshots/agent_${SCREEN}.bmp"

echo "Capturing screenshot..."
if ! curl -sS -f -m 15 "http://${CYD_DEVICE_IP}/screenshot" -o "${OUT_FILE}"; then
    echo "Error: Failed to capture screenshot."
    exit 1
fi

sleep 2

if command -v uv >/dev/null 2>&1 && [ -f "./scripts/convert-screenshots.py" ]; then
    echo "Converting to PNG..."
    uv run ./scripts/convert-screenshots.py > /dev/null
fi

PNG_FILE="screenshots/agent_${SCREEN}.png"
if [ -f "$PNG_FILE" ]; then
    echo "Success! Screenshot saved to $PNG_FILE"
else
    echo "Success! Screenshot saved to $OUT_FILE"
fi
