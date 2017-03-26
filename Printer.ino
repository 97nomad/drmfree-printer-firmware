// pins
const int FORWARD_X = 11;
const int BACKWARD_X = 12;
const int FORWARD_Y = 10;
const int BACKWARD_Y = 9;

volatile bool trigger = false;
volatile int counter = 0;
volatile bool is_busy = false;

void interrupt() {
  if (trigger) {
    counter--;
    trigger = false;
  } else {
    trigger = true;
  }
  if (counter <= 0) {
    disable_all();
    is_busy = false;
  }
}

void disable_all() {
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
  if (Serial.available() >= 2 && !is_busy && counter <= 0) {
    disable_all();  // Just in case
    is_busy = true;
    char literal = Serial.read();
    char value = Serial.read();
    Serial.println(counter);
    switch (literal) {
      case 'Q': // FORWARD X
        counter = value;
        digitalWrite(FORWARD_X, HIGH);
        break;
      case 'A': // BACKWARD X
        counter = value;
        digitalWrite(BACKWARD_X, HIGH);
        break;
      case 'W': // FORWARD Y
        digitalWrite(FORWARD_Y, HIGH);
        delay(value);
        disable_all();
        is_busy = false;
        break;
      case 'S': // BACKWARD Y
        digitalWrite(BACKWARD_Y, HIGH);
        delay(value);
        disable_all();
        is_busy = false;
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

