int colpin[]  = { 5, 6, 7 };
int ledpin[]  = { 8, 9, 10 };
int reedpin[] = { 2, 3, 4 };

bool state[3][3] = {
  { 1, 0, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 }
};

const int dim = 3;
const bool display = true;

void setup() {
  for (int i = 0; i < dim; i++) {
    pinMode(colpin[i],  OUTPUT);
    pinMode(ledpin[i],  OUTPUT);
    pinMode(reedpin[i], INPUT);
  }

  Serial.begin(9600);
  // put your setup code here, to run once:

  digitalWrite(ledpin[0], HIGH);
}

void loop() {
  for (int row = 0; row < dim; row++) {
    digitalWrite(colpin[row], HIGH);
    for (int col = 0; col < dim; col++) {
      state[row][col] = digitalRead(reedpin[col]);
      digitalWrite(ledpin[col], !state[row][col]);
    }
    delay(5);
    // delay(1000);
    digitalWrite(colpin[row], LOW);
  }
  // show();
  // delay(1000);
}

void show() {
  Serial.println("+---+---+---+");
  for (int row = 0; row < dim; row++) {
    for (int col = 0; col < dim; col++) {
      Serial.print("| ");
      Serial.print(state[row][col]);
      Serial.print(" ");
    }
    Serial.println("|");
    Serial.println("+---+---+---+");
  }
  Serial.println();
}
