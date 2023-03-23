#include "opcode.h"
#include <Arduino.h>
#include <FastLED.h>

OpKind get_opkind(const uint8_t* serial_read_buffer); 
bool handshake(); 

/**
 * @brief 
 * Data structure for providing a view into a stored instruction. 
 */
typedef struct Operation {
public: 
    const OpKind         kind; 
    const uint8_t *const data; 

    Operation() = delete; 
    Operation(const uint8_t* instruction, const size_t instr_len) 
    : _data_len(instr_len - 1), 
      _data_base(instruction + 1), 
      kind(get_opkind(instruction)), 
      data(_data_base) {}
    ~Operation() = default; 

    size_t data_len() { return _data_len; }
    CRGB* try_into_CRGB(); 
private: 
    const uint8_t *const _data_base; 
    const size_t         _data_len; 
}; 

/**
 * @brief 
 * Checks the first byte of an instruction to determine its operation variant. 
 * 
 * @param serial_read_buffer Array of bytes representing the read instruction.
 * @return Corresponding `OpKind` enumeration type. 
 */
OpKind get_opkind(const uint8_t* serial_read_buffer) {
    switch(*serial_read_buffer) {
        case SENSOR: 
            return Sensor; 
        case MAGNET: 
            return Magnet; 
        case LED: 
            return Led; 
        case QUIT: 
            return Quit; 
        default: 
            return Noop; 
    }
}

/**
 * Perform handshake with Raspi host *once*. 
 * 
 * @return true if handshake is successful. 
 * @return false otherwise. 
 */
bool handshake() {
    /* Listen for PC initiation */
    uint8_t handshake_buf[2] {0}; 
    // Serial.dtr()
    // while (!Serial.rts()) continue;
    Serial.readBytes(handshake_buf, sizeof(handshake_buf)); 

    uint8_t arduino_id; 
    if (handshake_buf[0] == HANDSHAKE) {
        // buffer[1..] should be Arduino ID, which is assumed to be non-zero
        arduino_id = handshake_buf[1]; 
        Serial.write(handshake_buf, sizeof(handshake_buf)); 
        Serial.flush();
        memset(handshake_buf, 0, 2); 

        Serial.readBytes(handshake_buf, 2); 

        if (handshake_buf[0] == HANDSHAKE && handshake_buf[1] == arduino_id) {
            handshake_buf[0] = ACK; 
            Serial.write(ACK); 
            Serial.flush();
            return true; 
        } else {
            return false; 
        }

    } else {
        return false; 
    }
}