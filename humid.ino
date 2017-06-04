#include <DHT.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define DISPLAY_REFRESH_RATE 2000
#define READ_INTERVAL_DHT22 2000

DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

float humidity = 0.0f;
float temperature = 0.0f;

void setup() {
  Serial.begin(9600);
  dht.begin();
  u8g2.begin();
}

void readDht22() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > READ_INTERVAL_DHT22) {
    previousMillis = millis();
    readHumidity();
    readTemperature();
  }
}

void readHumidity() {
  float humdityReading = dht.readHumidity();
  if (isnan(humdityReading)) {
    return;
  }
  humidity = humdityReading;
}

void readTemperature() {
  float temperatureReading = dht.readTemperature(false);
  if (isnan(temperatureReading)) {
    return;
  }
  temperature = temperatureReading;
}


void draw() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > DISPLAY_REFRESH_RATE) {
    previousMillis = millis();

    u8g2.firstPage();
    do {
      drawHumidity();
      drawTemperature();
    } while (u8g2.nextPage());
  }
}

void drawHumidity() {
  char buffer[10];
  char *humidityString = dtostrf(humidity, 2, 1, buffer);
  strcat(humidityString, "%");
  
  u8g2.setFont(u8g2_font_courB14_tf);
  u8g2.drawStr(0, 63, humidityString);

  u8g2.setFont(u8g2_font_trixel_square_tf);
  u8g2.drawStr(0, 44, "Luftfeuchtigkeit");
}

void drawTemperature() {
  char buffer[20];
  char *temperatureString = dtostrf(temperature, 2, 1, buffer);
  strcat(temperatureString, "\xB0");
  
  u8g2.setFont(u8g2_font_courB14_tf);
  u8g2.drawStr(60, 63, temperatureString);

  u8g2.setFont(u8g2_font_trixel_square_tf);
  u8g2.drawStr(60, 44, "Temperatur");
}

void loop() {
  readDht22();
  draw();
}
