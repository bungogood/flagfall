#include "communication.h"

#define BAUD_RATE 115200

char buffer[256]; 
bool handshake_flag = false; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!handshake_flag) {
    handshake_flag = handshake(); 
    if (handshake_flag) Serial.println("Listening..."); 
  } else {
    memset(buffer, 0, 256); // Set to NULL
    listen(LF_TERM, buffer, 256); 
    Serial.println(String(buffer)); 
  }
}