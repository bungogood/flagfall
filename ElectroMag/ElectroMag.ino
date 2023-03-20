const int pin = 3;
void setup() {
  // put your setup code here, to run once:
  pinMode(pin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

    if (Serial.available()){
        int state = Serial.parseInt();
        if (state == 1){
            Serial.println("MAGNET ON");
            int sensorValue = analogRead(A1);
            Serial.println(sensorValue);
            int outputValue = map(sensorValue, 0, 1023, 0 , 255);
            Serial.println(outputValue);
            analogWrite(pin, outputValue);
        }

        if (state == -1){
            Serial.println("MAGNET OFF");
            digitalWrite(pin, 0);
        }
    }
}