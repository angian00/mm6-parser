#pragma once

#include <stdint.h>
#include "geometry.h"

//--------------------------------------------------------------
//--- Data structures
//--------------------------------------------------------------

struct blv_data;


struct blv_data *blv_parse(char *raw_data);
void blv_extract_outlines(struct blv_data *p_blv, uint32_t *p_n_lines, struct point **p_lines);

