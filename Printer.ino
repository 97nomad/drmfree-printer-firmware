// pins
const int ENABLE_X = 13;
const int ENABLE_Y = 12;
const int FORWARD_X = 11;
const int BACKWARD_X = 12;
const int FORWARD_Y = 10;
const int BACKWARD_Y = 9;

volatile bool trigger = false;
volatile int counter = 0;
bool is_busy = false;

void interrupt() {
  if (trigger) {
    counter--;
  } else {
    trigger = true;
  }
  if (counter == 0) {
    disable_all();
    is_busy = false;
  }
}

void disable_all() {
  digitalWrite(ENABLE_X, LOW);
  digitalWrite(ENABLE_Y, LOW);
  digitalWrite(FORWARD_X, LOW);
  digitalWrite(BACKWARD_X, LOW);
  digitalWrite(FORWARD_Y, LOW);
  digitalWrite(BACKWARD_Y, LOW);
}

void setup() {
  attachInterrupt(0, interrupt, RISING);
  attachInterrupt(1, interrupt, RISING);
  pinMode(FORWARD_X, OUTPUT);
  pinMode(BACKWARD_X,OUTPUT);
  pinMode(BACKWARD_Y,OUTPUT);
  pinMode(FORWARD_Y,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() >= 2 && !is_busy) {
    disable_all();  // Just in case
    is_busy = true;
    char literal = Serial.read();
    char value = Serial.read();
    switch (literal) {
      case 'Q': // FORWARD X
        counter = value;
        digitalWrite(ENABLE_X, HIGH);
        digitalWrite(FORWARD_X, HIGH);
        break;
      case 'A': // BACKWARD X
        counter = value;
        digitalWrite(ENABLE_X, HIGH);
        digitalWrite(BACKWARD_X, HIGH);
        break;
      case 'W': // FORWARD Y
        counter = value;
        digitalWrite(ENABLE_Y, HIGH);
        digitalWrite(FORWARD_Y, HIGH);
        break;
      case 'S': // BACKWARD Y
        counter = value;
        digitalWrite(ENABLE_Y, HIGH);
        digitalWrite(BACKWARD_Y, HIGH);
        break;
      default:  // UNKNOWN COMMAND
        Serial.print("UNKNOWN COMMAND ");
        Serial.print(literal);
        Serial.println(value);
        is_busy = false;
        break;
    }
  }

}

