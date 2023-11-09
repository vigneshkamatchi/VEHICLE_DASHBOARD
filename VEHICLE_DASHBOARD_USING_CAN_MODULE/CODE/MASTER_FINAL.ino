#include <TinyGPS++.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <CAN.h>
TinyGPSPlus gps;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define SLAVE1_ID 0x101
#define SLAVE2_ID 0x102
unsigned long startTime = 0;
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  lcd.begin(16, 2);
  while (!CAN.begin(500E3)) {
    Serial.println("CAN initialization failed! Retrying...");
    delay(100);
  }
  Serial.println("CAN initialization successful!");
  startTime = millis();
}
void loop() {
  unsigned long currentTime = millis();
  while (Serial1.available()) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      lcd.clear();
      if (gps.location.isValid()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Lat:");
        lcd.print(gps.location.lat(), 6);
        lcd.setCursor(0, 1);
        lcd.print("Lon:");
        lcd.print(gps.location.lng(), 6);
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
        delay(1000);
        lcd.clear();
      }
      if (gps.time.isValid()) {
        lcd.clear();
        int istHour = (gps.time.hour() + 5) % 24; // Apply +5 hours offset
        int istMinute = (gps.time.minute() + 30) % 60; // Apply +30 minutes offset
        lcd.setCursor(0, 0);
        lcd.print("IST: ");
        if (istHour < 10) lcd.print("0");
        lcd.print(istHour);
        lcd.print(":");
        if (istMinute < 10) lcd.print("0");
        lcd.print(istMinute);
        lcd.print(":");
        if (gps.time.second() < 10) lcd.print("0");
        lcd.print(gps.time.second());
        Serial.print("IST: ");
        Serial.print(istHour);
        Serial.print(":");
        Serial.print(istMinute);
        Serial.print(":");
        Serial.println(gps.time.second());
        delay(1000);
      }
    }
  }
  if (CAN.parsePacket()) {
    if (CAN.packetId() == 0x124) {
      // Handle data received from Slave 1
      // Assuming Slave 1 sends a 4-byte float as temperature
      if (CAN.packetDlc() == 4) {
        float temperature;
        CAN.readBytes((char*)&temperature, 4);
        lcd.setCursor(0, 1);
        lcd.print("Tp1:");
        lcd.setCursor(5, 1);
        lcd.print(temperature, 0);
        lcd.print("C");
        delay(1000);
      }
    }
    else if (CAN.packetId() == 0x125) {
      // Handle data received from Slave 1
      // Assuming Slave 1 sends a 4-byte float as temperature
      if (CAN.packetDlc() == 4) {
        float temperature;
        CAN.readBytes((char*)&temperature, 4);
        lcd.setCursor(8, 1);
        lcd.print("Tp2:");
        lcd.setCursor(13, 1);
        lcd.print(temperature, 0);
        lcd.print("C");
        delay(1000);
      }
    }
    else if (CAN.packetId() == SLAVE1_ID) {
      lcd.clear();
      // Handle data received from Slave 1
      // Assuming Slave 1 sends a single char command ('s' or 't')
      char command = CAN.read();
      lcd.setCursor(0, 0);
      if (command == 's') {
        Serial.print("START");
        lcd.print("ER: Start");
        delay(1000);
        lcd.clear();
      } 
      else if (command == 't') {
        Serial.println("STOP");
        lcd.print("ER: Stop");
        delay(1000);
        lcd.clear();
      }
    }
    else if (CAN.packetId() == SLAVE2_ID) {
      lcd.clear();
      // Handle data received from Slave 2
      // Assuming Slave 2 sends a single char command ('s' or 't')
      char command = CAN.read();
      lcd.setCursor(0, 0);
      if (command == 's') {
        Serial.print("START");
        lcd.print("SR: Start");
        delay(1000);
        lcd.clear();
      } 
      else if (command == 't') {
        Serial.println("STOP");
        lcd.print("SR: Stop");
        delay(1000);
        lcd.clear();
      }
    }
  }
}