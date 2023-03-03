unsigned int WRITE_INTERVAL = 10;
size_t MAX_WRITE = 63;
int last_write = 0;

//String writeBuffer = "AAAA\nBBBB\nCCCC\nDDDD\nEEEE\nFFFF\nGGGG\nHHHH\nIIII\nJJJJ\nKKKK\nLLLL\nMMMM\nNNNN\nOOOO\nPPPP\nQQQQ\nRRRR\nSSSS\nTTTT\nUUUU\nVVVV\nWWWW\nXXXX\nYYYY\nZZZZ\n";
String writeBuffer = "";
String cmd = "";

// Global for tracking whether a command was executed
int errorCode = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
}

void writeFromBuffer() {
  // Write as much of the write buffer as possible to the serial, without blocking
  // Note that this will delete messages if there is a string longer than 63 bytes without newlines.
  if (!Serial) {
    return;
  }
  size_t availableSpace = Serial.availableForWrite();
  if (availableSpace == 0) {
    return;
  }
  size_t bufsize = writeBuffer.length();
  if (bufsize == 0) {
    return;
  }
  if (bufsize <= availableSpace) {
    // If we can write the whole thing, do that
    Serial.print(writeBuffer);
    writeBuffer = "";
  } else {
    // Otherwise, write an integer number of commands to the serial
    size_t lastNewline = 0;
    cmd = "";
    for (size_t i = 0; i < availableSpace; ++i) {
      cmd.concat(writeBuffer.charAt(i));
      if (writeBuffer.charAt(i) == '\n') {
        lastNewline = i;
        Serial.print(cmd);
        cmd = "";
      }
    }
    if (lastNewline == 0) {
      // This means there's a super long command blocking the buffer
      // Panic and delete everything we just looked at, hopefully will resolve itself
      writeBuffer.remove(0, availableSpace);
      error(1);
    } else {
      // Delete the commands that have been sent
      writeBuffer.remove(0, lastNewline + 1);
    }
  }
}

void loop() {
  if (millis() - last_write > WRITE_INTERVAL) {
    writeFromBuffer();
    last_write = millis();
  }
  // Read from serial until newline
  // Process and route message
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    errorCode = 0;
    if (input.length() < 3) {
      error(1);
    } else {
      sort(input);
    }
    if (errorCode == 0) {
      output("CON", input);
    } else {
      output("ERR", String(errorCode).concat(input));
    }
  }
}

void sort(String message) {
  // Dispatch the message handling
  // Requires that message is at least 3 characters
  String code = message.substring(0, 3);
  String data = message.substring(3);
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

void output(String code, String data) {
  if (data.length() + code.length() + 1 > MAX_WRITE) {
    error(3);
    return;
  }
  writeBuffer.concat(code);
  writeBuffer.concat(data);
  writeBuffer.concat('\n');
}

// SECTION: Commands
// These functions represent commands that the arduino will accept over serial

void pinDigitalRead(String data) { // PDR
  if (data.length() != 2) {
    error(3);
    return;
  }
  int pinNum = data.substring(0, 2).toInt();
  if (pinNum == 0) {
    error(3);
    return;
  }
  int result = digitalRead((uint8_t)pinNum);
  output("PIN", String(result));
}

void pinDigitalWrite(String data) { // PDW
  if (data.length() != 3) {
    error(3);
    return;
  }
  int pinNum = data.substring(0, 2).toInt();
  if (pinNum == 0) {
    error(3);
    return;
  }
  if (data.charAt(2) == '0') {
    digitalWrite((uint8_t)pinNum, LOW);
    //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to LOW");
  } else if (data.charAt(2) == '1') {
    digitalWrite((uint8_t)pinNum, HIGH);
    //Serial.print("Set pin "); Serial.print((uint8_t)pinNum); Serial.println(" to HIGH");
  } else {
    error(3);
    return;
  }
}

// Demo and debug commands

void echo(String data) { // ECH
  output("LOG", data);
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