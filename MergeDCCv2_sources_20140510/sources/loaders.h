#ifndef __LOADERS_H__
#define __LOADERS_H__

#include "one_include_to_rule_them_all.h"

#include "enums.h"
#include "allegro_animation.h"

void load_layer_DCC                     (int i, ANIMATION_DCC * dcc, int * update_cache);
void load_current_cof                   (void);
void update_animation_nb_colors_control (void);
int  load_file_from_mod_directory       (ENUM_MPQ m,   char * filename, char ** buffer, long * length);
int  load_internal_mpq_file             (ENUM_MPQ mpq, char * filename, char ** buffer, long * length);
int  load_file                          (char * filename, char ** buffer, long * length, int save_in_cache);
int  load_default_d2_file               (char * filename, char ** buffer, long * length, int save_in_cache);
void set_new_palette_from_PL2           (ENUM_D2_RESSOURCES pl2);

#endif
