#include "engine.h"

Engine::Engine(int pin_forward, int pin_backward) {
    this->pin_forward = pin_forward;
    this->pin_backward = pin_backward;
    this->lock = false;
}

void Engine::start_forward() {
    if (!this->lock) {
        digitalWrite(this->pin_forward, HIGH);
        this->lock = true;
    }
}

void Engine::start_backward() {
    if (!this->lock) {
        digitalWrite(this->pin_backward, HIGH);
        this->lock = true;
    }
}

void Engine::stop() {
    digitalWrite(this->pin_forward, LOW);
    digitalWrite(this->pin_backward, LOW);
    this->lock = false;
}