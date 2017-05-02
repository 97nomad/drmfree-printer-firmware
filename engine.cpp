#include "engine.h"

Engine::Engine(int forward, int backward, int pwm) {
    pin_forward = forward;
    pin_backward = backward;
    pin_pwm = pwm;
    is_working = false;
}

void Engine::StartForward() {
    if (!is_working) {
        digitalWrite(pin_forward, HIGH);
        is_working = true;
    }
}

void Engine::StartBackward() {
    if (!is_working) {
        digitalWrite(pin_backward, HIGH);
        is_working = true;
    }
}

void Engine::Stop() {
    digitalWrite(pin_forward, LOW);
    digitalWrite(pin_backward, LOW);
    SetSpeed(255);	// need for short-circuit braking
    is_working = false;
}

void Engine::SetSpeed(int speed) {
    analogWrite(pin_pwm, speed);
}
