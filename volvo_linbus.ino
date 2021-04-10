#include <SoftwareSerial.h>

// https://github.com/zapta/linbus/tree/master/analyzer/arduino
#include "lin_frame.h"

// Pins we use for MCP2004
#define RX_PIN 10
#define TX_PIN 11
#define FAULT_PIN 14
#define CS_PIN 8

#define SYN_FIELD 0x55
#define SWM_ID 0x20

SoftwareSerial LINBusSerial(RX_PIN, TX_PIN);

//Lbus = LIN BUS from Car
//Vss = Ground
//Vbb = +12V

// MCP2004 LIN bus frame:
// ZERO_BYTE SYN_BYTE ID_BYTE DATA_BYTES.. CHECKSUM_BYTE
// header:
//  zero_byte = break
//  SYN_BYTE = synchronisation byte
// ID_BYTE = identification byte
// message:
// DATA_BYTES = data byte, 1 - 4 bytes
// checksum_byte = checksum calculated


// IGN_KEY_ON     50 E 0 F1


byte b, i, n;
LinFrame frame;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Open serial communications to host (PC) and wait for port to open:
  Serial.begin(9600, SERIAL_8E1);
  Serial.println("LIN Debugging begins");

  LINBusSerial.begin(9600);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  pinMode(FAULT_PIN, OUTPUT);
  digitalWrite(FAULT_PIN, HIGH);

  frame = LinFrame();
}

void loop() {
  if (LINBusSerial.available()) {
    b = LINBusSerial.read();
    n = frame.num_bytes();
    // if sync field & more than 2 bytes in frame and previouse frame is zero
    if (b == SYN_FIELD && n > 2 && frame.get_byte(n - 1) == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      frame.pop_byte();
      handle_frame();
      frame.reset();
      digitalWrite(LED_BUILTIN, LOW);
    } else if (n == LinFrame::kMaxBytes) {
      frame.reset();
    } else {
      frame.append_byte(b);
    }
  }
}

void handle_frame() {
  if (frame.get_byte(0) != SWM_ID)
    return;

  // skip zero values 20 0 0 0 0 FF
  if (frame.get_byte(5) == 0xFF)
    return;

  if (!frame.isValid())
    return;


  dump_frame();
}

void dump_frame() {
  for (i = 0; i < frame.num_bytes(); i++) {
    Serial.print(frame.get_byte(i), HEX);
    Serial.print(" ");
  }
  Serial.println();
}


