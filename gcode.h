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
        GCode(int dpmm_x, int dpmm_y);
        Units units;

        void parse(int str);
        int convert_to_points_x(float input);
        int convert_to_points_y(float input);
    private:
        int dots_per_mm_x;
        int dots_per_mm_y;
};
#endif