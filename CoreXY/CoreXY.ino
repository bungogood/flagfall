#include<stdio.h>

#define HIGH_SPD 1000 
#define LOW_SPD  300
#define CALI_SPD 1500

#define UP         0
#define DOWN       1
#define LEFT       2
#define RIGHT      3
#define UP_LEFT    4
#define UP_RIGHT   5
#define DOWN_LEFT  6
#define DOWN_RIGHT 7

#define LIMIT_SW_PIN A0


//  --------
// |        |
// |        |
// M1 ---- M2


typedef struct StepperMotor {
    int DIR_PIN;
    int STEP_PIN;
    int DISABLE_PIN;
} StepperMotor;

StepperMotor M1, M2;


void setup()
{   
    // Setting motor connection
    M1.DIR_PIN = 4, M1.STEP_PIN = 5, M1.DISABLE_PIN = 12;
    M2.DIR_PIN = 7, M2.STEP_PIN = 6, M2.DISABLE_PIN = 8;

    // Setting motor pins to output
    pinMode(M1.DIR_PIN, OUTPUT);
    pinMode(M1.STEP_PIN, OUTPUT);
    pinMode(M1.DISABLE_PIN, OUTPUT);

    pinMode(M2.DIR_PIN, OUTPUT);
    pinMode(M2.STEP_PIN, OUTPUT);
    pinMode(M2.DISABLE_PIN, OUTPUT);

    // Senor
    pinMode(LIMIT_SW_PIN, INPUT);

    // Enable the motors
    digitalWrite(M1.DISABLE_PIN, LOW);
    digitalWrite(M2.DISABLE_PIN, LOW);

    Serial.begin(9600);
}
void loop()
{
    // calibration();
    // delay(10000);

    move_in_dir(6400 * 2, HIGH_SPD, RIGHT);
    move_in_dir(6400 * 2, HIGH_SPD, UP_RIGHT);
    move_in_dir(6400 * 2, HIGH_SPD, UP);
    move_in_dir(6400 * 2, HIGH_SPD, UP_LEFT);
    move_in_dir(6400 * 2, HIGH_SPD, LEFT);
    move_in_dir(6400 * 2, HIGH_SPD, DOWN_LEFT);
    move_in_dir(6400 * 2, HIGH_SPD, DOWN);
    move_in_dir(6400 * 2, HIGH_SPD, DOWN_RIGHT);
    delay(1000);

}


void calibration() {
    /*
    * Move to the DOWN LEFT corner
    */

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
        
    // Disable M1
    digitalWrite(M1.DISABLE_PIN, HIGH);
}


void move_single_motor(StepperMotor motor, int step, int speed, int direction) {

    int interval = (int) 10000 / speed;

    digitalWrite(motor.DIR_PIN, direction);

    // Enable the motor
    digitalWrite(motor.DISABLE_PIN, LOW);
    // Making move
    for (int i = 0; i < step; i++) {
        digitalWrite(motor.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(motor.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }
    delay(10);
    // Disable the motor
    digitalWrite(motor.DISABLE_PIN, HIGH);
}


// Move 2 motors at the same time
void move_in_dir(int step, int speed, int direction) {

    int interval = (int) 10000 / speed;
    
    // Enable the motor
    digitalWrite(M1.DISABLE_PIN, LOW);
    digitalWrite(M2.DISABLE_PIN, LOW);

    // Set up 8 moving directions
    switch (direction) {
        case UP:
            digitalWrite(M1.DIR_PIN, LOW);
            digitalWrite(M2.DIR_PIN, HIGH);
            break;
        case DOWN:
            digitalWrite(M1.DIR_PIN, HIGH);
            digitalWrite(M2.DIR_PIN, LOW);
            break;
        case LEFT:
            digitalWrite(M1.DIR_PIN, HIGH);
            digitalWrite(M2.DIR_PIN, HIGH);  
            break; 
        case RIGHT:
            digitalWrite(M1.DIR_PIN, LOW);
            digitalWrite(M2.DIR_PIN, LOW);
            break;

        case DOWN_RIGHT:
            move_single_motor(M2, step, speed, LOW);
            return 0;
        case UP_RIGHT:
            move_single_motor(M1, step, speed, LOW);
            return 0;
        case DOWN_LEFT:
            move_single_motor(M1, step, speed, HIGH);
            return 0;
        case UP_LEFT:
            move_single_motor(M2, step, speed, HIGH);
            return 0;
    }

    // Making move
    for (int i = 0; i < step; i++) {
        digitalWrite(M1.STEP_PIN, LOW);
        digitalWrite(M2.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(M1.STEP_PIN, HIGH);
        digitalWrite(M2.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }

    // Disable the motor
    digitalWrite(M1.DISABLE_PIN, HIGH);
    digitalWrite(M2.DISABLE_PIN, HIGH);
}
