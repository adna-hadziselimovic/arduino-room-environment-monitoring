#include <LiquidCrystal.h>
#include "DHT.h"

// LCD pin mapping
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// DHT sensor
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Rotary encoder pins
#define ENCODER_CLK 8
#define ENCODER_DT 9
#define ENCODER_SW 10

int lastClk = HIGH;
int screen = 0; // 0: T+H, 1: T+Status, 2: H+Status, 3: Combined Status (button)
bool showStatusScreen = false;

// Data logging variables
unsigned long lastLog = 0;
const unsigned long logInterval = 60000UL; // 1 minute in milliseconds

void setup() {
  Serial.begin(9600);  // Initialize serial communication for data logging
  lcd.begin(16, 2);
  dht.begin();

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
}

void loop() {
  handleEncoder();

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Log data every minute
  unsigned long now = millis();
  if (now - lastLog >= logInterval || lastLog == 0) {
    lastLog = now;
    logData(temp, humidity);
  }

  lcd.clear();

  if (showStatusScreen) {
    // Combined status screen (button press)
    lcd.setCursor(0, 0);
    lcd.print("Room Status:");
    lcd.setCursor(0, 1);
    lcd.print(getCombinedRoomStatus(temp, humidity));
    delay(2000); // Show for 2 seconds
    showStatusScreen = false;
    return;
  }

  switch (screen) {
    case 0:
      // Temperature & Humidity
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temp, 1);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Hum:  ");
      lcd.print(humidity, 1);
      lcd.print("%");
      break;
    case 1:
      // Temperature & Room Status
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temp, 1);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Status: ");
      lcd.print(getTempStatus(temp));
      break;
    case 2:
      // Humidity & Room Status
      lcd.setCursor(0, 0);
      lcd.print("Hum: ");
      lcd.print(humidity, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("Status: ");
      lcd.print(getHumidityStatus(humidity));
      break;
  }
  delay(500);
}

// Data logging function - outputs CSV format via Serial
void logData(float temp, float humidity) {
  Serial.print(temp, 1);
  Serial.print(",");
  Serial.println(humidity, 1);
}

// Rotary encoder handler
void handleEncoder() {
  int newClk = digitalRead(ENCODER_CLK);
  if (newClk != lastClk && newClk == LOW) {
    if (digitalRead(ENCODER_DT) != newClk) {
      screen++;
      if (screen > 2) screen = 0;
    } else {
      screen--;
      if (screen < 0) screen = 2;
    }
  }
  lastClk = newClk;

  // Button press for combined status screen
  if (digitalRead(ENCODER_SW) == LOW) {
    showStatusScreen = true;
    delay(300); // debounce
  }
}

// Status logic
String getTempStatus(float temp) {
  if (isnan(temp)) return "Error";
  if (temp < 10) return "Cold";
  else if (temp < 18) return "Cool";
  else if (temp < 30) return "Warm";
  else return "Hot";
}

String getHumidityStatus(float hum) {
  if (isnan(hum)) return "Error";
  if (hum < 30) return "Dry";
  else if (hum < 60) return "Normal";
  else return "Humid";
}

String getCombinedRoomStatus(float temp, float hum) {
  if (isnan(temp) || isnan(hum)) return "Sensor Error";
  // Combined status logic
  if (temp < 10 && hum < 30) return "Cold & Dry";
  if (temp < 10 && hum >= 60) return "Cold & Humid";
  if (temp >= 30 && hum >= 60) return "Hot & Humid";
  if (temp >= 30 && hum < 30) return "Hot & Dry";
  if (temp >= 18 && temp < 30 && hum >= 60) return "Warm & Humid";
  if (temp >= 18 && temp < 30 && hum < 30) return "Warm & Dry";
  if (temp >= 10 && temp < 30 && hum >= 30 && hum < 60) return "Comfortable";
  return "Normal";
}
