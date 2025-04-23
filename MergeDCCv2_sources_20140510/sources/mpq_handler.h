#ifndef __MPQ_HANDLER_H__
#define __MPQ_HANDLER_H__

#include "one_include_to_rule_them_all.h"

#include "enums.h"

void set_mpq_locale               (DWORD id);
void close_mpq                    (ENUM_MPQ m);
int  open_mpq                     (ENUM_MPQ m);
int  load_file_from_mpq           (ENUM_MPQ m, char * filename, char ** buffer, long * length);
int  test_file_existence_from_mpq (int m, char * filename);

#endif
