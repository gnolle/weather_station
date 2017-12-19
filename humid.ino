#include <DHT.h>
#include <U8g2lib.h>
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

const uint16_t DISPLAY_REFRESH_RATE = 1000;
const uint16_t READ_INTERVAL_DHT22 = 2000;
const uint16_t READ_INTERVAL_RTC = 900;
const uint16_t SD_WRITE_INTERVAL = 60000;

const uint8_t DHTTYPE = 22;
const uint8_t DHTPIN = 2;

const uint8_t OLED_WIDTH = 128;
const uint8_t OLED_HEIGHT = 64;

const uint8_t CHIP_SELECT = 4;

DHT dht;

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

File dataFile;

float humidity = 0.0f;
float temperature = 0.0f;

void setup() {
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  dht.setup(DHTPIN);
  u8g2.begin();
  SD.begin(CHIP_SELECT);
}

time_t getLocalTime() {
  static TimeChangeRule germanSummerTime = {"DEUS", Last, Sun, Mar, 2, 120};
  static TimeChangeRule germanWinterTime = {"DEUW", Last, Sun, Oct, 3, 60};
  static Timezone germanTime(germanSummerTime, germanWinterTime);
  time_t localTime = germanTime.toLocal(now());
  return localTime;
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
  float humdityReading = dht.getHumidity();
  if (isnan(humdityReading)) {
    return;
  }
  humidity = humdityReading;
}

void readTemperature() {
  float temperatureReading = dht.getTemperature();
  if (isnan(temperatureReading)) {
    return;
  }
  temperature = temperatureReading;
}

void writeToFile() {

  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > SD_WRITE_INTERVAL) {
    previousMillis = millis();
    
    char buffer[60];
    
    dataFile = SD.open("readings.txt", FILE_WRITE);
    time_t currentTime = getLocalTime();
    sprintf(buffer, "%i.%i.%i - %02i:%02i:%02i",
      day(currentTime),
      month(currentTime),
      year(currentTime),
      hour(currentTime),
      minute(currentTime),
      second(currentTime));

    char humidityBuffer[10];
    char *humidityString = dtostrf(humidity, 2, 1, humidityBuffer);

    char temperatureBuffer[20];
    char *temperatureString = dtostrf(temperature, 2, 1, temperatureBuffer);

    strcat(buffer, ";"); 
    strcat(buffer, humidityString);
    strcat(buffer, ";");
    strcat(buffer, temperatureString);
    dataFile.println(buffer);
    dataFile.close();
    Serial.println(buffer);
  }
}

void draw() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > DISPLAY_REFRESH_RATE) {
    previousMillis = millis();
    Serial.println(freeRam());
    u8g2.firstPage();
    do {
      drawHumidity();
      drawTemperature();
      drawTimeAndDate();
    } while (u8g2.nextPage());
  }
}

int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void drawHumidity() {
  char buffer[10];
  char *humidityString = dtostrf(humidity, 2, 1, buffer);
  strcat(humidityString, "%");

  u8g2.setFont(u8g2_font_trixel_square_tf);
  u8g2.drawStr(0, 63, humidityString);

  u8g2.setFont(u8g2_font_trixel_square_tf);
  u8g2.drawStr(0, 45, "Luftfeuchtigkeit");
}

void drawTemperature() {
  char buffer[20];
  char *temperatureString = dtostrf(temperature, 2, 1, buffer);
  strcat(temperatureString, "\xB0");

  u8g2.setFont(u8g2_font_trixel_square_tf);
  byte rightPosTempValue = OLED_WIDTH - u8g2.getStrWidth(buffer);
  u8g2.drawStr(rightPosTempValue, 63, temperatureString);

  u8g2.setFont(u8g2_font_trixel_square_tf);
  byte rightPosTempKey = OLED_WIDTH - u8g2.getStrWidth("Temperatur");
  u8g2.drawStr(rightPosTempKey, 45, "Temperatur");
}

void drawTimeAndDate() {
  char buffer[20];
  time_t currentTime = getLocalTime();
  byte hours = hour(currentTime);
  byte minutes = minute(currentTime);
  byte seconds = second(currentTime);
  sprintf(buffer, "%02i:%02i:%02i", hours, minutes, seconds);

  u8g2.setFont(u8g2_font_trixel_square_tf);

  byte centerPosTime = (OLED_WIDTH - u8g2.getStrWidth(buffer)) / 2;
  u8g2.drawStr(centerPosTime, 18, buffer);

  byte days = day(currentTime);
  byte months = month(currentTime);
  int years = year(currentTime);
  sprintf(buffer, "%i.%i.%i", days, months, years);

  u8g2.setFont(u8g2_font_trixel_square_tf);

  byte centerPosDate = (OLED_WIDTH - u8g2.getStrWidth(buffer)) / 2;
  u8g2.drawStr(centerPosDate, 32, buffer);
}

void loop() {
  readDht22();
  writeToFile();
  draw();
}
