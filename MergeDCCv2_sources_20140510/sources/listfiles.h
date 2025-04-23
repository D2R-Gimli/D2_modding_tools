#ifndef __LISTFILES_H__
#define __LISTFILES_H__

#include "one_include_to_rule_them_all.h"
#include "enums.h"
#include "dcctypes.h"


typedef char STR_2_LETTERS [2 + 1];
typedef char STR_3_LETTERS [3 + 1];

typedef struct COF_ROW_S
{
   struct COF_ROW_S * next;
   STR_2_LETTERS    token;
   STR_3_LETTERS    weapon_class;
   STR_2_LETTERS    mode;
} COF_ROW;

typedef struct DCC_ROW_S
{
   struct DCC_ROW_S * next;
   STR_2_LETTERS    token;
   STR_3_LETTERS    weapon_class;
   STR_2_LETTERS    mode;
   STR_2_LETTERS    layer;
   STR_3_LETTERS    variant;
} DCC_ROW;

typedef struct DCC_ROW_EXISTS_S
{
   STR_2_LETTERS token;
   STR_3_LETTERS weapon_class;
   STR_2_LETTERS mode;
   STR_2_LETTERS layer;
   STR_3_LETTERS variant;
   int           exists; // TRUE / FALSE
} DCC_ROW_EXISTS;

typedef struct CMAP_ID_S
{
   int initialized;       // TRUE / FALSE
   int tab_cmap_file_idx; // index in myglobals.application_datas.tab_cmap_file[]-> ; 0 = none
   int cmap_idx;          // colormap index from within the colormap file it belongs to ( tab_cmap_file[]->tab_index[] )
} CMAP_ID;

typedef struct LAYER_PARAMETERS_S
{
   int                    code_idx;
   char                   code_char  [2 + 1];
   WCHAR                  code_wchar [2 + 1];
   int                    shadow;
   int                    selectable;
   int                    transparency;
   ENUM_LAYER_EFFECT_TYPE transparency_type;
   ENUM_LAYER_EFFECT_TYPE transparency_type_user;
   int                    special_effect_level;   // used only with "[delete dark pixels]" ; -1 = not initialized yet, else from 0 (keep all pixels) to 255 (delete all pixels)
   STR_3_LETTERS          weapon_class_in_cof;
   DCC_ROW_EXISTS         * first_variant;
   STR_3_LETTERS          variant;
   CMAP_ID                colormap_identifiers;
} LAYER_DATAS;

typedef struct COF_ROW_EXISTS_S
{
   STR_2_LETTERS  token;
   STR_3_LETTERS  weapon_class;
   STR_2_LETTERS  mode;
   int            exists;          // TRUE / FALSE

   // following datas : filled only once the COF is extracted and decoded
   int            already_decoded; // TRUE / FALSE
   unsigned char  nb_layers;       // 1 to 16
   LAYER_DATAS    * layer_datas;   // pointer to a table of nb_layers elements
   unsigned char  nb_frames_per_direction;
   unsigned char  nb_directions;
   long           speed;           // 256 = 25 fps, 128 = 12.5 fps
   unsigned char  frame_event[256];
   unsigned char  * priority;      // priority[direction][frame][layer] = layer code to draw

   /* user specific datas */
   int            user_anim_offset_x;
   int            user_anim_offset_y;
   int            user_pivot_offset_x;
   int            user_pivot_offset_y;
} COF_ROW_EXISTS;

typedef struct WEAPCLASS_DATAS_S
{
   STR_3_LETTERS  code;
   long           first_COF_idx;
   long           nb_cofs_for_weapon_class;
   long           nb_modes;
   STR_2_LETTERS  * tab_mode_code;
} WEAPCLASS_DATAS;

typedef struct TOKEN_DATAS_S
{
   STR_2_LETTERS   code;
   long            first_COF_idx;
   long            nb_cofs_for_token;
   long            nb_weapon_class;
   WEAPCLASS_DATAS * tab_weapon_class;
} TOKEN_DATAS;

typedef struct CMAP_ROW_S
{
   struct CMAP_ROW_S  * next;
   ENUM_ROW_EXTENSION num_row_extension;
   char               * filename;
   int                exists; // TRUE / FALSE
} CMAP_ROW;

typedef struct LISTFILE_ANIMTYPE_S
{
   COF_ROW         * first_COF;          // initial linked list of COF, can contains a COF multiple times
   long            nb_cofs;
   COF_ROW_EXISTS  * tab_COF;            // table of unique COF to check for existence in MPQ
   long            nb_tokens;
   TOKEN_DATAS     * tab_token;          // table of unique token
   DCC_ROW         * first_DCC;          // initial linked list of DCC, can contains a DCC multiple times
   long            nb_dcc;
   DCC_ROW_EXISTS  * tab_DCC;            // table of unique DCC to check for existence in MPQ
   int             default_cmap_file_id; // player = "invgreybrown", monster = token code else "randtransforms", objects = none
} LISTFILE_ANIMTYPE;

typedef UBYTE CMAP    [256];
typedef CMAP  TABCMAP [256];
   
typedef struct COLORMAP_FILE_S
{
   int                nb_colormaps;
   int                * tab_index;       // int   i = tab_index [nb_colormaps - 1]
   CMAP               * tab_cmap;        // UBYTE c = tab_cmap  [nb_colormaps - 1] [255]
   ENUM_CMAP_TYPE     num_cmap_type;     // CMAPTYPE_PLAYER / CMAPTYPE_MONSTER / CMAPTYPE_PL2 / CMAPTYPE_SAMEAS
   ENUM_ROW_EXTENSION num_row_extension; // REXT_DAT_CMAP_ITM / REXT_DAT_CMAP_MON_PALSHIFT / REXT_DAT_CMAP_MON_GREENBLOOD / REXT_DAT_CMAP_MON_RANDTRANSFORMS
   int                sameas_layer_code; // for a "Same as" colormap type, here's the layer code from 0 to 15, index in myglobals.application_datas.tab_layer[]
   WCHAR              name [20 + 1];     // invgreybrown / SK / greenblood / randtransforms
} COLORMAP_FILE;

typedef struct COLORMAP_FILE_LINK_S
{
   struct COLORMAP_FILE_LINK_S * next;
   COLORMAP_FILE               * pt_cmap;
} COLORMAP_FILE_LINK;

typedef struct LISTFILE_DATAS_S
{
   LISTFILE_ANIMTYPE at [AT_MAX];            // table of Animation Types
   CMAP_ROW          * first_CMAP;           // linked list of unique colormaps

   COLORMAP_FILE_LINK * first_colormap_link; // temporary linked list, used before tab_cmap_file[] is build, then the the list is destroyed
   int                nb_cmap_files;
   COLORMAP_FILE      ** tab_cmap_file;      // table of pointers : tab_cmap_file[]->
} LISTFILE_DATAS;

void analyse_listfile   (char * cptr, long length);
void destroy_listfiles  (void);
int  load_all_listfiles (void);

#endif
