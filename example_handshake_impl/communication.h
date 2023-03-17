#include <Arduino.h>
#include <pins_arduino.h>

// I was thinking of using diff. messages but using diff. baud rates suffice. 
const char* const HANDSHAKE_PROMPT   = "PC TO ARDUINO_1\n"; 
const char* const HANDSHAKE_RESPONSE = "ARDUINO_1 TO PC\n"; 
const char LF_TERM = '\n'; 

/**
 * Perform handshake with Raspi host *once*. 
 * 
 * @return true if handshake is successful. 
 * @return false otherwise. 
 */
bool handshake() {
    /* 1. Listen for PC initiation */
    if (Serial.available()) {
        String prompt = Serial.readString(); 
        if (prompt == HANDSHAKE_PROMPT) {
            Serial.write(HANDSHAKE_RESPONSE); 
        } 
        /* 2. Wait for PC readback */
        while (true) {
            if (Serial.available()) {
                String readback = Serial.readString(); 
                if (readback == HANDSHAKE_RESPONSE) {
                    Serial.println("Handshake successful!"); 
                    return true; 
                }
            }
        }
    }
    return false; 
}

/**
 * Block until receives something from serial connection, then fills the buffer accordingly. 
 * 
 * This should? not cause buffer overflows as long as Serial.readBytesUntil does not cause buffer 
 * overflow. 
 * 
 * @param term_byte Terminator byte
 * @param buffer Pointer to beginning of buffer
 * @param count Size of buffer -- unchecked
 * @return size_t Number of bytes read into buffer
 */
size_t listen(char term_byte, char* buffer, size_t count) {
    while (true) {
        if (Serial.available()) {
            return Serial.readBytesUntil(term_byte, buffer, count); 
        }
    }
}

