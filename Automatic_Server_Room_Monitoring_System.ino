

/*

  AUTOMATIC SERVER ROOM MONITORING SYSTEM USING ESP32

  ---------------------------------------------------

  Features:

  - Temperature and humidity monitoring using DHT22

  - Voltage monitoring using PT through ADC

  - Current monitoring using CT through ADC

  - Water leakage detection using FC-37 sensor

  - 3.5" TFT LCD display using SPI

  - WiFi connection for remote monitoring

  - Buzzer alarm for abnormal conditions

 

  NOTE:

  Calibrate PT_CALIBRATION and CT_CALIBRATION according to your actual PT/CT circuit.

*/

 
 #include <WiFi.h>

#include <DHT.h>

#include <SPI.h>

#include <Adafruit_GFX.h>

#include <Adafruit_ILI9341.h>

 

// WIFI SETTINGS 

const char* WIFI_SSID = "YOUR_WIFI_NAME";

const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

 

// PIN DEFINITIONS

// DHT22

#define DHT_PIN 27

#define DHT_TYPE DHT22

 

// ADC inputs

#define PT_ADC_PIN 34       // Voltage from PT signal conditioning

#define CT_ADC_PIN 35       // Current from CT burden resistor

#define WATER_PIN 26        // Water leakage digital output

 

// Buzzer

#define BUZZER_PIN 25

 

// TFT LCD SPI pins

#define TFT_CS   5

#define TFT_DC   2

#define TFT_RST  4

#define TFT_MOSI 23

#define TFT_SCLK 18

#define TFT_MISO 19

 

// OBJECTS

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

 

// THRESHOLDS

float TEMP_HIGH = 30.0;       // Celsius

float HUMIDITY_HIGH = 70.0;   // Percent

float VOLT_LOW = 200.0;       // Volts

float VOLT_HIGH = 250.0;      // Volts

float CURRENT_HIGH = 10.0;    // Amperes

 

// CALIBRATION FACTORS

// Adjust after testing with a multimeter and clamp meter

float PT_CALIBRATION = 100.0;   // Converts ADC voltage to mains voltage

float CT_CALIBRATION = 20.0;    // Converts ADC voltage to current

 

// VVARIABLES 

float temperature = 0;

float humidity = 0;

float mainsVoltage = 0;

float loadCurrent = 0;

float powerValue = 0;

bool waterLeakage = false;

bool alarmState = false;

bool wifiConnected = false;

 

// FUNCTIONS

 

float readADCVoltage(int pin) {

  int rawValue = analogRead(pin);

  float adcVoltage = (rawValue / 4095.0) * 3.3;

  return adcVoltage;

}

 

float readMainsVoltage() {

  float adcVoltage = readADCVoltage(PT_ADC_PIN);

  float calculatedVoltage = adcVoltage * PT_CALIBRATION;

  return calculatedVoltage;

}

 

float readLoadCurrent() {

  float adcVoltage = readADCVoltage(CT_ADC_PIN);

  float calculatedCurrent = adcVoltage * CT_CALIBRATION;

  return calculatedCurrent;

}

 

void connectWiFi() {

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

 

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {

    delay(500);

    attempts++;

  }

 

  wifiConnected = (WiFi.status() == WL_CONNECTED);

}

 

void readSensors() {

  temperature = dht.readTemperature();

  humidity = dht.readHumidity();

 

  if (isnan(temperature)) temperature = 0;

  if (isnan(humidity)) humidity = 0;

 

  mainsVoltage = readMainsVoltage();

  loadCurrent = readLoadCurrent();

  powerValue = mainsVoltage * loadCurrent;

 

  int waterStatus = digitalRead(WATER_PIN);

 

  // Many FC-37 modules output LOW when water is detected.

  // Change this logic if your module behaves differently.

  waterLeakage = (waterStatus == LOW);

}

 

void checkAlarmConditions() {

  alarmState = false;

 

  if (temperature > TEMP_HIGH) alarmState = true;

  if (humidity > HUMIDITY_HIGH) alarmState = true;

  if (mainsVoltage < VOLT_LOW || mainsVoltage > VOLT_HIGH) alarmState = true;

  if (loadCurrent > CURRENT_HIGH) alarmState = true;

  if (waterLeakage) alarmState = true;

 

  digitalWrite(BUZZER_PIN, alarmState ? HIGH : LOW);

}

 

void drawHeader() {

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_CYAN);

  tft.setTextSize(2);

  tft.setCursor(20, 10);

  tft.println("SERVER ROOM MONITOR");

  tft.drawLine(0, 35, 320, 35, ILI9341_WHITE);

}

 

void updateDisplay() {

  drawHeader();

 

  tft.setTextSize(2);

  tft.setTextColor(ILI9341_WHITE);

 

  tft.setCursor(10, 50);

  tft.print("Temp: ");

  tft.print(temperature, 1);

  tft.println(" C");

 

  tft.setCursor(10, 80);

  tft.print("Humidity: ");

  tft.print(humidity, 1);

  tft.println(" %");

 

  tft.setCursor(10, 110);

  tft.print("Voltage: ");

  tft.print(mainsVoltage, 1);

  tft.println(" V");

 

  tft.setCursor(10, 140);

  tft.print("Current: ");

  tft.print(loadCurrent, 2);

  tft.println(" A");

 

  tft.setCursor(10, 170);

  tft.print("Power: ");

  tft.print(powerValue, 1);

  tft.println(" W");

 

  tft.setCursor(10, 200);

  tft.print("Water: ");

  if (waterLeakage) {

    tft.setTextColor(ILI9341_RED);

    tft.println("LEAKAGE");

  } else {

    tft.setTextColor(ILI9341_GREEN);

    tft.println("NORMAL");

  }

 

  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(10, 230);

  tft.print("WiFi: ");

  if (wifiConnected) {

    tft.setTextColor(ILI9341_GREEN);

    tft.println("CONNECTED");

  } else {

    tft.setTextColor(ILI9341_RED);

    tft.println("OFFLINE");

  }

 

  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(10, 260);

  tft.print("Status: ");

  if (alarmState) {

    tft.setTextColor(ILI9341_RED);

    tft.println("ALARM");

  } else {

    tft.setTextColor(ILI9341_GREEN);

    tft.println("NORMAL");

  }

}

 

void sendDataToCloud() {

  // Placeholder for remote monitoring.

  // You can replace this section with:

  // - ThingSpeak

  // - Blynk

  // - Firebase

  // - MQTT

  // - Web server

  // - Telegram alert

 

  if (WiFi.status() == WL_CONNECTED) {

    wifiConnected = true;

 

    Serial.println("Sending data to cloud...");

    Serial.print("Temperature: "); Serial.println(temperature);

    Serial.print("Humidity: "); Serial.println(humidity);

    Serial.print("Voltage: "); Serial.println(mainsVoltage);

    Serial.print("Current: "); Serial.println(loadCurrent);

    Serial.print("Power: "); Serial.println(powerValue);

    Serial.print("Water Leakage: "); Serial.println(waterLeakage ? "YES" : "NO");

    Serial.print("Alarm: "); Serial.println(alarmState ? "YES" : "NO");

  } else {

    wifiConnected = false;

  }

}

 

void setup() {

  Serial.begin(115200);

 

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(WATER_PIN, INPUT);

 

  digitalWrite(BUZZER_PIN, LOW);

 

  analogReadResolution(12);

  analogSetAttenuation(ADC_11db);

 

  dht.begin();

 

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();

  tft.setRotation(1);

 

  drawHeader();

  tft.setCursor(20, 80);

  tft.setTextColor(ILI9341_YELLOW);

  tft.setTextSize(2);

  tft.println("Initializing...");

 

  connectWiFi();

 

  delay(1000);

}

 

void loop() {

  readSensors();

  checkAlarmConditions();

  updateDisplay();

  sendDataToCloud();

 

  delay(2000);

}

