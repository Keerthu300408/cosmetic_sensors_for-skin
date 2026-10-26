#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>

// Prototype demo only. Replace sampleReadings() with drivers for your actual sensors.
constexpr bool DEMO_MODE = true;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr int BUTTON_PIN = 0;  // ESP32 BOOT button; change for your board.

const char *AP_SSID = "SmartSkinSensor-Demo";
const char *REPORT_URL = "http://192.168.4.1/";

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
WebServer server(80);

struct SkinReadings {
  int hydrationPercent;
  int oilPercent;
  float pH;
  float temperatureC;
};

SkinReadings readings = {42, 72, 5.6f, 33.2f};
bool readingsAvailable = false;
bool showQr = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
constexpr unsigned long DEBOUNCE_MS = 40;

void sampleReadings() {
  if (DEMO_MODE) {
    // Example values matching the prototype image; these are not live sensor data.
    readings = {42, 72, 5.6f, 33.2f};
    readingsAvailable = true;
    return;
  }

  // TODO: Read calibrated values from the selected hydration, oil, pH and
  // temperature sensor modules. Define safe ranges and handle sensor errors.
  readingsAvailable = false;
}

String oilLabel() {
  return readings.oilPercent >= 65 ? "High" : "Normal";
}

String makeReportPage() {
  String page;
  page.reserve(1800);
  page += F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>Smart Skin Sensor</title><style>"
            "body{font:16px Arial;max-width:520px;margin:24px auto;padding:0 16px;color:#172033}"
            "main{background:#f3f7fb;border-radius:16px;padding:20px}"
            "li{padding:8px}small{color:#586579}</style></head><body><main>");
  page += F("<h1>Skin analysis demo</h1><p><b>Sample data only</b> - not a diagnosis or medical advice.</p>");
  if (!readingsAvailable) {
    page += F("<p>Sensor drivers are not configured. No readings are available.</p>");
    page += F("</main></body></html>");
    return page;
  }
  page += F("<ul>");
  page += F("<li>Hydration: ");
  page += String(readings.hydrationPercent);
  page += F("%</li><li>Oil: ");
  page += oilLabel();
  page += F("</li><li>pH: ");
  page += String(readings.pH, 1);
  page += F("</li><li>Temperature: ");
  page += String(readings.temperatureC, 1);
  page += F("&deg;C</li></ul><small>Connect to the device Wi-Fi network to view this page.</small>");
  page += F("</main></body></html>");
  return page;
}

void drawReadings() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Skin analysis (DEMO)"));
  display.println();
  if (!readingsAvailable) {
    display.println(F("Sensor drivers pending"));
    display.println(F("No readings available"));
    display.display();
    return;
  }
  display.printf("Hydration: %d%%\n", readings.hydrationPercent);
  display.printf("Oil:       %s\n", oilLabel().c_str());
  display.printf("pH:        %.1f\n", readings.pH);
  display.printf("Temp:      %.1f C\n", readings.temperatureC);
  display.println();
  display.println(F("Press BOOT for QR"));
  display.display();
}

void drawQrCode() {
  QRCode qr;
  uint8_t qrBuffer[qrcode_getBufferSize(3)];
  qrcode_initText(&qr, qrBuffer, 3, ECC_LOW, REPORT_URL);

  display.clearDisplay();
  constexpr int scale = 2;
  const int offsetX = (OLED_WIDTH - qr.size * scale) / 2;
  const int offsetY = (OLED_HEIGHT - qr.size * scale) / 2;
  for (uint8_t y = 0; y < qr.size; y++) {
    for (uint8_t x = 0; x < qr.size; x++) {
      if (qrcode_getModule(&qr, x, y)) {
        display.fillRect(offsetX + x * scale, offsetY + y * scale,
                         scale, scale, SSD1306_WHITE);
      }
    }
  }
  display.display();
}

void handleButton() {
  const bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = buttonState;
  }
  if (buttonState == LOW && millis() - lastDebounceTime > DEBOUNCE_MS) {
    showQr = !showQr;
    if (showQr) {
      drawQrCode();
    } else {
      drawReadings();
    }
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(5);
    }
    lastButtonState = HIGH;
    lastDebounceTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed; check wiring/address."));
    while (true) {
      delay(1000);
    }
  }

  sampleReadings();
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_SSID)) {
    Serial.println(F("Could not start the demo Wi-Fi access point."));
    while (true) {
      delay(1000);
    }
  }

  server.on("/", HTTP_GET, []() {
    sampleReadings();
    server.send(200, "text/html; charset=utf-8", makeReportPage());
  });
  server.onNotFound([]() {
    server.sendHeader("Location", REPORT_URL, true);
    server.send(302, "text/plain", "");
  });
  server.begin();

  Serial.print(F("Join Wi-Fi: "));
  Serial.println(AP_SSID);
  Serial.print(F("Open report: "));
  Serial.println(REPORT_URL);
  drawReadings();
}

void loop() {
  server.handleClient();
  handleButton();
}
