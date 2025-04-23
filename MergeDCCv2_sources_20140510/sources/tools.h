#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "one_include_to_rule_them_all.h"

int  get_window_position_and_dimension (HWND h, int * x, int * y, int * width, int * height);
int  get_x_to_center_into_screen       (int width);
int  get_y_to_center_into_screen       (int height);
int  get_x_to_center_into_parent       (HWND h, int width);
int  get_y_to_center_into_parent       (HWND h, int height);
void char_to_wide_char                 (char * szSource, WCHAR * wszDest, int iDestByteSize);
int  validate_directory_path           (WCHAR * path);
int  validate_file_path                (WCHAR * path);
void save_current_configuration        (void);
void load_last_configuration           (void);
int  get_byte_value_from_wchar_string  (WCHAR * string);

#endif
