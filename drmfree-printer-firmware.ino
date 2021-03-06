#include <PID_v1.h>
#include "engine.h"
#include "gcode.h"

// engine pins
#define FORWARD_Y 12
#define BACKWARD_Y 11
#define FORWARD_X 10
#define BACKWARD_X 9
#define PWM_X 5
#define PWM_Y 6

// encoder pins
#define INTERRUPT_X 0
#define ENCODER_X_A 2
#define ENCODER_X_B 4
#define INTERRUPT_Y 1
#define ENCODER_Y_A 3
#define ENCODER_Y_B 7

// dots per mm
#define DPMM_X 6
#define DPMM_Y 25

// min and max output levels to pwm
int min_speed_x = 180;
int max_speed_x = 220;
int min_speed_y = 220;
int max_speed_y = 240;

// pwm output levels
double pwm_x = 255, pwm_y = 255;

bool debug = false;

unsigned long uptime = 0, last_uptime = 0;	// pid timer

String text_buffer = "";

GCode gcode(DPMM_X, DPMM_Y);

Engine x_engine(FORWARD_X, BACKWARD_X, PWM_X);
Engine y_engine(FORWARD_Y, BACKWARD_Y, PWM_Y);

double target_x = 0, target_y = 0;
volatile double coord_x = 0, coord_y = 0;
double prev_coord_x = 0, prev_coord_y = 0;	// for speed calculation
double speed_x = 0, speed_y = 0;

// pid parameters
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;
PID pid_x(&coord_x, &pwm_x, &target_x, consKp, consKi, consKd, DIRECT);
PID pid_y(&coord_y, &pwm_y, &target_y, consKp, consKi, consKd, DIRECT);
int window_size = 100;

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
	pinMode(ENCODER_Y_A, INPUT);
	pinMode(ENCODER_Y_B, INPUT);
	attachInterrupt(INTERRUPT_X, x_encoder_interrupt, FALLING);
	attachInterrupt(INTERRUPT_Y, y_encoder_interrupt, FALLING);
	pinMode(FORWARD_X, OUTPUT);
	pinMode(BACKWARD_X, OUTPUT);
	pinMode(BACKWARD_Y, OUTPUT);
	pinMode(FORWARD_Y, OUTPUT);

	pinMode(PWM_X, OUTPUT);
	pinMode(PWM_Y, OUTPUT);
	pid_x.SetSampleTime(window_size);
	pid_y.SetSampleTime(window_size);
	update_pid_limits();
	Serial.begin(9600);
}

void loop()
{
	// reading from UART buffer
	if (Serial.available() > 0 && state == waiting)
	{
		char symbol = Serial.read();
		if (symbol != '\n')
		{
			text_buffer += symbol;
		}
		else
		{
			parse_command(text_buffer);
			text_buffer = "";
		}
	}

	// software timer for "windows_size" seconds
	uptime = millis();
	int delta_t = uptime - last_uptime;
	if (delta_t >= window_size)
	{
		last_uptime = uptime;
		pid_interrupt();	// pid magic here

		// some debug output
		if (debug && state != waiting) 
		{
			Serial.print("PX:");
			Serial.print(coord_x);

			Serial.print(" PY:");
			Serial.print(coord_y);

			Serial.print(" SX:");
			Serial.print(speed_x);

			Serial.print(" SY:");
			Serial.print(speed_y);

			Serial.print(" OX:");
			Serial.print(pwm_x);

			Serial.print(" OY:");
			Serial.println(pwm_y);
		}
	}

	// stop engines in "target_{x,y}"
	if ((state == forward_x && coord_x >= target_x) 
			|| (state == backward_x && coord_x <= target_x))
		stop_x();
	else if ((state == forward_y && coord_y >= target_y) 
			|| (state == backward_y && coord_y <= target_y))
		stop_y();
}

// ********	TABLE OF COMMANDS	******** \\
// X..	— move along X coordinate		 \\
// Y..	— move along Y coordinate		 \\
// S	— stop all engines			 \\
// I	— information about position		 \\
// C	— set current position as zero coordinate\\
// G	— some ordinary g-codes			 \\
// N..	— set minimum pwm level to X engine	 \\
// M..	— set maximum pwm level to X engine	 \\
// J..	— set minimum pwm level to Y engine	 \\
// K..	— set maximum pwm level to Y engine	 \\
// D	— enable debug mode			 \\
// ********	*****************	******** \\

void parse_command(String text)
{
	switch (text[0])
	{
		case 'X':
			target_x = gcode.convert_to_points_x(text.substring(1).toFloat());
			pwm_x = min_speed_x;
			x_engine.SetSpeed(min_speed_x);
			if (target_x > coord_x)
				state_forward_x();
			else
				state_backward_x();
			pid_x.SetMode(AUTOMATIC);
			break;
		case 'Y':
			target_y = gcode.convert_to_points_y(text.substring(1).toFloat());
			pwm_y = min_speed_y;
			y_engine.SetSpeed(min_speed_y);
			if (target_y > coord_y)
				state_forward_y();
			else
				state_backward_y();
			pid_y.SetMode(AUTOMATIC);
			break;
		case 'S':
			disable_all();
			break;
		case 'I':
			Serial.print("PX:");
			Serial.print(coord_x);
			Serial.print(" PY:");
			Serial.println(coord_y);
			break;
		case 'C':
			coord_x = 0;
			coord_y = 0;
			target_x = 0;
			target_y = 0;
			break;
		case 'G':
			gcode.parse(text.substring(1).toInt());
			break;
		case 'N':
			min_speed_x = text.substring(1).toInt();
			update_pid_limits();
			break;
		case 'M':
			max_speed_x = text.substring(1).toInt();
			update_pid_limits();
			break;
		case 'J':
			min_speed_y = text.substring(1).toInt();
			update_pid_limits();
			break;
		case 'K':
			max_speed_y = text.substring(1).toInt();
			update_pid_limits();
			break;
		case 'D':
			debug = true;
			break;
		default:
			Serial.println("UNKNOWN COMMAND:" + text);
			break;
	}
	ok();
}

void pid_interrupt()
{
	// change pid mode
	if (abs(target_x - coord_x) <= 100)
		pid_x.SetTunings(consKp, consKi, consKd);
	else
		pid_x.SetTunings(aggKp, aggKi, aggKd);

	if (abs(target_y - coord_y) <= 100)
		pid_y.SetTunings(consKp, consKi, consKd);
	else
		pid_y.SetTunings(aggKp, aggKi, aggKd);

	// update speed
	speed_x = abs(prev_coord_x - coord_x) / (1000 / window_size);
	speed_y = abs(prev_coord_y - coord_y) / (1000 / window_size);
	prev_coord_x = coord_x;
	prev_coord_y = coord_y;

	// pid magic here
	if (pid_x.Compute())
		x_engine.SetSpeed(pwm_x);
	if (pid_y.Compute())
		y_engine.SetSpeed(pwm_y);
}

void state_forward_x() 
{
	state = forward_x;
	pid_x.SetControllerDirection(DIRECT);
	x_engine.StartForward();
}

void state_backward_x()
{
	state = backward_x;
	pid_x.SetControllerDirection(REVERSE);
	x_engine.StartBackward();

}

void state_forward_y() 
{
	state = forward_y;
	pid_y.SetControllerDirection(DIRECT);
	y_engine.StartForward();
}

void state_backward_y() 
{
	state = backward_y;
	pid_y.SetControllerDirection(REVERSE);
	y_engine.StartBackward();

}

void x_encoder_interrupt()
{
	if (digitalRead(ENCODER_X_B))
		coord_x++;
	else
		coord_x--;
}

void y_encoder_interrupt()
{
	if (digitalRead(ENCODER_Y_B))
		coord_y++;
	else
		coord_y--;
}

void disable_all()
{
	x_engine.Stop();
	y_engine.Stop();
}

void stop_x()
{
	pwm_x = 255;
	x_engine.Stop();
	pid_x.SetMode(MANUAL);
	state = waiting;
}

void stop_y()
{
	pwm_y = 255;
	y_engine.Stop();
	pid_y.SetMode(MANUAL);
	state = waiting;
}

void ok()
{
	Serial.println("OK");
}

void update_pid_limits() {
	pid_x.SetOutputLimits(min_speed_x, max_speed_x);
	pid_y.SetOutputLimits(min_speed_y, max_speed_y);
}
