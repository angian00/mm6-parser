#include "geometry.h"

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <inttypes.h>


//----------------------------------------------------------------------
//--- Implementation

void normalize_geometry(uint32_t n_points, struct point *points) {
    float min_x = INT_MAX;
    float max_x = INT_MIN;
    float min_y = INT_MAX;
    float max_y = INT_MIN;

 	for (uint32_t i=0; i < n_points; i++) {
		struct point *p = &points[i];
        if (p->x < min_x)
            min_x = p->x;
        if (p->x > max_x)
            max_x = p->x;
        if (p->y < min_y)
            min_y = p->y;
        if (p->y > max_y)
            max_y = p->y;
	}

	printf("min_x: %f, max_x: %f \n", min_x, max_x);
	printf("min_y: %f, max_y: %f \n", min_y, max_y);


 	for (uint32_t i=0; i < n_points; i++) {
		struct point *p = &points[i];
		// from -1 to 1
        p->x = 2 * ((p->x - min_x) / (max_x - min_x)) - 1;
        p->y = 2 * ((p->y - min_y) / (max_y - min_y)) - 1;


		// ... DEBUG: fix problems
/*        p->x = p->x - 1;
        if (p->x < -1)
        	p->x += 2;

        p->y = p->y - 1;
        if (p->y < -1)
        	p->y += 2;
*/
	}
}


void print_geometry(uint32_t n_points, struct point *points) {
 	for (uint32_t i=0; i < n_points; i++) {
		struct point *p = &points[i];
		printf(" %"PRIu32" %f %f \n", i, p->x, p->y);
	}
}
