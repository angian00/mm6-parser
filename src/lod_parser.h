#pragma once


typedef void (*lod_entry_cbk)(const char *gamedir, const char *lod_name, const char *file_name);

void lod_init();
void lod_dir_load(const char *gamedir, const char *lod_name);
void lod_dir_print(const char *gamedir, const char *lod_name);
void lod_dir_iterate(const char *gamedir, const char *lod_name, lod_entry_cbk cbk);

char *lod_file_load(const char *gamedir, const char *lod_name, const char *lod_entry_name);
