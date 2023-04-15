
#include "Arduino.h"

#include <Wire.h>

uint8_t ADDRESS      = byte(0x6C);  // byte(0x6C);
uint8_t MPX_I2C_ADDR = byte(0x70);  // multiplexer test
int controlMode      = 0;           // TODO: remove if not using
// 0 = Command line input
// 1 = Command line input (without commands)
// 2 = list of 1s and 0s

int presLimit       = 500;
String ventCommand1 = "PDW 1 7";
String ventCommand2 = "PDW 0 1 2 3 4 5 6 8 9";

// Global for tracking whether a command was executed
int errorCode = 0;

const size_t numValves  = 9;  //
const size_t numSensors = 5;  //

// array of pins for valves
// TODO: update serial to be indexed from 0
uint8_t valvePins[numValves] = {32, 33, 52, 51, 49, 50, 53, 48, 31};

// uint8_t sensorPins[numValves] = {32, 33, 52, 51, 49, 50, 53, 48, 31}; //TODO:
// upadte

void setup() {
    Wire.begin();
    Serial.begin(9600);  // bits per second
    Serial.println("Started: onboard.ino");

    Wire.setClock(1000);              // set the I2C clock rate in Hz
    Wire.setWireTimeout(3000, true);  // ms

    for (size_t pin = 0; pin < 9; ++pin) { pinMode(valvePins[pin], OUTPUT); }
}

void loop() {
    // Get input from serial
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        errorCode    = 0;
        if (input.length() < 3) {
            error(1);
        } else {
            processCommand(input);
        }
        if (errorCode != 0) {
            Serial.print("ERR");
            Serial.print(errorCode);
            Serial.println(input);
        }
    }

    // TODO; continue refactoring from here

    pullSensorsAndLog();
}

void pullSensorsAndLog() {
    if (controlMode == 0) { Serial.print("LOG"); }
    Serial.print(millis());
    Serial.print(",");

    // printing relay states:
    for (int i = 0; i < 9; i++) {
        String str = String(i);
        pinDigitalRead(str);
        Serial.print(",");
    }

    // printing sensor readings:
    for (int i = 0; i < 5; i++) {
        int multiErr = tcaselect(i);
        if (multiErr == 0) {
            Wire.beginTransmission(ADDRESS);
            int err = Wire.endTransmission();
            // Serial.print("Sensor ");
            // Serial.print(i);
            // Serial.print(": ");
            if (err == 0) {
                transmit(i);
            } else {
                // Serial.print("Cannot connect at address. Error Code: ");
                // Serial.println(err);
                Serial.print("-1,-1");
            }
        } else {
            Serial.print("Multiplexor Connection failed. Error code = ");
            Serial.println(multiErr);
        }
        if (i < 4) { Serial.print(","); }
    }

    Serial.println("");
}

int tcaselect(uint8_t i) {
    // Serial.println(i);
    if (i > 7) return;

    Wire.beginTransmission(MPX_I2C_ADDR);
    Wire.write(1 << i);
    int err = Wire.endTransmission();
    return err;
}

void transmit(int i) {
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
    int presMax = 0;
    if (i == 4) {
        presMax = 2900;
    } else {
        presMax = 1450;
    }

    if (Wire.available()) {
        int32_t reading = Wire.read();
        // reading <<= 8;
        reading |= Wire.read() << 8;
        int tempValue = -40 + 165 * (reading + 16000) / 32000;
        Serial.print(tempValue);
        // Serial.print(" C, ");
        Serial.print(",");
        reading = Wire.read();
        // reading <<= 8;
        reading |= Wire.read() << 8;
        int presValue = presMax * (reading + 16000) / 32000;
        Serial.print(presValue);
        if (i < 4 && presValue > presLimit) {
            processCommand(ventCommand1);
            processCommand(ventCommand2);
        }
        // Serial.print(" psi, ");
    }

    // while (Wire.available()) {

    // Serial.print(Wire.read(), HEX);
    // Serial.print(" ");
    // }
    // Serial.println("");
}

void processCommand(String message) {
    if (controlMode == 2) {
        processList(message);
        return;
    } else if (controlMode == 1) {
        pinDigitalWrite(message);
        return;
    }

    // Dispatch the message handling
    // Requires that message is at least 3 characters
    String code = message.substring(0, 3);
    String data = message.substring(4);
    // compareTo returns 0 if strings match
    if (code.compareTo("ECH") == 0) {
        echo(data);
    } else if (code.compareTo("PDW") == 0) {
        pinDigitalWrite(data);
    } else if (code.compareTo("PDR") == 0) {
        pinDigitalRead(data);
    } else {
        error(2);
    }
}

void error(int code) {
    errorCode = code;
    // CODES:
    // 0: No error
    // 1: Internal error (reached an unreachable state)
    // 1: Message too short
    // 2: Unknown code
    // 3: Reject message data
}

void pinDigitalRead(String data) {  // PDR
    // if (data.length() != 2) {
    //   error(3);
    //   return;
    // }
    char pin = data.charAt(0);
    if ((pin < 0) || (pin > 8)) {
        error(3);
        return;
    }
    int pinNum = valvePins[pin];
    int result = digitalRead((uint8_t)pinNum);
    // Serial.print("PIN ");
    Serial.print(result);
}

void pinDigitalWrite(String data) {  // PDW
    if (data.length() < 3) {
        error(3);
        return;
    }
    char dataVal = data.charAt(0);

    for (int pos = 2; pos < data.length(); pos += 2) {
        char pin = data.charAt(pos);
        // Serial.println(pin);
        if ((pin < 0) || (pin > 8)) {
            error(3);
            return;
        }
        int pinNum = valvePins[pin];
        if (dataVal == '0') {
            digitalWrite((uint8_t)pinNum, LOW);
            // Serial.print("Set pin "); Serial.print((uint8_t)pinNum);
            // Serial.println(" to LOW");
        } else if (dataVal == '1') {
            digitalWrite((uint8_t)pinNum, HIGH);
            // Serial.print("Set pin "); Serial.print((uint8_t)pinNum);
            // Serial.println(" to HIGH");
        } else {
            error(3);
            return;
        }
    }
}

// Demo and debug commands

void echo(String data) {  // ECH
    // Serial.print("LOG ");
    Serial.println(data);
}

void processList(String input) {
    String list[] = {"", "", "", "", "", "", "", "", "", "",
                     "", "", "", "", "", "", "", "", "", ""};
    int currIndex = 0;
    for (int i = 0; i < input.length(); i++) {
        if (input.charAt(i) == ',') {
            currIndex += 1;
        } else {
            list[currIndex].concat(input.charAt(i));
        }
    }
    for (int i = 0; i < 9; i++) {
        String str = String(i + 1);
        int pinNum = valvePins[str.charAt(0)];
        if (list[i].charAt(0) == '0') {
            digitalWrite((uint8_t)pinNum, LOW);
            // Serial.print("Set pin "); Serial.print((uint8_t)pinNum);
            // Serial.println(" to LOW");
        } else if (list[i].charAt(0) == '1') {
            digitalWrite((uint8_t)pinNum, HIGH);
            // Serial.print("Set pin "); Serial.print((uint8_t)pinNum);
            // Serial.println(" to HIGH");
        }
    }
}
