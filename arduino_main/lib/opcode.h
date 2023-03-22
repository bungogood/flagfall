/*
 * Opcodes, written in pure C.
 */

// 0x00 passed to prevent sending NULL bytes
#define SENSOR    0x01
#define MAGNET    0x02
#define LED       0x03

#define HANDSHAKE 0x10
#define ACK       0x20
#define QUIT      0xFF

/**
 * @brief
 * Enumerates the variants of operations to be worked by the arduino main program. 
 */
enum OpKind {
    Sensor, 
    Magnet, 
    Led, 
    Noop, 
    Quit
}; 

/**
 * @brief 
 * Checks the first byte of an instruction to determine its operation variant. 
 * 
 * @param serial_read_buffer Array of bytes representing the read instruction.
 * @return Corresponding `OpKind` enumeration type. 
 */
enum OpKind get_opkind(const unsigned char* serial_read_buffer) {
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