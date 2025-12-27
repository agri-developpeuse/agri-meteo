#include <Wire.h> // I2C
#include <SPI.h> // BME280 SPI

// ----- Libraries for DS18B20 -----
#include <OneWire.h>
#include <DallasTemperature.h>

// ----- Libraries for OLED and BME280 -----
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

// ----- Pins -----
#define PIN_DS18B20 4
#define BME_CS 7  // SPI chip select for BME280

// ----- OLED -----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----- DS18B20 -----
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);

// ----- BME280 (SPI) -----
Adafruit_BME280 bme(BME_CS); // uses hardware SPI

// ----- Globals -----
unsigned long lastMs = 0;

// ----- Setup and Loop -----
void setup() {
// Serial for debug and data output
  Serial.begin(115200);

  // I2C for OLED
  Wire.begin();

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Try 0x3D if needed (some modules)
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("ERR OLED init");
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Boot...");
  display.display();

  // DS18B20 init
  ds18b20.begin();

  // BME init (SPI)
  bool bmeOk = bme.begin(); // SPI init
  if (!bmeOk) {
    Serial.println("ERR BME280 init (SPI)");
    display.setCursor(0, 12);
    display.println("ERR BME280 SPI");
    display.display();
  } else {
    bme.setSampling(
      Adafruit_BME280::MODE_FORCED,
      Adafruit_BME280::SAMPLING_X1,   // temp
      Adafruit_BME280::SAMPLING_X1,   // pressure
      Adafruit_BME280::SAMPLING_X1,   // humidity
      Adafruit_BME280::FILTER_OFF
    );
  }

  Serial.println("OK BOOT");
}

void loop() {
  if (millis() - lastMs < 1000) return;
  lastMs = millis();

  // ---- DS18B20 ----
  ds18b20.requestTemperatures();
  float tSol = ds18b20.getTempCByIndex(0);

  // ---- BME280 ----
  // forced mode: take one measurement
  bme.takeForcedMeasurement();
  float tAir = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F; // hPa

  // ---- OLED ----
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Station V1 - OK");
  display.setCursor(0, 12);
  display.print("Air: "); display.print(tAir, 1); display.println(" C");
  display.print("HR : "); display.print(hum, 1); display.println(" %");
  display.print("P  : "); display.print(pres, 0); display.println(" hPa");
  display.print("Sol: "); display.print(tSol, 1); display.println(" C");
  display.display();

  // ---- Serial vers Pi (CSV) ----
  // timestamp_ms;airC;hum%;hPa;solC
  Serial.print(millis());
  Serial.print(';'); Serial.print(tAir, 2);
  Serial.print(';'); Serial.print(hum, 2);
  Serial.print(';'); Serial.print(pres, 2);
  Serial.print(';'); Serial.print(tSol, 2);
  Serial.println();
}
