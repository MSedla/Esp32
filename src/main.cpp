#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define Moisture_pin 34  // Pin for soil moisture sensor
#define scl 22          // Pin for I2C SCL
#define sda 21          // Pin for I2C SDA
#define TRIG_PIN 5     // Pin for ultrasonic sensor TRIG
#define ECHO_PIN 18    // Pin for ultrasonic sensor ECHO
#define pumpPin 19    // Pin for water pump control

float vlhkost_pudy_percent = 0; // Variable to store soil moisture percentage
int vlhkost_pudy_MIN = 4095; // Minimum analog value for soil moisture sensor (dry soil)
int vlhkost_pudy_MAX = 0;    // Maximum analog value for soil moisture
int puda_MIN_percent = 30; // Minimum soil moisture percentage to turn off the pump

float osa_x = 0.0; // Variable for X axis
float osa_y = 0.0; // Variable for Y axis
float osa_z = 0.0; // Variable for Z axis

int pozice_x = 100; // Variable for X position
int pozice_y = 25; // Variable for Y position
int sirka_obdelniku = 10; // Variable for width
int vyska_obdelniku = 50;  // Variable for height

int pauza = 200; // Pause duration in milliseconds

// SSD1306 display object
Adafruit_SSD1306 display(128, 64, &Wire, -1);

//Wifi credentials
const String ssid     = "SSID";
const String password = "PASSWORD";

void setup() {
  Serial.begin(115200);
  
  // Initialize the SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Setup pins for ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(pumpPin, OUTPUT);
}

struct BMPData {
  float teplota;
  float tlak;
};

BMPData BMP280Setup() {
  Adafruit_BMP280 bmp;
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor!");
    while (1);
  }
  
  BMPData data;
  data.teplota = bmp.readTemperature();
  data.tlak = bmp.readPressure() / 100.0F;
  
  return data;  // Vrátí obě hodnoty
}


float soilMoistureSetup() {
  int vlhkost_pudy_analog = analogRead(Moisture_pin); // Initialize soil moisture sensor pin
  float vlhkost_pudy_percent = map(vlhkost_pudy_analog, vlhkost_pudy_MAX, vlhkost_pudy_MIN, 0, 100); // Map analog value to percentage
  
  Serial.print("Vlhkost půdy: ");
  Serial.print(vlhkost_pudy_percent);
  Serial.println(" %");

  display.print("Vlhkost pudy:");
  display.print(vlhkost_pudy_percent);
  display.println(" %");

  return vlhkost_pudy_percent;  // Vrať hodnotu
}

float HCSR04Setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  long odezva = pulseIn(ECHO_PIN, HIGH);
  float vzdalenost_hladiny = odezva * 0.034 / 2; // Calculate distance in cm
  
  Serial.print("Vzdálenost: ");
  Serial.print(vzdalenost_hladiny);
  Serial.println(" cm");
  return vzdalenost_hladiny;  // Vrať hodnotu
}

void loop() {
  BMPData bmp_data = BMP280Setup();
  Serial.print("Teplota:");
  Serial.println(bmp_data.teplota);
  Serial.print("Tlak: ");
  Serial.println(bmp_data.tlak);

  display.setCursor(0, 0);

  vlhkost_pudy_percent = soilMoistureSetup();  // Ulož vrácené hodnotu
  float vzdalenost = HCSR04Setup();  // Ulož vrácené hodnotu
  
  if (vlhkost_pudy_percent < puda_MIN_percent) { // If soil moisture is below 30%
    digitalWrite(pumpPin, HIGH); // Turn on the water pump
    Serial.println("Pumpa zapnuta");
  } else {
    digitalWrite(pumpPin, LOW); // Turn off the water pump
    Serial.println("Pumpa vypnuta");
  }

  float objem_vody = (osa_x * osa_y * (osa_z - vzdalenost)); // Calculate water volume percentage
  Serial.print("Objem vody v nádrži: ");
  Serial.print(objem_vody);
  Serial.println(" ml");

  display.print("Rezervoar:");
  display.print(objem_vody);
  display.println(" ml");
  display.print("Teplota:");
  display.print(bmp_data.teplota);
  display.println(" °C");
  display.print("Tlak:");
  display.print(bmp_data.tlak);
  display.println(" hPa");

  display.drawRect(pozice_x, pozice_y, sirka_obdelniku, vyska_obdelniku, SSD1306_WHITE); // Draw rectangle on display
  if (objem_vody > 0) {
  display.fillRect(pozice_x, pozice_y, sirka_obdelniku, (osa_z - vzdalenost) / osa_z * vyska_obdelniku, SSD1306_WHITE); // Fill rectangle based on water volume
  }
  display.display(); // Update the display with new data

  delay(pauza); // Wait for pauza seconds before next reading
}