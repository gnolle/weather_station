#include <DHT.h>
#include <U8g2lib.h>
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <SPI.h>
#include <Wire.h>

#include "pitches.h"

#define DHTPIN 2
#define DHTTYPE DHT22
#define BUZZERPIN 4

#define DISPLAY_REFRESH_RATE 1000
#define READ_INTERVAL_DHT22 2000
#define READ_INTERVAL_RTC 900

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

char *week_abbr[7] = {
  "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"
};

byte melody[] = {
  NOTE_E3, NOTE_B2, NOTE_C3, NOTE_D3, NOTE_C3, NOTE_B2, NOTE_A2, NOTE_A2, NOTE_C3, NOTE_E3, NOTE_D3, NOTE_C3, NOTE_B2, NOTE_C3, NOTE_D3, NOTE_E3, NOTE_C3, NOTE_A2, NOTE_A2
};

float noteDurations[] = {
  4, 8, 8, 4, 8, 8, 4, 8, 8, 4, 8, 8, 8.0f / 3.0f, 8, 4, 4, 4, 4, 4
};

DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

float humidity = 0.0f;
float temperature = 0.0f;

void setup() {
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  dht.begin();
  u8g2.begin();

  for (int thisNote = 0; thisNote < 20; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZERPIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZERPIN);
  }
}

time_t getLocalTime() {
  static TimeChangeRule germanSummerTime = {"DEUS", Last, Sun, Mar, 2, 120};
  static TimeChangeRule germanWinterTime = {"DEUW", Last, Sun, Oct, 3, 60};
  static Timezone germanTime(germanSummerTime, germanWinterTime);
  time_t localTime = germanTime.toLocal(now());
  return localTime;
}

/*
  void playRandomSound() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > 500) {
    previousMillis = millis();
    tone(BUZZERPIN, random(2000, 5000));
  }
  }*/

void readDht22() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > READ_INTERVAL_DHT22) {
    previousMillis = millis();
    readHumidity();
    readTemperature();
  }
}

void readRtc() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis > READ_INTERVAL_RTC) {
    previousMillis = millis();
    readTime();
  }
}

void readHumidity() {
  float humdityReading = dht.readHumidity();
  if (isnan(humdityReading)) {
    return;
  }
  humidity = humdityReading;
}

void readTime() {
  time_t localTime = getLocalTime();
  Serial.println(localTime);
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
      drawTimeAndDate();
    } while (u8g2.nextPage());
  }
}

void drawHumidity() {
  char buffer[10];
  char *humidityString = dtostrf(humidity, 2, 1, buffer);
  strcat(humidityString, "%");

  u8g2.setFont(u8g2_font_courB12_tf);
  u8g2.drawStr(0, 63, humidityString);

  u8g2.setFont(u8g2_font_trixel_square_tf);
  u8g2.drawStr(0, 45, "Luftfeuchtigkeit");
}

void drawTemperature() {
  char buffer[20];
  char *temperatureString = dtostrf(temperature, 2, 1, buffer);
  strcat(temperatureString, "\xB0");

  u8g2.setFont(u8g2_font_courB12_tf);
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

  u8g2.setFont(u8g2_font_courB18_tf);

  byte centerPosTime = (OLED_WIDTH - u8g2.getStrWidth(buffer)) / 2;
  u8g2.drawStr(centerPosTime, 18, buffer);

  char *weekdayAbbr = week_abbr[weekday(currentTime) - 1];
  byte days = day(currentTime);
  byte months = month(currentTime);
  int years = year(currentTime);
  sprintf(buffer, "%s, %i.%i.%i", weekdayAbbr, days, months, years);

  u8g2.setFont(u8g2_font_courB08_tf);

  byte centerPosDate = (OLED_WIDTH - u8g2.getStrWidth(buffer)) / 2;
  u8g2.drawStr(centerPosDate, 32, buffer);
}

void loop() {
  readDht22();
  readRtc();
  draw();
  //playRandomSound();
}
