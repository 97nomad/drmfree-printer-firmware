#ifndef gcode_h
#define gcode_h

#include "Arduino.h"

enum Units
{
  points,
  millimeters,
  inches
};

class GCode
{
    public:
        GCode(int dpm, int dpi);
        Units units;

        void parse(int str);
        int convert_to_points(float input);
    private:
        int dots_per_mm;
        int dots_per_in;
};
#endif