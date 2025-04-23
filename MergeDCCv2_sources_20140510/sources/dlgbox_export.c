#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <wchar.h>
#include <errno.h>
#include "strings_define.h"
#include "defines.h"
#include "dlgbox_export.h"
#include "misc.h"
#include "dialog_maker.h"
#include "tools.h"

#define  DLGBOX_EXPORT_NB_CHILDREN  88

static LRESULT CALLBACK  callback_dlgbox_export                              (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_APP_EXPORT_IMAGES         (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_APP_EXPORT_UPDATE_BOX     (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_COMMAND                   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_DESTROY                   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_HSCROLL                   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_KEYDOWN                   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_SYSCOMMAND                (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_dlgbox_export_WM_TIMER                     (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_CLOSE_BUTN_for_BN_CLICKED           (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_DIRECTIONS_EDIT_for_EN_KILLFOCUS    (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_DIRECTORY_BUTN_for_BN_CLICKED       (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_DIRECTORY_EDIT_for_EN_KILLFOCUS     (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_SH_H_EDIT_for_EN_KILLFOCUS          (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_SH_S_EDIT_for_EN_KILLFOCUS          (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_EXPORT_BUTN_for_BN_CLICKED          (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_FILEFORMAT_COMBO_for_CBN_KILLFOCUS  (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_FILEFORMAT_COMBO_for_CBN_SELENDOK   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_FRAMES_EDIT_for_EN_KILLFOCUS        (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED          (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_MIRROR_BUTN_for_BN_CLICKED          (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_USING_BOX_BUTN_for_BN_CLICKED       (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK  callback_EXPORT_validate_value_for_EN_KILLFOCUS     (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void              create_all_directories_within_path                  (WCHAR * filename);
static void              create_new_preview_bitmap_from_box                  (void);
static int               decode_direction_order                              (void);
static int               decode_direction_order_with_warning                 (void);
static int               decode_frame_order                                  (void);
static int               decode_frame_order_with_warning                     (void);
static void              draw_preview_image                                  (void);
static void              helper_box_set_new_value                            (ENUM_CTRL_IDENTIFIER id, int * pt_value, HWND h_focus);
static int               init_dlgbox_export_controls                         (CREATE_DLGBOX * d);
static int               init_dlgbox_export_datas                            (void);
static int               init_listview                                       (ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam);
static int               init_trackbar                                       (ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam);
static int               init_update_preview_bitmap                          (EXPORT_PARAMETERS * ex);
static void              remove_edit_control_selection                       (ENUM_CTRL_IDENTIFIER id);
static void              reset_var_list_values                               (void);
static int               save_exported_animation_images                      (void);
static void              search_best_fit_colors                              (void);
static int               select_new_export_directory                         (HWND hParent, WCHAR * path);
static void              update_box_controls                                 (int update_user_box, int update_animation_box);
static void              update_export_box_values                            (int update_user_box, int update_animation_box);
static int               update_export_filesample                            (WCHAR * sample, EXPORTED_FRAME * ef);
static void              update_exported_frame_table                         (void);
static void              update_user_box_controls_state                      (void);


// ===========================================================================
// initialize a Shadow Height or Shadow Skew trackbar
// ===========================================================================
int init_trackbar(ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam)
{
   HWND h = NULL;


   dwParam = dwParam; // just to avoid a warning
   lpParam = lpParam; // just to avoid a warning

   h = myglobals.dlgbox_datas[dialog_ID].dlg.pChild[iChildIdx].handle;
   if (h != NULL)
   {
      SendMessage(h, TBM_SETRANGEMIN, (WPARAM) FALSE, (LPARAM) -100);
      SendMessage(h, TBM_SETRANGEMAX, (WPARAM) TRUE,  (LPARAM) 100);
      SendMessage(h, TBM_SETTICFREQ,  (WPARAM) 10,    (LPARAM) 100);
      SendMessage(h, TBM_SETLINESIZE, (WPARAM) 0,     (LPARAM) 1);
      SendMessage(h, TBM_SETPAGESIZE, (WPARAM) 0,     (LPARAM) 10);
   }

   return 0;
}


// ===========================================================================
// initialize the list view for filename variables replacment
// ===========================================================================
int init_listview(ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam)
{
   DLGBOX_EXPORT_DATAS * e    = & myglobals.dlgbox_export_datas;
   LVCOLUMN            column;
   HWND                h      = NULL;
   int                 idx    = 0;
   int                 i      = 0;


   h = myglobals.dlgbox_datas[dialog_ID].dlg.pChild[iChildIdx].handle;
   if (h == NULL)
      return 0;

   dwParam = dwParam; // just to avoid a warning
   lpParam = lpParam; // just to avoid a warning

   memset(e->var_list_data, 0, sizeof(e->var_list_data));
   i = VL_COF; wcscpy(e->var_list_data[i].code, TEXT("%COF")); wcscpy(e->var_list_data[i].description, TEXT("COF filename"));
   i = VL_TOK; wcscpy(e->var_list_data[i].code, TEXT("%TOK")); wcscpy(e->var_list_data[i].description, TEXT("Token"));
   i = VL_MOD; wcscpy(e->var_list_data[i].code, TEXT("%MOD")); wcscpy(e->var_list_data[i].description, TEXT("Mode"));
   i = VL_WPC; wcscpy(e->var_list_data[i].code, TEXT("%WPC")); wcscpy(e->var_list_data[i].description, TEXT("Weapon Class"));
   i = VL_DIR; wcscpy(e->var_list_data[i].code, TEXT("%DIR")); wcscpy(e->var_list_data[i].description, TEXT("Direction ID (in user order)"));
   i = VL_FRM; wcscpy(e->var_list_data[i].code, TEXT("%FRM")); wcscpy(e->var_list_data[i].description, TEXT("Frame ID (in user order)"));
   i = VL_IMG; wcscpy(e->var_list_data[i].code, TEXT("%IMG")); wcscpy(e->var_list_data[i].description, TEXT("Image ID (in user order)"));
   i = VL_DID; wcscpy(e->var_list_data[i].code, TEXT("%DID")); wcscpy(e->var_list_data[i].description, TEXT("Direction ID (in COF order)"));
   i = VL_FID; wcscpy(e->var_list_data[i].code, TEXT("%FID")); wcscpy(e->var_list_data[i].description, TEXT("Frame ID (in COF order)"));
   i = VL_IID; wcscpy(e->var_list_data[i].code, TEXT("%IID")); wcscpy(e->var_list_data[i].description, TEXT("Image ID (in COF order)"));
   i = VL_VAR; wcscpy(e->var_list_data[i].code, TEXT("%VAR")); wcscpy(e->var_list_data[i].description, TEXT("Variants list of all non-empty layers"));
   i = VL_VHD; wcscpy(e->var_list_data[i].code, TEXT("%VHD")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in HD layer (Head)"));
   i = VL_VTR; wcscpy(e->var_list_data[i].code, TEXT("%VTR")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in TR layer (Torso)"));
   i = VL_VLG; wcscpy(e->var_list_data[i].code, TEXT("%VLG")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in LG layer (Legs)"));
   i = VL_VRA; wcscpy(e->var_list_data[i].code, TEXT("%VRA")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in RA layer (Right arm)"));
   i = VL_VLA; wcscpy(e->var_list_data[i].code, TEXT("%VLA")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in LA layer (Left arm)"));
   i = VL_VRH; wcscpy(e->var_list_data[i].code, TEXT("%VRH")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in RH layer (Right hand)"));
   i = VL_VLH; wcscpy(e->var_list_data[i].code, TEXT("%VLH")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in LH layer (Left hand)"));
   i = VL_VSH; wcscpy(e->var_list_data[i].code, TEXT("%VSH")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in SH layer (Shield)"));
   i = VL_VS1; wcscpy(e->var_list_data[i].code, TEXT("%VS1")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S1 layer (Special 1)"));
   i = VL_VS2; wcscpy(e->var_list_data[i].code, TEXT("%VS2")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S2 layer (Special 2)"));
   i = VL_VS3; wcscpy(e->var_list_data[i].code, TEXT("%VS3")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S3 layer (Special 3)"));
   i = VL_VS4; wcscpy(e->var_list_data[i].code, TEXT("%VS4")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S4 layer (Special 4)"));
   i = VL_VS5; wcscpy(e->var_list_data[i].code, TEXT("%VS5")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S5 layer (Special 5)"));
   i = VL_VS6; wcscpy(e->var_list_data[i].code, TEXT("%VS6")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S6 layer (Special 6)"));
   i = VL_VS7; wcscpy(e->var_list_data[i].code, TEXT("%VS7")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S7 layer (Special 7)"));
   i = VL_VS8; wcscpy(e->var_list_data[i].code, TEXT("%VS8")); wcscpy(e->var_list_data[i].description, TEXT("Variant used in S8 layer (Special 8)"));

   // column headers

   memset( & column, 0, sizeof(column));
   column.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
   column.fmt      = LVCFMT_LEFT;

   column.cx       = 50;
   column.pszText  = TEXT("Code");
   column.iSubItem = 0;
   idx = SendMessage(h, LVM_INSERTCOLUMN, 0, (LPARAM) & column);

   column.cx       = 90;
   column.pszText  = TEXT("Value(s)");
   column.iSubItem = 1;
   idx = SendMessage(h, LVM_INSERTCOLUMN, 1, (LPARAM) & column);

   column.cx       = 230;
   column.pszText  = TEXT("Description");
   column.iSubItem = 2;
   idx = SendMessage(h, LVM_INSERTCOLUMN, 2, (LPARAM) & column);

   // rows
   reset_var_list_values();

   return 0;
}


// to avoid all the warnings like : warning C4428: universal-character-name encountered in source
#pragma warning( push )
#pragma warning(disable:4428)

// ===========================================================================
// create the dialog box window (with its children) : Export
// return 0 on success
// ===========================================================================
int create_dlgbox_export(void)
{
   ENUM_DLGBOX_ID       dialog_ID   = DLG_EXPORT;
   int                  dwidth      = 838;
   int                  dheight     = 686;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd        = NULL;
   CREATE_DLGBOX        * d         = NULL;
   CREATE_DLGBOX_CHILD  c [DLGBOX_EXPORT_NB_CHILDREN] =
   {
      // iChildID                          lpClassName     left top  width height         dwStyle                                                                                         dwExStyle          lpWindowName                       iFontID               lpParam pFuncInitChild           dwFuncInitChildParam    lpFuncInitChildParam handle
      // --------------------------------- --------------- ---- ---- ----- -------------- ----------------------------------------------------------------------------------------------- ------------------ ---------------------------------- --------------------- ------- ------------------------ ----------------------- -------------------- ------
      {ID_DLGBOX_EXPORT_FILES_GROUP,       WC_BUTTON,      12,  12,  808,  102,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Files"),                     FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTORY_LABEL,   WC_STATIC,      27,  31,  115,  16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Directory"),                 FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FILEFORMAT_LABEL,  WC_STATIC,      27,  57,  115,  16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Filename format"),           FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FILESAMPLE_LABEL,  WC_STATIC,      27,  83,  115,  16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Filename sample"),           FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTORY_EDIT,    WC_EDIT,        151, 27,  598,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTORY_BUTN,    WC_BUTTON,      758, 26,  50,   22,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("..."),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FILEFORMAT_COMBO,  WC_COMBOBOX,    151, 53,  598,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,                                 0,                 NULL,                              FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FILESAMPLE_EDIT,   WC_EDIT,        151, 79,  598,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_FORMAT_GROUP,      WC_BUTTON,      12,  120, 88,   180,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Format"),                    FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FORMAT_BMP,        WC_BUTTON,      31,  139, 55,   20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,                                          0,                 TEXT("BMP"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FORMAT_PCX,        WC_BUTTON,      31,  165, 55,   20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,                                                     0,                 TEXT("PCX"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FORMAT_PNG,        WC_BUTTON,      31,  191, 55,   20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,                                                     0,                 TEXT("PNG"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FORMAT_TGA,        WC_BUTTON,      31,  217, 55,   20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,                                                     0,                 TEXT("TGA"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FORMAT_BAM,        WC_BUTTON,      31,  243, 55,   20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,                                                     0,                 TEXT("BAM"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_IMAGES_GROUP,      WC_BUTTON,      110, 120, 378,  73,            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Images"),                    FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTIONS_LABEL,  WC_STATIC,      125, 139, 75,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Directions"),                FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTIONS_EDIT,   WC_EDIT,        204, 135, 214,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DIRECTIONS_BUTN,   WC_BUTTON,      427, 134, 50,   22,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("..."),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FRAMES_LABEL,      WC_STATIC,      125, 165, 75,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Frames"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FRAMES_EDIT,       WC_EDIT,        204, 161, 214,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_FRAMES_BUTN,       WC_BUTTON,      427, 160, 50,   22,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("..."),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_PALETTE_GROUP,     WC_BUTTON,      110, 199, 378,  101,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Palette"),                   FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BACKGROUND_LABEL,  WC_STATIC,      125, 238, 85,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Background"),                FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SHADOW_LABEL,      WC_STATIC,      125, 265, 85,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Shadow"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_INDEX_LABEL,       WC_STATIC,      219, 215, 54,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Index"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_RED_LABEL,         WC_STATIC,      296, 215, 54,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Red"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_GREEN_LABEL,       WC_STATIC,      356, 215, 54,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Green"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BLUE_LABEL,        WC_STATIC,      416, 215, 54,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Blue"),                      FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BGINDEX_EDIT,      WC_EDIT,        219, 234, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BGRED_EDIT,        WC_EDIT,        296, 234, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BGGREEN_EDIT,      WC_EDIT,        356, 234, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_BGBLUE_EDIT,       WC_EDIT,        416, 234, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SHINDEX_EDIT,      WC_EDIT,        219, 261, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SHRED_EDIT,        WC_EDIT,        296, 261, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SHGREEN_EDIT,      WC_EDIT,        356, 261, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SHBLUE_EDIT,       WC_EDIT,        416, 261, 54,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_USER_BOX_GROUP,    WC_BUTTON,      498, 120, 156,  180,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("User Box"),                  FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_LEFT_LABEL,   WC_STATIC,      512, 144, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Left"),                      FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_TOP_LABEL,    WC_STATIC,      512, 170, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Top"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_RIGHT_LABEL,  WC_STATIC,      512, 196, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Right"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_BOTTOM_LABEL, WC_STATIC,      512, 222, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Bottom"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_WIDTH_LABEL,  WC_STATIC,      512, 248, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Width"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_HEIGHT_LABEL, WC_STATIC,      512, 274, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Height"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_LEFT_EDIT,    WC_EDIT,        572, 140, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_TOP_EDIT,     WC_EDIT,        572, 166, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_RIGHT_EDIT,   WC_EDIT,        572, 192, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT,  WC_EDIT,        572, 218, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_WIDTH_EDIT,   WC_EDIT,        572, 244, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT,  WC_EDIT,        572, 270, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_ANIM_BOX_GROUP,    WC_BUTTON,      664, 120, 156,  180,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Animation Box"),             FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_LEFT_LABEL,   WC_STATIC,      678, 144, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Left"),                      FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_TOP_LABEL,    WC_STATIC,      678, 170, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Top"),                       FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_RIGHT_LABEL,  WC_STATIC,      678, 196, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Right"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_BOTTOM_LABEL, WC_STATIC,      678, 222, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Bottom"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_WIDTH_LABEL,  WC_STATIC,      678, 248, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Width"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_HEIGHT_LABEL, WC_STATIC,      678, 274, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Height"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_LEFT_EDIT,    WC_EDIT,        738, 140, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_TOP_EDIT,     WC_EDIT,        738, 166, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_RIGHT_EDIT,   WC_EDIT,        738, 192, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_BOTTOM_EDIT,  WC_EDIT,        738, 218, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_WIDTH_EDIT,   WC_EDIT,        738, 244, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_ANIM_HEIGHT_EDIT,  WC_EDIT,        738, 270, 69,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_PREVIEW_IMG,       WC_STATIC,      12,  312, 300,  300,           WS_CHILD | WS_VISIBLE | SS_ICON,                                                                0,                 TEXT(""),                          FONT_NONE,            NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_SHADOW_GROUP,      WC_BUTTON,      324, 306, 496,  145,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Shadow"),                    FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_H_LABEL,        WC_STATIC,      338, 329, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Height"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_H_EDIT,         WC_EDIT,        398, 326, 60,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_H_PNCT_LABEL,   WC_STATIC,      460, 329, 15,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("%"),                         FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_H_TRACKBAR,     TRACKBAR_CLASS, 487, 316, 320,  40,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_TOP | TBS_AUTOTICKS | TBS_TOOLTIPS,         0,                 TEXT(""),                          FONT_VERDANA_14,      NULL,   init_trackbar,           0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_S_LABEL,        WC_STATIC,      338, 367, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Skew"),                      FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_S_EDIT,         WC_EDIT,        398, 364, 60,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_S_PNCT_LABEL,   WC_STATIC,      460, 367, 15,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("%"),                         FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_S_TRACKBAR,     TRACKBAR_CLASS, 487, 357, 320,  40,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_TOP | TBS_AUTOTICKS | TBS_TOOLTIPS,         0,                 TEXT(""),                          FONT_VERDANA_14,      NULL,   init_trackbar,           0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_OFX_LABEL,      WC_STATIC,      338, 415, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Offset X"),                  FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_OFX_EDIT,       WC_EDIT,        398, 412, 60,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_OFY_LABEL,      WC_STATIC,      488, 415, 55,   16,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                WS_EX_TRANSPARENT, TEXT("Offset Y"),                  FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_SH_OFY_EDIT,       WC_EDIT,        548, 412, 60,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,                                  WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_UP_BUTN,           WC_BUTTON,      688, 398, 50,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("Up"),                        FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_LEFT_BUTN,         WC_BUTTON,      636, 411, 50,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("Left"),                      FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_RIGHT_BUTN,        WC_BUTTON,      740, 411, 50,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("Right"),                     FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_DOWN_BUTN,         WC_BUTTON,      688, 424, 50,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("Down"),                      FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_USING_BOX_GROUP,   WC_BUTTON,      324, 457, 146,  75,            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Box to use"),                FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USING_BOX_USER,    WC_BUTTON,      343, 476, 123,  20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,                                          0,                 TEXT("User"),                      FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_USING_BOX_ANIM,    WC_BUTTON,      343, 502, 123,  20,            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,                                                     0,                 TEXT("Animation"),                 FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_MIRROR_GROUP,      WC_BUTTON,      324, 538, 146,  75,            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                            0,                 TEXT("Options"),                   FONT_VERDANA_13,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_MIRROR_ANIMATION,  WC_BUTTON,      343, 557, 123,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX,                             0,                 TEXT("Mirror Directions"),         FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_VAR_LIST,          WC_LISTVIEW,    482, 463, 338, 150,            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER,                                          WS_EX_CLIENTEDGE,  TEXT(""),                          FONT_VERDANA_13,      NULL,   init_listview,           0,                      NULL,                NULL},

      {ID_DLGBOX_EXPORT_EXPORT_BUTN,       WC_BUTTON,      543, 623, 130,  HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_DEFPUSHBUTTON, 0,                 TEXT("Export"),                    FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_EXPORT_CLOSE_BUTN,        WC_BUTTON,      689, 623, 130,  HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,                 TEXT("Close"),                     FONT_VERDANA_14,      NULL,   NULL,                    0,                      NULL,                NULL},
   };


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODAL;

   // window class
   cw.cbSize        = sizeof(WNDCLASSEX);
   cw.style         = CS_HREDRAW | CS_VREDRAW;
   cw.lpfnWndProc   = callback_dlgbox_export;
   cw.cbClsExtra    = 0;
   cw.cbWndExtra    = 0;
   cw.hInstance     = myglobals.main_instance;
   cw.hIcon         = NULL;
   cw.hCursor       = LoadCursor(NULL, IDC_ARROW);
   cw.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   cw.lpszMenuName  = NULL;
   cw.lpszClassName = STR_DLGBOX_EXPORT_CLASS;
   cw.hIconSm       = NULL;

   d->pWndClassEx = & cw;

   // window
   d->window.hWndParent            = myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle;
   d->window.left                  = get_x_to_center_into_screen(dwidth);
   d->window.top                   = get_y_to_center_into_screen(dheight);
   d->window.width                 = dwidth;
   d->window.height                = dheight;
   d->window.dwStyle               = WS_POPUP | WS_BORDER | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
   d->window.dwExStyle             = 0;
   d->window.lpWindowName          = STR_DLGBOX_EXPORT_WINDOW_NAME;
   d->window.hMenu                 = NULL;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // children
   d->nbChildren = DLGBOX_EXPORT_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   // initialise some controls
   init_dlgbox_export_datas();
   MYASSERT_RETURN(init_dlgbox_export_controls(d) == 0, 1, NULL);

   // show and draw it
   stop_tick_25fps(1, 0);
   start_tick_25fps(0, 1);
   ShowWindow(d->window.handle, SW_SHOWNORMAL);
   UpdateWindow(d->window.handle);
   SetFocus(d->window.handle);

   return 0;
}

#pragma warning( pop )


// ===========================================================================
// initialise the Export dialog controls
// ===========================================================================
int init_dlgbox_export_controls(CREATE_DLGBOX * d)
{
   DLGBOX_EXPORT_DATAS   * e               = & myglobals.dlgbox_export_datas;
   int                   child_idx         = -1;
   int                   n                 = 0;
   WCHAR                 sample [MAX_PATH] = TEXT("");
   EXPORT_FILENAMEFORMAT * ptr_fnf         = NULL;
   ENUM_CTRL_IDENTIFIER  id                = ID_NULL;
   EXPORTED_FRAME        ef;
   EXPORTED_FRAME        * ptr_ef          = NULL;
   int                   nb_d              = 0;
   int                   nb_f              = 0;
   int                   w                 = 0;
   int                   h                 = 0;
   int                   offset_x          = 0;
   int                   offset_y          = 0;


   if (d == NULL)
      return 1;

   // directory   

   n = wcslen(e->directory) - 1;
   while ((n >= 0) && (e->directory[n] == TEXT(' ')))
   {
      e->directory[n] = 0;
      n--;
   }

   n = wcslen(e->directory) - 1;
   while ((n >= 0) && (e->directory[n] == TEXT('\\')))
   {
      e->directory[n] = 0;
      n--;
   }

   n = wcslen(e->directory);
   if (n > 0)
   {
      if (e->directory[n - 1] != TEXT('\\'))
         wcscat(e->directory, TEXT("\\"));
   }

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_DIRECTORY_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, e->directory);

   // filename format

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILEFORMAT_COMBO, & child_idx) == 0)
   {
      SetWindowText(d->pChild[child_idx].handle, e->filename_format);

      // filename format list
      ptr_fnf = e->filename_format_list;
      if (ptr_fnf != NULL)
      {
         while (wcslen((WCHAR *) ptr_fnf) > 0)
         {
            SendMessage(d->pChild[child_idx].handle, CB_ADDSTRING, 0, (LPARAM) ptr_fnf);
            ptr_fnf++;
         }
      }
   }

   // filename sample
   if (myglobals.cof_selection.current_cof != NULL)
   {
      nb_d = myglobals.cof_selection.current_cof->nb_directions;
      nb_f = myglobals.cof_selection.current_cof->nb_frames_per_direction;
      if ((nb_d > 0) && (nb_f > 0))
      {
         memset( & ef, 0, sizeof(ef));
         ef.direction_ID = ef.direction = nb_d - 1;
         ef.frame_ID     = ef.frame     = nb_f - 1;
         ef.image_ID     = ef.image     = (nb_d * nb_f) - 1;
         ptr_ef = & ef;
      }
   }

   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILESAMPLE_EDIT, FALSE);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILESAMPLE_EDIT, & child_idx) == 0)
   {
      if (update_export_filesample(sample, ptr_ef) != 0)
         wcscpy(sample, TEXT(""));

      SetWindowText(d->pChild[child_idx].handle, sample);
   }

   // formats
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FORMAT_PNG, FALSE); // FIXME
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FORMAT_BAM, FALSE); // FIXME
   id = ID_NULL;
   switch (e->enum_format)
   {
      case EXPORT_BMP : id = ID_DLGBOX_EXPORT_FORMAT_BMP; break;
      case EXPORT_PCX : id = ID_DLGBOX_EXPORT_FORMAT_PCX; break;
      case EXPORT_PNG : id = ID_DLGBOX_EXPORT_FORMAT_PNG; break;
      case EXPORT_TGA : id = ID_DLGBOX_EXPORT_FORMAT_TGA; break;
      case EXPORT_BAM : id = ID_DLGBOX_EXPORT_FORMAT_BAM; break;
   }

   if (id != ID_NULL)
   {
      if (get_dialog_child_index_from_ID(DLG_EXPORT, id, & child_idx) == 0)
         SendMessage(d->pChild[child_idx].handle, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
   }

   // boxes
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_BOX_GROUP,   FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_LEFT_EDIT,   FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_TOP_EDIT,    FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_RIGHT_EDIT,  FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_BOTTOM_EDIT, FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_WIDTH_EDIT,  FALSE);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_HEIGHT_EDIT, FALSE);
   id = ID_NULL;
   switch (e->enum_usebox)
   {
      case EXPORT_USEBOX_USER : id = ID_DLGBOX_EXPORT_USING_BOX_USER; break;
      case EXPORT_USEBOX_ANIM : id = ID_DLGBOX_EXPORT_USING_BOX_ANIM; break;
   }

   if (id != ID_NULL)
   {
      if (get_dialog_child_index_from_ID(DLG_EXPORT, id, & child_idx) == 0)
         SendMessage(d->pChild[child_idx].handle, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
   }

   update_user_box_controls_state();

   // directions
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_DIRECTIONS_BUTN, FALSE); // FIXME : need to suport directions edition
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_DIRECTIONS_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, e->string_direction);

   // frames
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FRAMES_BUTN, FALSE); // FIXME : need to suport frames edition
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FRAMES_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, e->string_frames);

   // palette background

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_background.index);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_BGINDEX_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_background.red);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_BGRED_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_background.green);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_BGGREEN_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_background.blue);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_BGBLUE_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   // palette shadow

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_shadow.index);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SHINDEX_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_shadow.red);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SHRED_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_shadow.green);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SHGREEN_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->palette_shadow.blue);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SHBLUE_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   // boxes
   update_box_controls(TRUE, TRUE);

   // preview image

   if (e->bmp_preview != NULL)
   {
      destroy_allegro_bitmap(e->bmp_preview);
      e->bmp_preview = NULL;
   }

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_PREVIEW_IMG, & child_idx) == 0)
   {
      get_window_position_and_dimension(d->pChild[child_idx].handle, NULL, NULL, & w, & h);
      e->bmp_preview = get_allegro_bitmap(w, h);
   }

   // shadow height

   swprintf(sample, MAX_PATH, TEXT("%d"), e->shadow_height_percent);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_H_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_H_TRACKBAR, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) e->shadow_height_percent);

   // shadow skew

   swprintf(sample, MAX_PATH, TEXT("%d"), e->shadow_skew_percent);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_S_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_S_TRACKBAR, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) e->shadow_skew_percent);

   // shadow offsets

   get_anim_user_offsets(myglobals.animation, & offset_x, & offset_y, MOVING_PIVOT);
   swprintf(sample, MAX_PATH, TEXT("%d"), e->shadow_offset_x);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_OFX_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   swprintf(sample, MAX_PATH, TEXT("%d"), e->shadow_offset_y);
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_OFY_EDIT, & child_idx) == 0)
      SetWindowText(d->pChild[child_idx].handle, sample);

   // mirror option
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_MIRROR_ANIMATION, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, BM_SETCHECK, (WPARAM) (e->mirror_sprite == TRUE) ? BST_CHECKED : BST_UNCHECKED, 0);

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_FILEFORMAT_COMBO : WM_COMMAND : CBN_SELENDOK
// filename format had just been changed, by selecting a new item in the list
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_FILEFORMAT_COMBO_for_CBN_SELENDOK(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                 child_idx         = -1;
   CREATE_DLGBOX       * dlg             = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   WCHAR               sample [MAX_PATH] = TEXT("");
   EXPORTED_FRAME      ef;
   EXPORTED_FRAME      * ptr_ef          = NULL;
   int                 nb_d              = 0;
   int                 nb_f              = 0;
   int                 curr_sel          = 0;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   // get the new filename format
   curr_sel = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
   if ((curr_sel >= 0) && (curr_sel < 10))
      wcscpy(e->filename_format, e->filename_format_list[curr_sel]);
   else
      wcscpy(e->filename_format, TEXT(""));

   // use it to update the filename sample
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILESAMPLE_EDIT, & child_idx) == 0)
   {
      if (myglobals.cof_selection.current_cof != NULL)
      {
         nb_d = myglobals.cof_selection.current_cof->nb_directions;
         nb_f = myglobals.cof_selection.current_cof->nb_frames_per_direction;
         if ((nb_d > 0) && (nb_f > 0))
         {
            memset( & ef, 0, sizeof(ef));
            ef.direction_ID = ef.direction = nb_d - 1;
            ef.frame_ID     = ef.frame     = nb_f - 1;
            ef.image_ID     = ef.image     = (nb_d * nb_f) - 1;
            ptr_ef = & ef;
         }
      }

      if (update_export_filesample(sample, ptr_ef) != 0)
         wcscpy(sample, TEXT(""));

      SetWindowText(dlg->pChild[child_idx].handle, sample);
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_FILEFORMAT_COMBO : WM_COMMAND : CBN_KILLFOCUS
// filename format had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_FILEFORMAT_COMBO_for_CBN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                 child_idx         = -1;
   CREATE_DLGBOX       * dlg             = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   WCHAR               sample [MAX_PATH] = TEXT("");
   EXPORTED_FRAME      ef;
   EXPORTED_FRAME      * ptr_ef          = NULL;
   int                 nb_d              = 0;
   int                 nb_f              = 0;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   // get the new filename format
   GetWindowText((HWND) lParam, e->filename_format, MAX_PATH);
                        
   // use it to update the filename sample
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILESAMPLE_EDIT, & child_idx) == 0)
   {
      if (myglobals.cof_selection.current_cof != NULL)
      {
         nb_d = myglobals.cof_selection.current_cof->nb_directions;
         nb_f = myglobals.cof_selection.current_cof->nb_frames_per_direction;
         if ((nb_d > 0) && (nb_f > 0))
         {
            memset( & ef, 0, sizeof(ef));
            ef.direction_ID = ef.direction = nb_d - 1;
            ef.frame_ID     = ef.frame     = nb_f - 1;
            ef.image_ID     = ef.image     = (nb_d * nb_f) - 1;
            ptr_ef = & ef;
         }
      }

      if (update_export_filesample(sample, ptr_ef) != 0)
         wcscpy(sample, TEXT(""));

      SetWindowText(dlg->pChild[child_idx].handle, sample);
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_SH_H_EDIT : WM_COMMAND : EN_KILLFOCUS
// shadow height edit control had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_SH_H_EDIT_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   HWND                h                 = (HWND) lParam;
   WCHAR               string [MAX_PATH] = TEXT("");
   int                 value             = 0;
   int                 child_idx         = -1;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   if (h != NULL)
   {
      // get the user input
      GetWindowText(h, string, MAX_PATH);

      // keep the value between bounds
      swscanf(string, TEXT("%d"), & value);
      if (value < -100)
         value = -100;
      else if (value > 100)
         value = 100;

      // save the value
      e->shadow_height_percent = value;

      // set the value back to the edit control
      swprintf(string, MAX_PATH, TEXT("%d"), value);
      SetWindowText(h, string);

      // set the value in the trackbar as well
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_H_TRACKBAR, & child_idx) == 0)
         SendMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.pChild[child_idx].handle, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) e->shadow_height_percent);

      // inform the application that the export box need to update its values
      PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_SH_S_EDIT : WM_COMMAND : EN_KILLFOCUS
// shadow skew edit control had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_SH_S_EDIT_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   HWND                h                 = (HWND) lParam;
   WCHAR               string [MAX_PATH] = TEXT("");
   int                 value             = 0;
   int                 child_idx         = -1;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   if (h != NULL)
   {
      // get the user input
      GetWindowText(h, string, MAX_PATH);

      // keep the value between bounds
      swscanf(string, TEXT("%d"), & value);
      if (value < -100)
         value = -100;
      else if (value > 100)
         value = 100;

      // save the value
      e->shadow_skew_percent = value;

      // set the value back to the edit control
      swprintf(string, MAX_PATH, TEXT("%d"), value);
      SetWindowText(h, string);

      // set the value in the trackbar as well
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_S_TRACKBAR, & child_idx) == 0)
         SendMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.pChild[child_idx].handle, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) e->shadow_skew_percent);

      // inform the application that the export box need to update its values
      PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_DIRECTORY_EDIT : WM_COMMAND : EN_KILLFOCUS
// directory path had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_DIRECTORY_EDIT_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e = & myglobals.dlgbox_export_datas;
   HWND                h   = (HWND) lParam;
   int                 n   = 0;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   if (h != NULL)
   {
      // get the new directory path
      GetWindowText(h, e->directory, MAX_PATH);

      if (wcslen(e->directory) > 0)
      {
         // make sure it exists
         if (validate_directory_path(e->directory) != TRUE)
         {
            SetWindowText(h, TEXT(""));
            e->directory[0] = 0;
         }

         // make sure it ends with an '\' character
         n = wcslen(e->directory);
         if (n > 0)
         {
            if (e->directory[n - 1] != TEXT('\\'))
            {
               wcscat(e->directory, TEXT("\\"));
               SetWindowText(h, e->directory);
            }
         }
      }
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_DIRECTORY_BUTN : WM_COMMAND : BN_CLICKED
// directory browse button had just been pushed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_DIRECTORY_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                 child_idx = -1;
   CREATE_DLGBOX       * dlg     = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS * e       = & myglobals.dlgbox_export_datas;
   HWND                h         = NULL;
   int                 n         = 0;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;
   lParam = lParam;

   // get the handle of the directory edit control
   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_DIRECTORY_EDIT, & child_idx) == 0)
      h = dlg->pChild[child_idx].handle;

   if (h != NULL)
   {
      // select a directory
      if (select_new_export_directory(NULL, e->directory) == TRUE)
         SetWindowText(h, e->directory);

      // make sure it ends with an '\' character
      n = wcslen(e->directory);
      if (n > 0)
      {
         if (e->directory[n - 1] != TEXT('\\'))
         {
            wcscat(e->directory, TEXT("\\"));
            SetWindowText(h, e->directory);
         }
      }
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_USING_BOX_XXX : WM_COMMAND : BN_CLICKED
// a "box to use" radio button had just been selected
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_USING_BOX_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   WORD                identifier = LOWORD(wParam);
   DLGBOX_EXPORT_DATAS * e        = & myglobals.dlgbox_export_datas;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   lParam = lParam;

   switch (identifier)
   {
      case ID_DLGBOX_EXPORT_USING_BOX_USER : e->enum_usebox = EXPORT_USEBOX_USER; break;
      case ID_DLGBOX_EXPORT_USING_BOX_ANIM : e->enum_usebox = EXPORT_USEBOX_ANIM; break;
   }

   update_user_box_controls_state();
   create_new_preview_bitmap_from_box();
   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_FORMAT_xxx : WM_COMMAND : BN_CLICKED
// a format radio button had just been selected
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   WORD                identifier = LOWORD(wParam);
   DLGBOX_EXPORT_DATAS * e        = & myglobals.dlgbox_export_datas;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   lParam = lParam;

   switch (identifier)
   {
      case ID_DLGBOX_EXPORT_FORMAT_BMP : e->enum_format = EXPORT_BMP; break;
      case ID_DLGBOX_EXPORT_FORMAT_PCX : e->enum_format = EXPORT_PCX; break;
      case ID_DLGBOX_EXPORT_FORMAT_PNG : e->enum_format = EXPORT_PNG; break;
      case ID_DLGBOX_EXPORT_FORMAT_TGA : e->enum_format = EXPORT_TGA; break;
      case ID_DLGBOX_EXPORT_FORMAT_BAM : e->enum_format = EXPORT_BAM; break;
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_MIRROR_xxx : WM_COMMAND : BN_CLICKED
// a mirror checkbox had just been clicked
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_MIRROR_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   WORD                identifier   = LOWORD(wParam);
   DLGBOX_EXPORT_DATAS * e          = & myglobals.dlgbox_export_datas;
   int                 child_idx    = -1;
   int                 * ptr_option = NULL;
   int                 state        = BST_INDETERMINATE;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   lParam = lParam;

   if (get_dialog_child_index_from_ID(DLG_EXPORT, (ENUM_CTRL_IDENTIFIER) identifier, & child_idx) == 0)
      state = SendMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.pChild[child_idx].handle, BM_GETCHECK, 0, 0);

   switch (identifier)
   {
      case ID_DLGBOX_EXPORT_MIRROR_ANIMATION : ptr_option = & e->mirror_sprite; break;
   }

   if (ptr_option != NULL)
   {
      if (state == BST_CHECKED)
         (* ptr_option) = TRUE;
      else
         (* ptr_option) = FALSE;

      // inform the application that the export box need to update its values
      PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
   }

   return 0;
}


// ===========================================================================
// helper for updating the value of 1 control of the export box
// ===========================================================================
void helper_box_set_new_value(ENUM_CTRL_IDENTIFIER id, int * pt_value, HWND h_focus)
{
   int           child_idx         = -1;
   HWND          h                 = NULL;
   WCHAR         string [MAX_PATH] = TEXT("");
   CREATE_DLGBOX * dlg             = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;


   if (get_dialog_child_index_from_ID(DLG_EXPORT, id, & child_idx) == 0)
   {
      h = dlg->pChild[child_idx].handle;
      if (h != NULL)
      {
         swprintf(string, MAX_PATH, TEXT("%ld"), (* pt_value));
         SetWindowText(h, string);            // set new text, and in the process remove the selection if there was any :( ... EM_SETSEL is needed to make a new selection
         if (h == h_focus)                    // the test is not necessary, but no need to send the message if we knoww it's useless in advance (see below)
            SendMessage(h, EM_SETSEL, 0, -1); // from the EM_SETSEL MSDN help page : if the edit control don't have the ES_NOHIDESEL style (our case here),
                                              //   then the selected text is highlighted ONLY when the edit control has the focus
      }                                                                          
   }                                                                             
}


// ===========================================================================
// update all the export box values
// ===========================================================================
void update_export_box_values(int update_user_box, int update_animation_box)
{
   DLGBOX_EXPORT_DATAS * e      = & myglobals.dlgbox_export_datas;
   EXPORT_BOX          * pt_box = NULL;
   HWND                h_focus  = GetFocus();


   if (update_user_box == TRUE)
   {
      pt_box = & e->user_box;
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_LEFT_EDIT,   & pt_box->left,   h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_RIGHT_EDIT,  & pt_box->right,  h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_TOP_EDIT,    & pt_box->top,    h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT, & pt_box->bottom, h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_WIDTH_EDIT,  & pt_box->width,  h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT, & pt_box->height, h_focus);
   }

   if (update_animation_box == TRUE)
   {
      pt_box = & e->anim_box;
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_LEFT_EDIT,   & pt_box->left,   h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_RIGHT_EDIT,  & pt_box->right,  h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_TOP_EDIT,    & pt_box->top,    h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_BOTTOM_EDIT, & pt_box->bottom, h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_WIDTH_EDIT,  & pt_box->width,  h_focus);
      helper_box_set_new_value(ID_DLGBOX_EXPORT_ANIM_HEIGHT_EDIT, & pt_box->height, h_focus);
   }
}


// ===========================================================================
// ID_DLGBOX_EXPORT_[BG|SH][INDEX|RED|GREEN|BLUE]_EDIT : WM_COMMAND : EN_KILLFOCUS
// ID_DLGBOX_EXPORT_[box field]_EDIT                   : WM_COMMAND : EN_KILLFOCUS
// ID_DLGBOX_EXPORT_SH_OF[X|Y]_EDIT                    : WM_COMMAND : EN_KILLFOCUS
// either : a palette entry had just been changed. keep it within the range [    0 ;  255] and search the new best fit color if it's a palette entry or keep the box dimensions coherents
//       or a box entry     had just been changed. keep it within the range [-2000 ; 2000] and search the new best fit color if it's a palette entry or keep the box dimensions coherents
//       or a shadow offset had just been changed. keep it within the range [-2000 ; 2000]
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_validate_value_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                  child_idx         = -1;
   WORD                 identifier        = LOWORD(wParam);
   CREATE_DLGBOX        * dlg             = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS  * e               = & myglobals.dlgbox_export_datas;
   HWND                 h                 = NULL;
   HWND                 h2                = NULL;
   WCHAR                string [MAX_PATH] = TEXT("");
   int                  update_bg         = FALSE;
   int                  update_sh         = FALSE;
   int                  update_offsets    = FALSE;
   long                 value             = 0;
   long                 value_min         = 0;
   long                 value_max         = 0;
   int                  new_preview_bmp   = FALSE;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   lParam = lParam;

   // get the handle of the control to validate
   h = NULL;
   if (get_dialog_child_index_from_ID(DLG_EXPORT, (ENUM_CTRL_IDENTIFIER) identifier, & child_idx) == 0)
      h = dlg->pChild[child_idx].handle;

   if (h != NULL)
   {
      switch (identifier)
      {
         case ID_DLGBOX_EXPORT_BGINDEX_EDIT :
         case ID_DLGBOX_EXPORT_BGRED_EDIT   :
         case ID_DLGBOX_EXPORT_BGGREEN_EDIT :
         case ID_DLGBOX_EXPORT_BGBLUE_EDIT  :
         case ID_DLGBOX_EXPORT_SHINDEX_EDIT :
         case ID_DLGBOX_EXPORT_SHRED_EDIT   :
         case ID_DLGBOX_EXPORT_SHGREEN_EDIT :
         case ID_DLGBOX_EXPORT_SHBLUE_EDIT  :
            value_min = 0;
            value_max = 255;
            break;

         case ID_DLGBOX_EXPORT_USER_LEFT_EDIT   :
         case ID_DLGBOX_EXPORT_USER_TOP_EDIT    :
         case ID_DLGBOX_EXPORT_USER_RIGHT_EDIT  :
         case ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT :
         case ID_DLGBOX_EXPORT_USER_WIDTH_EDIT  :
         case ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT :
         case ID_DLGBOX_EXPORT_SH_OFX_EDIT      :
         case ID_DLGBOX_EXPORT_SH_OFY_EDIT      :
            value_min       = -2000;
            value_max       = 2000;
            new_preview_bmp = TRUE;
            break;
      }

      // get the new value, and keep it within a valide range
      GetWindowText(h, string, MAX_PATH);
      value = _wtol(string);
      if (value < value_min)
         value = value_min;
      else if (value > value_max)
         value = value_max;

      // set it back
      swprintf(string, MAX_PATH, TEXT("%ld"), value);
      SetWindowText(h, string);

      // update export datas
      switch (identifier)
      {
         case ID_DLGBOX_EXPORT_BGINDEX_EDIT : e->palette_background.index = value; break;
         case ID_DLGBOX_EXPORT_BGRED_EDIT   : e->palette_background.red   = value; update_bg = TRUE; break;
         case ID_DLGBOX_EXPORT_BGGREEN_EDIT : e->palette_background.green = value; update_bg = TRUE; break;
         case ID_DLGBOX_EXPORT_BGBLUE_EDIT  : e->palette_background.blue  = value; update_bg = TRUE; break;
         case ID_DLGBOX_EXPORT_SHINDEX_EDIT : e->palette_shadow.index     = value; break;
         case ID_DLGBOX_EXPORT_SHRED_EDIT   : e->palette_shadow.red       = value; update_sh = TRUE; break;
         case ID_DLGBOX_EXPORT_SHGREEN_EDIT : e->palette_shadow.green     = value; update_sh = TRUE; break;
         case ID_DLGBOX_EXPORT_SHBLUE_EDIT  : e->palette_shadow.blue      = value; update_sh = TRUE; break;

         case ID_DLGBOX_EXPORT_USER_LEFT_EDIT   : e->user_box.left   = value; break;
         case ID_DLGBOX_EXPORT_USER_TOP_EDIT    : e->user_box.top    = value; break;
         case ID_DLGBOX_EXPORT_USER_RIGHT_EDIT  : e->user_box.right  = value; break;
         case ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT : e->user_box.bottom = value; break;
         case ID_DLGBOX_EXPORT_USER_WIDTH_EDIT  : e->user_box.width  = value; break;
         case ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT : e->user_box.height = value; break;

         case ID_DLGBOX_EXPORT_SH_OFX_EDIT : e->shadow_offset_x = value; update_offsets = TRUE; break;
         case ID_DLGBOX_EXPORT_SH_OFY_EDIT : e->shadow_offset_y = value; update_offsets = TRUE; break;
      }

      // validate the user box
      switch (identifier)
      {
         case ID_DLGBOX_EXPORT_USER_LEFT_EDIT   :
            if (e->user_box.left > e->user_box.right)
               e->user_box.left = e->user_box.right;
            e->user_box.width = e->user_box.right - e->user_box.left + 1;
            break;

         case ID_DLGBOX_EXPORT_USER_TOP_EDIT    :
            if (e->user_box.top > e->user_box.bottom)
               e->user_box.top = e->user_box.bottom;
            e->user_box.height = e->user_box.bottom - e->user_box.top + 1;
            break;

         case ID_DLGBOX_EXPORT_USER_RIGHT_EDIT  :
            if (e->user_box.right < e->user_box.left)
               e->user_box.right = e->user_box.left;
            e->user_box.width = e->user_box.right - e->user_box.left + 1;
            break;

         case ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT :
            if (e->user_box.bottom < e->user_box.top)
               e->user_box.bottom = e->user_box.top;
            e->user_box.height = e->user_box.bottom - e->user_box.top + 1;
            break;

         case ID_DLGBOX_EXPORT_USER_WIDTH_EDIT  :
            if (e->user_box.width < 1)
               e->user_box.width = 1;
            value = e->user_box.left + e->user_box.width - 1;
            if (value > 2000)
            {
               e->user_box.left  = 0;
               e->user_box.right = 0;
               e->user_box.width = 1;
            }
            else
               e->user_box.right = value;
            break;

         case ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT :
            if (e->user_box.height < 1)
               e->user_box.height = 1;
            value = e->user_box.top + e->user_box.height - 1;
            if (value > 2000)
            {
               e->user_box.top    = 0;
               e->user_box.bottom = 0;
               e->user_box.height = 1;
            }
            else
               e->user_box.bottom = value;
            break;
      }

      // update the user box controls with all new values
      switch (identifier)
      {
         case ID_DLGBOX_EXPORT_USER_LEFT_EDIT   :
         case ID_DLGBOX_EXPORT_USER_TOP_EDIT    :
         case ID_DLGBOX_EXPORT_USER_RIGHT_EDIT  :
         case ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT :
         case ID_DLGBOX_EXPORT_USER_WIDTH_EDIT  :
         case ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT :
            update_export_box_values(TRUE, FALSE);
            break;
      }

      // keep the background and shadow index differents
      if (identifier == ID_DLGBOX_EXPORT_BGINDEX_EDIT)
      {
         if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SHINDEX_EDIT, & child_idx) == 0)
         {
            h2 = dlg->pChild[child_idx].handle;
            if (value == e->palette_shadow.index)
            {
               e->palette_background.index = 0;
               e->palette_shadow.index     = 1;
               SetWindowText(h,  TEXT("0"));
               SetWindowText(h2, TEXT("1"));
            }
         }
      }
      else if (identifier == ID_DLGBOX_EXPORT_SHINDEX_EDIT)
      {
         if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_BGINDEX_EDIT, & child_idx) == 0)
         {
            h2 = dlg->pChild[child_idx].handle;
            if (value == e->palette_background.index)
            {
               e->palette_background.index = 0;
               e->palette_shadow.index     = 1;
               SetWindowText(h,  TEXT("1"));
               SetWindowText(h2, TEXT("0"));
            }
         }
      }

   }

   // search the new best fit color(s)
   if ((update_bg == TRUE) || (update_sh == TRUE))
      search_best_fit_colors();

   if (new_preview_bmp == TRUE)
      create_new_preview_bitmap_from_box();
   
   // if shadow offsets changed, inform the application that the export box need to update its values
   if (update_offsets == TRUE)
      PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);

   //return 0;
   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// ID_DLGBOX_EXPORT_DIRECTIONS_EDIT : WM_COMMAND : EN_KILLFOCUS
// directions string had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_DIRECTIONS_EDIT_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e = & myglobals.dlgbox_export_datas;
   HWND                h   = (HWND) lParam;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   if (h != NULL)
   {
      // get the new directions string
      GetWindowText(h, e->string_direction, MAX_PATH);

      // decode the string
      if (decode_direction_order_with_warning() != 0)
         memset( & e->anim_box, 0, sizeof(EXPORT_BOX));
      else
      {
         if (decode_frame_order() != 0)
            memset( & e->anim_box, 0, sizeof(EXPORT_BOX));
         else
         {
            // update exported animation box
            PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
         }
      }

      update_exported_frame_table();
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_FRAMES_EDIT : WM_COMMAND : EN_KILLFOCUS
// frames string had just been changed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_FRAMES_EDIT_for_EN_KILLFOCUS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e = & myglobals.dlgbox_export_datas;
   HWND                h   = (HWND) lParam;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;

   if (h != NULL)
   {
      // get the new frames string
      GetWindowText(h, e->string_frames, MAX_PATH);

      // decode the string
      if (decode_frame_order_with_warning() != 0)
         memset( & e->anim_box, 0, sizeof(EXPORT_BOX));
      else
      {
         if (decode_direction_order() != 0)
            memset( & e->anim_box, 0, sizeof(EXPORT_BOX));
         else
         {
            // update exported box
            PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
         }
      }

      update_exported_frame_table();
   }

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_EXPORT_BUTN : WM_COMMAND : BN_CLICKED
// Export button had just been pushed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_EXPORT_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;
   lParam = lParam;

   // inform the application that the animation needs to be exported right now
   PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_IMAGES, 0, 0);

   return 0;
}


// ===========================================================================
// ID_DLGBOX_EXPORT_CLOSE_BUTN : WM_COMMAND : BN_CLICKED
// Close button had just been pushed
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_CLOSE_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   // just to avoid warnings
   msg    = msg;
   wParam = wParam;
   lParam = lParam;

   myglobals.dlgbox_datas[DLG_EXPORT].is_closing = TRUE;
   myglobals.dlgbox_export_datas.is_closing_by_close_button = TRUE;
   SendMessage(hwnd, WM_CLOSE, 0, 0);

   return 0;
}


// ===========================================================================
// WM_COMMAND
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_COMMAND(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   WORD identifier        = LOWORD(wParam);
   WORD notification_code = HIWORD(wParam);
   int  n                 = 0;
   static const struct
   {
      ENUM_CTRL_IDENTIFIER identifier;
      WORD                 notification_code;
      LRESULT (CALLBACK    * ptrFunc) (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   } data [] = {
      // identifier                       notification_code      ptrFunc
      // -------------------------------  ---------------------  --------------------------------------------------
      {ID_DLGBOX_EXPORT_DIRECTORY_EDIT,   EN_KILLFOCUS,          callback_EXPORT_DIRECTORY_EDIT_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_DIRECTORY_BUTN,   BN_CLICKED,            callback_EXPORT_DIRECTORY_BUTN_for_BN_CLICKED},

      {ID_DLGBOX_EXPORT_FILEFORMAT_COMBO, CBN_KILLFOCUS,         callback_EXPORT_FILEFORMAT_COMBO_for_CBN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_FILEFORMAT_COMBO, CBN_EDITCHANGE,        callback_EXPORT_FILEFORMAT_COMBO_for_CBN_KILLFOCUS}, // each time the user change 1 character
      {ID_DLGBOX_EXPORT_FILEFORMAT_COMBO, CBN_SELENDOK,          callback_EXPORT_FILEFORMAT_COMBO_for_CBN_SELENDOK},  // each time the user select a new item in the list

      {ID_DLGBOX_EXPORT_FORMAT_BMP,       BN_CLICKED,            callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_FORMAT_PCX,       BN_CLICKED,            callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_FORMAT_PNG,       BN_CLICKED,            callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_FORMAT_TGA,       BN_CLICKED,            callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_FORMAT_BAM,       BN_CLICKED,            callback_EXPORT_FORMAT_BUTN_for_BN_CLICKED},

      {ID_DLGBOX_EXPORT_DIRECTIONS_EDIT,  EN_KILLFOCUS,          callback_EXPORT_DIRECTIONS_EDIT_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_FRAMES_EDIT,      EN_KILLFOCUS,          callback_EXPORT_FRAMES_EDIT_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_BGINDEX_EDIT,     EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_BGRED_EDIT,       EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_BGGREEN_EDIT,     EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_BGBLUE_EDIT,      EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_SHINDEX_EDIT,     EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_SHRED_EDIT,       EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_SHGREEN_EDIT,     EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_SHBLUE_EDIT,      EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_USER_LEFT_EDIT,   EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_USER_TOP_EDIT,    EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_USER_RIGHT_EDIT,  EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT, EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_USER_WIDTH_EDIT,  EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT, EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_SH_OFX_EDIT,      EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_SH_OFY_EDIT,      EN_KILLFOCUS,          callback_EXPORT_validate_value_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_UP_BUTN,          BN_CLICKED,            callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_DOWN_BUTN,        BN_CLICKED,            callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_LEFT_BUTN,        BN_CLICKED,            callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_RIGHT_BUTN,       BN_CLICKED,            callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED},

      {ID_DLGBOX_EXPORT_SH_H_EDIT,        EN_KILLFOCUS,          callback_EXPORT_SH_H_EDIT_for_EN_KILLFOCUS},
      {ID_DLGBOX_EXPORT_SH_S_EDIT,        EN_KILLFOCUS,          callback_EXPORT_SH_S_EDIT_for_EN_KILLFOCUS},

      {ID_DLGBOX_EXPORT_USING_BOX_USER,   BN_CLICKED,            callback_EXPORT_USING_BOX_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_USING_BOX_ANIM,   BN_CLICKED,            callback_EXPORT_USING_BOX_BUTN_for_BN_CLICKED},

      {ID_DLGBOX_EXPORT_MIRROR_ANIMATION, BN_CLICKED,            callback_EXPORT_MIRROR_BUTN_for_BN_CLICKED},

      {ID_DLGBOX_EXPORT_EXPORT_BUTN,      BN_CLICKED,            callback_EXPORT_EXPORT_BUTN_for_BN_CLICKED},
      {ID_DLGBOX_EXPORT_CLOSE_BUTN,       BN_CLICKED,            callback_EXPORT_CLOSE_BUTN_for_BN_CLICKED},

      {ID_NULL, 0, NULL}
   };


   if (identifier == IDCANCEL)
      return callback_EXPORT_CLOSE_BUTN_for_BN_CLICKED(hwnd, msg, wParam, lParam);
   else if (identifier == IDOK)
      return callback_EXPORT_EXPORT_BUTN_for_BN_CLICKED(hwnd, msg, wParam, lParam);
   else
   {
      for (n = 0; data[n].identifier != ID_NULL; n++)
      {
         if ((identifier == data[n].identifier) && (notification_code == data[n].notification_code) && (data[n].ptrFunc != NULL))
            return data[n].ptrFunc(hwnd, msg, wParam, lParam);
      }
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// WM_SYSCOMMAND
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_SYSCOMMAND(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if ((wParam & 0xFFF0) == SC_CLOSE)
      myglobals.dlgbox_datas[DLG_EXPORT].is_closing = TRUE;

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// WM_DESTROY
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_DESTROY(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;
   lParam = lParam;

   if (myglobals.dlgbox_datas[DLG_EXPORT].is_closing == TRUE)
   {
      // if (myglobals.dlgbox_export_datas.is_closing_by_close_button == FALSE)
      save_current_configuration();

      stop_tick_25fps(0, 1);

      close_dialog(DLG_EXPORT);

      if (myglobals.animation != NULL)
         start_tick_25fps(1, 0);

      DESTROYME(myglobals.dlgbox_export_datas.bmp_preview, destroy_allegro_bitmap)
   }

   return 0;
}


// ===========================================================================
// WM_APP_EXPORT_IMAGES
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_APP_EXPORT_IMAGES(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   stop_tick_25fps(0, 1);
   save_exported_animation_images();
   start_tick_25fps(0, 1);

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// WM_APP_EXPORT_UPDATE_BOX
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_APP_EXPORT_UPDATE_BOX(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e = & myglobals.dlgbox_export_datas;


   stop_tick_25fps(0, 1);

   search_exported_animation_box(
      myglobals.animation,
      e->direction_order,
      e->frame_order,
      & e->anim_box,
      & e->palette_background,
      e->color_frequency,
      e->mirror_sprite,
      e->shadow_height_percent,
      e->shadow_skew_percent,
      e->shadow_offset_x,
      e->shadow_offset_y
   );

   update_export_box_values(FALSE, TRUE);
   create_new_preview_bitmap_from_box();

   start_tick_25fps(0, 1);

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// WM_KEYDOWN
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_KEYDOWN(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (wParam == VK_ESCAPE)
      return callback_dlgbox_export_WM_DESTROY(hwnd, msg, wParam, lParam);

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// WM_TIMER
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_TIMER(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   EXPORT_PARAMETERS ex;


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   wParam = wParam;
   lParam = lParam;

   if (init_update_preview_bitmap( & ex) == 0)
   {
      // init OK
      if (update_preview_bitmap( & ex) != 0)
      {
         // preview image has been prepared with a new image, now draw it
         draw_preview_image();
      }
   }

   return 0;
}


// ===========================================================================
// WM_HSCROLL
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export_WM_HSCROLL(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                 child_idx         = -1;
   CREATE_DLGBOX       * d               = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   WCHAR               string [MAX_PATH] = TEXT("");
   int                 value             = 0;


   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_H_TRACKBAR, & child_idx) == 0)
   {
      if ((HWND) lParam == d->pChild[child_idx].handle)
      {
         // shadow height trackbar
         value = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);

         e->shadow_height_percent = value;

         // set the value to the edit control
         if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_H_EDIT, & child_idx) == 0)
         {
            swprintf(string, MAX_PATH, TEXT("%d"), value);
            SetWindowText(d->pChild[child_idx].handle, string);
         }

         // inform the application that the export box need to update its values, but in few frames, not right now
         e->update_anim_box_at_tick = get_current_tick_25fps_preview() + e->nb_ticks_before_update_anim_box;
         return 0;
      }
   }

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_S_TRACKBAR, & child_idx) == 0)
   {
      if ((HWND) lParam == d->pChild[child_idx].handle)
      {
         // shadow skew trackbar
         value = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);

         e->shadow_skew_percent = value;

         // set the value to the edit control
         if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_S_EDIT, & child_idx) == 0)
         {
            swprintf(string, MAX_PATH, TEXT("%d"), value);
            SetWindowText(d->pChild[child_idx].handle, string);
         }

         // inform the application that the export box need to update its values, but in few frames, not right now
         e->update_anim_box_at_tick = get_current_tick_25fps_preview() + e->nb_ticks_before_update_anim_box;
         return 0;
      }
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// callback function for the dialog box : Export
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_export(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg)
   {
      case WM_COMMAND               : return callback_dlgbox_export_WM_COMMAND               (hwnd, msg, wParam, lParam);
      case WM_SYSCOMMAND            : return callback_dlgbox_export_WM_SYSCOMMAND            (hwnd, msg, wParam, lParam);
      case WM_DESTROY               : return callback_dlgbox_export_WM_DESTROY               (hwnd, msg, wParam, lParam);
      case WM_APP_EXPORT_IMAGES     : return callback_dlgbox_export_WM_APP_EXPORT_IMAGES     (hwnd, msg, wParam, lParam);
      case WM_APP_EXPORT_UPDATE_BOX : return callback_dlgbox_export_WM_APP_EXPORT_UPDATE_BOX (hwnd, msg, wParam, lParam);
      case WM_KEYDOWN               : return callback_dlgbox_export_WM_KEYDOWN               (hwnd, msg, wParam, lParam);
      case WM_TIMER                 : return callback_dlgbox_export_WM_TIMER                 (hwnd, msg, wParam, lParam);
      case WM_HSCROLL               : return callback_dlgbox_export_WM_HSCROLL               (hwnd, msg, wParam, lParam);
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// open the operating system common dialog box for selecting a directory
// return TRUE if the user pushed the OK button
// ===========================================================================
int select_new_export_directory(HWND hParent, WCHAR * path)
{
   BROWSEINFO       bi  = {0};
   PIDLIST_ABSOLUTE res = NULL;


   if (path == NULL)
      return FALSE;

   memset( & bi, 0, sizeof(bi));
   bi.hwndOwner      = hParent;
   bi.pidlRoot       = NULL;
   bi.pszDisplayName = path;
   bi.lpszTitle      = STR_SELECT_EXPORT_DIRECTORY_TITLE;
   bi.ulFlags        = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
   bi.lpfn           = NULL;
   bi.lParam         = 0;
   bi.iImage         = 0;

   memset( & res, 0, sizeof(PIDLIST_ABSOLUTE));
   res = SHBrowseForFolder( & bi); // http://msdn.microsoft.com/en-us/library/bb762115%28v=vs.85%29.aspx
   if (res == NULL)
      return FALSE; // CANCEL button was pushed, no need to call CoTaskMemFree() in this case

   SHGetPathFromIDList(res, path);
   DESTROYME(res, CoTaskMemFree)

   if (validate_directory_path(path) != TRUE)
   {
      path[0] = 0;
      return FALSE;
   }

   return TRUE;
}


// ===========================================================================
// update the sample string, given the current format and parameters
// return 0 if success
// ===========================================================================
int update_export_filesample(WCHAR * sample, EXPORTED_FRAME * ef)
{
   DLGBOX_EXPORT_DATAS * e         = & myglobals.dlgbox_export_datas;
   WCHAR               * fmt       = e->filename_format;
   HWND                h           = NULL;
   int                 child_idx   = -1;
   CREATE_DLGBOX       * dlg       = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   int                 n           = 0;
   int                 n2          = 0;
   WCHAR               c           = 0;
   WCHAR               code[4 + 1] = TEXT("");


   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_FILESAMPLE_EDIT, & child_idx) == 0)
      h = dlg->pChild[child_idx].handle;

   if (sample == NULL)
      return 1;

   wcscpy(sample, TEXT(""));

   if ((h == NULL) || (ef == NULL))
      return 1;

   if ( (ef->direction_ID < 0) || (ef->direction_ID >= 32) || (ef->frame_ID < 0) || (ef->frame_ID >= 256) || (ef->image_ID < 0) || (ef->image_ID >= (32 * 256))
        ||
        (ef->direction < 0) || (ef->direction >= 32) || (ef->frame < 0) || (ef->frame >= 256) || (ef->image < 0) || (ef->image >= (32 * 256)) )
   {
      return 1;
   }

   c = fmt[n];
   while (c != 0)
   {
      if (c == TEXT('%'))
      {
         memset(code, 0, sizeof(code));
         wcsncpy(code, fmt + n, 4);

         if      (wcscmp(code, e->var_list_data[VL_COF].code) == 0) wcscat(sample, e->var_list_data[VL_COF].value);
         else if (wcscmp(code, e->var_list_data[VL_TOK].code) == 0) wcscat(sample, e->var_list_data[VL_TOK].value);
         else if (wcscmp(code, e->var_list_data[VL_MOD].code) == 0) wcscat(sample, e->var_list_data[VL_MOD].value);
         else if (wcscmp(code, e->var_list_data[VL_WPC].code) == 0) wcscat(sample, e->var_list_data[VL_WPC].value);
         else if (wcscmp(code, e->var_list_data[VL_VAR].code) == 0) wcscat(sample, e->var_list_data[VL_VAR].value);
         else if (wcscmp(code, e->var_list_data[VL_VHD].code) == 0) wcscat(sample, e->var_list_data[VL_VHD].value);
         else if (wcscmp(code, e->var_list_data[VL_VTR].code) == 0) wcscat(sample, e->var_list_data[VL_VTR].value);
         else if (wcscmp(code, e->var_list_data[VL_VLG].code) == 0) wcscat(sample, e->var_list_data[VL_VLG].value);
         else if (wcscmp(code, e->var_list_data[VL_VRA].code) == 0) wcscat(sample, e->var_list_data[VL_VRA].value);
         else if (wcscmp(code, e->var_list_data[VL_VLA].code) == 0) wcscat(sample, e->var_list_data[VL_VLA].value);
         else if (wcscmp(code, e->var_list_data[VL_VRH].code) == 0) wcscat(sample, e->var_list_data[VL_VRH].value);
         else if (wcscmp(code, e->var_list_data[VL_VLH].code) == 0) wcscat(sample, e->var_list_data[VL_VLH].value);
         else if (wcscmp(code, e->var_list_data[VL_VSH].code) == 0) wcscat(sample, e->var_list_data[VL_VSH].value);
         else if (wcscmp(code, e->var_list_data[VL_VS1].code) == 0) wcscat(sample, e->var_list_data[VL_VS1].value);
         else if (wcscmp(code, e->var_list_data[VL_VS2].code) == 0) wcscat(sample, e->var_list_data[VL_VS2].value);
         else if (wcscmp(code, e->var_list_data[VL_VS3].code) == 0) wcscat(sample, e->var_list_data[VL_VS3].value);
         else if (wcscmp(code, e->var_list_data[VL_VS4].code) == 0) wcscat(sample, e->var_list_data[VL_VS4].value);
         else if (wcscmp(code, e->var_list_data[VL_VS5].code) == 0) wcscat(sample, e->var_list_data[VL_VS5].value);
         else if (wcscmp(code, e->var_list_data[VL_VS6].code) == 0) wcscat(sample, e->var_list_data[VL_VS6].value);
         else if (wcscmp(code, e->var_list_data[VL_VS7].code) == 0) wcscat(sample, e->var_list_data[VL_VS7].value);
         else if (wcscmp(code, e->var_list_data[VL_VS8].code) == 0) wcscat(sample, e->var_list_data[VL_VS8].value);
         else if (wcscmp(code, e->var_list_data[VL_DIR].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%02d"), ef->direction);
         else if (wcscmp(code, e->var_list_data[VL_FRM].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%03d"), ef->frame);
         else if (wcscmp(code, e->var_list_data[VL_IMG].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%04d"), ef->image);
         else if (wcscmp(code, e->var_list_data[VL_DID].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%02d"), ef->direction_ID);
         else if (wcscmp(code, e->var_list_data[VL_FID].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%03d"), ef->frame_ID);
         else if (wcscmp(code, e->var_list_data[VL_IID].code) == 0) swprintf(sample + n2, MAX_PATH, TEXT("%04d"), ef->image_ID);
         else
         {
            wcscat(sample, TEXT("%"));
            n -= 3;
         }
         n2 = wcslen(sample);
         n += 4;
      }
      else
      {
         sample[n2]   = c;
         sample[n2+1] = 0;
         n2++;
         n++;
      }
      c = fmt[n];
   }

   return 0;
}


// ===========================================================================
// prepare the datas that will be displayed in the dialog box
// return 0 if success
// ===========================================================================
int init_dlgbox_export_datas(void)
{
   DLGBOX_EXPORT_DATAS * pDatas = & myglobals.dlgbox_export_datas;
   COF_ROW_EXISTS      * pCof   = NULL;
   int                 i        = 0;
   int                 max_d    = 0;
   int                 max_f    = 0;


   if (pDatas->datas_state == EDS_TO_INITIALIZE)
   {
      wcscpy(pDatas->filename_format, TEXT("%COF\\d%DIR_f%FRM"));

      pDatas->enum_format = EXPORT_BMP;
      pDatas->enum_usebox = EXPORT_USEBOX_ANIM;

      pDatas->palette_background.index = 0;
      pDatas->palette_background.red   = 0;
      pDatas->palette_background.green = 255;
      pDatas->palette_background.blue  = 0;

      pDatas->palette_shadow.index = 1;
      pDatas->palette_shadow.red   = 0;
      pDatas->palette_shadow.green = 0;
      pDatas->palette_shadow.blue  = 0;

      pDatas->mirror_sprite         = FALSE;
      pDatas->shadow_height_percent = 50;
      pDatas->shadow_skew_percent   = -100;

      pDatas->user_box.left   = -100;
      pDatas->user_box.top    = -200;
      pDatas->user_box.right  = 100;
      pDatas->user_box.bottom = 100;
      pDatas->user_box.width  = pDatas->user_box.right  - pDatas->user_box.left + 1;
      pDatas->user_box.height = pDatas->user_box.bottom - pDatas->user_box.top + 1;

      pDatas->shadow_offset_x = 0;
      pDatas->shadow_offset_y = 0;

      pDatas->datas_state = EDS_JUST_LOADED;
   }

   if (pDatas->datas_state == EDS_JUST_LOADED)
   {
      if (pDatas->filename_format_list == NULL)
      {
         pDatas->filename_format_list = (EXPORT_FILENAMEFORMAT *) calloc(10 + 1, sizeof(EXPORT_FILENAMEFORMAT));
         MYASSERT_RETURN(pDatas->filename_format_list != NULL, -1, "calloc() error");
         wcscpy(pDatas->filename_format_list[ 0], TEXT("%COF\\d%DIR_f%FRM"));
         wcscpy(pDatas->filename_format_list[ 1], TEXT("%COF\\d%DIR_f%FRM_i%IMG"));
         wcscpy(pDatas->filename_format_list[ 2], TEXT("%COF\\i%IMG_d%DIR_f%FRM"));
         wcscpy(pDatas->filename_format_list[ 3], TEXT("%COF\\i%IMG"));
         wcscpy(pDatas->filename_format_list[ 4], TEXT("%COF\\D%DID_F%FID"));
         wcscpy(pDatas->filename_format_list[ 5], TEXT("%COF\\D%DID_F%FID_I%IID"));
         wcscpy(pDatas->filename_format_list[ 6], TEXT("%COF\\I%IID_D%DID_F%FID"));
         wcscpy(pDatas->filename_format_list[ 7], TEXT("%COF\\I%IID"));
         wcscpy(pDatas->filename_format_list[ 8], TEXT("%TOK\\%WPC\\%MOD\\D%DID_F%FID"));
         wcscpy(pDatas->filename_format_list[ 9], TEXT("%COF\\%VAR\\d%DIR_f%FRM"));
         wcscpy(pDatas->filename_format_list[10], TEXT("")); // IMPORTANT ! DO NOT DELETE, DO NOT CHANGE, AND KEEP IT THE LAST ROW
      }

      search_best_fit_colors();
      pDatas->nb_ticks_before_update_anim_box = 25; // FIXME : should be a user adjustable option

      pDatas->datas_state = EDS_ALL_READY;
   }

   pCof = myglobals.cof_selection.current_cof;
   if (pCof == NULL)
   {
      pDatas->nb_directions           = 0;
      pDatas->nb_frames_per_direction = 0;

      wcscpy(pDatas->string_direction, TEXT(""));
      wcscpy(pDatas->string_frames,    TEXT(""));

      for (i=0; i < VL_MAX; i++)
         wcscpy(pDatas->var_list_data[i].value, TEXT(""));

      wcscpy(pDatas->string_direction, TEXT(""));
      wcscpy(pDatas->string_frames,    TEXT(""));

      memset( & pDatas->anim_box, 0, sizeof(EXPORT_BOX));
   }
   else
   {
      pDatas->nb_directions           = pCof->nb_directions;
      pDatas->nb_frames_per_direction = pCof->nb_frames_per_direction;

      max_d = pDatas->nb_directions;
      max_f = pDatas->nb_frames_per_direction;

      if (max_d <= 0)
         swprintf(pDatas->string_direction, MAX_PATH, TEXT(""));
      else if (max_d == 1)
         swprintf(pDatas->string_direction, MAX_PATH, TEXT("0"));
      else
         swprintf(pDatas->string_direction, MAX_PATH, TEXT("0-%d"), max_d - 1);

      if (max_f <= 0)
         swprintf(pDatas->string_frames, MAX_PATH, TEXT(""));
      else if (max_f == 1)
         swprintf(pDatas->string_frames, MAX_PATH, TEXT("0"));
      else
         swprintf(pDatas->string_frames, MAX_PATH, TEXT("0-%d"), max_f - 1);

      decode_direction_order_with_warning();
      decode_frame_order_with_warning();

      update_exported_frame_table();

      PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
   }

   return 0;
}


// ===========================================================================
// ask to decode the direction string and pop up errors if needed
// return 0 if success
// ===========================================================================
int decode_direction_order_with_warning(void)
{
   DLGBOX_EXPORT_DATAS * pDatas = & myglobals.dlgbox_export_datas;
   int                 ret      = 0;


   ret = decode_direction_order();
   if (ret != 0)
   {
      char buffer [300] = "";
      char * comment    = NULL;
      int  i            = 0;

      for (i = 0; i < 32; i++)
         pDatas->direction_order[i] = -1;

      switch(ret)
      {
         case -1  : comment = "Direction string is empty"; break;
         case -2  : comment = "There should have been a number right before a '-'"; break;
         case -3  : comment = "Range syntax error (example : 2-3-4 is not allowed)"; break;
         case -4  : comment = "A direction exceed the maximum number of directions in the animation"; break;
         case -5  : comment = "Incorrect range (example : 3-2 is not allowed)"; break;
         case -6  : comment = "A direction (from within a range) is present more than once"; break;
         case -7  : comment = "There can't be more than 32 directions to export"; break;
         case -8  : comment = "A direction is present more than once"; break;
         case -9  : comment = "There can't be more than 32 directions to export"; break;
         case -10 : comment = "Unknown character\n\nOnly allowed : '0' to '9', space ' ' and minus '-'"; break;
      }

      if (comment != NULL)
         sprintf(buffer, "%s\n\nerror code : %d", comment, ret);
      else
         sprintf(buffer, "unknown error\n\nerror code : %d", ret);

      MessageBoxA(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, buffer, "Error", MB_ICONERROR | MB_OK);
   }

   return ret;
}


// ===========================================================================
// decode the direction string and find the order to export
// return 0 if success
// ===========================================================================
int decode_direction_order(void)
{
   DLGBOX_EXPORT_DATAS * e            = & myglobals.dlgbox_export_datas;
   int                 max            = wcslen(e->string_direction);
   WCHAR               c              = 0;
   int                 n              = 0;
   int                 value          = 0;
   int                 value_start    = 0;
   int                 value_end      = 0;
   int                 idx            = 0;
   int                 idx2           = 0;
   int                 decoding_value = FALSE;
   int                 decoding_range = FALSE;
   WCHAR               * string       = NULL;


   // initialize the direction order table
   for (n = 0; n < 32; n++)
      e->direction_order[n] = -1;

   // search the start of the string
   for (n = 0; n < max; n++)
   {
      if (e->string_direction[n] != ' ')
         break;
   }
   string = e->string_direction + n;

   // search the end of the string
   max = wcslen(string);
   for (n = max - 1; n >= 0; n--)
   {
      if (string[n] != ' ')
         break;
      else
         string[n] = 0;
   }

   max = wcslen(string);
   if (max == 0)
      return -1; // empty string

   // add 1 space to the end of string (needed just because of the string decoding logic)
   wcscat(string, TEXT(" "));
   max++;

   // decode the string
   for (n = 0; n < max; n++)
   {
      c = string[n];

      switch (c)
      {
         case TEXT('0') : value *= 10;             decoding_value = TRUE; break;
         case TEXT('1') : value *= 10; value += 1; decoding_value = TRUE; break;
         case TEXT('2') : value *= 10; value += 2; decoding_value = TRUE; break;
         case TEXT('3') : value *= 10; value += 3; decoding_value = TRUE; break;
         case TEXT('4') : value *= 10; value += 4; decoding_value = TRUE; break;
         case TEXT('5') : value *= 10; value += 5; decoding_value = TRUE; break;
         case TEXT('6') : value *= 10; value += 6; decoding_value = TRUE; break;
         case TEXT('7') : value *= 10; value += 7; decoding_value = TRUE; break;
         case TEXT('8') : value *= 10; value += 8; decoding_value = TRUE; break;
         case TEXT('9') : value *= 10; value += 9; decoding_value = TRUE; break;

         case TEXT('-') :
            if (decoding_value == FALSE)
               return -2; // there should have been a number right before the '-'

            decoding_value = FALSE;

            if (decoding_range == TRUE)
               return -3; // there was already a '-'

            value_start = value;
            value = 0;
            decoding_range = TRUE;
            break;

         case TEXT(' ') :
            if (decoding_value == TRUE)
            {
               if (value >= e->nb_directions)
                  return -4; // direction don't exists
            }

            if (decoding_range == TRUE)
            {
               decoding_range = FALSE;
               value_end = value;
               if (value_end <= value_start)
                  return -5; // incorrect range

               for (value = value_start; value <= value_end; value++)
               {
                  for (idx2 = 0; idx2 < idx; idx2++)
                  {
                     if (e->direction_order[idx2] == value)
                        return -6; // direction already present
                  }

                  if (idx >= 32)
                     return -7; // too much directions

                  e->direction_order[idx] = value; // add this direction
                  idx++;
               }
            }
            else if (decoding_value == TRUE)
            {
               for (idx2 = 0; idx2 < idx; idx2++)
               {
                  if (e->direction_order[idx2] == value)
                     return -8; // direction already present
               }

               if (idx >= 32)
                  return -9; // too much directions

               e->direction_order[idx] = value; // add this direction
               idx++;
            }

            decoding_value = FALSE;
            value = 0;
            break;

         default :
            return -10; // unknown character
            break;
      }
   }

   // remove the added space to the end of string
   string[max - 1] = 0;

   return 0;
}


// ===========================================================================
// ask to decode the frame string and pop up errors if needed
// return 0 if success
// ===========================================================================
int decode_frame_order_with_warning(void)
{
   DLGBOX_EXPORT_DATAS * pDatas = & myglobals.dlgbox_export_datas;
   int                 ret      = 0;


   ret = decode_frame_order();
   if (ret != 0)
   {
      char buffer [300] = "";
      char * comment    = NULL;
      int  i            = 0;

      for (i = 0; i < 256; i++)
         pDatas->frame_order[i] = -1;

      switch(ret)
      {
         case -1  : comment = "Frame string is empty"; break;
         case -2  : comment = "There should have been a number right before a '-'"; break;
         case -3  : comment = "Range syntax error (example : 2-3-4 is not allowed)"; break;
         case -4  : comment = "A frame exceed the maximum number of frames per direction in the animation"; break;
         case -5  : comment = "Incorrect range (example : 3-2 is not allowed)"; break;
         case -6  : comment = "A frame (from within a range) is present more than once"; break;
         case -7  : comment = "There can't be more than 256 frames to export"; break;
         case -8  : comment = "A frame is present more than once"; break;
         case -9  : comment = "There can't be more than 256 frames to export"; break;
         case -10 : comment = "Unknown character\n\nOnly allowed : '0' to '9', space ' ' and minus '-'"; break;
      }

      if (comment != NULL)
         sprintf(buffer, "%s\n\nerror code : %d", comment, ret);
      else
         sprintf(buffer, "unknown error\n\nerror code : %d", ret);

      MessageBoxA(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, buffer, "Error", MB_ICONERROR | MB_OK);
   }

   return ret;
}


// ===========================================================================
// decode the frame string and find the order to export
// return 0 if success
// ===========================================================================
int decode_frame_order(void)
{
   DLGBOX_EXPORT_DATAS * e            = & myglobals.dlgbox_export_datas;
   int                 max            = wcslen(e->string_frames);
   WCHAR               c              = 0;
   int                 n              = 0;
   int                 value          = 0;
   int                 value_start    = 0;
   int                 value_end      = 0;
   int                 idx            = 0;
   int                 idx2           = 0;
   int                 decoding_value = FALSE;
   int                 decoding_range = FALSE;
   WCHAR               * string       = NULL;


   // initialize the frame order table
   for (n = 0; n < 256; n++)
      e->frame_order[n] = -1;

   // search the start of the string
   for (n = 0; n < max; n++)
   {
      if (e->string_frames[n] != ' ')
         break;
   }
   string = e->string_frames + n;

   // search the end of the string
   max = wcslen(string);
   for (n = max - 1; n >= 0; n--)
   {
      if (string[n] != ' ')
         break;
      else
         string[n] = 0;
   }

   max = wcslen(string);
   if (max == 0)
      return -1; // empty string

   // add 1 space to the end of string (needed just because of the string decoding logic)
   wcscat(string, TEXT(" "));
   max++;

   // decode the string
   for (n = 0; n < max; n++)
   {
      c = string[n];

      switch (c)
      {
         case TEXT('0') : value *= 10;             decoding_value = TRUE; break;
         case TEXT('1') : value *= 10; value += 1; decoding_value = TRUE; break;
         case TEXT('2') : value *= 10; value += 2; decoding_value = TRUE; break;
         case TEXT('3') : value *= 10; value += 3; decoding_value = TRUE; break;
         case TEXT('4') : value *= 10; value += 4; decoding_value = TRUE; break;
         case TEXT('5') : value *= 10; value += 5; decoding_value = TRUE; break;
         case TEXT('6') : value *= 10; value += 6; decoding_value = TRUE; break;
         case TEXT('7') : value *= 10; value += 7; decoding_value = TRUE; break;
         case TEXT('8') : value *= 10; value += 8; decoding_value = TRUE; break;
         case TEXT('9') : value *= 10; value += 9; decoding_value = TRUE; break;

         case TEXT('-') :
            if (decoding_value == FALSE)
               return -2; // there should have been a number right before the '-'

            decoding_value = FALSE;

            if (decoding_range == TRUE)
               return -3; // there was already a '-'

            value_start = value;
            value = 0;
            decoding_range = TRUE;
            break;

         case TEXT(' ') :
            if (decoding_value == TRUE)
            {
               if (value >= e->nb_frames_per_direction)
                  return -4; // frame don't exists
            }

            if (decoding_range == TRUE)
            {
               decoding_range = FALSE;
               value_end = value;
               if (value_end <= value_start)
                  return -5; // incorrect range

               for (value = value_start; value <= value_end; value++)
               {
                  for (idx2 = 0; idx2 < idx; idx2++)
                  {
                     if (e->frame_order[idx2] == value)
                        return -6; // frame already present
                  }

                  if (idx >= 256)
                     return -7; // too much frames

                  e->frame_order[idx] = value; // add this frame
                  idx++;
               }
            }
            else if (decoding_value == TRUE)
            {
               for (idx2 = 0; idx2 < idx; idx2++)
               {
                  if (e->frame_order[idx2] == value)
                     return -8; // frame already present
               }

               if (idx >= 256)
                  return -9; // too much frames

               e->frame_order[idx] = value; // add this frame
               idx++;
            }

            decoding_value = FALSE;
            value = 0;
            break;

         default :
            return -10; // unknown character
            break;
      }
   }

   // remove the added space to the end of string
   string[max - 1] = 0;

   return 0;
}


// ===========================================================================
// update the box controls with the current value
// ===========================================================================
void update_box_controls(int update_user_box, int update_animation_box)
{
   DLGBOX_EXPORT_DATAS   * e               = & myglobals.dlgbox_export_datas;
   WCHAR                 sample [MAX_PATH] = TEXT("");
   int                   child_idx         = -1;
   CREATE_DLGBOX         * d               = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;


   if (update_user_box == TRUE)
   {
      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.left);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_LEFT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.top);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_TOP_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.right);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_RIGHT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.bottom);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.width);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_WIDTH_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->user_box.height);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);
   }

   if (update_animation_box == TRUE)
   {
      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.left);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_LEFT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.top);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_TOP_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.right);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_RIGHT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.bottom);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_BOTTOM_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.width);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_WIDTH_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);

      swprintf(sample, MAX_PATH, TEXT("%d"), e->anim_box.height);
      if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_ANIM_HEIGHT_EDIT, & child_idx) == 0)
         SetWindowText(d->pChild[child_idx].handle, sample);
   }
}


// ===========================================================================
// scan 'filename', extracts all directories one by one and create them along the way as necessary
// ===========================================================================
void create_all_directories_within_path(WCHAR * filename)
{
   int   i                        = 0;
   int   length                   = 0;
   WCHAR path    [MAX_PATH]       = TEXT("");
   WCHAR message [MAX_PATH + 300] = TEXT("");


   if (filename == NULL)
      return;

   length = wcslen(filename);
   for (i = 0; i < length; i++)
   {
      if (filename[i] == TEXT('\\'))
      {
         if (filename[i+1] == TEXT('\\'))
            continue;

         if (wcsncpy_s(path, sizeof(path) / 2, filename, i) == 0)
         {
            if (_waccess(path, 0) == -1)
            {
               if (errno == ENOENT)
               {
                  if (CreateDirectoryW(path, NULL) == 0)
                  {
                     swprintf(message, sizeof(message) / 2, TEXT("Error while trying to create a directory.\n\nPath = %s"), path);
                     MessageBoxW(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
                  }
               }
            }
         }
      }
   }
}


// ===========================================================================
// function for qsort(), to sort tab_ptr[]
// ===========================================================================
int qsort_helper_compare_sample_path(const void * a, const void * b)
{
   WCHAR ** pa = (WCHAR **) a;
   WCHAR ** pb = (WCHAR **) b;

   return wcscmp(*pa, *pb);
}


// ===========================================================================
// create the files to export
// return 0 if success
// ===========================================================================
int save_exported_animation_images(void)
{
   typedef WCHAR SAMPLE_PATH [MAX_PATH];

   DLGBOX_EXPORT_DATAS  * e                   = & myglobals.dlgbox_export_datas;
   WCHAR                sample [MAX_PATH]     = TEXT("");
   SAMPLE_PATH          * tab_sample_path     = NULL;
   WCHAR                * extension           = TEXT("");
   FILE                 * out                 = NULL;
   FILE                 * out_pivot           = NULL;
   void                 * bmp                 = NULL;
   char                 message [300]         = "";
   int                  good_frames           = 0;
   int                  bad_frames            = 0;
   void                 * bmp_pivot           = NULL;
   int                  nb_overwrite          = 0;
   WCHAR                first_name [MAX_PATH] = TEXT("");
   int                  ret                   = 0;
   WCHAR                w_msg [300+MAX_PATH]  = TEXT("");
   EXPORT_BOX           * ptr_box_to_use      = NULL;
   int                  i                     = 0;
   int                  nb_files              = 0;
   WCHAR                ** tab_ptr            = NULL;
   EXPORTED_FRAME       * ef                  = NULL;
   EXPORT_PARAMETERS    ex;


   if (e->nb_exported_frame <= 0)
      return 0;

   if (e->update_anim_box_at_tick != 0)
   {
      e->update_anim_box_at_tick = 0;

      // inform the application that the export box need to update its values, right now
      SendMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
   }

   memset( & ex, 0, sizeof(ex));

   switch (e->enum_usebox)
   {
      case EXPORT_USEBOX_USER : ptr_box_to_use = & e->user_box; break;
      case EXPORT_USEBOX_ANIM : ptr_box_to_use = & e->anim_box; break;
   }
   MYASSERT_RETURN(ptr_box_to_use != NULL, -1, NULL);

   if ((ptr_box_to_use->width <= 0) || (ptr_box_to_use->height <= 0))
   {
      sprintf(message, "incorrect box dimensions (%d ; %d)", ptr_box_to_use->width, ptr_box_to_use->height);
      MessageBoxA(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, message, "Error during Export", MB_ICONERROR | MB_OK);
      return -1;
   }

   // get the file extension to use
   switch(e->enum_format)
   {
      case EXPORT_BMP : extension = TEXT(".bmp"); break;
      case EXPORT_PCX : extension = TEXT(".pcx"); break;
      case EXPORT_PNG : extension = TEXT(".png"); break;
      case EXPORT_TGA : extension = TEXT(".tga"); break;
      case EXPORT_BAM : extension = TEXT(".bam"); break;
   }

   // table of sample paths

   tab_sample_path = (SAMPLE_PATH *) calloc(e->nb_exported_frame, sizeof(SAMPLE_PATH));
   MYASSERT_RETURN(tab_sample_path != NULL, -1, "tab_sample_path = (SAMPLE_PATH *) calloc(e->nb_exported_frame, sizeof(SAMPLE_PATH))");

   // check if we're trying to overwrite existing files
   for (i = 0; i < e->nb_exported_frame; i++)
   {
      ef = & e->exported_frame[i];
      if (update_export_filesample(sample, ef) == 0)
      {
         swprintf(tab_sample_path[i], MAX_PATH, TEXT("%s%s%s"), e->directory, sample, extension);
         if (_waccess(tab_sample_path[i], 0) == 0)
         {
            // file already exists
            nb_overwrite++;
            if (nb_overwrite == 1)
               wcscpy(first_name, tab_sample_path[i]);
         }
      }
   }

   // if files already exists, ask what to do
   if (nb_overwrite != 0)
   {
      if (nb_overwrite == 1)
         wsprintf(w_msg, TEXT("Warning ! You're about to overwrite this file :\n%s\n\nDo you really want to replace it ?"), first_name);
      else
         wsprintf(w_msg, TEXT("Warning ! You're about to overwrite %d files.\n\nFirst existing file : %s\n\nDo you really want to replace them ?"), nb_overwrite, first_name);

      ret = MessageBox(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, w_msg, TEXT("Export"), MB_ICONSTOP | MB_OKCANCEL | MB_DEFBUTTON2 | MB_APPLMODAL);
      if (ret != IDOK)
      {
         MessageBox(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, TEXT("Export aborted"), TEXT("Export"), MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1 | MB_APPLMODAL);
         DESTROYME(tab_sample_path, free);
         return 0;
      }
   }

   // check how many different files will be created

   tab_ptr = (WCHAR **) calloc(e->nb_exported_frame, sizeof(WCHAR *));
   if (tab_ptr == NULL)
   {
      DESTROYME(tab_sample_path, free);
      MYASSERT_RETURN(tab_ptr != NULL, -1, "tab_ptr = (WCHAR **) calloc(e->nb_exported_frame, sizeof(WCHAR *))");
   }

   for (i = 0; i < e->nb_exported_frame; i++)
      tab_ptr[i] = (WCHAR *) tab_sample_path[i];

   qsort(tab_ptr, e->nb_exported_frame, sizeof(WCHAR *), qsort_helper_compare_sample_path);

   nb_files = 0;
   if (wcslen((WCHAR *) tab_ptr[0]) > 0)
      nb_files = 1;

   for (i = 1; i < e->nb_exported_frame; i++)
   {
      if (wcslen((WCHAR *) tab_ptr[i]) == 0)
         continue;

      if (wcscmp(tab_ptr[i - 1], tab_ptr[i]) != 0)
         nb_files++;
   }

   if (nb_files != e->nb_exported_frame)
   {
      wsprintf(
         w_msg,
         TEXT("Warning ! Given the current filename format, you're about to create only %d file%s, but there %s %d image%s to export.\n\n%d image%s will be lost.\n\nDo you really want to continue ?"),
         nb_files,
         nb_files > 1 ? TEXT("s") : TEXT(""),
         e->nb_exported_frame > 1 ? TEXT("are") : TEXT("is"),
         e->nb_exported_frame,
         e->nb_exported_frame > 1 ? TEXT("s") : TEXT(""),
         e->nb_exported_frame - nb_files,
         e->nb_exported_frame - nb_files > 1 ? TEXT("s") : TEXT("")
      );

      ret = MessageBox(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, w_msg, TEXT("Export"), MB_ICONSTOP | MB_OKCANCEL | MB_DEFBUTTON2 | MB_APPLMODAL);
      if (ret != IDOK)
      {
         MessageBox(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, TEXT("Export aborted"), TEXT("Export"), MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1 | MB_APPLMODAL);
         DESTROYME(tab_sample_path, free)
         DESTROYME(tab_ptr,         free)
         return 0;
      }
   }

   DESTROYME(tab_ptr, free);

   // create the image that'll be used for all frames, based on the box dimensions
   bmp = get_allegro_bitmap(ptr_box_to_use->width, ptr_box_to_use->height);
   if (bmp == NULL)
   {
      DESTROYME(tab_sample_path, free)
      return -1;
   }

   // create the image that'll show the pivot
   bmp_pivot = get_allegro_bitmap(ptr_box_to_use->width, ptr_box_to_use->height);
   if (bmp_pivot == NULL)
   {
      DESTROYME(tab_sample_path, free)
      DESTROYME(bmp,             destroy_allegro_bitmap)
      return -1;
   }

   // create all images
   for (i = 0; i < e->nb_exported_frame; i++)
   {
      ef = & e->exported_frame[i];
      bad_frames++;
      if (wcslen((WCHAR *) tab_sample_path[i]) > 0)
      {
         create_all_directories_within_path(tab_sample_path[i]);

         out = _wfopen(tab_sample_path[i], TEXT("wb"));
         if (out != NULL)
         {
            ex.file                  = out;
            ex.format                = e->enum_format;
            ex._image                = bmp;
            ex._anim                 = myglobals.animation;
            ex.COF_dir               = ef->direction_ID;
            ex.f                     = ef->frame_ID;
            ex.box                   = ptr_box_to_use;
            ex.bg                    = & e->palette_background;
            ex.sh                    = & e->palette_shadow;
            ex.temp_directory        = myglobals.temp_directory;
            ex.bg2                   = e->bg2;
            ex.sh2                   = e->sh2;
            ex.mirror_sp             = (e->mirror_sprite == TRUE) ? 1 : 0;
            ex._image_pivot          = bmp_pivot;
            ex.is_first_frame        = (i == 0) ? 1 : 0;
            ex.is_last_frame         = (i >= (e->nb_exported_frame - 1)) ? 1 : 0;
            ex.color_white           = e->white;
            ex.pivot_image_x         = - ptr_box_to_use->left;
            ex.pivot_image_y         = - ptr_box_to_use->top;
            ex.file_pivot            = NULL;
            ex.shadow_height_percent = e->shadow_height_percent;
            ex.shadow_skew_percent   = e->shadow_skew_percent;
            ex.shadow_offset_x       = e->shadow_offset_x;
            ex.shadow_offset_y       = e->shadow_offset_y;
            ex.shadow_cmap           = NULL;

            if (ex.is_last_frame == 1)
            {
               WCHAR tmp_path [MAX_PATH] = TEXT("");

               if (update_export_filesample(sample, ef) == 0)
               {
                  swprintf(tmp_path, MAX_PATH, TEXT("%s%s.pivot%s"), e->directory, sample, extension);
                  out_pivot     = _wfopen(tmp_path, TEXT("wb"));
                  ex.file_pivot = out_pivot;
               }
            }

            if (merge_layers_of_this_frame( & ex) == 0)
            {
               bad_frames--;
               good_frames++;
            }

            fclose(out);
            out = NULL;

            if (out_pivot != NULL)
            {
               fclose(out_pivot);
               out_pivot = NULL;
            }
         }
      }
   }

   if (bad_frames != 0)
   {
      sprintf(message, "%d frame%s couldn't be saved for some reason", bad_frames, (bad_frames > 1) ? "s" : "");
      MessageBoxA(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, message, "Error during Export", MB_ICONERROR | MB_OK);
   }
   else
   {
      sprintf(message, "%d frame%s successfully saved", good_frames, (good_frames > 1) ? "s were" : " was");
      MessageBoxA(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, message, "Export done", MB_ICONINFORMATION | MB_OK);
   }

   if (bmp != NULL)
      destroy_allegro_bitmap(bmp);

   if (bmp_pivot != NULL)
      destroy_allegro_bitmap(bmp_pivot);

   DESTROYME(tab_sample_path, free)

   return 0;
}


// ===========================================================================
// remove the selection of an edit control
// ===========================================================================
void remove_edit_control_selection(ENUM_CTRL_IDENTIFIER id)
{
   int           child_idx = -1;
   HWND          h         = NULL;
   CREATE_DLGBOX * dlg     = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;


   if (get_dialog_child_index_from_ID(DLG_EXPORT, id, & child_idx) == 0)
   {
      h = dlg->pChild[child_idx].handle;
      if (h != NULL)
         SendMessage(h, EM_SETSEL, (WPARAM) -1, (LPARAM) -1); // FIXME it does not seems to work well
   }                                                                             
}


// ===========================================================================
// enable or disable access to the user box, depending on the box to use
// ===========================================================================
void update_user_box_controls_state(void)
{
   DLGBOX_EXPORT_DATAS * e      = & myglobals.dlgbox_export_datas;
   int                 activate = FALSE;


   if (e->enum_usebox == EXPORT_USEBOX_USER)
      activate = TRUE;

   if (activate == FALSE)
   {
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_LEFT_EDIT);
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_TOP_EDIT);
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_RIGHT_EDIT);
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT);
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_WIDTH_EDIT);
      remove_edit_control_selection(ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT);
   }

   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_BOX_GROUP,   activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_LEFT_EDIT,   activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_TOP_EDIT,    activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_RIGHT_EDIT,  activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_BOTTOM_EDIT, activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_WIDTH_EDIT,  activate);
   modify_enable_state_by_child_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_USER_HEIGHT_EDIT, activate);
}


// ===========================================================================
// update the list of frames the user wants to export
// ===========================================================================
void update_exported_frame_table(void)
{
   DLGBOX_EXPORT_DATAS * e    = & myglobals.dlgbox_export_datas;
   EXPORTED_FRAME      * ef   = NULL;
   int                 d      = 0;
   int                 f      = 0;
   int                 n      = 0;
   int                 nb_frm = 0;


   memset (e->exported_frame, 0, sizeof(e->exported_frame));
   e->nb_exported_frame = 0;

   if (myglobals.cof_selection.current_cof == NULL)
   {
      reset_var_list_values();
      return;
   }

   nb_frm = myglobals.cof_selection.current_cof->nb_frames_per_direction;

   for (d = 0; (d < 32) && (e->direction_order[d] != -1); d++)
   {
      for (f = 0; (f < 256) && (e->frame_order[f] != -1); f++)
      {
         ef = & e->exported_frame[n];

         // unique identifiers in COF
         ef->direction_ID = e->direction_order[d];
         ef->frame_ID     = e->frame_order[f];
         ef->image_ID     = (ef->direction_ID * nb_frm) + ef->frame_ID;

         // user order
         ef->direction = d;
         ef->frame     = f;
         ef->image     = n;

         n++;
      }
   }

   e->nb_exported_frame = n;

   reset_var_list_values();
}


// ===========================================================================
// draw the preview image on screen
// ===========================================================================
void draw_preview_image(void)
{
   CREATE_DLGBOX       * d       = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   DLGBOX_EXPORT_DATAS * e       = & myglobals.dlgbox_export_datas;
   HDC                 dc        = NULL;
   HWND                h         = NULL;
   int                 child_idx = -1;
   int                 width     = 0;
   int                 height    = 0;

   
   if (e->bmp_preview == NULL)
      return;

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_PREVIEW_IMG, & child_idx) != 0)
      return;

   h = d->pChild[child_idx].handle;
   if (h == NULL)
      return;

   get_window_position_and_dimension(h, NULL, NULL, & width, & height);
   dc = GetDC(h);
   allegro_blit_to_hdc(e->bmp_preview, dc, 0, 0, 0, 0, width, height);
   ReleaseDC(h, dc);
}


// ===========================================================================
// search background and shadow palette index that best represent their desired color components
// then search their alternatives (2nd best representing the same colors)
// and then search the best color representative of white
// Note : the order of search is important, as once a color is determined, the others can't use the same
// ===========================================================================
void search_best_fit_colors()
{
   DLGBOX_EXPORT_DATAS  * e  = & myglobals.dlgbox_export_datas;
   EXPORT_PALETTE_ENTRY * bg = & e->palette_background;
   EXPORT_PALETTE_ENTRY * sh = & e->palette_shadow;
   int exceptions [256];


   memset(exceptions, 0, sizeof(exceptions));

   exceptions[0] = 1;

   bg->best_fit_index = animation_best_fit_color_with_exceptions(bg->red, bg->green, bg->blue, exceptions);
   exceptions[bg->best_fit_index] = 1;

   sh->best_fit_index = animation_best_fit_color_with_exceptions(sh->red, sh->green, sh->blue, exceptions);
   exceptions[sh->best_fit_index] = 1;

   e->bg2 = animation_best_fit_color_with_exceptions(bg->red, bg->green, bg->blue, exceptions);
   exceptions[e->bg2] = 1;

   e->sh2 = animation_best_fit_color_with_exceptions(sh->red, sh->green, sh->blue, exceptions);
   exceptions[e->sh2] = 1;

   e->white = animation_best_fit_color_with_exceptions(255, 255, 255, exceptions);
}


// ===========================================================================
// update the memory preview bitmap, unless this is the same tick as the precedent call
// return 0 on success
// ===========================================================================
int init_update_preview_bitmap(EXPORT_PARAMETERS * ex)
{
   DLGBOX_EXPORT_DATAS  * e       = & myglobals.dlgbox_export_datas;
   int                  n         = 0;
   static unsigned long last_tick = (unsigned long) -1;
   unsigned long        tick      = 0;

   
   tick = get_current_tick_25fps_preview();
   if (tick == last_tick)
      return 1;

   if (e->update_anim_box_at_tick != 0)
   {
      if (tick >= e->update_anim_box_at_tick)
      {
         e->update_anim_box_at_tick = 0;

         // inform the application that the export box need to update its values, right now
         SendMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_APP_EXPORT_UPDATE_BOX, 0, 0);
      }
   }

   last_tick = tick;

   if (myglobals.dlgbox_export_datas.bmp_preview == NULL)
      return 2;

   if ((ex == NULL) || (myglobals.animation == NULL) || (e->nb_exported_frame <= 0))
   {
      allegro_clear_bitmap(myglobals.dlgbox_export_datas.bmp_preview);
      return 3;
   }

   memset(ex, 0, sizeof(EXPORT_PARAMETERS));

   ex->file   = NULL;
   ex->format = EXPORT_NULL;
   ex->_image = myglobals.dlgbox_export_datas.bmp_preview_box; // preview image, the dimensions of the box that the exportation will be
   ex->_anim  = myglobals.animation;

   // choose the image to draw now, within all the ones that would be exported with the current settings
   n = tick % e->nb_exported_frame;

   // get the direction and frame of that image
   ex->COF_dir = e->exported_frame[n].direction_ID;
   ex->f       = e->exported_frame[n].frame_ID;

   ex->box = NULL;
   switch (e->enum_usebox)
   {
      case EXPORT_USEBOX_USER : ex->box = & e->user_box; break;
      case EXPORT_USEBOX_ANIM : ex->box = & e->anim_box; break;
      default :
         allegro_clear_bitmap(myglobals.dlgbox_export_datas.bmp_preview);
         return 4;
   }

   if ((ex->box->width <= 0) || (ex->box->height <= 0))
   {
      allegro_clear_bitmap(myglobals.dlgbox_export_datas.bmp_preview);
      return 5;
   }

   ex->bg                    = & e->palette_background;
   ex->sh                    = & e->palette_shadow;
   ex->temp_directory        = myglobals.temp_directory;
   ex->bg2                   = e->bg2;
   ex->sh2                   = e->sh2;
   ex->mirror_sp             = (e->mirror_sprite == TRUE) ? 1 : 0;
   ex->_image_pivot          = NULL;
   ex->is_first_frame        = 0;
   ex->is_last_frame         = 0;
   ex->color_white           = e->white;
   ex->pivot_image_x         = 0;
   ex->pivot_image_y         = 0;
   ex->file_pivot            = NULL;
   ex->_image_control        = myglobals.dlgbox_export_datas.bmp_preview; // preview image, the dimensions of the application control. bmp_preview_box will be copy/paste into it
   ex->shadow_height_percent = e->shadow_height_percent;
   ex->shadow_skew_percent   = e->shadow_skew_percent;
   ex->shadow_offset_x       = e->shadow_offset_x;
   ex->shadow_offset_y       = e->shadow_offset_y;

   return 0;
}


// ===========================================================================
// create a new 'bmp_preview_box', with the dimensions of the current active box
// if any problem, 'bmp_preview_box' will be NULL
// ===========================================================================
void create_new_preview_bitmap_from_box(void)
{
   DLGBOX_EXPORT_DATAS  * e   = & myglobals.dlgbox_export_datas;
   EXPORT_BOX           * box = NULL;


   if (e->bmp_preview_box != NULL)
   {
      destroy_allegro_bitmap(e->bmp_preview_box);
      e->bmp_preview_box = NULL;
   }

   switch (e->enum_usebox)
   {
      case EXPORT_USEBOX_USER : box = & e->user_box; break;
      case EXPORT_USEBOX_ANIM : box = & e->anim_box; break;
      default :
         return;
   }

   if ((box->width <= 0) || (box->height <= 0))
      return;

   e->bmp_preview_box = get_allegro_bitmap(box->width, box->height);
}


// ===========================================================================
// delete all items of the filename variables list, and then fill it again with the current values
// ===========================================================================
void reset_var_list_values(void)
{
   DLGBOX_EXPORT_DATAS * e                = & myglobals.dlgbox_export_datas;
   COF_ROW_EXISTS      * pCof             = myglobals.cof_selection.current_cof;
   LVITEM              item;
   HWND                h                  = NULL;
   int                 i                  = 0;
   int                 child_idx          = -1;
   char                strchar [MAX_PATH] = "";
   WCHAR               wcchar  [MAX_PATH] = TEXT("");
   int                 iCOF               = -1;
   int                 n                  = 0;
   int                 nb                 = 0;


   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_VAR_LIST, & child_idx) != 0)
      return;

   h = myglobals.dlgbox_datas[DLG_EXPORT].dlg.pChild[child_idx].handle;
   if (h == NULL)
      return;

   // disable redraw for now
   SendMessage(h, WM_SETREDRAW, (WPARAM) FALSE, 0);

   // delete old values
   SendMessage(h, LVM_DELETEALLITEMS, 0, 0);

   // update the values

   sprintf(strchar, "%s%s%s", pCof->token, pCof->mode, pCof->weapon_class);
   i = VL_COF; char_to_wide_char(strchar, e->var_list_data[i].value, sizeof(e->var_list_data[i].value));

   i = VL_TOK; char_to_wide_char(pCof->token, e->var_list_data[i].value, sizeof(e->var_list_data[i].value));

   i = VL_MOD; char_to_wide_char(pCof->mode, e->var_list_data[i].value, sizeof(e->var_list_data[i].value));

   i = VL_WPC; char_to_wide_char(pCof->weapon_class, e->var_list_data[i].value, sizeof(e->var_list_data[i].value));

   i = VL_VHD; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VTR; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VLG; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VRA; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VLA; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VRH; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VLH; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VSH; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS1; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS2; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS3; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS4; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS5; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS6; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS7; wcscpy(e->var_list_data[i].value, TEXT(""));
   i = VL_VS8; wcscpy(e->var_list_data[i].value, TEXT(""));

   i = VL_VAR;
   wcscpy(e->var_list_data[i].value, TEXT(""));
   for (n = 0; n < pCof->nb_layers; n++)
   {
      iCOF = myglobals.application_datas.control_to_COF_layer_map[n];

      if (strlen(pCof->layer_datas[iCOF].variant) == 0)
         continue;

      if (nb != 0)
         wcscat(e->var_list_data[i].value, TEXT("_"));

      wcscat(e->var_list_data[i].value, pCof->layer_datas[iCOF].code_wchar);
      char_to_wide_char(pCof->layer_datas[iCOF].variant, wcchar, sizeof(wcchar));
      wcscat(e->var_list_data[i].value, wcchar);

      nb++;

      if     (strcmp(pCof->layer_datas[iCOF].code_char, "HD") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VHD].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "TR") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VTR].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "LG") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VLG].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "RA") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VRA].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "LA") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VLA].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "RH") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VRH].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "LH") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VLH].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "SH") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VSH].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S1") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS1].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S2") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS2].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S3") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS3].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S4") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS4].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S5") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS5].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S6") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS6].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S7") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS7].value, 10);
      else if(strcmp(pCof->layer_datas[iCOF].code_char, "S8") == 0) char_to_wide_char(pCof->layer_datas[iCOF].variant, e->var_list_data[VL_VS8].value, 10);
   }

   // directions, frames, and images ranges
   if (e->nb_exported_frame > 0)
   {
      n = sizeof(e->var_list_data[VL_DIR].value);
      i = e->nb_exported_frame - 1;

      swprintf(e->var_list_data[VL_DIR].value, n, TEXT("%d ... %d"), e->exported_frame[0].direction,    e->exported_frame[i].direction);
      swprintf(e->var_list_data[VL_DID].value, n, TEXT("%d ... %d"), e->exported_frame[0].direction_ID, e->exported_frame[i].direction_ID);

      swprintf(e->var_list_data[VL_FRM].value, n, TEXT("%d ... %d"), e->exported_frame[0].frame,        e->exported_frame[i].frame);
      swprintf(e->var_list_data[VL_FID].value, n, TEXT("%d ... %d"), e->exported_frame[0].frame_ID,     e->exported_frame[i].frame_ID);

      swprintf(e->var_list_data[VL_IMG].value, n, TEXT("%d ... %d"), e->exported_frame[0].image,        e->exported_frame[i].image);
      swprintf(e->var_list_data[VL_IID].value, n, TEXT("%d ... %d"), e->exported_frame[0].image_ID,     e->exported_frame[i].image_ID);
   }

   // prepare insertions
   SendMessage(h, LVM_SETITEMCOUNT, VL_MAX, 0);

   // insert new values
   memset( & item, 0, sizeof(item));
   item.mask = LVIF_TEXT;
   for (i = 0; i < VL_MAX; i++)
   {
      item.iItem    = i;
      item.iSubItem = 0;
      item.pszText  = e->var_list_data[i].code;
      SendMessage(h, LVM_INSERTITEM, 0, (LPARAM) & item); // item

      item.iItem    = i;
      item.iSubItem = 1;
      item.pszText  = e->var_list_data[i].value;
      SendMessage(h, LVM_SETITEM, 0, (LPARAM) & item); // sub item

      item.iItem    = i;
      item.iSubItem = 2;
      item.pszText  = e->var_list_data[i].description;
      SendMessage(h, LVM_SETITEM, 0, (LPARAM) & item); // sub item
   }

   // re-enable redraw
   SendMessage(h, WM_SETREDRAW, (WPARAM) TRUE, 0);
}


// ===========================================================================
// ID_DLGBOX_EXPORT_xxxx_BUTN : WM_COMMAND : BN_CLICKED
// a shadow offset arrow button have been puched
// ===========================================================================
LRESULT CALLBACK callback_EXPORT_OFFSXY_ARROWS_BUTN_for_BN_CLICKED(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DLGBOX_EXPORT_DATAS * e               = & myglobals.dlgbox_export_datas;
   CREATE_DLGBOX       * dlg             = & myglobals.dlgbox_datas[DLG_EXPORT].dlg;
   WORD                identifier        = LOWORD(wParam);
   HWND                hx                = NULL;
   HWND                hy                = NULL;
   int                 child_idx         = -1;
   WCHAR               string [MAX_PATH] = TEXT("");


   // just to avoid warnings
   hwnd   = hwnd;
   msg    = msg;
   lParam = lParam;

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_OFX_EDIT, & child_idx) == 0)
      hx = dlg->pChild[child_idx].handle;

   if (get_dialog_child_index_from_ID(DLG_EXPORT, ID_DLGBOX_EXPORT_SH_OFY_EDIT, & child_idx) == 0)
      hy = dlg->pChild[child_idx].handle;

   switch (identifier)
   {
      case ID_DLGBOX_EXPORT_UP_BUTN    : e->shadow_offset_y--; break;
      case ID_DLGBOX_EXPORT_LEFT_BUTN  : e->shadow_offset_x--; break;
      case ID_DLGBOX_EXPORT_RIGHT_BUTN : e->shadow_offset_x++; break;
      case ID_DLGBOX_EXPORT_DOWN_BUTN  : e->shadow_offset_y++; break;
   }

   if (e->shadow_offset_x < -2000) e->shadow_offset_x = -2000;
   if (e->shadow_offset_x >  2000) e->shadow_offset_x =  2000;
   if (e->shadow_offset_y < -2000) e->shadow_offset_y = -2000;
   if (e->shadow_offset_y >  2000) e->shadow_offset_y =  2000;

   if (hx != NULL)
   {
      swprintf(string, MAX_PATH, TEXT("%ld"), e->shadow_offset_x);
      SetWindowText(hx, string);
   }

   if (hy != NULL)
   {
      swprintf(string, MAX_PATH, TEXT("%ld"), e->shadow_offset_y);
      SetWindowText(hy, string);
   }

   // inform the application that the export box need to update its values, but in few frames, not right now
   e->update_anim_box_at_tick = get_current_tick_25fps_preview() + e->nb_ticks_before_update_anim_box;

   return 0;
}
