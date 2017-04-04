#ifndef engine_h
#define engine_h

#include "Arduino.h"

class Engine {
    public:
        Engine(int forward, int backward, int pwm);
        void StartForward();
        void StartBackward();
        void SetSpeed(int speed);
        void Stop();
    private:
        int pin_forward;
        int pin_backward;
        int pin_pwm;
        bool lock;
};

#endif