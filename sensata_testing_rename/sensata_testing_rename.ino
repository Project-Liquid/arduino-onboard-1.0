#include <Wire.h>

uint8_t ADDRESS = byte(0x6C);
uint8_t NEW_ADDRESS = byte(0x60);

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Started: SENSATA_TESTING.ino");
}

uint8_t ai = 0;

void loop() {
  Wire.beginTransmission(ai);
  if (Wire.endTransmission() == 0) {
    Serial.print("\nConnected successfully at: ");
    Serial.print(ai, HEX);
    Wire.beginTransmission(ai);
    Wire.write(byte(0x2E));
    Wire.requestFrom(ai, (uint8_t)6);
    Serial.print("~~ Got:");
    while (Wire.available()) {
      Serial.print(" ");
      Serial.print(Wire.read(), HEX);
    }
    Wire.endTransmission();
    Serial.println("Beginning hack...");
    Wire.beginTransmission(ai);
    Wire.write(byte(0xA0));
    Wire.write(byte(0x00));
    Wire.write(byte(0x00));
    Wire.endTransmission();
    Wire.beginTransmission(ai);
    Wire.write(byte(0x02));
    Wire.write(byte(0x00));
    Wire.write(byte(0x00));
    Wire.endTransmission();
    Wire.requestFrom(ai, (uint8_t)2);
    Wire.beginTransmission(ai);
    Wire.write(byte(0x42));
    Wire.write(byte(0x6C));
    Wire.write(byte(0x00));
    Wire.endTransmission();
    Wire.beginTransmission(ai);
    Wire.write(byte(0x80));
    Wire.write(byte(0x00));
    Wire.write(byte(0x00));
    Wire.endTransmission();
  } else {
    //Serial.print("\nCouldn't connect at: ");
    //Serial.print(ai, HEX);
  }
  ++ai;
}

void attemptToSet() {
  Wire.beginTransmission(ADDRESS);
  if (Wire.endTransmission() != 0) {
    Wire.beginTransmission(ADDRESS);
    Wire.write(byte(0x00));
    Wire.write(byte(NEW_ADDRESS));
    Wire.endTransmission();
    Serial.print("Set new address to ");
    Serial.println(NEW_ADDRESS, HEX);
  } else {
    Serial.println("Failed to find device at address.");
  }
  delay(1000);
}
