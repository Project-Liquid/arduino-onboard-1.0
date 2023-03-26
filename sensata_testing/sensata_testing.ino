#include <Wire.h>

uint8_t ADDRESS = byte(0x6C);
uint8_t REGISTER = byte(0x00);

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Started: SENSATA_TESTING.ino");
  Wire.setClock(1000);
  Wire.setWireTimeout(3000, true);
}

void loop() {
  Serial.println("Start loop");
  Wire.beginTransmission(ADDRESS);
  if (Wire.endTransmission() == 0) {
    Serial.println("End transmission did not crash");
    transmit();
  } else {
    Serial.println("Cannot connect at address.");
  }
  delay(1000);
}

void transmit() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(byte(0x2E));

  Wire.endTransmission();

  Wire.requestFrom(ADDRESS, (uint8_t)4);

  if (Wire.available()) {
    Serial.print("Received (");
    Serial.print(Wire.available(), HEX);
    Serial.print(" bytes): ");
  } else {
    Serial.println("No data available!");
  }

  // while (Wire.available()) {
  //   Serial.print(Wire.read(), HEX);
  //   Serial.print(" ");
  // }

  if (Wire.available()) {
    int32_t reading = Wire.read();
    //reading <<= 8;
    reading |= Wire.read() << 8;
    Serial.print(-40 + 165 * (reading + 16000) / 32000);
    Serial.print(" C ");
    reading = Wire.read();
    //reading <<= 8;
    reading |= Wire.read() << 8;
    Serial.print(2900 * (reading + 16000) / 32000);
    Serial.print(" psi ");
  }

  // while (Wire.available()) {

  //   Serial.print(Wire.read(), HEX);
  //   Serial.print(" ");
  // }
  Serial.println("");
}

// 0x6C == 108