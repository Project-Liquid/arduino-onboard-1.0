/*
Documentation:

This code will print out the following at each time interview:

  --------------
  Time: ######
  Sensor 0 Temperature/Pressure (Nitrous Inlet)
  Sensor 1 Temperature/Pressure (IPA Inlet)
  Sensor 2 Temperature/Pressure (Nitrous Tank)
  Sensor 3 Temperature/Pressure (IPA Tank)
  Sensor 4 Temperature/Pressure (Nitrogen Inlet)

Command Mode Format:
 
“PDW A X Y Z….”
PDW = PinDigitalWrite to set a relay
A = 1 or 0
X, Y, Z, …. = relays you want to set

1 = N2O Pressurant Line (normally closed)
2 = IPA Pressurant Line (normally closed)
3 = N2O Run Valve (normally open)
4 = IPA Run Valve (normally open)
5 = N2O Vent Valve (normally open)
6 = IPA Vent Valve (normally open)
7 = Pneumatics Line Valve (normally closed) -- ACTUALLY NORMALLY OPEN
8 = Pneumatics Line Vent Valve (normally open)
9 = Purge Valve (normally closed)

Useful Commands:
- Setup (closing normally open valves) = PDW 1 3 4 5 6 8
- Pressurizing Tanks = PDW 1 1 2
- Stop Pressurizing Tanks = PDW 0 1 2
- Activating Pressurant Line = PDW 1 7
- Closing/venting Pressurant Line = PDW 0 7 8
- Opening Run lines = PDW 0 3 4
- Closing Run Lines = PDW 1 3 4
- Opening all vents = PDW 0 5 6 8
- Final Depress = PDW 1 1 2 7 —> PDW 0 5 6 8
- Reverting back to off state = PDW 0 1 2 3 4 5 6 7 8 9

Close all the vents and run valves = PDW 1 3 4 5 6 8


*/


#include <Wire.h>

uint8_t ADDRESS = byte(0x6C); //byte(0x6C);
uint8_t REGISTER = byte(0x00);
uint8_t MULTIPLEXER = byte(0x70);  //multiplexer test
int controlMode = 0;
// 0 = Command line input
// 1 = Command line input (without commands)
// 2 = list of 1s and 0s

unsigned int WRITE_INTERVAL = 10;
size_t MAX_WRITE = 63;
int last_write = 0;


int presLimit = 500;
String ventCommand1 = "PDW 1 7";
String ventCommand2 = "PDW 0 1 2 3 4 5 6 8 9";

//String writeBuffer = "AAAA\nBBBB\nCCCC\nDDDD\nEEEE\nFFFF\nGGGG\nHHHH\nIIII\nJJJJ\nKKKK\nLLLL\nMMMM\nNNNN\nOOOO\nPPPP\nQQQQ\nRRRR\nSSSS\nTTTT\nUUUU\nVVVV\nWWWW\nXXXX\nYYYY\nZZZZ\n";
String writeBuffer = "";
String cmd = "";

// Global for tracking whether a command was executed
int errorCode = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Started: SENSATA_TESTING.ino");
  

  Wire.setClock(1000);
  Wire.setWireTimeout(3000, true);
  
  pinMode((int8_t) 31, OUTPUT);
  pinMode((int8_t) 32, OUTPUT);
  pinMode((int8_t) 33, OUTPUT);
  pinMode((int8_t) 48, OUTPUT);
  pinMode((int8_t) 49, OUTPUT);
  pinMode((int8_t) 50, OUTPUT);
  pinMode((int8_t) 51, OUTPUT);
  pinMode((int8_t) 52, OUTPUT);
  pinMode((int8_t) 53, OUTPUT);
}

void loop() {
  //Serial.println("--------------------------");
  //Serial.print("Start loop with address: ");
  //input:
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    errorCode = 0;
    if (input.length() < 3) {
      error(1);
    } else {
      sort(input);
    }
    if (errorCode != 0) {
      Serial.print("ERR");
      Serial.print(errorCode);
      Serial.println(input);
    }
  }

  //display:
  //Time:
  //Serial.println("--------------");
  //Serial.print("Time = ");
  if (controlMode == 0 ) {
    Serial.print("LOG");
  }
  Serial.print(millis());
  Serial.print(",");

  //printing relay states:
  for (int i = 0; i < 9; i++) {
    String str = String(i+1);
    pinDigitalRead(str);
    Serial.print(",");
  }

  //printing sensor readings:
  for (int i = 0; i < 5; i++) {
    int multiErr = tcaselect(i);
    if (multiErr == 0) {
      Wire.beginTransmission(ADDRESS);
      int err = Wire.endTransmission();
      //Serial.print("Sensor ");
      //Serial.print(i);
      //Serial.print(": ");
      if (err == 0) {
        transmit(i);
      } else {
        //Serial.print("Cannot connect at address. Error Code: ");
        //Serial.println(err);
        Serial.print("-1,-1");
      }
    }
    else {
      Serial.print("Multiplexor Connection failed. Error code = ");
      Serial.println(multiErr);
    }
    if (i < 4) {
      Serial.print(",");
    }
  }

  Serial.println("");
  
}

int tcaselect(uint8_t i) {
  //Serial.println(i);
  if (i > 7) return;

  Wire.beginTransmission(MULTIPLEXER);
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
    //reading <<= 8;
    reading |= Wire.read() << 8;
    int tempValue = -40 + 165 * (reading + 16000) / 32000;
    Serial.print(tempValue);
    //Serial.print(" C, ");
    Serial.print(",");
    reading = Wire.read();
    //reading <<= 8;
    reading |= Wire.read() << 8;
    int presValue = presMax * (reading + 16000) / 32000;
    Serial.print(presValue);
    if (i < 4 && presValue > presLimit) {
      sort(ventCommand1);
      sort(ventCommand2);
    } 
    //Serial.print(" psi, ");
  }

  // while (Wire.available()) {

  //   Serial.print(Wire.read(), HEX);
  //   Serial.print(" ");
  // }
  //Serial.println("");
}

void sort(String message) {
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
  } else if (code.compareTo("REP") == 0) {
    repeat(data);
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

void pinDigitalRead(String data) { // PDR
  // if (data.length() != 2) {
  //   error(3);
  //   return;
  // }
  char pin = data.charAt(0);
  //Serial.print(pin);
  int pinNum = getPinOnBoard(pin);
  if (pinNum == -1) {
    error(3);
    return;
  }
  int result = digitalRead((uint8_t)pinNum);
  //Serial.print("PIN ");
  Serial.print(result);
}

int getPinOnBoard(char pin){
  if (pin == '1') { //N20 pressurant line
    return(32);
  } else if (pin == '2') { //IPA pressurant line
    return(33);
  } else if (pin == '3') { //N20 Run Valve
    return(52);
  }else if (pin == '4') { //IPA Run Valve
    return(51);
  }else if (pin == '5') { //N20 Vent Valve
    return(49);
  }else if (pin == '6') { //IPA Vent Valve
    return(50);
  }else if (pin == '7') { //Pneumatics Line Valve
    return(53);
  }else if (pin == '8') { //Pneumatics Line Vent Valve
    return(48);
  }else if (pin == '9') { //Purge Valve
    return(31);
  } else {
    return -1;
  }
}

void pinDigitalWrite(String data) { // PDW
  if (data.length() < 3) {
    error(3);
    return;
  }
  char dataVal =  data.charAt(0); 

  for (int pos = 2; pos < data.length(); pos +=2) {
    char pin = data.charAt(pos);
    //Serial.println(pin);
    int pinNum = getPinOnBoard(pin);
    if (pinNum == -1) {
      error(3);
      return;
    }
    if (dataVal == '0') {
      digitalWrite((uint8_t)pinNum, LOW);
      //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to LOW");
    } else if (dataVal == '1') {
      digitalWrite((uint8_t)pinNum, HIGH);
      //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to HIGH");
    } else {
      error(3);
      return;
    }
  }
}
// Demo and debug commands

void echo(String data) { // ECH
  //Serial.print("LOG ");
  Serial.println(data);
}

void repeat(String data) { // REP
  if (data.length() < 2) {
    error(3);
    return;
  }
  int numTimes = data.substring(0, 2).toInt();
  if (numTimes == 0) {
    error(3);
    return;
  }
  String toRepeat = data.substring(2);
  if (toRepeat.length() > MAX_WRITE - 4) {
    error(3);
    return;
  }
  for (int i = 0; i < numTimes; ++i) {
    echo(toRepeat);
  }
}

void processList(String input) {
  String list[] = {"","","","","","","","","","","","","","","","","","","",""};
  int currIndex = 0;
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == ',') {
      currIndex += 1;
    } else {
      list[currIndex].concat(input.charAt(i));
    }
  }
  for (int i = 0; i < 9; i++) {
    String str = String(i+1);
    int pinNum = getPinOnBoard(str.charAt(0));
    if (list[i].charAt(0) == '0') {
      digitalWrite((uint8_t)pinNum, LOW);
      //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to LOW");
    } else if (list[i].charAt(0) == '1') {
      digitalWrite((uint8_t)pinNum, HIGH);
      //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to HIGH");
    }
  }
}
