#ifndef __ENUMS_H__
#define __ENUMS_H__

#include "one_include_to_rule_them_all.h"
#include "defines.h"

// identifier of all objects in all windows of the application
// each control in the application has its own ID this way
typedef enum ENUM_CTRL_IDENTIFIER_N
{
   ID_NULL = 0,
   
   // 1 ... 999 = reserved ... probably to avoid conflicts with some win32 API common ID / behaviour ?

   // cache debug window : all the text in 1 big control (for now)
   ID_CACHE_DEBUG_TEXT = 1000,

   // controls in the dialog box with the progression bar that scan COF & DCC existences
   ID_DLGBOX_COF_IN_P_ICON,
   ID_DLGBOX_COF_IN_P_LABEL,
   ID_DLGBOX_COF_IN_P_PBAR,
   ID_DLGBOX_COF_IN_P_NBFILES,
   ID_DLGBOX_COF_IN_P_EXISTS_L,
   ID_DLGBOX_COF_IN_P_EXISTS_R,
   ID_DLGBOX_COF_IN_P_OK,

   // 'Setup' dialog box : main controls
   ID_DLGBOX_SETUP_TAB,
   ID_DLGBOX_SETUP_OK,
   ID_DLGBOX_SETUP_CANCEL,

   // 'Setup' dialog box : Tab 1 (MPQ)
   ID_DLGBOX_SETUP_TABMPQ_LABEL_PATCH_D2,
   ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2,
   ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2,
   ID_DLGBOX_SETUP_TABMPQ_LABEL_D2EXP,
   ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP,
   ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP,
   ID_DLGBOX_SETUP_TABMPQ_LABEL_D2DATA,
   ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA,
   ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA,
   ID_DLGBOX_SETUP_TABMPQ_LABEL_D2CHAR,
   ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR,
   ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR,

   // 'Setup' dialog box : tab 2 (MOD Directory)
   ID_DLGBOX_SETUP_TABMODDIR_LABEL,
   ID_DLGBOX_SETUP_TABMODDIR_EDIT,
   ID_DLGBOX_SETUP_TABMODDIR_BUTN,

   // main window : timer control + animated preview like in Diablo II (handled by the Allegro library, the 'screen' in Allegro)
   ID_DLGBOX_MAIN_APP_TIMER,
   ID_DLGBOX_MAIN_ANIMATION,

   // main window : 1st group ; quick COF selection browser
   ID_DLGBOX_MAIN_COFSEL_GROUP,
   ID_DLGBOX_MAIN_ANIMTYPE_LABEL,
   ID_DLGBOX_MAIN_ANIMTYPE_LIST,
   ID_DLGBOX_MAIN_TOKEN_LABEL,
   ID_DLGBOX_MAIN_TOKEN_LIST,
   ID_DLGBOX_MAIN_WEAPCLASS_LABEL,
   ID_DLGBOX_MAIN_WEAPCLASS_LIST,
   ID_DLGBOX_MAIN_MODES_LABEL,
   ID_DLGBOX_MAIN_MODES_LIST,

   // main window : some (hopefuly temporary) controls useful for the user. The 2 buttons are absolutly not handy !
   //               it needs plenty of other controls, like animation controls (speed / stop / play / some options / ...)
   ID_DLGBOX_MAIN_BUT_ZOOM_MINUS,
   ID_DLGBOX_MAIN_BUT_ZOOM_PLUS,
   ID_DLGBOX_MAIN_BUT_STOP,
   ID_DLGBOX_MAIN_BUT_PLAY,
   ID_DLGBOX_MAIN_BUT_DIR_MINUS,
   ID_DLGBOX_MAIN_BUT_DIR_PLUS,
   ID_DLGBOX_MAIN_BUT_FRM_MINUS,
   ID_DLGBOX_MAIN_BUT_FRM_PLUS,
   ID_DLGBOX_MAIN_NB_ANIM_COLORS,

   // main window : 2nd group ; all layers elements (graphics file and their settings) of the COF selected in the 1st group
   //               here it's just the labels
   ID_DLGBOX_MAIN_LAYER_GROUP,
   ID_DLGBOX_MAIN_CODE_LABEL,
   ID_DLGBOX_MAIN_VARIANT_LABEL,
   ID_DLGBOX_MAIN_CMAPTYPE_LABEL,
   ID_DLGBOX_MAIN_CMAPFILE_LABEL,
   ID_DLGBOX_MAIN_CMAPINDEX_LABEL,
   ID_DLGBOX_MAIN_GFXTYPE_LABEL,
   ID_DLGBOX_MAIN_GFXLEVEL_LABEL,

   // main window : layers : 16 rows * 7 columns of controls ID
   //               the "*_BASE" are the base ID, all the 16 * 7 folowing ID are deduced from them
   ID_DLGBOX_MAIN_CODE_BASE,
   ID_DLGBOX_MAIN_VARIANT_BASE,
   ID_DLGBOX_MAIN_CMAPTYPE_BASE,
   ID_DLGBOX_MAIN_CMAPFILE_BASE,
   ID_DLGBOX_MAIN_CMAPINDEX_BASE,
   ID_DLGBOX_MAIN_GFXTYPE_BASE,
   ID_DLGBOX_MAIN_GFXLEVEL_BASE,

   // main window : layers : reservation of ID for the 16 rows * 7 columns
   ID_RESERVED_01 = ID_DLGBOX_MAIN_CODE_BASE + (16 * DLGBOX_MAIN_CTRL_PER_ROW),

   // export window : 1st group ; "Files"
   ID_DLGBOX_EXPORT_FILES_GROUP,
   ID_DLGBOX_EXPORT_DIRECTORY_LABEL,
   ID_DLGBOX_EXPORT_FILEFORMAT_LABEL,
   ID_DLGBOX_EXPORT_FILESAMPLE_LABEL,
   ID_DLGBOX_EXPORT_DIRECTORY_EDIT,
   ID_DLGBOX_EXPORT_DIRECTORY_BUTN,
   ID_DLGBOX_EXPORT_FILEFORMAT_COMBO,
   ID_DLGBOX_EXPORT_FILESAMPLE_EDIT,

   // export window : 2nd group ; "Format"
   ID_DLGBOX_EXPORT_FORMAT_GROUP,
   ID_DLGBOX_EXPORT_FORMAT_BMP,
   ID_DLGBOX_EXPORT_FORMAT_PCX,
   ID_DLGBOX_EXPORT_FORMAT_PNG,
   ID_DLGBOX_EXPORT_FORMAT_TGA,
   ID_DLGBOX_EXPORT_FORMAT_BAM,

   // export window : 3rd group ; "Images"
   ID_DLGBOX_EXPORT_IMAGES_GROUP,
   ID_DLGBOX_EXPORT_DIRECTIONS_LABEL,
   ID_DLGBOX_EXPORT_DIRECTIONS_EDIT,
   ID_DLGBOX_EXPORT_DIRECTIONS_BUTN,
   ID_DLGBOX_EXPORT_FRAMES_LABEL,
   ID_DLGBOX_EXPORT_FRAMES_EDIT,
   ID_DLGBOX_EXPORT_FRAMES_BUTN,

   // export window : 4th group ; "Palette"
   ID_DLGBOX_EXPORT_PALETTE_GROUP,
   ID_DLGBOX_EXPORT_BACKGROUND_LABEL,
   ID_DLGBOX_EXPORT_SHADOW_LABEL,
   ID_DLGBOX_EXPORT_INDEX_LABEL,
   ID_DLGBOX_EXPORT_RED_LABEL,
   ID_DLGBOX_EXPORT_GREEN_LABEL,
   ID_DLGBOX_EXPORT_BLUE_LABEL,
   ID_DLGBOX_EXPORT_BGINDEX_EDIT,
   ID_DLGBOX_EXPORT_BGRED_EDIT,
   ID_DLGBOX_EXPORT_BGGREEN_EDIT,
   ID_DLGBOX_EXPORT_BGBLUE_EDIT,
   ID_DLGBOX_EXPORT_SHINDEX_EDIT,
   ID_DLGBOX_EXPORT_SHRED_EDIT,
   ID_DLGBOX_EXPORT_SHGREEN_EDIT,
   ID_DLGBOX_EXPORT_SHBLUE_EDIT,

   // export window : 5th group ; "User box" (user can change the values)
   ID_DLGBOX_EXPORT_USER_BOX_GROUP,
   ID_DLGBOX_EXPORT_USER_LEFT_LABEL,
   ID_DLGBOX_EXPORT_USER_TOP_LABEL,
   ID_DLGBOX_EXPORT_USER_RIGHT_LABEL,
   ID_DLGBOX_EXPORT_USER_BOTTOM_LABEL,
   ID_DLGBOX_EXPORT_USER_WIDTH_LABEL,
   ID_DLGBOX_EXPORT_USER_HEIGHT_LABEL,
   ID_DLGBOX_EXPORT_USER_LEFT_EDIT,
   ID_DLGBOX_EXPORT_USER_TOP_EDIT,
   ID_DLGBOX_EXPORT_USER_RIGHT_EDIT,
   ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT,
   ID_DLGBOX_EXPORT_USER_WIDTH_EDIT,
   ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT,

   // export window : 6th group ; "Animation box"
   //                 automatically filled with the smallest possible enclosing blox
   //                 note : this box always contain the pivot, more obvious with flying animations, like Monster token B9
   ID_DLGBOX_EXPORT_ANIM_BOX_GROUP,
   ID_DLGBOX_EXPORT_ANIM_LEFT_LABEL,
   ID_DLGBOX_EXPORT_ANIM_TOP_LABEL,
   ID_DLGBOX_EXPORT_ANIM_RIGHT_LABEL,
   ID_DLGBOX_EXPORT_ANIM_BOTTOM_LABEL,
   ID_DLGBOX_EXPORT_ANIM_WIDTH_LABEL,
   ID_DLGBOX_EXPORT_ANIM_HEIGHT_LABEL,
   ID_DLGBOX_EXPORT_ANIM_LEFT_EDIT,
   ID_DLGBOX_EXPORT_ANIM_TOP_EDIT,
   ID_DLGBOX_EXPORT_ANIM_RIGHT_EDIT,
   ID_DLGBOX_EXPORT_ANIM_BOTTOM_EDIT,
   ID_DLGBOX_EXPORT_ANIM_WIDTH_EDIT,
   ID_DLGBOX_EXPORT_ANIM_HEIGHT_EDIT,

   // export window : big preview of the frames that would be exported with the current setting
   ID_DLGBOX_EXPORT_PREVIEW_IMG,

   // export window : 7th group ; "Shadow"
   ID_DLGBOX_EXPORT_SHADOW_GROUP,
   ID_DLGBOX_EXPORT_SH_H_LABEL,
   ID_DLGBOX_EXPORT_SH_H_EDIT,
   ID_DLGBOX_EXPORT_SH_H_PNCT_LABEL,
   ID_DLGBOX_EXPORT_SH_H_TRACKBAR,
   ID_DLGBOX_EXPORT_SH_S_LABEL,
   ID_DLGBOX_EXPORT_SH_S_EDIT,
   ID_DLGBOX_EXPORT_SH_S_PNCT_LABEL,
   ID_DLGBOX_EXPORT_SH_S_TRACKBAR,
   ID_DLGBOX_EXPORT_SH_OFX_LABEL,
   ID_DLGBOX_EXPORT_SH_OFX_EDIT,
   ID_DLGBOX_EXPORT_SH_OFY_LABEL,
   ID_DLGBOX_EXPORT_SH_OFY_EDIT,
   ID_DLGBOX_EXPORT_UP_BUTN,
   ID_DLGBOX_EXPORT_DOWN_BUTN,
   ID_DLGBOX_EXPORT_LEFT_BUTN,
   ID_DLGBOX_EXPORT_RIGHT_BUTN,

   // export window : 8th group ; "Box to use"
   ID_DLGBOX_EXPORT_USING_BOX_GROUP,
   ID_DLGBOX_EXPORT_USING_BOX_USER,
   ID_DLGBOX_EXPORT_USING_BOX_ANIM,

   // export window : 9th group ; "Options"
   ID_DLGBOX_EXPORT_MIRROR_GROUP,
   ID_DLGBOX_EXPORT_MIRROR_ANIMATION,

   // export window : list of all variables (+ their values) useable in the "Filename Format" control
   ID_DLGBOX_EXPORT_VAR_LIST,

   // export window : buttons in the far bottom / right of the window
   ID_DLGBOX_EXPORT_EXPORT_BUTN,
   ID_DLGBOX_EXPORT_CLOSE_BUTN,

   // main window : menu bar (still too short, and unfinished)
   ID_MENU_MAIN_EXPORT,
   ID_MENU_MAIN_EXPORT_ACCEL,
   ID_MENU_MAIN_OPTIONS,
   ID_MENU_MAIN_EXIT,
   ID_MENU_MAIN_COPY,
   ID_MENU_MAIN_WINDOWCACHE,
   ID_MENU_MAIN_ABOUT,

   // last ID
   ID_MAX
} ENUM_CTRL_IDENTIFIER;

// appplication's fonts ID
typedef enum ENUM_MY_FONT_N
{
   FONT_NONE = -1,

   // proportional (each character has its own width)
   FONT_VERDANA_12 = 0,
   FONT_VERDANA_13,
   FONT_VERDANA_13_BOLD,
   FONT_VERDANA_14,
   FONT_VERDANA_14_BOLD,

   // fixed (all characters have the same width)
   FONT_CONSOLAS_14,
   FONT_CONSOLAS_15,

   FONT_MAX
} ENUM_MY_FONT;

// mainly for the cache window : origin of a file extracted, usually an MPQ, but can be a directory also
typedef enum ENUM_MPQ_N
{
   MPQ_DEFAULT_D2_FILES = -1, // application own ressource directory (ressources\default_d2_files)
   MPQ_MOD_DIRECTORY    = 0,
   MPQ_PATCH_D2,
   MPQ_D2EXP,
   MPQ_D2DATA,
   MPQ_D2CHAR,
   MPQ_MAX
} ENUM_MPQ;

// animation types
// it lacks : missiles and overlays (there's no COF for both tough)
typedef enum ENUM_ANIMTYPE_N
{
   AT_NULL     = -1,
   AT_PLAYERS  = 0,
   AT_MONSTERS,
   AT_OBJECTS,
   AT_MAX
} ENUM_ANIMTYPE;

// elements of a COF path. Also : identify the type of 1 listbox in the COF selection group
typedef enum ENUM_COFSELTYPE_N
{
   CST_NONE      = -1,
   CST_ANIMTYPE  = 0,
   CST_TOKEN,
   CST_WEAPCLASS,
   CST_MODES,
   CST_MAX
} ENUM_COFSELTYPE;

// type of 1 file ressource (used by the application) of the game Diablo II
typedef enum ENUM_D2_RESSOURCES_TYPE_N
{
   D2RT_NULL = 0,
   D2RT_TEXT,
   D2RT_PALETTE,
   D2RT_ANIMDATA_D2,
   D2RT_MAX
} ENUM_D2_RESSOURCES_TYPE;

// ID of 1 file ressource (used by the application) of the game Diablo II
typedef enum ENUM_D2_RESSOURCES_N
{
   D2R_NONE        = -1,
   D2R_COMPOSIT    =  0,
   D2R_WEAPONCLASS,
   D2R_PLRTYPE,
   D2R_PLRMODE,
   D2R_OBJMODE,
   D2R_COLORS,
   D2R_MONSTATS,
   D2R_OBJECTS,
   D2R_ACT1PAL,
   D2R_ACT2PAL,
   D2R_ACT3PAL,
   D2R_ACT4PAL,
   D2R_ACT5PAL,
   D2R_ANIMDATA_D2,
   D2R_MAX
} ENUM_D2_RESSOURCES;

// type of 1 row in a "(listfile)" file beeing analysed (only the useful types for the application)
typedef enum ENUM_ROW_EXTENSION_N
{
   REXT_NULL = -1,
   REXT_COF  = 0,                    // *.cof
   REXT_DCC,                         // *.dcc
   REXT_DC6,                         // *.dc6
   REXT_DAT_CMAP_ITM,                // data/global/items/palette/*.dat
   REXT_DAT_CMAP_MON_GREENBLOOD,     // data/global/monsters/greenblood.dat
   REXT_DAT_CMAP_MON_RANDTRANSFORMS, // data/global/monsters/randtransforms.dat
   REXT_DAT_CMAP_MON_PALSHIFT,       // data/global/monsters/??/cof/palshift.dat
   // REXT_PL2,                         // .../*.pl2
   REXT_MAX
} ENUM_ROW_EXTENSION;

// type of a window created by the application "API" [dialog_maker.c] : modal or not.
// modal windows in the application : setup / export / dialog box with the progression bar
typedef enum DLGBOX_MODAL_TYPE_N
{
   DLGMT_MODLESS = 0,
   DLGMT_MODAL
} DLGBOX_MODAL_TYPE;

// internal identifier of all windows created by the application
// (except the cache debug window, because this one is made directly, without the help of the application "API" [dialog_maker.c])
typedef enum ENUM_DLGBOX_ID_N
{
   DLG_NONE             = -1,
   DLG_COF_IN_PROGRESS  = 0,
   DLG_SETUP_TAB_MPQ,
   DLG_SETUP_TAB_MODDIR,
   DLG_SETUP_TAB_CACHE,
   DLG_SETUP, // in order for IsDialogMessage() to work best [in WinMain()], we have to order the dialogs enumeration from children to parent (but why ?)
   DLG_EXPORT,
   DLG_MAIN,
   DLG_MAX
} ENUM_DLGBOX_ID;

// export window : how some controls have to be initialized
//                 works in pair with load / save config file
typedef enum ENUM_EXPORT_DATA_STATE_N
{
   EDS_TO_INITIALIZE = 0,
   EDS_JUST_LOADED,
   EDS_ALL_READY,
   EDS_MAX
} ENUM_EXPORT_DATA_STATE;

// export windows : variables useable in the filename format control
typedef enum ENUM_VAR_LIST_N
{
   VL_COF =  0,
   VL_TOK,
   VL_MOD,
   VL_WPC,
   VL_DIR,
   VL_FRM,
   VL_IMG,
   VL_DID,
   VL_FID,
   VL_IID,
   VL_VAR,
   VL_VHD,
   VL_VTR,
   VL_VLG,
   VL_VRA,
   VL_VLA,
   VL_VRH,
   VL_VLH,
   VL_VSH,
   VL_VS1,
   VL_VS2,
   VL_VS3,
   VL_VS4,
   VL_VS5,
   VL_VS6,
   VL_VS7,
   VL_VS8,
   VL_MAX
} ENUM_VAR_LIST;

// layer special effect type. 0-6 are valid values within the Diablo II COF files (5 is not used in the COF)
// other values (7-...) are specific to this application
typedef enum ENUM_LAYER_EFFECT_TYPE_N
{
   LAYEREFFECT_25_TRANS         =  0, // 25 % translucency
   LAYEREFFECT_50_TRANS         =  1, // 50 % translucency
   LAYEREFFECT_75_TRANS         =  2, // 75 % translucency
   LAYEREFFECT_SCREEN           =  3, // screen
   LAYEREFFECT_LUMINANCE        =  4, // luminance
   LAYEREFFECT_SPIDER_WEB_TRANS =  6, // spider web translucency
   LAYEREFFECT_DEL_DARK_PIXELS  =  7, // [delete dark pixels]
   LAYEREFFECT_NONE,
   LAYEREFFECT_NOT_INITIALIZED,
   LAYEREFFECT_MAX
} ENUM_LAYER_EFFECT_TYPE;

// layer special effect ROW id (order of appearance in the control)
typedef enum ENUM_LAYER_EFFECT_ROW_N
{
   LAYEREFFECTROW_NONE = 0,
   LAYEREFFECTROW_25_TRANS,
   LAYEREFFECTROW_50_TRANS,
   LAYEREFFECTROW_75_TRANS,
   LAYEREFFECTROW_SCREEN,
   LAYEREFFECTROW_LUMINANCE,
   LAYEREFFECTROW_SPIDER_WEB_TRANS,
   LAYEREFFECTROW_DEL_DARK_PIXELS,
   LAYEREFFECTROW_MAX
} ENUM_LAYER_EFFECT_TYPE;

// colormap type
typedef enum ENUM_CMAP_TYPE_N
{
   CMAPTYPE_DEFAULT_FOR_ANIMTYPE = -1,
   CMAPTYPE_NONE                 = 0,
   CMAPTYPE_PLAYER,
   CMAPTYPE_MONSTER,
   CMAPTYPE_PL2,
   CMAPTYPE_SAMEAS,
   CMAPTYPE_MAX
} ENUM_CMAP_TYPE;

#endif
