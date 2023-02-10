#include<stdio.h>

#define HIGH_SPD 1000 
#define LOW_SPD 300



typedef struct StepperMotor {
    int DIR_PIN;
    int STEP_PIN;
    int DISABLE_PIN;
} StepperMotor;

StepperMotor M1, M2;

void setup()
{
    
    M1.DIR_PIN = 4, M1.STEP_PIN = 5, M1.DISABLE_PIN = 12;
    M2.DIR_PIN = 7, M2.STEP_PIN = 6, M2.DISABLE_PIN = 8;

    // M1.DIR_PIN = 2, M1.STEP_PIN = 3, M1.DISABLE_PIN = 12;


    pinMode(M1.DIR_PIN, OUTPUT);
    pinMode(M1.STEP_PIN, OUTPUT);
    pinMode(M1.DISABLE_PIN, OUTPUT);

    pinMode(M2.DIR_PIN, OUTPUT);
    pinMode(M2.STEP_PIN, OUTPUT);
    pinMode(M2.DISABLE_PIN, OUTPUT);

    Serial.begin(9600);
}
void loop()
{

    move_single_motor(M2, 6400 * 2, LOW_SPD, 0);
    delay(100);

    move_single_motor(M1, 6400 * 2, LOW_SPD, 0);
    delay(100);

    move_single_motor(M2, -6400 * 2, LOW_SPD, 0);
    delay(100);

    move_single_motor(M1, -6400 * 2, LOW_SPD, 0);
    delay(100);
    // move_both(6400, LOW_SPD, 0);
    // delay(100000);
}



void move_single_motor(StepperMotor motor, int step, int speed, bool direction) {

    int interval = (int) 10000 / speed;

    if (step > 0) {
        digitalWrite(motor.DIR_PIN, HIGH);
    } else {
        digitalWrite(motor.DIR_PIN, LOW);
        step = abs(step);
    }

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
    digitalWrite(motor.DISABLE_PIN, LOW);
}


// Move 2 motors at the same time
void move_both(int step, int speed, int direction) {

    int interval = (int) 10000 / speed;
    
    // Enable the motor
    digitalWrite(M1.DISABLE_PIN, LOW);
    digitalWrite(M2.DISABLE_PIN, LOW);

    // Set direction
    digitalWrite(M1.DIR_PIN, LOW);
    digitalWrite(M2.DIR_PIN, LOW);

    // Making move
    for (int i = 0; i < step; i++) {
        digitalWrite(M1.STEP_PIN, HIGH);
        digitalWrite(M2.STEP_PIN, LOW);
        delayMicroseconds(interval);
        digitalWrite(M1.STEP_PIN, LOW);
        digitalWrite(M2.STEP_PIN, HIGH);
        delayMicroseconds(interval);
    }
    delay(10);
    // Disable the motor
    // digitalWrite(M1.DISABLE_PIN, HIGH);
    // digitalWrite(M2.DISABLE_PIN, HIGH);
}
