#include <FastLED.h>
#include "../../flagfall/example_handshake_impl/communication.h"

// ===================== Serial-comm Configuration =====================
bool volatile handshake_flag = false; 

// ====================== LED Configuration ======================
#define LED_PIN 2
#define NUM_LEDS 64
// LED State
CRGB leds[NUM_LEDS];


// =================== Reed Switch Configuration ===================
int rsw_in_pin[8]  = { 30, 31, 32, 33, 34, 35, 36, 37 };
int rsw_out_pin[8] = { 47, 46, 45, 44, 43, 42, 41, 40 };
// Reed Switch State
bool rsw_state[8][8] = { 0 };


// ====================== CoreXY Configuration ======================
/* 
*  CoreXY Layout
*   .---------.
*   |         |
*   |         |
*   |[O]======|
*  [M1]-----[M2]
*/
#define HIGH_SPD 1000 
#define LOW_SPD  200
#define CALI_SPD 1500
#define SPD_TO_INTERVAL(spd) (int) 10000 / spd
#define MM_TO_STEPS(mm) (long) mm * 159

#define UP         0
#define DOWN       1
#define LEFT       2
#define RIGHT      3
#define UP_LEFT    4
#define UP_RIGHT   5
#define DOWN_LEFT  6
#define DOWN_RIGHT 7

#define LIMIT_SW_PIN A0
// Stepper Motor Pins
typedef struct StepperMotor {
    const int DIR_PIN;
    const int STEP_PIN;
    const int DISABLE_PIN;
} StepperMotor;

StepperMotor M1 = { 4, 5, 12 }, M2 = { 7, 6, 8 };

// Stepper Motor State
typedef struct Position {
    int x;
    int y;
} Position;

Position current_pos = { -1, -1 };
const int MAX_X = 500;
const int MAX_Y = 560;
const int MIN_X = 0;
const int MIN_Y = 0;
const int OFFSET_X = 0;
const int OFFSET_Y = 0;

// ====================== Electromagnet Configuration ======================
#define ELECTROMAGNET_PIN 9


// ====================== Main Program ======================

void setup() {
    Serial.begin(115200);
    LED_setup(16);
    rsw_setup();
    // core_xy_setup();
    // calibration();

}

void loop() {
    /* [comm] handshake */
    if (!handshake_flag) {
        handshake_flag = handshake(); 
        if (!handshake_flag) return; 
        Serial.println("Listening..."); 
    }

    /* After handshake complete */
    rsw_state_update();

    // [FIX-COMM]
    if (read_op_from_serial() == Ops::READ_SENSOR) {
        rsw_state_display();
    }

    bool result[8][8];
    bool result2[8][8];
    transpose(rsw_state, result2);
    filp_row(result2, result);

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (result[i][j]) {
                set_LED_xy(i + 1, j + 1, CRGB::Gold);
            } else {
                set_LED_xy(i + 1, j + 1, CRGB::Turquoise);
            }
        }
    }
    FastLED.show();
}

void transpose(bool matrix[8][8], bool result[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result[j][i] = matrix[i][j];
        }
    }
}

void filp_row(bool matrix[8][8], bool result[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result[i][j] = matrix[i][7 - j];
        }
    }
}





// ====================== LED Functions ======================

/*
* The setup() function in LedControl
* 
* @param brightness: Brightness of all LEDs, [0 - 255], recommended 16
*/
void LED_setup(int brightness) {
    // Clip brightness to avoid burning out LEDs
    if (brightness > 50) {
        brightness = 50;
    }
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
}

/*
* Set the color of a LED at position (x, y)
*
* @param x: Row number [1 - 8]
* @param y: Column number [1 - 8]
*/ 
void set_LED_xy(int row, int col, CRGB color) {
    row--;
    col--;
    if (col % 2 == 0) {
        row = 7 - row;
    }
    leds[(col * 8) + row] = color;
}

/*
* Set the color of a LED at position i
*
* @param i: Integer between 0 (bottom left) and 63 (top right)
*/
void set_LED_ith(int i, CRGB color) {
    if (i / 8 % 2 == 0) {
        i = 7 - (i % 8);
    }
    leds[i] = color;
}


// =================== Reed Switch Functions ===================

/*
* The setup() function in ReedSwitchDetection
*/
void rsw_setup() {
    // set the reed switch pins to output and input
    for (int i = 0; i < 8; i++) {
        pinMode(rsw_out_pin[i], OUTPUT);
        digitalWrite(rsw_out_pin[i], LOW);
        pinMode(rsw_in_pin[i], INPUT);
    }
}

/* 
* Update the state of reed switches,
* stored in rsw_state[8][8]
*/
void rsw_state_update() {
    for (int i = 0; i < 8; i++) {
        digitalWrite(rsw_out_pin[i], HIGH);
        for (int j = 0; j < 8; j++) {
            rsw_state[i][j] = digitalRead(rsw_in_pin[j]);
        }
        digitalWrite(rsw_out_pin[i], LOW);
        delay(10);
    }
}


/*
* Display the state of reed switches
*/
void rsw_state_display() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print(rsw_state[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println();
}

// =================== Stepper Motor Functions ===================

/*
* The setup() function in CoreXY
*/
void core_xy_setup() {
    // Setting motor pins to output
    pinMode(M1.DIR_PIN, OUTPUT);
    pinMode(M1.STEP_PIN, OUTPUT);
    pinMode(M1.DISABLE_PIN, OUTPUT);

    pinMode(M2.DIR_PIN, OUTPUT);
    pinMode(M2.STEP_PIN, OUTPUT);
    pinMode(M2.DISABLE_PIN, OUTPUT);

    // Senor
    pinMode(LIMIT_SW_PIN, INPUT);

    // DisEnable the motors
    digitalWrite(M1.DISABLE_PIN, HIGH);
    digitalWrite(M2.DISABLE_PIN, HIGH);
}

/*
* Calibrate the gantry to the left down corner.
*/
void calibration() {
    Serial.println("CALIBRATING...");
    // Disable M2 so that it is not locked
    digitalWrite(M2.DISABLE_PIN, HIGH);
    // Enable M1 to rotate
    digitalWrite(M1.DISABLE_PIN, LOW);

    int interval = (int) 10000 / CALI_SPD;

    // Set the direction of M1
    digitalWrite(M1.DIR_PIN, HIGH);

    // Rotate M1 until hit the switch at A0
    while (!digitalRead(LIMIT_SW_PIN)) {
        digitalWrite(M1.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(M1.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }

    // Move a bit more to make sure it stays at corner
    for (int i = 0; i < 500; i++) {
        digitalWrite(M1.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(M1.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }
    delay(100);
    // Disable M1
    digitalWrite(M1.DISABLE_PIN, HIGH);

    // Reset the current position
    current_pos.x = -OFFSET_X;
    current_pos.y = -OFFSET_Y;
    
    Serial.println("CALIBRATION DONE");

}



/*
* Move the gantry to the target position.
* The movement is achieved by combining 2 movements in 8 directions.
*
* @param target_pos: the target position - (Position){x, y}
* @param speed: the speed of the gantry (HIGH_SPD, LOW_SPD)
*/
void move_to(Position target_pos, int speed) {
    // Print start position
    Serial.print("[ ");
    Serial.print(current_pos.x);
    Serial.print(", ");
    Serial.print(current_pos.y);
    Serial.print(" ]");

    // Check if the target position is valid
    if (target_pos.x < MIN_X || target_pos.x > MAX_X || 
        target_pos.y < MIN_Y || target_pos.y > MAX_Y) {
        Serial.println("Invalid target position");
        return;
    }
    // Calibrate if not calibrated yet
    if (current_pos.x == -1 && current_pos.y == -1) {
        calibration();
    }
    // Calculate the distance to move
    int dist_x = target_pos.x - current_pos.x;
    int dist_y = target_pos.y - current_pos.y;

    // First move in horizontal or vertical direction to align diagonally
    int abs_hv_dist = abs(abs(dist_x) - abs(dist_y));
    if (abs(dist_x) > abs(dist_y) && dist_x >= 0) {
        move_mm_in_dir(abs_hv_dist, speed, RIGHT);
    } else if (abs(dist_x) > abs(dist_y) && dist_x < 0) {
        move_mm_in_dir(abs_hv_dist, speed, LEFT);
    } else if (abs(dist_x) <= abs(dist_y) && dist_y >= 0) {
        move_mm_in_dir(abs_hv_dist, speed, UP);
    } else if (abs(dist_x) <= abs(dist_y) && dist_y < 0) {
        move_mm_in_dir(abs_hv_dist, speed, DOWN);
    }

    // Then move diagonally to reach the target position
    int abs_diag_dist = min(abs(dist_x), abs(dist_y));
    if (dist_x >= 0 && dist_y >= 0) {
        move_mm_in_dir(abs_diag_dist, speed, UP_RIGHT);
    } else if (dist_x >= 0 && dist_y < 0) {
        move_mm_in_dir(abs_diag_dist, speed, DOWN_RIGHT);
    } else if (dist_x < 0 && dist_y >= 0) {
        move_mm_in_dir(abs_diag_dist, speed, UP_LEFT);
    } else if (dist_x < 0 && dist_y < 0) {
        move_mm_in_dir(abs_diag_dist, speed, DOWN_LEFT);
    }

    // Update the current position
    current_pos.x = target_pos.x;
    current_pos.y = target_pos.y;

    // Print the end position
    Serial.print("\t-->    [ ");
    Serial.print(current_pos.x);
    Serial.print(", ");
    Serial.print(current_pos.y);
    Serial.println(" ]");

}

/*
* Move the gantry in 8 directions
* Should not be called directly
* 
* @param dist: distance to move (in mm)
* @param speed: speed to move (HIGH_SPD or LOW_SPD)
* @param direction: UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT
*/
void move_mm_in_dir(int dist, int speed, int direction) {

    long step = MM_TO_STEPS(dist);
    int interval = SPD_TO_INTERVAL(speed);
    
    // Enable the motor
    digitalWrite(M1.DISABLE_PIN, LOW);
    digitalWrite(M2.DISABLE_PIN, LOW);

    // Set up 8 moving directions
    switch (direction) {
        case UP:
            current_pos.y += dist;
            digitalWrite(M1.DIR_PIN, LOW);
            digitalWrite(M2.DIR_PIN, HIGH);
            break;
        case DOWN:
            current_pos.y -= dist;
            digitalWrite(M1.DIR_PIN, HIGH);
            digitalWrite(M2.DIR_PIN, LOW);
            break;
        case LEFT:
            current_pos.x -= dist;
            digitalWrite(M1.DIR_PIN, HIGH);
            digitalWrite(M2.DIR_PIN, HIGH);  
            break; 
        case RIGHT:
            current_pos.x += dist;
            digitalWrite(M1.DIR_PIN, LOW);
            digitalWrite(M2.DIR_PIN, LOW);
            break;
        case DOWN_RIGHT:
            current_pos.x += dist;
            current_pos.y -= dist;
            move_single_motor(M2, step * 2, speed, LOW);
            return;
        case UP_RIGHT:
            current_pos.x += dist;
            current_pos.y += dist;
            move_single_motor(M1, step * 2, speed, LOW);
            return;
        case DOWN_LEFT:
            current_pos.x -= dist;
            current_pos.y -= dist;
            move_single_motor(M1, step * 2, speed, HIGH);
            return;
        case UP_LEFT:
            current_pos.x -= dist;
            current_pos.y += dist;
            move_single_motor(M2, step * 2, speed, HIGH);
            return;
    }

    // Making move
    for (long i = 0; i < step; i++) {
        digitalWrite(M1.STEP_PIN, LOW);
        digitalWrite(M2.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(M1.STEP_PIN, HIGH);
        digitalWrite(M2.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }
    delay(10);
    // Disable the motor
    digitalWrite(M1.DISABLE_PIN, HIGH);
    digitalWrite(M2.DISABLE_PIN, HIGH);
}

/*
* Move a single motor clockwise or counter-clockwise
* Should not be called directly
* 
* @param motor: M1 or M2
* @param step: number of steps to move
* @param direction: LOW (0) for clockwise, HIGH (1) for counter-clockwise
*/
void move_single_motor(StepperMotor motor, long step, int speed, bool direction) {

    int interval = (int) 10000 / speed;

    // Enable the motors
    digitalWrite(M1.DISABLE_PIN, LOW);
    digitalWrite(M2.DISABLE_PIN, LOW);

    // Set the direction
    digitalWrite(motor.DIR_PIN, direction);

    // Making move
    for (long i = 0; i < step; i++) {
        digitalWrite(motor.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(motor.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }
    delay(10);

    // Disable the motors
    digitalWrite(M1.DISABLE_PIN, HIGH);
    digitalWrite(M2.DISABLE_PIN, HIGH);
}


// ================== Electromagnet Functions ==================

/*
* The setup function for the electromagnet
*/
void electromagnet_setup() {
    pinMode(ELECTROMAGNET_PIN, OUTPUT);
    magnet_off();
}
/*
* Turn on the electromagnet
*/
void magnet_on() {
    digitalWrite(ELECTROMAGNET_PIN, HIGH);
}

/*
* Turn off the electromagnet
*/
void magnet_off() {
    digitalWrite(ELECTROMAGNET_PIN, LOW);
}