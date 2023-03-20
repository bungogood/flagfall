int rsw_in_pin[8]  = { 30, 31, 32, 33, 34, 35, 36, 37 };
int rsw_out_pin[8] = { 47, 46, 45, 44, 43, 42, 41, 40 };

// keep the state of reed switches, initialize all to 0
bool rsw_state[8][8] = { 0 };

void setup() {
    rsw_setup();
    Serial.begin(115200);

}

void loop() {
    rsw_state_update();
    rsw_state_display();
    // delay(1000);
}

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