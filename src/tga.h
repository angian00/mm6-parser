#pragma once

//--------------------------------------------------------------
//--- Data structures
//--------------------------------------------------------------

struct tga_data;



//----------------------------------------------------------------------
//--- Function declarations
//----------------------------------------------------------------------

struct tga_data *tga_parse(char *raw_data);
void tga_export_png(char *out_path, struct tga_data *p_tga);
void tga_export_bmp(char *out_path, struct tga_data *p_tga);
