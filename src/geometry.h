#pragma once

#include <stdint.h>


//--------------------------------------------------------------
//--- Data structures

struct point {
    float x;
    float y;
};

//--------------------------------------------------------------
//--- Function declaration

void normalize_geometry(uint32_t n_points, struct point *points);
void print_geometry(uint32_t n_points, struct point *points);

