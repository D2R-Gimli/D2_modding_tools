#ifndef __ALLEGRO_ANIMATION_H__
#define __ALLEGRO_ANIMATION_H__

#include "one_include_to_rule_them_all.h"
#include <stdio.h>
#include <time.h>

#include "dccinfo.h"
#include "enums.h"

typedef void ANIMATION; // opaque type. True type is defined as ANIMATION_PRIVATE and is present only in 'allegro_animation.c'

typedef struct DC6_DIRECTION_S
{
   DCC_BOX_S box;
} DC6_DIRECTION;

typedef struct DC6_FRAME_S
{
   BITMAP    * bmp;
   DCC_BOX_S box;
   long      length;
   void      * data;
} DC6_FRAME;

typedef struct DC6_S
{
   UBYTE         * file;
   long          size;
   DC6_DIRECTION direction [32];
   DC6_FRAME     frame [32][256];
} DC6_S;

typedef struct ANIMATION_DCC_S
{
   int                    layer_idx;
   void *                 file;
   long                   length;
   int                    is_dc6;
   DCC_S *                dcc;
   DC6_S *                dc6;
   int                    shadows;
   int                    transparency;
   ENUM_LAYER_EFFECT_TYPE transparency_type;
   ENUM_LAYER_EFFECT_TYPE transparency_type_user; // -1 = none, else 0 1 2 3 4 or 6 (not 5)
   int                    special_effect_level;   // used only with "[delete dark pixels]" ; from 0 (keep all pixels) to 255 (delete all pixels)
   UBYTE *                user_palshift;          // Palette Shift : NULL = none, else pointer to 256 bytes
} ANIMATION_DCC;

typedef struct ANIMATION_LAYER_LIST_S
{
   ANIMATION_DCC layer[16];
} ANIMATION_LAYER_LIST;

typedef struct APP_COLOR_S
{
   UBYTE  r;
   UBYTE  g;
   UBYTE  b;
   double luminance; // between 0.0 and 1.0
} APP_COLOR;

typedef APP_COLOR APP_PALETTE [256];

typedef struct CREATE_ANIMATION_PARAMS_S
{
   ANIMATION_LAYER_LIST * layer_list;
   unsigned char        nb_directions;
   unsigned char        nb_frames_per_direction;
   unsigned char        nb_layers;
   long                 speed;
   long                 animdatad2_speed;
   unsigned char        * priority; // priority[direction][frame][layer] = layer code to draw
   UBYTE                * pl2;
   clock_t              dcc_decode_nb_clocks;
} CREATE_ANIMATION_PARAMS;

typedef enum MOVING_TYPE_N
{
   MOVING_NULL,
   MOVING_ANIMATION,
   MOVING_PIVOT
} MOVING_TYPE;

typedef struct EXPORT_BOX_S
{
   int left;
   int top;
   int right;
   int bottom;
   int width;
   int height;
} EXPORT_BOX;

typedef struct EXPORT_PALETTE_ENTRY_S
{
   int index;
   int red;
   int green;
   int blue;
   int best_fit_index;
} EXPORT_PALETTE_ENTRY;

typedef enum EXPORT_FORMAT_N
{
   EXPORT_NULL = -1,
   EXPORT_BMP  = 0,
   EXPORT_PCX  = 1,
   EXPORT_PNG  = 2,
   EXPORT_TGA  = 3,
   EXPORT_BAM  = 4,
   EXPORT_MAX  = 5
} EXPORT_FORMAT;

typedef struct EXPORT_PARAMETERS_S
{
   FILE                 * file;
   EXPORT_FORMAT        format;
   void                 * _image;
   ANIMATION            * _anim;
   int                  COF_dir;
   int                  f;
   EXPORT_BOX           * box;
   EXPORT_PALETTE_ENTRY * bg;
   EXPORT_PALETTE_ENTRY * sh;
   char                 * temp_directory;
   int                  bg2;
   int                  sh2;
   int                  mirror_sp;
   void                 * _image_pivot;
   int                  is_first_frame;
   int                  is_last_frame;
   int                  color_white;
   int                  pivot_image_x;
   int                  pivot_image_y;
   FILE                 * file_pivot;
   void                 * _image_control;
   int                  shadow_skew_percent;
   int                  shadow_height_percent;
   int                  shadow_offset_x;
   int                  shadow_offset_y;
   UBYTE                * shadow_cmap;
} EXPORT_PARAMETERS;


void          add_layer_to_animation_part1             (ANIMATION * _anim, ANIMATION_DCC * new_layer, long job_ID);
void          add_layer_to_animation_part2             (ANIMATION * _anim, ANIMATION_DCC * new_layer);
void          allegro_blit_to_hdc                      (void * bmp, HDC dc, int sx, int sy, int dx, int dy, int w, int h);
void          allegro_clear_bitmap                     (void * bmp);
void          analyse_animation_colors                 (ANIMATION * _anim);
int           animation_best_fit_color_with_exceptions (int r, int g, int b, int * exceptions);
void          animation_direction_minus                (ANIMATION * _anim);
void          animation_direction_plus                 (ANIMATION * _anim);
void          animation_force_redraw                   (ANIMATION * _anim);
void          animation_frame_minus                    (ANIMATION * _anim);
void          animation_frame_plus                     (ANIMATION * _anim);
int           animation_must_be_updated                (void);
void          clear_animation                          (void);
ANIMATION *   create_animation                         (CREATE_ANIMATION_PARAMS * params);
int           dc6_decode                               (DC6_S * dc6, long dir_bitfield);
void          destroy_allegro_bitmap                   (void * _bmp);
void          destroy_animation                        (ANIMATION * _anim);
void          draw_current_animation_frame             (ANIMATION * _anim);
void          fix_PL2_colormaps                        (UBYTE * pl2);
void *        get_allegro_bitmap                       (int width, int height);
void          get_anim_nb_and_current_directions       (ANIMATION * _anim, int * nb_directions, int * current_direction);
int           get_anim_nb_colors                       (ANIMATION * _anim);
int           get_anim_user_offsets                    (ANIMATION * _anim, int * offset_x, int * offset_y, MOVING_TYPE moving_type);
int           get_COF_to_DCC_direction                 (ANIMATION * _anim, int COF_direction, int mirror);
unsigned long get_current_tick_25fps                   (void);
unsigned long get_current_tick_25fps_preview           (void);
int           merge_layers_of_this_frame               (EXPORT_PARAMETERS * ex);
void          my_allegro_exit                          (void);
int           my_allegro_init                          (HWND h, int width, int height);
void          reset_tick_25fps                         (void);
void          set_anim_user_offsets                    (ANIMATION * _anim, int offset_x, int offset_y, MOVING_TYPE moving_type);
void          set_animation_direction                  (ANIMATION * _anim, int direction);
void          set_animation_palette                    (APP_PALETTE pal);
void          set_layer_transparency_type_user         (ANIMATION * _anim, int layer_idx, ENUM_LAYER_EFFECT_TYPE new_transparency_type_user);
void          set_layer_special_effect_level           (ANIMATION * _anim, int layer_idx, int new_special_effect_level);
void          set_layer_colormap                       (ANIMATION * _anim, int layer_idx, UBYTE * palshift);
void          set_new_preview_zoom                     (int zoom);
void          start_tick_25fps                         (int anim, int preview);
void          stop_tick_25fps                          (int anim, int preview);
void          test_animation                           (void);
int           test_dcc_library_integrity               (void);
int           search_exported_animation_box            (ANIMATION * _anim, int * direction_order, int * frame_order, EXPORT_BOX * box, EXPORT_PALETTE_ENTRY * bg, int * color_frequency, int mirror_sp, int shadow_height_percent, int shadow_skew_percent, int shadow_offset_x, int shadow_offset_y);
int           update_preview_bitmap                    (EXPORT_PARAMETERS * ex);

#endif
