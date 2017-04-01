#include "engine.h"

// pins
#define FORWARD_X 11
#define BACKWARD_X 12
#define FORWARD_Y 10
#define BACKWARD_Y 9

volatile bool trigger = false;
volatile int coord_x = 0;
int coord_y = 0;

String text_buffer = "";

Engine x_engine(FORWARD_X, BACKWARD_X);
Engine y_engine(FORWARD_Y, BACKWARD_Y);

int target_x = 0;
int target_y = 0;

enum State {
  waiting,
  forward_x,
  backward_x,
  forward_y,
  backward_y
};
State state = waiting;

void setup() {
  attachInterrupt(0, interrupt_one, RISING);
  attachInterrupt(1, interrupt_two, CHANGE);
  pinMode(FORWARD_X, OUTPUT);
  pinMode(BACKWARD_X,OUTPUT);
  pinMode(BACKWARD_Y,OUTPUT);
  pinMode(FORWARD_Y,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0 && state == waiting) {
    char symbol = Serial.read();
    if (symbol != '\n') {
      text_buffer += symbol;
    } else {
      parse_string(text_buffer);
      text_buffer = "";
    }
  }

  if (state == forward_x && coord_x >= target_x) {
    x_engine.stop();
    set_ok();
  } else if (state == backward_x && coord_x <= target_x) {
    x_engine.stop();
    set_ok();
  } else if (state == forward_y && coord_y >= target_y) {
    y_engine.stop();
    set_ok();
  } else if (state == backward_y && coord_y <= target_y) {
    y_engine.stop();
    set_ok();
  }
}

void parse_string(String text) {
  switch(text[0]) {
    case 'X':
      target_x = text.substring(1).toInt();
      if (target_x < coord_x) {
        state = backward_x;
        x_engine.start_backward();
      }
      else {
        state = forward_x;
        x_engine.start_forward();
      }
      break;
    case 'Y':
      target_y = text.substring(1).toInt();
      if (target_y < coord_y) {
        state = backward_y;
        y_engine.start_backward();
        delay(coord_y - target_y);
        coord_y = target_y;
      } else {
        state = forward_y;
        y_engine.start_forward();
        delay(target_y - coord_y);
        coord_y = target_y;
      }
      break;
    case 'S':
      disable_all();
      Serial.println("OK");
      break;
    case 'I':
      Serial.print("X: ");
      Serial.print(coord_x);
      Serial.print(" Y: ");
      Serial.println(coord_y);
      break;
    case 'Z':
      coord_x = 0;
      coord_y = 0;
      target_x = 0;
      target_y = 0;
      Serial.println("OK");
      break;
    default:
      Serial.println("UNKNOWN COMMAND: " + text);
      break;
  }
}

void interrupt_one() {
  if (trigger) {
    coord_x--;
  } else {
    coord_x++;
  }
}

void interrupt_two() {
  trigger = !trigger;
}

void disable_all() {
  x_engine.stop();
  y_engine.stop();
}

void set_ok() {
  Serial.println("OK");
  state = waiting;
}