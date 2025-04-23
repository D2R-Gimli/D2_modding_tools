#ifndef __MISC_H__
#define __MISC_H__

#include "one_include_to_rule_them_all.h"

#include <windows.h>
#include "enums.h"
#include "defines.h"
#include "file_cache.h"
#include "listfiles.h"
#include "dialog_maker.h"
#include "allegro_animation.h"


typedef struct MPQ_DATAS_S
{
   WCHAR  * filename;
   WCHAR  path [MAX_PATH];
   HANDLE storm_handle;
} MPQ_DATAS;

#define MY_CONFIGURATION_EXPECTED_VERSION "0.08"
#define MY_CONFIGURATION_VERSION_SIZE     16
typedef struct MY_CONFIGURATION_S
{
   char          version[MY_CONFIGURATION_VERSION_SIZE];
   unsigned long checksum;
   WCHAR         mpq_path              [MPQ_MAX][MAX_PATH];

   int           have_export_datas;
   WCHAR         last_export_directory [MAX_PATH];
   WCHAR         filename_format       [MAX_PATH];
   int           enum_format;
   int           enum_usebox;
   int           background_index;
   int           background_red;
   int           background_green;
   int           background_blue;
   int           shadow_index;
   int           shadow_red;
   int           shadow_green;
   int           shadow_blue;
   int           mirror_sprite;
   int           userbox_left;
   int           userbox_top;
   int           userbox_right;
   int           userbox_bottom;
   int           shadow_skew_percent;
   int           shadow_height_percent;
   int           shadow_offset_x;
   int           shadow_offset_y;
} MY_CONFIGURATION;

typedef struct MY_FONT_S
{
   HFONT hfont;
} MY_FONT;

typedef struct MYPATHS_S
{
   WCHAR ressources               [MAX_PATH];
   WCHAR ressources_fonts         [MAX_PATH];
   WCHAR ressources_default_files [MAX_PATH];
   WCHAR listfile                 [MAX_PATH];
   WCHAR ressources_bin           [MAX_PATH];
   WCHAR debug                    [MAX_PATH];
   WCHAR debug_extracted_files    [MAX_PATH];
} MYPATHS;

typedef struct MODE_WCLASS_S
{
   int active; // 0 = no, 1 = yes
   int idx_wclass;
   int idx_mode;
} MODE_WCLASS;

typedef struct D2_TXT_S
{
   int           nb_rows;
   int           nb_cols;
   unsigned long * tab_cell;
} D2_TXT;

typedef struct D2_RESSOURCES_DATAS_S
{
   char                    filename [MAX_PATH];
   long                    length;
   char                    * buffer;
   ENUM_D2_RESSOURCES_TYPE type;
   D2_TXT                  txt;
} D2_RESSOURCES_DATAS;

typedef struct MAIN_DATAS_S
{
   MPQ_DATAS           mpq          [MPQ_MAX];
   MY_FONT             my_font      [FONT_MAX];
   MYPATHS             path;
   CACHE_DATAS         cache;
   D2_RESSOURCES_DATAS d2_ressource [D2R_MAX];
   char                * root_path  [AT_MAX];
   int                 allegro_initialized;        // TRUE / FALSE
   HACCEL              hAccel;                     // keyboard accelerators
   int                 dccjob_threads_initialized; // TRUE / FALSE
} MAIN_DATAS;

typedef struct WINDOW_POSITION_DIMENSIONS_S
{
   int x;
   int y;
   int width;
   int height;
} WINDOW_POSITION_DIMENSIONS;

typedef struct APP_LAYER_S
{
   char  code_char [ 2 + 1];
   WCHAR code      [ 2 + 1];
   WCHAR name      [10 + 1];
} APP_LAYER;

typedef struct APP_WEAPCLASS_S
{
   WCHAR code [ 3 + 1];
   WCHAR name [25 + 1];
} APP_WEAPCLASS;

typedef struct APP_PLRTYPE_S
{
   WCHAR code [ 2 + 1];
   WCHAR name [15 + 1];
} APP_PLRTYPE;

typedef struct APP_PLRMODE_S
{
   WCHAR code [ 2 + 1];
   WCHAR name [10 + 1];
} APP_PLRMODE;

typedef struct APP_OBJMODE_S
{
   WCHAR code [ 2 + 1];
   WCHAR name [10 + 1];
} APP_OBJMODE;

typedef struct APP_COLORS_S
{
   WCHAR code [ 4 + 1];
   WCHAR name [15 + 1];
} APP_COLORS;

typedef struct APP_MONSTATS_S
{
   WCHAR code [ 2 + 1];
   WCHAR name [30 + 1];
} APP_MONSTATS;

typedef struct APP_OBJECTS_S
{
   WCHAR code [  2 + 1];
   WCHAR name [ 30 + 1];
} APP_OBJECTS;

typedef struct APPLICATION_DATAS_S
{
   int                nb_layers; // as many layers as defined in data/global/excel/composit.txt
   int                nb_weapclass;
   int                nb_plrtype;
   int                nb_plrmode;
   int                nb_objmode;
   int                nb_colors;
   int                nb_monstats;
   int                nb_objects;
   APP_LAYER          * tab_layer;
   APP_WEAPCLASS      * tab_weapclass;
   APP_PLRTYPE        * tab_plrtype;
   APP_PLRMODE        * tab_plrmode;
   APP_OBJMODE        * tab_objmode;
   APP_COLORS         * tab_colors;
   APP_MONSTATS       * tab_monstats;
   APP_OBJECTS        * tab_objects;
   int                control_to_COF_layer_map [16]; // controls dialog order TO COF_ROW_EXISTS->layer_datas[]
} APPLICATION_DATAS;

typedef struct COF_SELECTION_S
{
   long           animation_type_idx;
   long           token_idx;
   long           weapon_class_idx;
   long           mode_idx;
   STR_3_LETTERS  weapon_class_desired_code;
   STR_2_LETTERS  mode_desired_code;
   COF_ROW_EXISTS * current_cof;
} COF_SELECTION;

typedef struct DLGBOX_DATAS_S
{
   DLGBOX_MODAL_TYPE type_modal;
   ENUM_DLGBOX_ID    prev_modal_window;
   ENUM_DLGBOX_ID    next_modal_window;
   int               is_closing; // TRUE / FALSE
   int               is_active;  // TRUE / FALSE;
   CREATE_DLGBOX     dlg;
} DLGBOX_DATAS;

typedef WCHAR EXPORT_FILENAMEFORMAT [MAX_PATH];

typedef enum EXPORT_USEBOX_N
{
   EXPORT_USEBOX_USER = 0,
   EXPORT_USEBOX_ANIM,
   EXPORT_USEBOX_MAX
} EXPORT_USEBOX;

typedef struct EXPORTED_FRAME_S
{
   // identifiers in a COF
   int direction_ID;
   int frame_ID;
   int image_ID;

   // user order
   int direction;
   int frame;
   int image;

} EXPORTED_FRAME;

typedef struct VAR_LIST_DATA_S
{
   WCHAR code        [  4 + 1];
   WCHAR value       [200 + 1];
   WCHAR description [100 + 1];
} VAR_LIST_DATA;

typedef struct DLGBOX_EXPORT_DATAS_S
{
   ENUM_EXPORT_DATA_STATE datas_state;
   WCHAR                  directory  [MAX_PATH];
   EXPORT_FILENAMEFORMAT  filename_format;
   EXPORT_FILENAMEFORMAT  * filename_format_list;
   EXPORT_FORMAT          enum_format;
   EXPORT_USEBOX          enum_usebox;
   int                    nb_directions;
   int                    nb_frames_per_direction;
   WCHAR                  string_direction [MAX_PATH];
   WCHAR                  string_frames    [MAX_PATH];
   EXPORT_PALETTE_ENTRY   palette_background;
   EXPORT_PALETTE_ENTRY   palette_shadow;
   int                    bg2;
   int                    sh2;
   int                    white;
   EXPORT_BOX             user_box; // struct is in allegro_animation.h
   EXPORT_BOX             anim_box; // struct is in allegro_animation.h
   int                    direction_order [ 32];
   int                    frame_order     [256];
   int                    color_frequency [256];
   int                    is_closing_by_close_button;
   int                    mirror_sprite;
   EXPORTED_FRAME         exported_frame [32 * 256];
   int                    nb_exported_frame;
   void                   * bmp_preview;
   void                   * bmp_preview_box;
   int                    shadow_skew_percent;
   int                    shadow_height_percent;
   unsigned long          update_anim_box_at_tick; // non zero = update the animation box when the global timer count will be at least this tick
   int                    nb_ticks_before_update_anim_box; // FIXME : should be a user adjustable option
   VAR_LIST_DATA          var_list_data [VL_MAX];
   int                    shadow_offset_x;
   int                    shadow_offset_y;
} DLGBOX_EXPORT_DATAS;

typedef struct USER_PREVIEW_SETTINGS_S
{
   int zoom; // from 1 to 4
} USER_PREVIEW_SETTINGS;

typedef struct MYGLOBALS_S
{
   HINSTANCE             main_instance;      // application instance, given to WinMain()
   HWND                  hwnd_cache_debug;
   HWND                  hwnd_cache_static;
   MAIN_DATAS            datas;
   APPLICATION_DATAS     application_datas;
   LISTFILE_DATAS        listfile_datas;
   COF_SELECTION         cof_selection;
   DLGBOX_DATAS          dlgbox_datas     [DLG_MAX];
   int                   class_registered [DLG_MAX]; // 0 = not yet registered
   ANIMATION             * animation;
   int                   drawing_animation;
   DLGBOX_EXPORT_DATAS   dlgbox_export_datas;
   char                  * temp_directory;
   int                   update_animation_message_posted; // TRUE / FALSE
   USER_PREVIEW_SETTINGS user_preview_settings;
} MYGLOBALS;


extern MYGLOBALS myglobals;


void            my_show_error                 (char * expression, char * comment, char * file, char * function, long line);
int             create_my_fonts               (void);
void            destroy_my_fonts              (void);
int             get_my_ressources_directories (void);
void            make_letter_uppercase         (int * c);
void            make_string_uppercase         (char * s);
ENUM_MPQ        get_mpq_enum_from_id          (int id);
ENUM_COFSELTYPE get_cst_enum_from_id          (int id);

#endif
