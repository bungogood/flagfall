#include <stdio.h>

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

/* 
*  CoreXY Layout
*   .---------.
*   |         |
*   |         |
*   |[O]======|
*  [M1]-----[M2]
*/

typedef struct StepperMotor {
    const int DIR_PIN;
    const int STEP_PIN;
    const int DISABLE_PIN;
} StepperMotor;

StepperMotor M1 = {4, 5, 12}, M2 = {7, 6, 8};


typedef struct Position {
    int x;
    int y;
} Position;

Position current_pos = {-1, -1};
const int MAX_X = 500;
const int MAX_Y = 560;
const int MIN_X = 0;
const int MIN_Y = 0;

void setup() {   
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

    Serial.begin(115200);


    calibration();
    
    delay(2000);
}


void loop() {
    // move_to((Position){100, 100}, HIGH_SPD);
    // delay(500);
    // move_to((Position){200, 300}, HIGH_SPD);
    // delay(500);
    // move_to((Position){0, 200}, HIGH_SPD);
    // delay(2000);

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
    current_pos.x = 0;
    current_pos.y = 0;
    
    Serial.println("DONE");

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

