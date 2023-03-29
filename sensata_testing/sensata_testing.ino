#include <Wire.h>

uint8_t ADDRESS = byte(0x6C);  //byte(0x6C);
uint8_t REGISTER = byte(0x00);
uint8_t MULTIPLEXER = byte(0x70);  //multiplexer test


int tcaselect(uint8_t i) {
  //Serial.println(i);
  if (i > 7) return;

  Wire.beginTransmission(MULTIPLEXER);
  Wire.write(1 << i);
  int err = Wire.endTransmission();
  return err;
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
  Serial.println("--------------");
  Serial.print("Time:");
  Serial.println(millis());
  // Channel # = Placement (Technical Name) ; Alias ; I^2C Pin = Color - I^2C Pin = Color
  // Channel 0 = Base Plate Middle Hole (Nitrous Inlet) ; Steve ; SCL Pin = Yellow - SDA Pin = Green
  // Channel 1 = Other Base Plate Hole (IPA Inlet) ; Tony ; SCL Pin = Yellow - SDA Pin = Green
  // Channel 2 = Middle Hole Back Plate (Nitrous Tank) ; Bruce ;  SCL Pin = Yellow - SDA Pin = Brown
  // Channel 3 = Right of Middle Hole Back Plate (IPA Tank) ; Natasha ; SCL Pin = Yellow - SDA Pin = Brown
  // Channel 4 = Left of Middle Hole Back Plate (Nitrogen Inlet) ; Clint ; SCL Pin = Yellow - SDA Pin = Brown
  // Channel 5 = 
  for (int i = 0; i < 5; i++) {
    int multiErr = tcaselect(i);
    if (multiErr == 0) {
      Wire.beginTransmission(ADDRESS);
      int err = Wire.endTransmission();
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(": ");
      if (err == 0) {
        transmit();
      } else {
        Serial.print("Cannot connect at address. Error Code: ");
        Serial.println(err);
      }
    }
    else {
      Serial.print("Multiplexor Connection failed. Error code = ");
      Serial.println(multiErr);
    }
  }
  delay(200);
  //ADDRESS ++;
}

void transmit() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(byte(0x2E));

  Wire.endTransmission();

  Wire.requestFrom(ADDRESS, (uint8_t)4);

  // if (Wire.available()) {
  //   Serial.print("Received (");
  //   Serial.print(Wire.available(), HEX);
  //   Serial.print(" bytes): ");
  // } else {
  //   Serial.println("No data available!");
  // }

  // while (Wire.available()) {
  //   Serial.print(Wire.read(), HEX);
  //   Serial.print(" ");
  // }

  if (Wire.available()) {
    int32_t reading = Wire.read();
    //reading <<= 8;
    reading |= Wire.read() << 8;
    Serial.print("Bridge Temp: ");
    Serial.print(-40 + 165 * (reading + 16000) / 32000);
    Serial.print(" C, ");
    reading = Wire.read();
    //reading <<= 8;
    reading |= Wire.read() << 8;
    Serial.print("Pressure: ");
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