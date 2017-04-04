#include "engine.h"

Engine::Engine(int forward, int backward, int pwm) {
    pin_forward = forward;
    pin_backward = backward;
    pin_pwm = pwm;
    lock = false;
}

void Engine::StartForward() {
    if (!lock) {
        digitalWrite(pin_forward, HIGH);
        lock = true;
    }
}

void Engine::StartBackward() {
    if (!lock) {
        digitalWrite(pin_backward, HIGH);
        lock = true;
    }
}

void Engine::Stop() {
    digitalWrite(pin_forward, LOW);
    digitalWrite(pin_backward, LOW);
    SetSpeed(255);
    lock = false;
}

void Engine::SetSpeed(int speed) {
    analogWrite(pin_pwm, speed);
}