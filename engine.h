#ifndef engine_h
#define engine_h

#include "Arduino.h"

class Engine {
    public:
        Engine(int pin_forward, int pin_backward);
        void start_forward();
        void start_backward();
        void stop();
    private:
        int pin_forward;
        int pin_backward;
        bool lock;
};

#endif