#include "gcode.h"

GCode::GCode(int dpmm_x, int dpmm_y) {
    units = points;
    dots_per_mm_x = dpmm_x;
    dots_per_mm_y = dpmm_y;
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

int GCode::convert_to_points_x(float input) {
    switch(units) {
        case points:
            return input;
        case millimeters:
            return input * dots_per_mm_x;
        case inches:
            return input * dots_per_mm_x * 25.4;
    }
}

int GCode::convert_to_points_y(float input) {
    switch(units) {
        case points:
            return input;
        case millimeters:
            return input * dots_per_mm_y;
        case inches:
            return input * dots_per_mm_y * 25.4;
    }
}