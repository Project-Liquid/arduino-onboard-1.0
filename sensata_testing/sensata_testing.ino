#include <Wire.h>

uint8_t ADDRESS = byte(0x6C); //byte(0x6C);
uint8_t REGISTER = byte(0x00);
uint8_t MULTIPLEXER = byte(0x70); //multiplexer test


void tcaselect(uint8_t i) {
  println(i);
  if (i > 7) return;

  Wire.beginTransmission(MULTIPLEXER);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Started: SENSATA_TESTING.ino");
  

  Wire.setClock(1000);
  Wire.setWireTimeout(3000, true);
  
}

void loop() {
  //Serial.println("--------------------------");
  //Serial.print("Start loop with address: ");
  //Serial.println(ADDRESS, HEX);
  tcaselect(0);
  Wire.beginTransmission(ADDRESS);
  int err = Wire.endTransmission();
  if (err == 0) {
    //Serial.println("End transmission did not crash");
    transmit();
  } else {
    Serial.print("Cannot connect at address. Error Code: ");
    Serial.println(err);
  }
  delay(200);
  //ADDRESS ++;
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