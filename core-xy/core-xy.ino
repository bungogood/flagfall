#include <stdio.h>
#include <limits.h>

#define MAX_X 600
#define MAX_Y 540
#define SQRT2 1.41421356237

#define HIGH_SPD 1000 
#define LOW_SPD  300
#define CALI_SPD 1500

#define LIMIT_SW_PIN A0

const int fullRev = 6400;
const int dps = 225;

const int default_speed = 1000;

/*
  +--------+
  |        |
  |        |
  ML ---- MR
*/

// stored in mm
typedef struct Vec {
  float x;
  float y;
} Vec;

typedef struct StepperMotor {
  int DIR_PIN;
  int STEP_PIN;
  int DISABLE_PIN;
} StepperMotor;

const StepperMotor ML = { DIR_PIN: 4, STEP_PIN: 5, DISABLE_PIN: 12 };
const StepperMotor MR = { DIR_PIN: 7, STEP_PIN: 6, DISABLE_PIN: 8  };

Vec loc;

void setup() {
  // Setting motor pins to output
  pinMode(ML.DIR_PIN, OUTPUT);
  pinMode(ML.STEP_PIN, OUTPUT);
  pinMode(ML.DISABLE_PIN, OUTPUT);

  pinMode(MR.DIR_PIN, OUTPUT);
  pinMode(MR.STEP_PIN, OUTPUT);
  pinMode(MR.DISABLE_PIN, OUTPUT);

  // Senor
  pinMode(LIMIT_SW_PIN, INPUT);

  Serial.begin(9600);

  calibration();
  position({ 7, 7 });
}

void loop() {

}

void position(Vec pos) {
  int ox = 0;
  int oy = 0;
  int scale = 48;
  go({ ox + (pos.x * scale) + (scale / 2), oy + (pos.y * scale) + (scale / 2) });
}

/**
 * Calibarates the core xy reference fame by moving to Bottom Left (0, 0)
 */
void calibration() {
  // Disable MR so that it is not locked
  digitalWrite(MR.DISABLE_PIN, HIGH);

  // Enable ML to rotate
  digitalWrite(ML.DISABLE_PIN, LOW);

  int interval = (int) 10000 / CALI_SPD;
  
  // Set the direction of ML
  digitalWrite(ML.DIR_PIN, HIGH);

  // Rotate ML until hit the switch at A0
  while (!digitalRead(LIMIT_SW_PIN)) {
    digitalWrite(ML.STEP_PIN, LOW);
    delayMicroseconds(interval);
    digitalWrite(ML.STEP_PIN, HIGH);
    delayMicroseconds(interval);
  }
        
  // Disable ML
  digitalWrite(ML.DISABLE_PIN, HIGH);

  // Reset location
  loc = { 0, 0 };
}

/**
 * Moves the core xy to a coordinate
 * Using a default speed
 *
 * @param dest is the destination coordinate in mm which the core xy moves to
 */
void go(Vec dest) {
  move({ dest.x - loc.x, dest.y - loc.y }, default_speed);
}

/**
 * Moves the core xy to a coordinate
 *
 * @param dest is the destination coordinate in mm which the core xy moves to
 * @param speed is the speed to move to the destination
 */
void go(Vec dest, int speed) {
  if (0 <= dest.x && dest.x <= MAX_X && 0 <= dest.y && dest.y <= MAX_Y) {
    move({ dest.x - loc.x, dest.y - loc.y }, speed);
  } else {
    // invalid coordinate
  }
}

/**
 * Move the core xy by a vector
 * SHOULD NOT BE CALLED DIRECTLY
 * Instead call go
 *
 * @param diff is a vector which the core xy moves along in mm
 * @param speed is the speed to move along the vector
 */
void move(Vec diff, int speed) {
  int interval = (int) 10000 / speed;

  int dl = (int) (diff.x + diff.y) / SQRT2;
  int dr = (int) (diff.x - diff.y) / SQRT2;

  // Enable the motor
  digitalWrite(ML.DISABLE_PIN, LOW);
  digitalWrite(MR.DISABLE_PIN, LOW);

  // directions
  digitalWrite(ML.DIR_PIN, dl < 0);
  digitalWrite(MR.DIR_PIN, dr < 0);

  StepperMotor continous, descrete;

  dl = abs(dl); dr = abs(dr);
  double gradient = min(dl, dr) == 0 ? INT_MAX : max(dl, dr) / min(dl, dr);
  if (dl > dr) {
    continous = ML; descrete = MR;
  } else {
    continous = MR; descrete = ML;
  }

  // Making move
  long steps = (long) dps * max(dl, dr);
  double counter = 0;

  for (long i = 0; i < steps; i++) {
    counter++;
    digitalWrite(continous.STEP_PIN, LOW);
    if (counter > gradient) digitalWrite(descrete.STEP_PIN, LOW);
    delayMicroseconds(interval);
    digitalWrite(continous.STEP_PIN, HIGH);
    if (counter > gradient) {
      digitalWrite(descrete.STEP_PIN, HIGH);
      counter -= gradient;
    }
    delayMicroseconds(interval);
  }

  delay(100);

  // Disable the motor
  digitalWrite(ML.DISABLE_PIN, HIGH);
  digitalWrite(MR.DISABLE_PIN, HIGH);
  loc.x += diff.x; loc.y += diff.y;
}
