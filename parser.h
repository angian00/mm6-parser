#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>


struct lod_gheader {
  char id[4];
  char game[9];
  char unknown[256-13];
  char dir[16];
  unsigned long dir_start;
  unsigned long dir_length;
  unsigned long unknown2;
  unsigned long num_files;
};


struct lod_dir_entry {
  char name[16];
  unsigned long start_offset;
  unsigned long length;
  unsigned long u1;
  unsigned long u2;
};



struct blv_compressed_blv6_header
{
  unsigned long compressed_size;
  unsigned long uncompressed_size;
//  unsigned char data[];
};




void parse_lod(const char filename[]);

void read_lod_header(FILE *fp, struct lod_gheader *p_header);
void read_lod_dir_entry(FILE *fp, unsigned long pos, struct lod_dir_entry *p_dir_data);

void read_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);

#endif  //PARSER_H

