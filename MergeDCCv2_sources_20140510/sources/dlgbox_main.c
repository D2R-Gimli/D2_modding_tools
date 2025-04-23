#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "dlgbox_main.h"
#include "misc.h"
#include "strings_define.h"
#include "tools.h"
#include "mpq_handler.h"
#include "d2_files.h"
#include "loaders.h"
#include "cof_selection.h"
#include "dlgbox_setup.h"
#include "dlgbox_export.h"
#include "dccjob.h"


#define  DLGBOX_MAIN_NB_CHILDREN      (20 + (17 * DLGBOX_MAIN_CTRL_PER_ROW))
#define  DLGBOX_MAIN_ACCELERATORS_NB  1


static LRESULT CALLBACK     callback_dlgbox_main                (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HMENU                create_main_menu                    (void);
static ENUM_CTRL_IDENTIFIER get_layer_control_id                (ENUM_CTRL_IDENTIFIER base, int index);
static int                  init_dropdown_list                  (ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam);
static void                 modify_special_effect_level         (HWND h, ENUM_CTRL_IDENTIFIER id, int display, int selected_row);
static void                 new_colormap_type                   (LAYER_DATAS * pLayer, ENUM_CMAP_TYPE new_cmap_type);
static int                  reload_all_listfiles                (void);
static void                 update_colormapfile_list            (int row);
static void                 update_colormapindex_list           (int row);
static void                 update_colormaptype_list            (int row);
static void                 set_new_colormap_to_animation_layer (LAYER_DATAS * pLayer);


struct LAYER_EFFECT_ROW_DATAS_S
{
   enum ENUM_LAYER_EFFECT_TYPE_N enum_type;
   int                           used_by_COF_files; // TRUE / FALSE
   WCHAR *                       label_and_format;
   int                           display_levels;    // TRUE / FALSE
} layer_effect_row_datas[ LAYEREFFECTROW_MAX ] =
{
   {LAYEREFFECT_NONE,             FALSE, TEXT("(none)%s"),                  FALSE}, // LAYEREFFECTROW_NONE
   {LAYEREFFECT_25_TRANS,         TRUE,  TEXT("25%% translucency%s"),       FALSE}, // LAYEREFFECTROW_25_TRANS
   {LAYEREFFECT_50_TRANS,         TRUE,  TEXT("50%% translucency%s"),       FALSE}, // LAYEREFFECTROW_50_TRANS
   {LAYEREFFECT_75_TRANS,         TRUE,  TEXT("75%% translucency%s"),       FALSE}, // LAYEREFFECTROW_75_TRANS
   {LAYEREFFECT_SCREEN,           TRUE,  TEXT("screen%s"),                  FALSE}, // LAYEREFFECTROW_SCREEN
   {LAYEREFFECT_LUMINANCE,        TRUE,  TEXT("luminance%s"),               FALSE}, // LAYEREFFECTROW_LUMINANCE
   {LAYEREFFECT_SPIDER_WEB_TRANS, TRUE,  TEXT("spider web translucency%s"), FALSE}, // LAYEREFFECTROW_SPIDER_WEB_TRANS
   {LAYEREFFECT_DEL_DARK_PIXELS,  FALSE, TEXT("[delete dark pixels]"),      TRUE}   // LAYEREFFECTROW_DEL_DARK_PIXELS
};


// ===========================================================================
// initialize a dropdownlist combobox (mainly : special effect level)
// dwParam = number of elements of 4 characters each
// ===========================================================================
int init_dropdown_list(ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam)
{
   HWND h = NULL;

   lpParam = lpParam; // just to avoid a warning
   h = myglobals.dlgbox_datas[dialog_ID].dlg.pChild[iChildIdx].handle;
   if (h != NULL)
   {
      SendMessage(h, CB_INITSTORAGE,   (WPARAM) dwParam, dwParam * sizeof(WCHAR) * 4);
      // SendMessage(h, CB_SETMINVISIBLE, (WPARAM) 20, 0);
   }

   return 0;
}


// ===========================================================================
// create the dialog box window (with its children) : Main
// return 0 on success
// ===========================================================================
int create_dlgbox_main(HINSTANCE hInst, int iCmdShow)
{
   ENUM_DLGBOX_ID       dialog_ID        = DLG_MAIN;
   int                  dwidth           = 1202;
   int                  dheight          = 688;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd             = NULL;
   CREATE_DLGBOX        * d              = NULL;
   int                  child_idx        = -1;
   HWND                 h                = NULL;
   WCHAR                title      [200] = TEXT("");
   char                 version    [200] = "";
   WCHAR                wc_version [200] = TEXT("");
   HMENU                hMenu            = NULL;
   int                  i                = 0;
   int                  n                = 0;
   int                  i_label          = 0;
   int                  i_base           = 0;
   int                  i_group          = 0;
   int                  x                = 0;
   int                  y                = 0;
   int                  dx               = 0;
   int                  dy               = 0;
   int                  r                = 0;
   int                  idx              = 0;
   CREATE_DLGBOX_CHILD  c [DLGBOX_MAIN_NB_CHILDREN] =
   {
      // iChildID                      lpClassName    left top  width height         dwStyle                                                                                                    dwExStyle          lpWindowName               iFontID              lpParam pFuncInitChild      dwFuncInitChildParam lpFuncInitChildParam handle
      // ----------------------------  -------------  ---- ---  ----- -------------- ---------------------------------------------------------------------------------------------------------  -----------------  ------------------------  --------------------  ------- ------------------- -------------------- -------------------- ------
      {ID_DLGBOX_MAIN_COFSEL_GROUP,    WC_BUTTON,     10,  10,  300,  310,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                                       0,                 TEXT("COF selection"),    FONT_VERDANA_13,      NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_ANIMTYPE_LABEL,  WC_STATIC,     20,  34,  70,   30,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, STR_LABEL_ANIMATION_TYPE, FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_TOKEN_LABEL,     WC_STATIC,     100, 48,  60,   20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, STR_LABEL_TOKEN,          FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_WEAPCLASS_LABEL, WC_STATIC,     170, 34,  60,   30,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, STR_LABEL_WEAPON_CLASS,   FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_MODES_LABEL,     WC_STATIC,     240, 48,  60,   20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, STR_LABEL_MODE,           FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_ANIMTYPE_LIST,   WC_LISTBOX,    20,  70,  70,   70,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY,                         0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_TOKEN_LIST,      WC_LISTBOX,    100, 70,  60,   250,           WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY | WS_VSCROLL | LBS_SORT, 0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_WEAPCLASS_LIST,  WC_LISTBOX,    170, 70,  60,   250,           WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY | WS_VSCROLL | LBS_SORT, 0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_MODES_LIST,      WC_LISTBOX,    240, 70,  60,   250,           WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LBS_DISABLENOSCROLL | LBS_NOTIFY | WS_VSCROLL | LBS_SORT, 0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_ANIMATION,       WC_STATIC,     10,  330, 300,  300,           WS_CHILD | WS_VISIBLE /* | SS_SUNKEN*/,                                                                    0,                 NULL,                     FONT_NONE,            NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_BUT_ZOOM_MINUS,  WC_BUTTON,     320, 490, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Zoom -"),           FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_ZOOM_PLUS,   WC_BUTTON,     415, 490, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Zoom +"),           FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_STOP,        WC_BUTTON,     320, 520, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Stop"),             FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_PLAY,        WC_BUTTON,     415, 520, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Play"),             FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_DIR_MINUS,   WC_BUTTON,     320, 550, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Direction -"),      FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_DIR_PLUS,    WC_BUTTON,     415, 550, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Direction +"),      FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_FRM_MINUS,   WC_BUTTON,     320, 580, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Frame -"),          FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_BUT_FRM_PLUS,    WC_BUTTON,     415, 580, 90,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                               0,                 TEXT("Frame +"),          FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_NB_ANIM_COLORS,  WC_STATIC,     320, 617, 330,  14,            WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                           0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_LAYER_GROUP,     WC_BUTTON,     320, 10,  862,  469,           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,                                                                       0,                 TEXT("Layers"),           FONT_VERDANA_13,      NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_CODE_LABEL,      WC_STATIC,     0,   -22, 120,  20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Code"),             FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_VARIANT_LABEL,   WC_STATIC,     0,   -22, 70,   20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Variant"),          FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPTYPE_LABEL,  WC_STATIC,     0,   -36, 90,   34,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Colormap Type"),    FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPFILE_LABEL,  WC_STATIC,     0,   -22, 140,  20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Colormap File"),    FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPINDEX_LABEL, WC_STATIC,     0,   -22, 140,  20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Colormap Index"),   FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_GFXTYPE_LABEL,   WC_STATIC,     0,   -22, 200,  20,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Special Effect"),   FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_GFXLEVEL_LABEL,  WC_STATIC,     0,   -36, 70,   34,            WS_CHILD | WS_VISIBLE | SS_CENTER,                                                                         WS_EX_TRANSPARENT, TEXT("Sp. Effect Level"), FONT_VERDANA_14_BOLD, NULL,   NULL,               0,                   NULL,                NULL},

      {ID_DLGBOX_MAIN_CODE_BASE,       WC_COMBOBOX,   0,   0,   120,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,                                                     0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_VARIANT_BASE,    WC_COMBOBOX,   0,   0,   70,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,                                        0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPTYPE_BASE,   WC_COMBOBOX,   0,   0,   90,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,                                                     0,                 NULL,                     FONT_VERDANA_14,      NULL,   init_dropdown_list, 10,                  NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPFILE_BASE,   WC_COMBOBOX,   0,   0,   140,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,                                        0,                 NULL,                     FONT_VERDANA_14,      NULL,   init_dropdown_list, 50,                  NULL,                NULL},
      {ID_DLGBOX_MAIN_CMAPINDEX_BASE,  WC_COMBOBOX,   0,   0,   140,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,                                                     0,                 NULL,                     FONT_VERDANA_14,      NULL,   init_dropdown_list, 300,                 NULL,                NULL},
      {ID_DLGBOX_MAIN_GFXTYPE_BASE,    WC_COMBOBOX,   0,   0,   200,  20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,                                                     0,                 NULL,                     FONT_VERDANA_14,      NULL,   NULL,               0,                   NULL,                NULL},
      {ID_DLGBOX_MAIN_GFXLEVEL_BASE,   WC_COMBOBOX,   0,   0,   70,   20,            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,                                        0,                 NULL,                     FONT_VERDANA_14,      NULL,   init_dropdown_list, 256,                 NULL,                NULL},

      // reserved : 15 * DLGBOX_MAIN_CTRL_PER_ROW rows
   };
   // keyboard accelerators
   ACCEL tab_accelerators [DLGBOX_MAIN_ACCELERATORS_NB] =
   {
      {FVIRTKEY | FCONTROL, 0x58, ID_MENU_MAIN_EXPORT_ACCEL} /* Ctrl + X = File / Export */
   };

   // dynamic initialisation of the (label row + 16 rows) * (DLGBOX_MAIN_CTRL_PER_ROW controls per row)

   // scan all the controls in c[] to find the indexes where 3 special elements are
   for (i = 0; i < DLGBOX_MAIN_NB_CHILDREN; i++)
   {
      switch (c[i].iChildID)
      {
         case ID_DLGBOX_MAIN_CODE_LABEL  : i_label = i; break;
         case ID_DLGBOX_MAIN_CODE_BASE   : i_base  = i; break;
         case ID_DLGBOX_MAIN_LAYER_GROUP : i_group = i; break;
         default                         :              break;
      }
   }

   // all layer rows are within the group, deduce top/left of the 1st control of the 1st row from the group control top/left
   x  = c[i_group].left + 10;
   y  = c[i_group].top  + 60;
   dx = 2;  // space between 2 controls of the same row
   dy = 25; // 

   // initialize top/left of the 7 labels
   for (r = 0; r < DLGBOX_MAIN_CTRL_PER_ROW; r++)
   {
      idx = i_label + r;
      c[idx].left  = x;
      c[idx].top  += y;
      x += c[idx].width + dx;
   }

   // for each of the 15 layer rows after the first (rows 2 thru 16)
   for (n = 1; n < 16; n++)
   {
      // for each of the 7 columns
      for (r = 0; r < DLGBOX_MAIN_CTRL_PER_ROW; r++)
      {
         // copy the data of this control from the datas of the same control of the 1st row
         memcpy( & c[i_base + n*DLGBOX_MAIN_CTRL_PER_ROW + r], & c[i_base + r], sizeof(CREATE_DLGBOX_CHILD));
      }
   }

   // initialize top/left of all the 16 * 7 controls
   for (n = 0; n < 16; n++)
   {
      x  = c[i_group].left + 10;
      for (r = 0; r < DLGBOX_MAIN_CTRL_PER_ROW; r++)
      {
         idx = i_base + (n * DLGBOX_MAIN_CTRL_PER_ROW) + r;
         c[idx].iChildID += (n * DLGBOX_MAIN_CTRL_PER_ROW);
         c[idx].left      = x;
         c[idx].top       = y + (n * dy);
         x += c[idx].width + dx;
      }
   }

   // dynamic initialisation is finished

   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODLESS;

   // window class
   cw.cbSize        = sizeof(WNDCLASSEX);
   cw.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
   cw.lpfnWndProc   = callback_dlgbox_main;
   cw.cbClsExtra    = 0;
   cw.cbWndExtra    = 0;
   cw.hInstance     = hInst;
   cw.hIcon         = NULL;
   cw.hCursor       = LoadCursor(NULL, IDC_ARROW);
   cw.hbrBackground = GetSysColorBrush(COLOR_BTNFACE); // (HBRUSH) GetStockObject(WHITE_BRUSH) // CreateSolidBrush(RGB(255, 0, 255))
   cw.lpszMenuName  = NULL;
   cw.lpszClassName = STR_DLGBOX_MAIN_CLASS;
   cw.hIconSm       = NULL;

   d->pWndClassEx = & cw;

   sprintf(version, "v. Alpha, %s, %s", __DATE__, __TIME__);
   char_to_wide_char(version, wc_version, sizeof(wc_version));
   swprintf(title, sizeof(title) / 2, TEXT("%s (%s)"), STR_DLGBOX_MAIN_WINDOW_NAME, wc_version);

   // menu
   hMenu = create_main_menu();
   MYASSERT_RETURN(hMenu != NULL, 1, "hMenu = create_main_menu()");

   // keyboard accelerators
   myglobals.datas.hAccel = CreateAcceleratorTable(tab_accelerators, DLGBOX_MAIN_ACCELERATORS_NB);
   MYASSERT_RETURN(myglobals.datas.hAccel != NULL, 1, "hAccel = CreateAcceleratorTable()");

   // window
   d->window.hWndParent            = NULL;
   d->window.left                  = 130; // get_x_to_center_into_screen(dwidth);
   d->window.top                   = 240; // get_y_to_center_into_screen(dheight);
   d->window.width                 = dwidth;
   d->window.height                = dheight;
   d->window.dwStyle               = WS_SYSMENU;
   d->window.dwExStyle             = 0;
   d->window.lpWindowName          = title;
   d->window.hMenu                 = hMenu;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // children
   d->nbChildren = DLGBOX_MAIN_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   // init the listboxes

   // Animation Type
   if (get_dialog_child_index_from_ID(dialog_ID, ID_DLGBOX_MAIN_ANIMTYPE_LIST, & child_idx) == 0)
   {
      h = d->pChild[child_idx].handle;
      SendMessage(h, LB_INITSTORAGE, (WPARAM) AT_MAX, (LPARAM) 200);
      SendMessage(h, LB_ADDSTRING,   (WPARAM) 0,      (LPARAM) TEXT("Players"));
      SendMessage(h, LB_ADDSTRING,   (WPARAM) 0,      (LPARAM) TEXT("Monsters"));
      SendMessage(h, LB_ADDSTRING,   (WPARAM) 0,      (LPARAM) TEXT("Objects"));
   }

   // Token
   if (get_dialog_child_index_from_ID(dialog_ID, ID_DLGBOX_MAIN_TOKEN_LIST, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, LB_INITSTORAGE, (WPARAM) 2000, (LPARAM) 2000 * sizeof(STR_2_LETTERS) * 2);

   // Weapon Class
   if (get_dialog_child_index_from_ID(dialog_ID, ID_DLGBOX_MAIN_WEAPCLASS_LIST, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, LB_INITSTORAGE, (WPARAM) 50, (LPARAM) 50 * sizeof(STR_2_LETTERS) * 2);

   // Mode
   if (get_dialog_child_index_from_ID(dialog_ID, ID_DLGBOX_MAIN_MODES_LIST, & child_idx) == 0)
      SendMessage(d->pChild[child_idx].handle, LB_INITSTORAGE, (WPARAM) 50, (LPARAM) 50 * sizeof(STR_2_LETTERS) * 2);

   // load all listfiles, and update COF selection listboxes
   MYASSERT_RETURN(reload_all_listfiles() == 0, 1, NULL);

   // show and draw it
   ShowWindow(d->window.handle, iCmdShow); // SW_SHOWNORMAL
   UpdateWindow(d->window.handle);
   SetFocus(d->window.handle);

   return 0;
}


// ===========================================================================
// callback function for the dialog box : Main
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_main(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int                  i                       = 0;
   int                  id                      = ID_NULL;
   int                  is_new_COF              = FALSE;
   ENUM_DLGBOX_ID       dialog_ID               = DLG_MAIN;
   int                  child_idx               = -1;
   CREATE_DLGBOX        * d                     = NULL;
   HWND                 h                       = NULL;
   int                  layer_idx               = -1;
   int                  COF_layer_idx           = -1;
   int                  ctrl_idx                = -1;
   DCC_ROW_EXISTS       * pDcc                  = NULL;
   LAYER_DATAS          * pLayer                = NULL;
   int                  row                     = 0;
   int                  update_cache            = FALSE;
   short int            mouse_x                 = 0;
   short int            mouse_y                 = 0;
   // ----- static --------------------------------------------------------------------------
   static int           animation_is_moving     = FALSE;
   static MOVING_TYPE   moving_type             = MOVING_NULL;
   static int           start_x                 = 0;
   static int           start_y                 = 0;
   static int           anim_start_x            = 0;
   static int           anim_start_y            = 0;
   static int           internal_user_direction = 0; // always as if the animation preview was with 32 directions, to help keeping consistency while browsing between COFs
   static int           animation_is_stopped    = FALSE;
   // ---------------------------------------------------------------------------------------
   CREATE_DLGBOX_CHILD  * pChild                = NULL;
   ENUM_CTRL_IDENTIFIER id_sp_eff_lev           = ID_NULL;
   COF_ROW_EXISTS       * pCof                  = NULL;
   LISTFILE_DATAS       * lfd                   = & myglobals.listfile_datas;
   int                  zoom                    = myglobals.user_preview_settings.zoom;


   switch(msg)
   {
      case WM_TIMER : // time to check if the animation in the export window has to be updated : duplicate the Timer message to the export window
         if (myglobals.dlgbox_datas[DLG_EXPORT].is_active == TRUE)
            PostMessage(myglobals.dlgbox_datas[DLG_EXPORT].dlg.window.handle, WM_TIMER, wParam, lParam);
         break;

      case WM_MOUSEMOVE : // the mouse is moving -------------------------------------------------------------
         if (animation_is_moving == TRUE)
         {
            mouse_x = LOWORD (lParam);
            mouse_y = HIWORD (lParam);
            set_anim_user_offsets(myglobals.animation, anim_start_x + (mouse_x - start_x) / zoom, anim_start_y + (mouse_y - start_y) / zoom, moving_type);

            if (animation_is_stopped == TRUE)
            {
               animation_force_redraw(myglobals.animation);
               PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
            }
         }
         return 0;

      case WM_LBUTTONDBLCLK : // mouse left button double click --------------------------------------------------
         if (animation_is_moving == TRUE)
            break;
         else
         {
            if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_ANIMATION, & child_idx) == 0)
            {
               mouse_x = LOWORD (lParam);
               mouse_y = HIWORD (lParam);
               pChild = & myglobals.dlgbox_datas[DLG_MAIN].dlg.pChild[child_idx];
               if (    (mouse_x >= pChild->left) && (mouse_x < (pChild->left + pChild->width ))
                    && (mouse_y >= pChild->top)  && (mouse_y < (pChild->top  + pChild->height)) )
               {
                  // [ctrl + ] left mouse button double click within the animated preview : reset either the animation position or its pivot
                  set_anim_user_offsets(myglobals.animation, 0, 0, (wParam & MK_CONTROL) ? MOVING_PIVOT : MOVING_ANIMATION);

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;
               }
            }
         }
         break;

      case WM_LBUTTONDOWN : // mouse left button pushed down ---------------------------------------------------------------
         if (animation_is_moving == TRUE)
            break;
         else
         {
            if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_ANIMATION, & child_idx) == 0)
            {
               mouse_x = LOWORD (lParam);
               mouse_y = HIWORD (lParam);
               pChild = & myglobals.dlgbox_datas[DLG_MAIN].dlg.pChild[child_idx];
               if (    (mouse_x >= pChild->left) && (mouse_x < (pChild->left + pChild->width ))
                    && (mouse_y >= pChild->top)  && (mouse_y < (pChild->top  + pChild->height)) )
               {
                  // left mouse button pushed down within the animated preview : start moving the animation position or its pivot

                  animation_is_moving = TRUE;

                  moving_type = MOVING_ANIMATION;
                  if (wParam & MK_CONTROL)
                     moving_type = MOVING_PIVOT;

                  start_x = mouse_x;
                  start_y = mouse_y;
                  SetCapture(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle);
                  get_anim_user_offsets(myglobals.animation, & anim_start_x, & anim_start_y, moving_type);

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
               }
            }
            return 0;
         }
         break;

      case WM_LBUTTONUP : // mouse left button released --------------------------------------------------------------------
         if (animation_is_moving == FALSE)
            break;
         else
         {
            // we were moving the animation or its pivot, now its finished : set its new position
            mouse_x = LOWORD (lParam);
            mouse_y = HIWORD (lParam);
            set_anim_user_offsets(myglobals.animation, anim_start_x + (mouse_x - start_x) / zoom, anim_start_y + (mouse_y - start_y) / zoom, moving_type);
            animation_is_moving = FALSE;
            moving_type = MOVING_NULL;
            ReleaseCapture();

            if (animation_is_stopped == TRUE)
            {
               animation_force_redraw(myglobals.animation);
               PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
            }
            return 0;
         }
         break;

      case WM_APP_MPQ_PATHS_CHANGED : // Application private message : new MPQ / Mod Directory paths : reload all ---------------------------------

         stop_tick_25fps(1, 0);
         reset_tick_25fps();

         pCof = myglobals.cof_selection.current_cof;
         if (pCof != NULL)
         {
            get_anim_user_offsets(myglobals.animation, & pCof->user_anim_offset_x,  & pCof->user_anim_offset_y,  MOVING_ANIMATION);
            get_anim_user_offsets(myglobals.animation, & pCof->user_pivot_offset_x, & pCof->user_pivot_offset_y, MOVING_PIVOT);
         }

         DESTROYME(myglobals.animation, destroy_animation)

         // close the old MPQ files, open the new ones
         for (i = 0; i < MPQ_MAX; i++)
         {
            if (i == MPQ_MOD_DIRECTORY)
               continue;

            if (myglobals.datas.mpq[i].storm_handle != NULL)
               close_mpq((ENUM_MPQ) i);

            if (open_mpq((ENUM_MPQ) i) != 0)
               myglobals.datas.mpq[i].path[0] = 0;
         }

         // a mod Directory or an mpq path has changed, reload all Diablo II ressource files
         destroy_cache();
         reload_all_listfiles();

         if (load_d2_ressources() != 0)
            MessageBox(NULL, STR_ERROR_SETUP_MPQ_PATHS, TEXT("Diablo II Ressource file(s) not found"), MB_ICONWARNING | MB_OK);
         debug_cache();

         load_current_cof();

         pCof = myglobals.cof_selection.current_cof;
         if (pCof != NULL)
         {
            set_anim_user_offsets(myglobals.animation, pCof->user_anim_offset_x,  pCof->user_anim_offset_y,  MOVING_ANIMATION);
            set_anim_user_offsets(myglobals.animation, pCof->user_pivot_offset_x, pCof->user_pivot_offset_y, MOVING_PIVOT);
         }
         break;

      case WM_APP_UPDATE_ANIMATION : // Application private message : it's time to draw a new frame of the animation ---------------------------------
         if (myglobals.animation != NULL)
         {
            if (myglobals.drawing_animation == FALSE)
            {
               myglobals.drawing_animation = TRUE;
               draw_current_animation_frame(myglobals.animation);
               myglobals.drawing_animation = FALSE;
            }
         }
         myglobals.update_animation_message_posted = FALSE;
         return 0;

      case WM_CTLCOLORSTATIC: // STATIC (labels) ------------------------------------------------------------
         id  = LOWORD(wParam);
         if ((id == ID_DLGBOX_MAIN_ANIMTYPE_LABEL) || (id == ID_DLGBOX_MAIN_TOKEN_LABEL) || (id == ID_DLGBOX_MAIN_WEAPCLASS_LABEL) || (id == ID_DLGBOX_MAIN_MODES_LABEL))
         {
            SetBkMode((HDC) wParam, TRANSPARENT);
            SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
            //SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
            return (LRESULT) GetStockObject(HOLLOW_BRUSH);
         }
         else if (id == ID_DLGBOX_MAIN_NB_ANIM_COLORS)
         {
            SetBkMode       ((HDC) wParam, OPAQUE);
            SetBkColor      ((HDC) wParam, GetSysColor(COLOR_BTNFACE));
            SetTextColor    ((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
            SetDCBrushColor ((HDC) wParam, GetSysColor(COLOR_BTNFACE));
            return (LRESULT) GetStockObject(DC_BRUSH);
         }
         break;

      case WM_COMMAND : // BUTTON, LISTBOX, EDIT, COMBOBOX ----------------------------------------------------
         id  = LOWORD(wParam);
         if ((id >= ID_DLGBOX_MAIN_CODE_BASE) && (id < ID_RESERVED_01))
         {
            // 1 control of 1 layer, search which one
            d             = & myglobals.dlgbox_datas[dialog_ID].dlg;
            layer_idx     = (id - ID_DLGBOX_MAIN_CODE_BASE) / DLGBOX_MAIN_CTRL_PER_ROW; // which layer row    ?
            ctrl_idx      = (id - ID_DLGBOX_MAIN_CODE_BASE) % DLGBOX_MAIN_CTRL_PER_ROW; // which layer column ?
            COF_layer_idx = myglobals.application_datas.control_to_COF_layer_map[layer_idx];
            pLayer        = & myglobals.cof_selection.current_cof->layer_datas[COF_layer_idx];
            switch (ctrl_idx)
            {
               case 0 :
                  // ===========================
                  // column = code
                  // ===========================
                  break;

               case 1 :
                  // ===========================
                  // column = variant
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new variant selected : load a new DCC and update the animation
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           ANIMATION_DCC dcc    = {0};
                           long          job_ID = 0;

                           // get variant code associated with this row
                           pDcc = (DCC_ROW_EXISTS *) SendMessage(h, CB_GETITEMDATA, row, 0);
                           if (pDcc == NULL)
                              strcpy(pLayer->variant, "");
                           else
                              strcpy(pLayer->variant, pDcc->variant);

                           // load new dcc
                           memset( & dcc, 0, sizeof(dcc));
                           dcc.layer_idx = pLayer->code_idx;
                           load_layer_DCC(COF_layer_idx, & dcc, & update_cache);
                           if (update_cache == TRUE)
                              debug_cache();

                           job_ID = get_new_job_ID();

                           add_layer_to_animation_part1(myglobals.animation, & dcc, job_ID);
                           dccjob_wait_for_tasks(job_ID);
                           add_layer_to_animation_part2(myglobals.animation, & dcc);

                           DESTROYME(dcc.file, free)

                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }

                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case 2 :
                  // ===========================
                  // column = colormap type
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new colormap type selected
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           int cmap_type = CMAPTYPE_NONE;

                           cmap_type = (int) SendMessage(h, CB_GETITEMDATA, row, 0);
                           if ((cmap_type < 0) || (cmap_type >= CMAPTYPE_MAX))
                              cmap_type = CMAPTYPE_NONE;

                           if (cmap_type == CMAPTYPE_NONE)
                           {
                              // colormap is desactivated for this layer
                              pLayer->colormap_identifiers.tab_cmap_file_idx = 0;
                              pLayer->colormap_identifiers.cmap_idx          = -1;
                           }
                           else
                           {
                              // new colormap type : apply default colormap file and index for this new type
                              new_colormap_type(pLayer, (ENUM_CMAP_TYPE) cmap_type);
                           }

                           update_colormapfile_list(layer_idx);
                           update_colormapindex_list(layer_idx);

                           set_new_colormap_to_animation_layer(pLayer);
                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case 3 :
                  // ===========================
                  // column = colormap file
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new colormap file selected
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           COLORMAP_FILE * old_cmap_file = NULL;
                           COLORMAP_FILE * new_cmap_file = NULL;
                           int           cmap_file_idx   = 0;
                           int           old_row_index   = 0;
                           int           old_true_index  = -1;
                           int           new_true_index  = -1;
                           int           n               = 0;
                           int           same_found      = FALSE;


                           // old colormap
                           old_row_index = pLayer->colormap_identifiers.cmap_idx;
                           cmap_file_idx = pLayer->colormap_identifiers.tab_cmap_file_idx;
                           if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
                              old_cmap_file = lfd->tab_cmap_file[cmap_file_idx];

                           if (old_cmap_file != NULL)
                           {
                              if (old_cmap_file->num_cmap_type == CMAPTYPE_SAMEAS)
                                 old_true_index = -1;
                              else
                                 old_true_index = old_cmap_file->tab_index[old_row_index];
                           }

                           // new colormap

                           cmap_file_idx = (int) SendMessage(h, CB_GETITEMDATA, row, 0);
                           if ((cmap_file_idx < 0) || (cmap_file_idx >= lfd->nb_cmap_files))
                              cmap_file_idx = 0;

                           if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
                              new_cmap_file = lfd->tab_cmap_file[cmap_file_idx];

                           if ((old_cmap_file != NULL) && (new_cmap_file != NULL))
                           {
                              if ((old_cmap_file->num_cmap_type == CMAPTYPE_PLAYER) && (new_cmap_file->num_cmap_type == CMAPTYPE_PLAYER))
                              {
                                 // scan the new file and try to find the same (true) colormap index
                                 for (n = 0; n < new_cmap_file->nb_colormaps; n++)
                                 {
                                    new_true_index = new_cmap_file->tab_index[n];
                                    if (old_true_index == new_true_index)
                                    {
                                       // it exists, just change the file, and keep the same (true) colormap index (possibly on a different row tough)
                                       pLayer->colormap_identifiers.tab_cmap_file_idx = cmap_file_idx;
                                       pLayer->colormap_identifiers.cmap_idx          = n;
                                       same_found = TRUE;
                                    }
                                 }
                              }
                           }

                           if (same_found == FALSE)
                           {
                              pLayer->colormap_identifiers.tab_cmap_file_idx = cmap_file_idx;
                              if (new_cmap_file == NULL)
                                 pLayer->colormap_identifiers.cmap_idx = -1;
                              else
                              {
                                 if (new_cmap_file->num_cmap_type == CMAPTYPE_SAMEAS)
                                    pLayer->colormap_identifiers.cmap_idx = -1;
                                 else
                                    pLayer->colormap_identifiers.cmap_idx = 0; // let's see a colormap when a new file is selected
                              }
                           }

                           update_colormapindex_list(layer_idx);

                           set_new_colormap_to_animation_layer(pLayer);
                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case 4 :
                  // ===========================
                  // column = colormap index
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new colormap index within the current file was selected
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           CMAP_ID       * cmap_id     = & pLayer->colormap_identifiers;
                           int           cmap_file_idx = cmap_id->tab_cmap_file_idx;
                           COLORMAP_FILE * cmap_file   = NULL;
                           int           cmap_index    = -1;

                           if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
                              cmap_file = lfd->tab_cmap_file[cmap_file_idx];

                           if (cmap_file != NULL)
                           {
                              cmap_index = (int) SendMessage(h, CB_GETITEMDATA, row, 0);
                              if ((cmap_index < 0) || (cmap_index >= cmap_file->nb_colormaps))
                                 cmap_file_idx = -1;
                           }

                           pLayer->colormap_identifiers.cmap_idx = cmap_index;
                           update_colormapindex_list(layer_idx);

                           set_new_colormap_to_animation_layer(pLayer);
                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case 5 :
                  // ===========================
                  // column = special effect
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new special effect selected 
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           // deduce the special effect from the row
                           if ((row >= 0) && (row < LAYEREFFECTROW_MAX))
                              i = layer_effect_row_datas[row].enum_type;

                           set_layer_transparency_type_user(myglobals.animation, pLayer->code_idx, (ENUM_LAYER_EFFECT_TYPE) i);
                           pLayer->transparency_type_user = (ENUM_LAYER_EFFECT_TYPE) i;

                           // enable or disable the control "special effect level", depending if the "delete dark pixels" effect is selected or not
                           id_sp_eff_lev = get_layer_control_id(ID_DLGBOX_MAIN_GFXLEVEL_BASE, layer_idx);
                           if (get_dialog_child_index_from_ID(dialog_ID, id_sp_eff_lev, & child_idx) == 0)
                           {
                              h = d->pChild[child_idx].handle;
                              modify_special_effect_level(h, id_sp_eff_lev, (i == LAYEREFFECT_DEL_DARK_PIXELS) ? TRUE : FALSE, pLayer->special_effect_level);
                              set_layer_special_effect_level(myglobals.animation, pLayer->code_idx, pLayer->special_effect_level);
                           }

                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case 6 :
                  // ===========================
                  // special effect level
                  // ===========================
                  if (HIWORD(wParam) == CBN_SELCHANGE)
                  {
                     // new special effect level selected 
                     if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
                     {
                        // get row of the new selection
                        h = d->pChild[child_idx].handle;
                        row = SendMessage(h, CB_GETCURSEL, 0, 0);
                        if (row != CB_ERR)
                        {
                           pLayer->special_effect_level = row;
                           set_layer_special_effect_level(myglobals.animation, pLayer->code_idx, row);

                           analyse_animation_colors(myglobals.animation);
                           update_animation_nb_colors_control();
                        }
                     }
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;
            }
         }
         else
         {
            // NOT a control of a layer
            switch (id)
            {
               case ID_DLGBOX_MAIN_ANIMTYPE_LIST  :
               case ID_DLGBOX_MAIN_TOKEN_LIST     :
               case ID_DLGBOX_MAIN_WEAPCLASS_LIST :
               case ID_DLGBOX_MAIN_MODES_LIST     :
                  // COF Selection
                  if (HIWORD(wParam) == LBN_SELCHANGE)
                  {
                     // new COF element selected
                     is_new_COF = FALSE;
                     cof_selection_listbox_clicked(get_cst_enum_from_id(id), & is_new_COF);
                     if (is_new_COF == TRUE)
                     {
                        pCof = myglobals.cof_selection.current_cof;
                        if (pCof != NULL)
                        {
                           // save the current user animation & pivot offsets into the precedent COF datas (for when he'll come back)
                           get_anim_user_offsets(myglobals.animation, & pCof->user_anim_offset_x,  & pCof->user_anim_offset_y,  MOVING_ANIMATION);
                           get_anim_user_offsets(myglobals.animation, & pCof->user_pivot_offset_x, & pCof->user_pivot_offset_y, MOVING_PIVOT);
                        }

                        // load the current COF animation
                        load_current_cof();

                        pCof = myglobals.cof_selection.current_cof;
                        if (pCof != NULL)
                        {
                           int new_nb_directions     = 0;
                           int new_current_direction = 0;

                           // apply the user animation & pivot offsets saved in the current COF datas to the animation preview
                           set_anim_user_offsets(myglobals.animation, pCof->user_anim_offset_x,  pCof->user_anim_offset_y,  MOVING_ANIMATION);
                           set_anim_user_offsets(myglobals.animation, pCof->user_pivot_offset_x, pCof->user_pivot_offset_y, MOVING_PIVOT);

                           // for the new animation, keep the same user direction as in the precedent animation
                           get_anim_nb_and_current_directions(myglobals.animation, & new_nb_directions, NULL);
                           new_current_direction = internal_user_direction * new_nb_directions / 32;
                           if (new_current_direction < 0)
                              new_current_direction = 0;
                           else if (new_current_direction >= new_nb_directions)
                              new_current_direction = new_nb_directions - 1;
                           set_animation_direction(myglobals.animation, new_current_direction);
                        }
                     }
                  }
                  return 0;

               case ID_MENU_MAIN_EXPORT :
               case ID_MENU_MAIN_EXPORT_ACCEL :
                  // open the export dialog
                  if (myglobals.dlgbox_datas[DLG_EXPORT].is_active == FALSE)
                  {
                     if ((HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == 1))
                        create_dlgbox_export();
                  }
                  return 0;

               case ID_MENU_MAIN_OPTIONS :
                  // open the setup dialog
                  if (HIWORD(wParam) == BN_CLICKED)
                     create_dlgbox_setup();
                  return 0;

               case ID_MENU_MAIN_EXIT :
                  // exit the application
                  PostQuitMessage(WM_QUIT);
                  return 0;

               case ID_DLGBOX_MAIN_BUT_STOP :
                  // stop the animation
                  stop_tick_25fps(1, 0);
                  animation_is_stopped = TRUE;
                  return 0;

               case ID_DLGBOX_MAIN_BUT_PLAY :
                  // resume the animation
                  start_tick_25fps(1, 0);
                  animation_is_stopped = FALSE;
                  return 0;

               case ID_DLGBOX_MAIN_BUT_DIR_MINUS :
               case ID_DLGBOX_MAIN_BUT_DIR_PLUS  :
                  // animation direction -- / ++
                  if (myglobals.animation != NULL)
                  {
                     int new_nb_directions     = 0;
                     int new_current_direction = 0;

                     if (id == ID_DLGBOX_MAIN_BUT_DIR_MINUS)
                        animation_direction_minus(myglobals.animation); // direction --
                     else
                        animation_direction_plus(myglobals.animation); // direction ++

                     get_anim_nb_and_current_directions(myglobals.animation, & new_nb_directions, & new_current_direction);
                     if ((new_nb_directions >= 4) && (new_nb_directions <= 32))
                        internal_user_direction = new_current_direction * (32 / new_nb_directions);
                     else
                        internal_user_direction = 0;

                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case ID_DLGBOX_MAIN_BUT_FRM_MINUS :
                  // animation frame --
                  if (myglobals.animation != NULL)
                     animation_frame_minus(myglobals.animation);

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case ID_DLGBOX_MAIN_BUT_FRM_PLUS :
                  // animation frame ++
                  if (myglobals.animation != NULL)
                     animation_frame_plus(myglobals.animation);

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;

               case ID_DLGBOX_MAIN_BUT_ZOOM_MINUS :
               case ID_DLGBOX_MAIN_BUT_ZOOM_PLUS  :
                  // animation zoom -- / ++
                  if (myglobals.animation != NULL)
                  {
                     int new_zoom = myglobals.user_preview_settings.zoom;

                     if      (id == ID_DLGBOX_MAIN_BUT_ZOOM_MINUS) new_zoom--;
                     else if (id == ID_DLGBOX_MAIN_BUT_ZOOM_PLUS)  new_zoom++;

                     if (new_zoom < 1)
                        new_zoom = 1;
                     else if (new_zoom > 4)
                        new_zoom = 4;

                     myglobals.user_preview_settings.zoom = new_zoom;
                     set_new_preview_zoom(new_zoom);

                     // recenter the animation
                     set_anim_user_offsets(myglobals.animation, 0, 0, MOVING_ANIMATION);
                  }

                  if (animation_is_stopped == TRUE)
                  {
                     animation_force_redraw(myglobals.animation);
                     PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_UPDATE_ANIMATION, 0, 0);
                  }
                  return 0;
            }
         }
         break;

      case WM_DESTROY: // exit the application -------------------------------------------------
         if (myglobals.datas.allegro_initialized == TRUE)
         {
            DESTROYME(myglobals.animation, destroy_animation)
            my_allegro_exit();
            myglobals.datas.allegro_initialized = FALSE;
         }
         PostQuitMessage(WM_QUIT);
         return 0;
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// reload all the listfiles, then update the COF selection listboxes
// return 0 on succees
// ===========================================================================
int reload_all_listfiles(void)
{
   WCHAR message [500] = TEXT("");
   int   ret           = 0;
   int   is_new_COF    = FALSE;


   destroy_listfiles();

   // load the application listfile and the MPQ internal listfiles, then build some lists
   ret = load_all_listfiles();
   modify_enable_state_by_child_ID(DLG_COF_IN_PROGRESS, ID_DLGBOX_COF_IN_P_OK, TRUE); // now the OK button must be clikable
   if (ret != 0)
   {
      swprintf(message, sizeof(message) / 2, TEXT("load_all_listfiles() != 0"));
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return 1;
   }

   // initialize all the COF selection listboxes
   cof_selection_listbox_clicked(CST_NONE, & is_new_COF); // CST_NONE is special case for total reinitialisation

   return 0;
}


// ===========================================================================
// create the application main menu
// ===========================================================================
HMENU create_main_menu(void)
{
   HMENU hMenu   = NULL;
   HMENU hFile   = NULL;
   HMENU hEdit   = NULL;
   HMENU hWindow = NULL;
   HMENU hHelp   = NULL;


   MYASSERT_RETURN((hFile   = CreateMenu()) != NULL, NULL, NULL);
   MYASSERT_RETURN((hEdit   = CreateMenu()) != NULL, NULL, NULL);
   MYASSERT_RETURN((hWindow = CreateMenu()) != NULL, NULL, NULL);
   MYASSERT_RETURN((hHelp   = CreateMenu()) != NULL, NULL, NULL);
   MYASSERT_RETURN((hMenu   = CreateMenu()) != NULL, NULL, NULL);

   MYASSERT_RETURN(AppendMenu(hFile,   MF_ENABLED | MF_STRING | MF_UNCHECKED,            ID_MENU_MAIN_EXPORT,      TEXT("Export...\tCtrl+X")  ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hFile,   MF_SEPARATOR,                                     0,                        NULL                       ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hFile,   MF_ENABLED | MF_STRING | MF_UNCHECKED,            ID_MENU_MAIN_EXIT,        TEXT("Exit\tAlt+F4")       ) != 0, NULL, NULL);

   MYASSERT_RETURN(AppendMenu(hEdit,   MF_GRAYED  | MF_STRING | MF_UNCHECKED,            ID_MENU_MAIN_COPY,        TEXT("Copy")               ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hEdit,   MF_SEPARATOR,                                     0,                        NULL                       ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hEdit,   MF_ENABLED | MF_STRING | MF_UNCHECKED,            ID_MENU_MAIN_OPTIONS,     TEXT("Preferences...")     ) != 0, NULL, NULL);

   MYASSERT_RETURN(AppendMenu(hWindow, MF_GRAYED  | MF_STRING | MF_CHECKED,              ID_MENU_MAIN_WINDOWCACHE, TEXT("Cache Informations") ) != 0, NULL, NULL);

   MYASSERT_RETURN(AppendMenu(hHelp,   MF_GRAYED  | MF_STRING | MF_UNCHECKED,            ID_MENU_MAIN_ABOUT,       TEXT("About...")           ) != 0, NULL, NULL);

   MYASSERT_RETURN(AppendMenu(hMenu,   MF_ENABLED | MF_STRING | MF_UNCHECKED | MF_POPUP, (UINT_PTR) hFile,         TEXT("File")               ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hMenu,   MF_ENABLED | MF_STRING | MF_UNCHECKED | MF_POPUP, (UINT_PTR) hEdit,         TEXT("Edit")               ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hMenu,   MF_ENABLED | MF_STRING | MF_UNCHECKED | MF_POPUP, (UINT_PTR) hWindow,       TEXT("Window")             ) != 0, NULL, NULL);
   MYASSERT_RETURN(AppendMenu(hMenu,   MF_ENABLED | MF_STRING | MF_UNCHECKED | MF_POPUP, (UINT_PTR) hHelp,         TEXT("Help")               ) != 0, NULL, NULL);

   return hMenu;
}


// ===========================================================================
// helper for init_layer_controls()
// ===========================================================================
ENUM_CTRL_IDENTIFIER get_layer_control_id(ENUM_CTRL_IDENTIFIER base, int index)
{
   return (ENUM_CTRL_IDENTIFIER) (base + (index * DLGBOX_MAIN_CTRL_PER_ROW));
}


// ===========================================================================
// update layers control with current (new) selected COF datas
// ===========================================================================
void init_layer_controls(void)
{
   ENUM_DLGBOX_ID                  dialog_ID                      = DLG_MAIN;
   DLGBOX_DATAS                    * dd                           = NULL;
   CREATE_DLGBOX                   * d                            = NULL;
   int                             child_idx                      = -1;
   HWND                            h                              = NULL;
   ENUM_CTRL_IDENTIFIER            id                             = ID_NULL;
   COF_SELECTION                   * pCofSel                      = & myglobals.cof_selection;
   COF_ROW_EXISTS                  * pCof                         = pCofSel->current_cof;
   APPLICATION_DATAS               * app                          = & myglobals.application_datas;
   DCC_ROW_EXISTS                  * pDcc                         = NULL;
   int                             i                              = 0;
   int                             k                              = 0;
   int                             c                              = 0;
   LAYER_DATAS                     * pLayer                       = NULL;
   WCHAR                           str[100]                       = TEXT("");
   int                             r                              = 0;
   int                             n                              = 0;
   int                             pos                            = -1;
   int                             p                              = -1;
   char                            str_char[100]                  = "";
   int                             s                              = 0;
   int                             s_default                      = 0;
   int                             display_levels                 = FALSE;
   struct LAYER_EFFECT_ROW_DATAS_S * pEffectRow                   = NULL;
   CMAP_ID                         * cmap_id                      = NULL;
   ENUM_CTRL_IDENTIFIER            column_to_base_id [7] = {ID_DLGBOX_MAIN_CODE_BASE,
                                                            ID_DLGBOX_MAIN_VARIANT_BASE,
                                                            ID_DLGBOX_MAIN_CMAPTYPE_BASE,
                                                            ID_DLGBOX_MAIN_CMAPFILE_BASE,
                                                            ID_DLGBOX_MAIN_CMAPINDEX_BASE,
                                                            ID_DLGBOX_MAIN_GFXTYPE_BASE,
                                                            ID_DLGBOX_MAIN_GFXLEVEL_BASE};


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   // clear all layer controls, and disable the unused ones in the COF
   for (i = 0; i < 16; i++)
   {
      for (c = 0; c < 7; c++)
      {
         id = ID_NULL;
         if ((c >= 0) && (c < 7))
            id = get_layer_control_id(column_to_base_id[c], i);

         if (get_dialog_child_index_from_ID(dialog_ID, id, & child_idx) == 0)
         {
            h = d->pChild[child_idx].handle;
            SendMessage(h, CB_RESETCONTENT, 0, 0);
            if ( (pCof == NULL) || ((pCof != NULL) && (i >= pCof->nb_layers)) )
               modify_enable_state_by_child_ID(dialog_ID, id, FALSE);
            else
               modify_enable_state_by_child_ID(dialog_ID, id, TRUE);
         }
      }
   }

   for (i = 0; i < 16; i++)
      app->control_to_COF_layer_map[i] = -1;

   if (pCof == NULL)
      return;

   // display layer datas. In composit.txt order, not order of appeareance in the COF

   r = 0;
   for (i = 0; i < 16; i++)
   {
      for (n = 0; n < pCof->nb_layers; n++)
      {
         pLayer = & pCof->layer_datas[n]; // in the selected COF, the current settings for this current layer row can be read from 'pLayer' now
         if (pLayer->code_idx != i)
            continue;

         // we found the layer in the COF, it has current the composit code analysed

         app->control_to_COF_layer_map[r] = n; // layer row 'r' is using the layer 'n' in the COF

         // code name of this layer
         id = get_layer_control_id(ID_DLGBOX_MAIN_CODE_BASE, r);
         if (get_dialog_child_index_from_ID(dialog_ID, id, & child_idx) == 0)
         {
            h = d->pChild[child_idx].handle;
            if ((pLayer->code_idx >= 0) && (pLayer->code_idx < app->nb_layers))
            {
               swprintf(str, sizeof(str) / 2, TEXT("%s, %s"), pLayer->code_wchar, app->tab_layer[pLayer->code_idx].name);
               SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
               SendMessage(h, CB_SETCURSEL, 0, 0);
            }
            else
            {
               swprintf(str, sizeof(str) / 2, TEXT("0x%02X, invalid"), pLayer->code_idx);
               SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
               SendMessage(h, CB_SETCURSEL, 0, 0);
            }
         }

         // variant of this layer
         id = get_layer_control_id(ID_DLGBOX_MAIN_VARIANT_BASE, r);
         if (get_dialog_child_index_from_ID(dialog_ID, id, & child_idx) == 0)
         {
            h = d->pChild[child_idx].handle;

            p = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) STR_LAYER_VARIANT_NONE);
            SendMessage(h, CB_SETITEMDATA, (WPARAM) p, (LPARAM) NULL);

            pDcc = pLayer->first_variant;
            pos = -1;
            while (pDcc != NULL)
            {
               if ( (strcmp(pCof->token,                 pDcc->token       ) == 0) &&
                    (strcmp(pLayer->weapon_class_in_cof, pDcc->weapon_class) == 0) &&
                    (strcmp(pCof->mode,                  pDcc->mode        ) == 0) &&
                    (strcmp(pLayer->code_char,           pDcc->layer       ) == 0) )
               {
                  if (pDcc->exists == TRUE)
                  {
                     strcpy(str_char, pDcc->variant);
                     char_to_wide_char(str_char, str, sizeof(str));
                     if (strcmp(pDcc->variant, pLayer->variant) == 0)
                     {
                        pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str); // use 'pos' instead of 'p' to remember that we have selected a row
                        SendMessage(h, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) pDcc);
                        SendMessage(h, CB_SETCURSEL, (WPARAM) pos, 0);
                     }
                     else
                     {
                        p = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
                        SendMessage(h, CB_SETITEMDATA, (WPARAM) p, (LPARAM) pDcc);
                     }
                  }
                  pDcc++;
               }
               else
                  pDcc = NULL;
            }

            if (pos == -1)
               SendMessage(h, CB_SETCURSEL, 0, 0);
         }

         // if needed, initialize the colormap settings given the animation type of this COF
         cmap_id = & pLayer->colormap_identifiers;
         if (cmap_id->initialized == FALSE)
            new_colormap_type(pLayer, CMAPTYPE_DEFAULT_FOR_ANIMTYPE);

         // colormap type of this layer
         update_colormaptype_list(r);

         // colormap file of this layer
         update_colormapfile_list(r);

         // colormap index in the colormap file of this layer
         update_colormapindex_list(r);

         // apply the colormap of this layer to the animation
         set_new_colormap_to_animation_layer(pLayer);

         // special effect
         display_levels = FALSE;
         id = get_layer_control_id(ID_DLGBOX_MAIN_GFXTYPE_BASE, r);
         if (get_dialog_child_index_from_ID(dialog_ID, id, & child_idx) == 0)
         {
            h = d->pChild[child_idx].handle;

            s_default = 0;
            if (pLayer->transparency != 0)
            {
               for (k = 0; k < LAYEREFFECTROW_MAX; k++)
               {
                  pEffectRow = & layer_effect_row_datas[k];
                  if (pLayer->transparency_type == pEffectRow->enum_type)
                  {
                     if (pEffectRow->used_by_COF_files == TRUE)
                     {
                        s_default = k;
                        break;
                     }
                  }
               }
            }

            // fill the text entry of all rows
            for (s = 0; s < LAYEREFFECTROW_MAX; s++)
            {
               pEffectRow = & layer_effect_row_datas[s];
               swprintf(str, sizeof(str) / 2, pEffectRow->label_and_format, (s == s_default) ? TEXT(" (*)") : TEXT(""));
               pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
               if (pLayer->transparency_type_user == pEffectRow->enum_type)
               {
                  SendMessage(h, CB_SETCURSEL, (WPARAM) pos, 0);
                  display_levels = pEffectRow->display_levels;
               }
            }
         }

         // special effect level
         id = get_layer_control_id(ID_DLGBOX_MAIN_GFXLEVEL_BASE, r);
         if (get_dialog_child_index_from_ID(dialog_ID, id, & child_idx) == 0)
         {
            h = d->pChild[child_idx].handle;
            modify_special_effect_level(h, id, display_levels, pLayer->special_effect_level);
         }

         // next layer row
         r++;
         break;
      }
   }
}


// ===========================================================================
// disable or enable a special effect level control
// display = TRUE / FALSE
// ===========================================================================
void modify_special_effect_level(HWND h, ENUM_CTRL_IDENTIFIER id, int display, int selected_row)
{
   int   i        = 0;
   int   pos      = 0;
   WCHAR str[100] = TEXT("");


   if (h == NULL)
      return;

   SendMessage(h, CB_RESETCONTENT, 0, 0);
   if (display == FALSE)
      modify_enable_state_by_child_ID(DLG_MAIN, id, FALSE);
   else
   {
      modify_enable_state_by_child_ID(DLG_MAIN, id, TRUE);
      for (i = 0; i < 256; i++)
      {
         swprintf(str, sizeof(str) / 2, TEXT("%d"), i);
         pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
         if (i == selected_row)
            SendMessage(h, CB_SETCURSEL, (WPARAM) pos, 0);
      }
   }
}


// ===========================================================================
// a new COF was selected, update the colormap type list of a layer
// ===========================================================================
void update_colormaptype_list(int row)
{
   WCHAR                * str_cmap_type [CMAPTYPE_MAX] = { TEXT("(none)"), TEXT("Player"), TEXT("Monster"), TEXT("PL2"), TEXT("Same as")};
   DLGBOX_DATAS         * dd                           = & myglobals.dlgbox_datas[DLG_MAIN];
   CREATE_DLGBOX        * d                            = & dd->dlg;
   APPLICATION_DATAS    * app                          = & myglobals.application_datas;
   LISTFILE_DATAS       * lfd                          = & myglobals.listfile_datas;
   COF_ROW_EXISTS       * pCof                         = myglobals.cof_selection.current_cof;
   int                  child_idx                      = -1;
   HWND                 h                              = NULL;
   ENUM_CTRL_IDENTIFIER id                             = ID_NULL;
   LAYER_DATAS          * pLayer                       = NULL;
   int                  layer_type_id                  = 0;
   CMAP_ID              * cmap_identifiers             = NULL;
   COLORMAP_FILE        * cmap_file                    = NULL;
   int                  k                              = 0;
   int                  cof_layer_idx                  = -1;
   int                  tab_cmap_file_idx              = 0;
   int                  pos                            = -1;


   if (pCof == NULL)
      return;

   if ((row < 0) || (row >= pCof->nb_layers))
      return;

   cof_layer_idx = app->control_to_COF_layer_map[row];
   if (cof_layer_idx == -1)
      return;

   pLayer = & pCof->layer_datas[cof_layer_idx];

   id = get_layer_control_id(ID_DLGBOX_MAIN_CMAPTYPE_BASE, row);
   if (get_dialog_child_index_from_ID(DLG_MAIN, id, & child_idx) == 0)
   {
      h = d->pChild[child_idx].handle;

      cmap_identifiers  = & pLayer->colormap_identifiers;

      tab_cmap_file_idx = cmap_identifiers->tab_cmap_file_idx;
      if ((tab_cmap_file_idx >= 0) && (tab_cmap_file_idx < lfd->nb_cmap_files))
      {
         cmap_file = lfd->tab_cmap_file[tab_cmap_file_idx];
         if (cmap_file != NULL)
            layer_type_id = cmap_file->num_cmap_type;

         for (k = 0; k < CMAPTYPE_MAX; k++)
         {
            if (k == CMAPTYPE_PL2)
               continue; // FIXME

            pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str_cmap_type[k]);
            SendMessage(h, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) k);
            if (k == layer_type_id)
               SendMessage(h, CB_SETCURSEL, (WPARAM) pos, 0);
         }
      }
   }
}


// ===========================================================================
// a new colormap type was selected, update the colormap file list of a layer
// ===========================================================================
void update_colormapfile_list(int row)
{
   DLGBOX_DATAS         * dd               = & myglobals.dlgbox_datas[DLG_MAIN];
   CREATE_DLGBOX        * d                = & dd->dlg;
   APPLICATION_DATAS    * app              = & myglobals.application_datas;
   LISTFILE_DATAS       * lfd              = & myglobals.listfile_datas;
   COF_ROW_EXISTS       * pCof             = myglobals.cof_selection.current_cof;
   int                  child_idx          = -1;
   HWND                 h                  = NULL;
   ENUM_CTRL_IDENTIFIER id                 = ID_NULL;
   LAYER_DATAS          * pLayer           = NULL;
   CMAP_ID              * cmap_identifiers = NULL;
   COLORMAP_FILE        * cmap_file        = NULL;
   COLORMAP_FILE        * tmp_cmap_file    = NULL;
   int                  k                  = 0;
   int                  cof_layer_idx      = -1;
   int                  tab_cmap_file_idx  = 0;
   int                  pos                = -1;
   int                  nb_rows_added      = 0;
   WCHAR                buffer [100]       = TEXT("");
   int                  i                  = 0;
   int                  idx                = 0;


   if (pCof == NULL)
      return;

   if ((row < 0) || (row >= pCof->nb_layers))
      return;

   cof_layer_idx = app->control_to_COF_layer_map[row];
   if (cof_layer_idx == -1)
      return;

   pLayer = & pCof->layer_datas[cof_layer_idx];

   id = get_layer_control_id(ID_DLGBOX_MAIN_CMAPFILE_BASE, row);
   if (get_dialog_child_index_from_ID(DLG_MAIN, id, & child_idx) == 0)
   {
      h = d->pChild[child_idx].handle;

      SendMessage(h, CB_RESETCONTENT, 0, 0);

      cmap_identifiers  = & pLayer->colormap_identifiers;
      tab_cmap_file_idx = cmap_identifiers->tab_cmap_file_idx;
      if ((tab_cmap_file_idx >= 0) && (tab_cmap_file_idx < lfd->nb_cmap_files))
         cmap_file = lfd->tab_cmap_file[tab_cmap_file_idx];

      if (cmap_file != NULL)
      {
         for (k = 0; k < lfd->nb_cmap_files; k++)
         {
            tmp_cmap_file = lfd->tab_cmap_file[k];
            if (tmp_cmap_file != NULL)
            {
               if (tmp_cmap_file->num_cmap_type == cmap_file->num_cmap_type)
               {
                  if (cmap_file->num_cmap_type == CMAPTYPE_SAMEAS)
                  {
                     // avoid "same as myself"
                     if (tmp_cmap_file->sameas_layer_code == pLayer->code_idx)
                        continue;

                     // avoid "same as a layer NOT in the COF"
                     idx = tmp_cmap_file->sameas_layer_code;
                     for (i = 0; i < pCof->nb_layers; i++)
                     {
                        if (pCof->layer_datas[i].code_idx == idx)
                           break;
                     }
                     if (i >= pCof->nb_layers)
                        continue;

                     swprintf(buffer, sizeof(buffer) / 2, TEXT("%s, %s"), app->tab_layer[idx].code, app->tab_layer[idx].name);
                     pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) buffer);
                  }
                  else
                     pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) tmp_cmap_file->name);

                  SendMessage(h, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) k);
                  nb_rows_added++;
                  if (cmap_file == tmp_cmap_file)
                     SendMessage(h, CB_SETCURSEL, (WPARAM) pos, 0);
               }
            }
         }
      }

      if (nb_rows_added == 0)
         modify_enable_state_by_child_ID(DLG_MAIN, id, FALSE);
      else
         modify_enable_state_by_child_ID(DLG_MAIN, id, TRUE);
   }
}


// ===========================================================================
// a new colormap file was selected, update the colormap index list of a layer
// ===========================================================================
void update_colormapindex_list(int row)
{
   DLGBOX_DATAS         * dd                           = & myglobals.dlgbox_datas[DLG_MAIN];
   CREATE_DLGBOX        * d                            = & dd->dlg;
   APPLICATION_DATAS    * app                          = & myglobals.application_datas;
   LISTFILE_DATAS       * lfd                          = & myglobals.listfile_datas;
   COF_ROW_EXISTS       * pCof                         = myglobals.cof_selection.current_cof;
   int                  child_idx                      = -1;
   HWND                 h                              = NULL;
   ENUM_CTRL_IDENTIFIER id                             = ID_NULL;
   LAYER_DATAS          * pLayer                       = NULL;
   CMAP_ID              * cmap_identifiers             = NULL;
   COLORMAP_FILE        * cmap_file                    = NULL;
   int                  k                              = 0;
   int                  cof_layer_idx                  = -1;
   int                  tab_cmap_file_idx              = 0;
   int                  pos                            = 0;
   int                  pos_selected                   = -1;
   int                  nb_rows_added                  = 0;
   WCHAR                str[100]                       = TEXT("");
   int                  color_name_index               = 0;


   if (pCof == NULL)
      return;

   if ((row < 0) || (row >= pCof->nb_layers))
      return;

   cof_layer_idx = app->control_to_COF_layer_map[row];
   if (cof_layer_idx == -1)
      return;

   pLayer = & pCof->layer_datas[cof_layer_idx];

   id = get_layer_control_id(ID_DLGBOX_MAIN_CMAPINDEX_BASE, row);
   if (get_dialog_child_index_from_ID(DLG_MAIN, id, & child_idx) == 0)
   {
      h = d->pChild[child_idx].handle;

      SendMessage(h, CB_RESETCONTENT, 0, 0);

      cmap_identifiers  = & pLayer->colormap_identifiers;
      tab_cmap_file_idx = cmap_identifiers->tab_cmap_file_idx;
      if ((tab_cmap_file_idx >= 0) && (tab_cmap_file_idx < lfd->nb_cmap_files))
         cmap_file = lfd->tab_cmap_file[tab_cmap_file_idx];

      nb_rows_added = 0;
      if (cmap_file != NULL)
      {

         for (k = 0; k < cmap_file->nb_colormaps; k++)
         {
            if (k == 0)
            {
               pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) TEXT("(none)"));
               SendMessage(h, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) -1);
               nb_rows_added++;
               pos_selected = pos;
            }

            if (cmap_file->num_row_extension == REXT_DAT_CMAP_ITM)
            {
               color_name_index = cmap_file->tab_index[k];
               if ((app->tab_colors != NULL) && (k < app->nb_colors))
                  swprintf(str, sizeof(str) / 2, TEXT("%02d, %s"), cmap_file->tab_index[k], app->tab_colors[color_name_index].name);
               else
                  swprintf(str, sizeof(str) / 2, TEXT("%02d, ?"), cmap_file->tab_index[k]);
            }
            else
               swprintf(str, sizeof(str) / 2, TEXT("%d"), cmap_file->tab_index[k]);

            pos = SendMessage(h, CB_ADDSTRING, 0, (LPARAM) str);
            SendMessage(h, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) k);
            nb_rows_added++;
            if (cmap_identifiers->cmap_idx == k)
               pos_selected = pos;
         }
      }

      if (nb_rows_added == 0)
         modify_enable_state_by_child_ID(DLG_MAIN, id, FALSE);
      else
         modify_enable_state_by_child_ID(DLG_MAIN, id, TRUE);

      if (pos_selected != -1)
         SendMessage(h, CB_SETCURSEL, (WPARAM) pos_selected, 0);
   }
}


// ===========================================================================
// new colormap type is selected in the list
// save this new value in the layer datas, and modify colormap file and index
// ===========================================================================
void new_colormap_type(LAYER_DATAS * pLayer, ENUM_CMAP_TYPE new_cmap_type)
{
   CMAP_ID           * cmap_id    = NULL;
   COLORMAP_FILE     * cmapfile   = NULL;
   WCHAR             code [2 + 1] = TEXT("");
   int               k            = 0;
   int               i            = 0;
   int               n            = 0;
   int               x            = 0;
   COF_SELECTION     * pCofSel    = & myglobals.cof_selection;
   LISTFILE_DATAS    * lfd        = & myglobals.listfile_datas;
   COF_ROW_EXISTS    * pCof       = pCofSel->current_cof;
   APPLICATION_DATAS * app        = & myglobals.application_datas;
   int               TR_exists    = FALSE;
   int               SH_exists    = FALSE;
   int               LH_exists    = FALSE;
   int               sameasTR_idx = 0;
   int               idx          = 0;


   if (pLayer == NULL)
      return;

   cmap_id = & pLayer->colormap_identifiers;

   cmap_id->cmap_idx = -1;

   switch (new_cmap_type)
   {
      case CMAPTYPE_DEFAULT_FOR_ANIMTYPE :
         cmap_id->initialized       = TRUE;
         cmap_id->tab_cmap_file_idx = 0;
         k = pCofSel->animation_type_idx;
         if ((k == AT_PLAYERS) || (k == AT_MONSTERS))
         {
            for (i = 0; i < pCof->nb_layers; i++)
            {
               if      (strcmp(pCof->layer_datas[i].code_char, "TR") == 0) TR_exists = TRUE;
               else if (strcmp(pCof->layer_datas[i].code_char, "SH") == 0) SH_exists = TRUE;
               else if (strcmp(pCof->layer_datas[i].code_char, "LH") == 0) LH_exists = TRUE;
            }

            if (TR_exists == TRUE)
            {
               sameasTR_idx = 0;
               for (n = 0; n < lfd->nb_cmap_files; n++)
               {
                  cmapfile = lfd->tab_cmap_file[n];
                  if (cmapfile != NULL)
                  {
                     if (cmapfile->num_cmap_type == CMAPTYPE_SAMEAS)
                     {
                        if (strcmp(app->tab_layer[ cmapfile->sameas_layer_code ].code_char, "TR") == 0)
                        {
                           sameasTR_idx = n;
                           break;
                        }
                     }
                  }
               }

               if (sameasTR_idx != 0)
               {
                  if ((strcmp(pLayer->code_char, "HD") == 0) || (strcmp(pLayer->code_char, "TR") == 0) || (strcmp(pLayer->code_char, "RH") == 0))
                  {
                     // independant layer in all case, don't use "same as TR"
                  }
                  else if ((k == AT_PLAYERS) && (strcmp(pLayer->code_char, "S3") == 0))
                  {
                     // Necromancer's S3 is an independant layer, don't use "same as TR"
                  }
                  else if ((strcmp(pLayer->code_char, "SH") == 0) || (strcmp(pLayer->code_char, "LH") == 0))
                  {
                     // layer is SH or LH
                     if ((SH_exists == FALSE) || (LH_exists == FALSE))
                     {
                        // only one of them exists in the COF, this is an independant layer, don't use "same as TR"
                     }
                     else
                     {
                        // both SH and LH exists
                        if (strcmp(pLayer->code_char, "LH") == 0)
                        {
                           // LH is a dependant layer in this case, use "same as TR"
                           cmap_id->tab_cmap_file_idx = sameasTR_idx;
                           return;
                        }
                     }
                  }
                  else
                  {
                     // dependant layer, use "same as TR"
                     cmap_id->tab_cmap_file_idx = sameasTR_idx;
                     return;
                  }
               }
            }
         }
         break;

      case CMAPTYPE_NONE :
         cmap_id->tab_cmap_file_idx = 0;
         return;

      case CMAPTYPE_PLAYER :
         k = AT_PLAYERS;
         break;

      case CMAPTYPE_MONSTER :
         k = AT_MONSTERS;
         break;

      case CMAPTYPE_SAMEAS :
         k = pCofSel->animation_type_idx;

         // search the first available layer not already in "same as" mode
         for (i = 0; i < 16; i++)
         {
            idx = app->control_to_COF_layer_map[i];
            if ((idx >= 0) && (idx < pCof->nb_layers))
            {
               if (pCof->layer_datas[idx].code_idx == pLayer->code_idx)
                  continue;

               n = pCof->layer_datas[idx].colormap_identifiers.tab_cmap_file_idx;
               if ((n >= 0) && (n < lfd->nb_cmap_files))
               {
                  cmapfile = lfd->tab_cmap_file[n];
                  if ( (cmapfile == NULL)
                       || 
                       ((cmapfile != NULL) && (cmapfile->num_cmap_type != CMAPTYPE_SAMEAS)) )
                  {
                     // the current analysed layer is the one we want to use as "same as" model
                     // search the "same as" virtual entry corresponding to this layer code
                     for (x = 0; x < lfd->nb_cmap_files; x++)
                     {
                        cmapfile = lfd->tab_cmap_file[x];
                        if (cmapfile != NULL)
                        {
                           if (cmapfile->num_cmap_type == CMAPTYPE_SAMEAS)
                           {
                              if (strcmp(app->tab_layer[ cmapfile->sameas_layer_code ].code_char, pCof->layer_datas[idx].code_char) == 0)
                              {
                                 cmap_id->tab_cmap_file_idx = x;
                                 return;
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         cmap_id->tab_cmap_file_idx = 0;
         return;
         break;

      default :
         return;
   }

   if ((k >= 0) && (k < AT_MAX))
      cmap_id->tab_cmap_file_idx = lfd->at[k].default_cmap_file_id;

   if (k == AT_MONSTERS)
   {
      for (n = 0; n < lfd->nb_cmap_files; n++)
      {
         cmapfile = lfd->tab_cmap_file[n];
         if (cmapfile != NULL)
         {
            char_to_wide_char(pCof->token, code, sizeof(code));
            if (wcsicmp(code, cmapfile->name) == 0)
            {
               cmap_id->tab_cmap_file_idx = n;
               break;
            }
         }
      }
   }
}


// ===========================================================================
//
// ===========================================================================
void set_new_colormap_to_animation_layer(LAYER_DATAS * pLayer)
{
   CMAP_ID        * cmap_identifiers   = NULL;
   int            cmap_file_idx        = 0;
   LISTFILE_DATAS * lfd                = & myglobals.listfile_datas;
   COLORMAP_FILE  * cmap_file          = NULL;
   int            cmap_idx             = 0;
   COF_ROW_EXISTS * pCof               = myglobals.cof_selection.current_cof;
   int            i                    = 0;
   int            n                    = 0;
   int            layer_to_change [16] = {0};
   int            cof_layer_scan  [20] = {0};
   UBYTE          * new_cmap           = NULL;
   int            done                 = FALSE;


   if (pLayer == NULL)
      return;

   memset(layer_to_change, 0, sizeof(layer_to_change));
   memset(cof_layer_scan , 0, sizeof(cof_layer_scan));
   layer_to_change[ pLayer->code_idx ] = 1;

   cmap_identifiers = & pLayer->colormap_identifiers;
   cmap_file_idx    = cmap_identifiers->tab_cmap_file_idx;
   cmap_idx         = cmap_identifiers->cmap_idx;

   cmap_file = NULL;
   if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
      cmap_file = lfd->tab_cmap_file[cmap_file_idx];

   new_cmap = NULL;
   if (cmap_file != NULL)
   {
      if (cmap_file->num_cmap_type != CMAPTYPE_SAMEAS)
      {
         // true colormap, use it
         if ((cmap_idx >= 0) && (cmap_idx < cmap_file->nb_colormaps))
            new_cmap = cmap_file->tab_cmap[cmap_idx];
      }
      else
      {
         // virtual colormap, let's find the true one
         done = FALSE;
         while ( ! done)
         {
            done = TRUE;
            for (n = 0; n < pCof->nb_layers; n++)
            {
               pLayer = & pCof->layer_datas[n];
               if ((pLayer->code_idx == cmap_file->sameas_layer_code) && (cof_layer_scan[n] == 0))
               {
                  // on the layer which serves as model
                  cof_layer_scan[n] = 1;
                  cmap_identifiers  = & pLayer->colormap_identifiers;
                  cmap_file_idx     = cmap_identifiers->tab_cmap_file_idx;
                  cmap_idx          = cmap_identifiers->cmap_idx;
                  cmap_file         = NULL;
                  if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
                     cmap_file = lfd->tab_cmap_file[cmap_file_idx];
                  if (cmap_file != NULL)
                  {
                     if (cmap_file->num_cmap_type == CMAPTYPE_SAMEAS)
                        done = FALSE;
                     else
                     {
                        if ((cmap_idx >= 0) && (cmap_idx < cmap_file->nb_colormaps))
                           new_cmap = cmap_file->tab_cmap[cmap_idx];
                     }
                     break;
                  }
                  else
                  {
                     new_cmap = NULL;
                     break;
                  }
               }
            }
         }
      }
   }

   // now, let's find the other layers that are using the modified layer as a "same as" model
   done = FALSE;
   while ( ! done)
   {
      done = TRUE;
      for (n = 0; n < pCof->nb_layers; n++)
      {
         pLayer = & pCof->layer_datas[n];

         cmap_identifiers  = & pLayer->colormap_identifiers;
         cmap_file_idx     = cmap_identifiers->tab_cmap_file_idx;
         cmap_idx          = cmap_identifiers->cmap_idx;
         cmap_file         = NULL;
         if ((cmap_file_idx >= 0) && (cmap_file_idx < lfd->nb_cmap_files))
            cmap_file = lfd->tab_cmap_file[cmap_file_idx];
         if (cmap_file != NULL)
         {
            if (cmap_file->num_cmap_type == CMAPTYPE_SAMEAS)
            {
               if (layer_to_change[ cmap_file->sameas_layer_code ] == 1)
               {
                  if (layer_to_change[ pLayer->code_idx ] == 0)
                  {
                     layer_to_change[ pLayer->code_idx ] = 1;
                     done = FALSE;
                     break;
                  }
               }
            }
         }
      }
   }

   // update all necessary layers with the new colormap
   for (i = 0; i < 16; i++)
   {
      if (layer_to_change[i] == 1)
         set_layer_colormap(myglobals.animation, i, new_cmap);
   }
}
