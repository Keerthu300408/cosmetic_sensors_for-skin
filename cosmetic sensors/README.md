# Smart Skin Sensor prototype demo

This Arduino sketch is a UI/demo starting point for the ESP32 concept in
`Picture1.png`. It shows sample readings on an SSD1306 OLED, hosts a simple
report page over the ESP32's Wi-Fi access point, and displays a QR code to that
page when the BOOT button is pressed.

## Build

1. Install the ESP32 board support in Arduino IDE and select your ESP32 board.
2. Install the **Adafruit GFX Library**, **Adafruit SSD1306**, and **QRCode**
   Arduino libraries.
3. Open `SmartSkinSensor.ino`, select the board's port, and upload.
4. Connect a phone to `SmartSkinSensor-Demo` (the demo access point has no
   password), then scan the on-device QR code or open `http://192.168.4.1/`.

## Prototype wiring assumptions

- SSD1306 128x64 I2C OLED at address `0x3C`; use the board's default SDA/SCL
  pins.
- The BOOT button is GPIO 0 with the internal pull-up enabled.
- Sensor drivers are intentionally not included because the image does not
  identify exact sensor models, calibration, or signal-conditioning circuits.
  Implement the sensor reads in `sampleReadings()` before setting `DEMO_MODE`
  to `false`.

The displayed values are fixed examples, not measurements. The page is served
without authentication on an open local access point; use only with demo data.
Do not use this prototype to diagnose or treat skin conditions. A real product
needs validated sensor hardware, calibration, privacy/security controls, and
appropriate review before collecting personal data or making recommendations.
