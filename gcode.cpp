#include "gcode.h"

GCode::GCode(int dpm, int dpi) {
    units = points;
    dots_per_mm = dpm;
    dots_per_in = dpi;
}

void GCode::parse(int str) {
    switch(str) {
        case 20:
            units = inches;
        case 21:
            units = millimeters;
        default:
            break;
    }
}

int GCode::convert_to_points(float input) {
    switch(units) {
        case points:
            return input;
        case millimeters:
            return input * dots_per_mm;
        case inches:
            return input * dots_per_in;
    }
}