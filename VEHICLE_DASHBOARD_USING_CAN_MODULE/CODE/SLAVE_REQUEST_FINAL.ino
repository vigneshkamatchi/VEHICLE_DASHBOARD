#include <SPI.h>
#include <CAN.h>
#include <DFRobot_DHT11.h>
#include <Keypad.h>

DFRobot_DHT11 DHT;
#define DHT11_PIN A1

#include <Arduino.h>

#define RECEIVER_ID 0x102

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
String pressedCharacters = ""; // Stores the pressed characters

bool startRequested = false;
bool stopRequested = false;

// Keep track of whether the password has been entered or not
bool passwordEntered = false;

void setup() {
  Serial.begin(9600);

  while (!CAN.begin(500E3)) {
    Serial.println("CAN initialization failed! Retrying...");
    delay(1000);
  }
  Serial.println("CAN initialization successful!");
  CAN.filter(RECEIVER_ID, 0); // Set filter to only receive packets with RECEIVER_ID

  // Initialize the passwordEntered flag to false
  passwordEntered = false;
}

void loop() {
  char key = keypad.getKey();

  static String entered = ""; // Static variable to preserve the entered password

  if (key) {
    if (key == '#') { // Check if the key is the "Enter" key
      if (entered == "1234") {
        startRequested = true;
        stopRequested = false;
        passwordEntered = true; // Set passwordEntered flag
        Serial.println("Start request received.");
        CAN.beginPacket(0x102);
        CAN.write('s');
        CAN.endPacket();
      } else if (entered == "5678") {
        stopRequested = true;
        startRequested = false;
        passwordEntered = true; // Set passwordEntered flag
        Serial.println("Stop request received.");
        CAN.beginPacket(0x102);
        CAN.write('t');
        CAN.endPacket();
      }
      entered = ""; // Reset the entered password after processing
    }
    else {
      // If a numeric key is pressed, add it to the entered password
      entered += key;
      delay(100); // Small delay to avoid multiple keypresses
    }

    // Check for special keys to reset the entered password
    if (key == '*') {
      passwordEntered = false;
      entered = "";
      startRequested = false; // Reset startRequested flag
      stopRequested = false;  // Reset stopRequested flag
      Serial.println("Entered keys reset.");
    }
  }
  
  if (startRequested) {
    float temperature = readTemperature();

    // Prepare the data to be sent
    unsigned char data[4];
    memcpy(data, &temperature, sizeof(temperature));

    // Send the data through CAN bus
    CAN.beginPacket(0x124);
    CAN.write(data, 4);
    CAN.endPacket();
  }
  delay(100);
}

float readTemperature() {
  DHT.read(DHT11_PIN);
  float temperature = DHT.temperature;
  if(temperature!=255){
  Serial.println(temperature);
  return temperature;
  }
}
