#include <PID_v1.h>
#include "engine.h"
#include "gcode.h"

// pins
#define FORWARD_X 11
#define BACKWARD_X 12
#define FORWARD_Y 10
#define BACKWARD_Y 9

#define INTERRUPT_X 0
#define ENCODER_X_A 2
#define ENCODER_X_B 3
#define PWM_X 5

#define DPMM_X 6
#define DPMM_Y 4

bool debug = false;

volatile int coord_x = 0;
int coord_y = 0;

String text_buffer = "";
GCode gcode(6, 150);

Engine x_engine(FORWARD_X, BACKWARD_X, PWM_X);
Engine y_engine(FORWARD_Y, BACKWARD_Y, PWM_X);

int target_x = 0;
int target_y = 0;

unsigned long uptime, last_uptime = 0;

double endpoint, input, output = 0;
int window_size = 100;
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;
PID pid_x(&input, &output, &endpoint, consKp, consKi, consKd, DIRECT);

enum State
{
  waiting,
  forward_x,
  backward_x,
  forward_y,
  backward_y
};
State state = waiting;

void setup()
{
  pinMode(ENCODER_X_A, INPUT);
  pinMode(ENCODER_X_B, INPUT);
  attachInterrupt(INTERRUPT_X, x_encoder_interrupt, FALLING);
  pinMode(FORWARD_X, OUTPUT);
  pinMode(BACKWARD_X, OUTPUT);
  pinMode(BACKWARD_Y, OUTPUT);
  pinMode(FORWARD_Y, OUTPUT);

  pinMode(PWM_X, OUTPUT);
  pid_x.SetSampleTime(window_size);
  pid_x.SetOutputLimits(200, 255); // Set min and max limits to PWM
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0 && state == waiting)
  {
    char symbol = Serial.read();
    if (symbol != '\n')
    {
      text_buffer += symbol;
    }
    else
    {
      parse_string(text_buffer);
      text_buffer = "";
    }
  }

  // Timer
  uptime = millis();
  int delta_t = uptime - last_uptime;
  if (delta_t >= window_size)
  {
    pid_interrupt();
    last_uptime = uptime;
  }

  if ((state == forward_x && coord_x >= target_x) || (state == backward_x && coord_x <= target_x))
    stop_x();
  else if ((state == forward_y && coord_y >= target_y) || (state == backward_y && coord_y <= target_y))
    stop_y();
}

void parse_string(String text)
{
  switch (text[0])
  {
  case 'X':
    target_x = gcode.convert_to_points(text.substring(1).toFloat());
    endpoint = target_x;
    pid_x.SetMode(AUTOMATIC);
    if (target_x < coord_x)
    {
      state = backward_x;
      pid_x.SetControllerDirection(REVERSE);
      x_engine.StartBackward();
    }
    else
    {
      state = forward_x;
      pid_x.SetControllerDirection(DIRECT);
      x_engine.StartForward();
    }
    break;
  case 'Y':
    target_y = gcode.convert_to_points(text.substring(1).toFloat());
    if (target_y < coord_y)
    {
      state = backward_y;
      y_engine.StartBackward();
      delay(coord_y - target_y);
      coord_y = target_y;
    }
    else
    {
      state = forward_y;
      y_engine.StartForward();
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
    ok();
    break;
  case 'D':
    debug = !debug;
    ok();
    break;
  case 'G':
    gcode.parse(text.substring(1).toInt());
    ok();
    break;
  default:
    Serial.println("UNKNOWN COMMAND: " + text);
    break;
  }
}

void pid_interrupt()
{
  input = coord_x;
  if (abs(target_x - coord_x) <= 100)
  {
    pid_x.SetTunings(consKp, consKi, consKd);
  }
  else
  {
    pid_x.SetTunings(aggKp, aggKi, aggKd);
  }
  if (pid_x.Compute())
    x_engine.SetSpeed(output);
  if (debug && (state == forward_x || state == backward_x))
  {
    Serial.println(output);
  }
}

void x_encoder_interrupt()
{
  if (digitalRead(ENCODER_X_B))
  {
    coord_x--;
  }
  else
  {
    coord_x++;
  }
}

void disable_all()
{
  x_engine.Stop();
  y_engine.Stop();
}

void stop_x()
{
  x_engine.Stop();
  pid_x.SetMode(MANUAL);
  output = 255;
  Serial.println("OK");
  state = waiting;
}

void stop_y()
{
  y_engine.Stop();
  Serial.println("OK");
  state = waiting;
}

void ok() {
  Serial.println("OK");
}