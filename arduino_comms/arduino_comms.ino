#include "communication.hpp"

bool volatile handshake_flag = false; 
uint8_t buf[512] {0}; 

void setup() {
    Serial.begin(115200); 
    // while (!handshake_flag) {
    //     handshake_flag = handshake(); 
    // }
}

void loop() {
    if (Serial.available()) {
        size_t read_amnt = Serial.readBytes(buf, sizeof(buf)); 
        Serial.write(buf, read_amnt); 
        Serial.flush(); 
    }
}
