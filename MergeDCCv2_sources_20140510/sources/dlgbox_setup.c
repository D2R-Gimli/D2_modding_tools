#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include "defines.h"
#include "strings_define.h"
#include "misc.h"
#include "dlgbox_setup.h"
#include "dialog_maker.h"
#include "tools.h"


#define  DLGBOX_SETUP_NB_CHILDREN             3
#define  DLGBOX_SETUP_TAB_MPQ_NB_CHILDREN     12
#define  DLGBOX_SETUP_TAB_MODDIR_NB_CHILDREN  3

static int       dlgbox_setup_init_tab                   (ENUM_DLGBOX_ID dialog_ID, int child_idx, DWORD dw, LPVOID lp);
static int       create_dlgbox_setup_tab_mpq             (HWND hTabParent);
static int       create_dlgbox_setup_tab_moddir          (HWND hTabParent);
LRESULT CALLBACK callback_dlgbox_setup                   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK callback_dlgbox_setup_tab               (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static int       validate_new_mod_directory_path         (WCHAR * path);
static int       select_new_mod_directory                (HWND hParent, WCHAR * path);
static int       select_new_mpq_file                     (HWND hParent, WCHAR * path, int m);
static int       update_application_with_new_setup_paths (int * has_truly_changed);
static int       init_dlgbox_setup_paths                 (void);


typedef struct DLG_SETUP_DATAS_S
{
   int setup_has_changed;
} DLG_SETUP_DATAS;


// Tab                : http://msdn.microsoft.com/en-us/library/bb760548%28v=vs.85%29.aspx
// Tab Control Styles : http://msdn.microsoft.com/en-us/library/bb760549%28v=vs.85%29.aspx

// ===========================================================================
// create the dialog box window (with its children) : Setup
// return 0 on success
// ===========================================================================
int create_dlgbox_setup(void)
{
   ENUM_DLGBOX_ID       dialog_ID   = DLG_SETUP;
   int                  dwidth      = 600;
   int                  dheight     = 285;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd        = NULL;
   CREATE_DLGBOX        * d         = NULL;
   int                  child_idx   = -1;
   HWND                 h           = NULL;
   DLG_SETUP_DATAS      * dlg_datas = NULL;
   CREATE_DLGBOX_CHILD  c [DLGBOX_SETUP_NB_CHILDREN] =
   {
      // iChildID              lpClassName    left top  width height         dwStyle                                                                                         dwExStyle lpWindowName    iFontID          lpParam pFuncInitChild           dwFuncInitChildParam    lpFuncInitChildParam handle
      // --------------------  -------------  ---- ---  ----- -------------  ----------------------------------------------------------------------------------------------  --------- --------------  ---------------  ------- -----------------------  ----------------------- -------------------- ------
      {ID_DLGBOX_SETUP_TAB,    WC_TABCONTROL, 10,  10,  573,  200,           WS_CHILD | WS_VISIBLE | WS_TABSTOP,                                                             0,        TEXT(""),       FONT_VERDANA_14, NULL,   dlgbox_setup_init_tab,   0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_OK,     WC_BUTTON,     400, 220, 86,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_DEFPUSHBUTTON, 0,        TEXT("OK"),     FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_CANCEL, WC_BUTTON,     497, 220, 86,   HEIGHT_BUT_14, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,                    0,        TEXT("Cancel"), FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
   };


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODAL;

   // window class
   cw.cbSize        = sizeof(WNDCLASSEX);
   cw.style         = CS_HREDRAW | CS_VREDRAW;
   cw.lpfnWndProc   = callback_dlgbox_setup;
   cw.cbClsExtra    = 0;
   cw.cbWndExtra    = 0;
   cw.hInstance     = myglobals.main_instance;
   cw.hIcon         = NULL;
   cw.hCursor       = LoadCursor(NULL, IDC_ARROW);
   cw.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   cw.lpszMenuName  = NULL;
   cw.lpszClassName = STR_DLGBOX_SETUP_CLASS;
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
   d->window.lpWindowName          = STR_DLGBOX_SETUP_WINDOW_NAME;
   d->window.hMenu                 = NULL;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // children
   d->nbChildren = DLGBOX_SETUP_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   // create all dialogs for each tabs
   if (get_dialog_child_index_from_ID(dialog_ID, ID_DLGBOX_SETUP_TAB, & child_idx) == 0)
   {
      h = d->pChild[child_idx].handle;
      MYASSERT_RETURN(create_dlgbox_setup_tab_mpq(h) == 0, 1, NULL);
      MYASSERT_RETURN(create_dlgbox_setup_tab_moddir(h) == 0, 1, NULL);
      MYASSERT_RETURN(init_dlgbox_setup_paths() == 0, 1, NULL);
   }

   // private setup dialog datas
   dlg_datas = (DLG_SETUP_DATAS *) calloc(1, sizeof(DLG_SETUP_DATAS));
   MYASSERT_RETURN(dlg_datas != NULL, 1, NULL);
   dlg_datas->setup_has_changed = FALSE; // by default the user will make no changes
   dd->dlg.dlg_datas = dlg_datas;

   // show and draw it
   ShowWindow(d->window.handle, SW_SHOWNORMAL);
   UpdateWindow(d->window.handle);
   SetFocus(d->window.handle);

   return 0;
}


// ===========================================================================
// create the tabs
// ===========================================================================
int dlgbox_setup_init_tab(ENUM_DLGBOX_ID dialog_ID, int child_idx, DWORD dw, LPVOID lp)
{
   DLGBOX_DATAS         * dd          = NULL;
   CREATE_DLGBOX        * d           = NULL;
   CREATE_DLGBOX_WINDOW * w           = NULL;
   CREATE_DLGBOX_CHILD  * pc          = NULL;
   CREATE_DLGBOX_CHILD  * c           = NULL;
   WCHAR                * tab_name [] = {TEXT("MPQ archives"), TEXT("Mod directory"), TEXT("Cache"), NULL};
   int                  i             = 0;
   TCITEM               tci;


   MYASSERT_RETURN(dialog_ID == DLG_SETUP, 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   MYASSERT_RETURN(w->handle != NULL, 1, NULL);
   pc = d->pChild;
   MYASSERT_RETURN(pc != NULL, 1, NULL);
   MYASSERT_RETURN((child_idx >= 0) && (child_idx < d->nbChildren), 1, NULL);
   c = & pc[child_idx];
   MYASSERT_RETURN(c->handle != NULL, 1, NULL);

   dw = dw;
   lp = lp;

   memset ( & tci, 0, sizeof(tci));
   tci.mask   = TCIF_TEXT;
   tci.iImage = -1;
   for (i = 0; tab_name[i] != NULL; i++)
   {
      tci.pszText    = tab_name[i];
      tci.cchTextMax = wcslen(tci.pszText) + 1;
      MYASSERT_RETURN(TabCtrl_InsertItem(c->handle, i, & tci) != -1, 1, NULL);
   }

   return 0;
}
   

// ===========================================================================
// callback function for the dialog box : Setup
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_setup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ENUM_DLGBOX_ID  dialog_ID         = DLG_SETUP;
   DLGBOX_DATAS    * dd              = NULL;
   int             id                = ID_NULL;
   WORD            w                 = 0;
   NMHDR           * pNm             = NULL;
   HWND            h                 = NULL;
   int             i                 = 0;
   ENUM_DLGBOX_ID  tab_dlg_id [3]    = {DLG_SETUP_TAB_MPQ, DLG_SETUP_TAB_MODDIR, DLG_SETUP_TAB_CACHE};
   int             has_truly_changed = FALSE;
   DLG_SETUP_DATAS * dlg_datas       = NULL;


   dd        = & myglobals.dlgbox_datas[dialog_ID];
   dlg_datas = (DLG_SETUP_DATAS *) dd->dlg.dlg_datas;

   switch(msg)
   {

      case WM_SYSCOMMAND : // user selected the Exit Sysmenu command ------------------------
         w = (wParam & 0xFFF0);
         if (w == SC_CLOSE)
         {
            dd->is_closing = TRUE;
            if (dlg_datas != NULL)
               dlg_datas->setup_has_changed = FALSE;
         }
         break;

      case WM_COMMAND : // BUTTON, LISTBOX, EDIT ---------------------------------------------
         id  = LOWORD(wParam);
         if ((id == IDOK) || (id == IDCANCEL))
         {
            dd->is_closing = TRUE;
            if (dlg_datas != NULL)
               dlg_datas->setup_has_changed = (id == IDOK) ? TRUE : FALSE;
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
         }
         else if (HIWORD(wParam) == BN_CLICKED)
         {
            switch (id)
            {
               case ID_DLGBOX_SETUP_OK : 
               case ID_DLGBOX_SETUP_CANCEL : 
                  dd->is_closing = TRUE;
                  if (dlg_datas != NULL)
                     dlg_datas->setup_has_changed = (id == ID_DLGBOX_SETUP_OK) ? TRUE : FALSE;
                  SendMessage(hwnd, WM_CLOSE, 0, 0);
                  break;
            }
         }
         break;

      case WM_NOTIFY : // TABS --------------------------------------------------------------
         pNm = (NMHDR *) lParam;
         i = TabCtrl_GetCurSel(pNm->hwndFrom);
         if ((i >= 0) && (i < 3))
         {
            h = myglobals.dlgbox_datas[tab_dlg_id[i]].dlg.window.handle;
            if (h != NULL)
            {
               switch (pNm->code)
               {
                  case TCN_SELCHANGING : ShowWindow(h, SW_HIDE); return FALSE;
                  case TCN_SELCHANGE   : ShowWindow(h, SW_SHOWNORMAL); break;
               }
            }
         }
         break;

      case WM_DESTROY: // close the dialog boxes ----------------------------------------------
         if (dd->is_closing == TRUE)
         {
            if (dlg_datas->setup_has_changed == TRUE)
            {
               dlg_datas->setup_has_changed = FALSE;
               has_truly_changed = FALSE;
               update_application_with_new_setup_paths( & has_truly_changed);
               if (has_truly_changed == TRUE)
               {
                  // save the current Mod directory and MPQ paths configuration
                  save_current_configuration();

                  // inform the application that since mpq paths have changed, all files will need to be reloaded
                  PostMessage(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, WM_APP_MPQ_PATHS_CHANGED, 0, 0);
               }
            }

            close_dialog(DLG_SETUP_TAB_MODDIR);
            close_dialog(DLG_SETUP_TAB_MPQ);
            close_dialog(dialog_ID);
         }
         break;
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// create the dialog box window (with its children) : tab 1 (MPQ)
// return 0 on success
// ===========================================================================
int create_dlgbox_setup_tab_mpq(HWND hTabParent)
{
   ENUM_DLGBOX_ID       dialog_ID = DLG_SETUP_TAB_MPQ;
   int                  dwidth    = 0;
   int                  dheight   = 0;
   int                  parent_width  = 0;
   int                  parent_height = 0;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd      = NULL;
   CREATE_DLGBOX        * d       = NULL;
   RECT                 rect;
   int                  i         = 0;
   CREATE_DLGBOX_CHILD  c [DLGBOX_SETUP_TAB_MPQ_NB_CHILDREN] =
   {
      // iChildID                             lpClassName    left top  width height dwStyle                                                                      dwExStyle          lpWindowName            iFontID          lpParam pFuncInitChild           dwFuncInitChildParam    lpFuncInitChildParam handle
      // -----------------------------------  -------------  ---- ---  ----- ------ ---------------------------------------------------------------------------  -----------------  ----------------------  ---------------  ------- -----------------------  ----------------------- -------------------- ------
      {ID_DLGBOX_SETUP_TABMPQ_LABEL_PATCH_D2, WC_STATIC,     10,  20,  105,  23,    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,                            WS_EX_TRANSPARENT, STR_LABEL_PATCH_D2_MPQ, FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2,  WC_EDIT,       115, 20,  0,    23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER,   WS_EX_CLIENTEDGE,  TEXT(""),               FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2,  WC_BUTTON,     0,   20,  50,   23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER, 0,                 TEXT("..."),            FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_LABEL_D2EXP,    WC_STATIC,     10,  50,  105,  23,    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,                            WS_EX_TRANSPARENT, STR_LABEL_D2EXP_MPQ,    FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP,     WC_EDIT,       115, 50,  0,    23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER,   WS_EX_CLIENTEDGE,  TEXT(""),               FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP,     WC_BUTTON,     0,   50,  50,   23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER, 0,                 TEXT("..."),            FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_LABEL_D2DATA,   WC_STATIC,     10,  80,  105,  23,    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,                            WS_EX_TRANSPARENT, STR_LABEL_D2DATA_MPQ,   FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA,    WC_EDIT,       115, 80,  0,    23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER,   WS_EX_CLIENTEDGE,  TEXT(""),               FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA,    WC_BUTTON,     0,   80,  50,   23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER, 0,                 TEXT("..."),            FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_LABEL_D2CHAR,   WC_STATIC,     10,  110, 105,  23,    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,                            WS_EX_TRANSPARENT, STR_LABEL_D2CHAR_MPQ,   FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR,    WC_EDIT,       115, 110, 0,    23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER,   WS_EX_CLIENTEDGE,  TEXT(""),               FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR,    WC_BUTTON,     0,   110, 50,   23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER, 0,                 TEXT("..."),            FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
   };


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODLESS;

   // window class
   cw.cbSize        = sizeof(WNDCLASSEX);
   cw.style         = CS_HREDRAW | CS_VREDRAW;
   cw.lpfnWndProc   = callback_dlgbox_setup_tab;
   cw.cbClsExtra    = 0;
   cw.cbWndExtra    = 0;
   cw.hInstance     = myglobals.main_instance;
   cw.hIcon         = NULL;
   cw.hCursor       = LoadCursor(NULL, IDC_ARROW);
   cw.hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
   cw.lpszMenuName  = NULL;
   cw.lpszClassName = STR_DLGBOX_SETUP_TAB_MPQ_CLASS;
   cw.hIconSm       = NULL;

   d->pWndClassEx = & cw;

   if (get_window_position_and_dimension(hTabParent, NULL, NULL, & parent_width, & parent_height) != 0)
      return 1;

   memset( & rect, 0, sizeof(rect));
   TabCtrl_AdjustRect(hTabParent, TRUE, & rect);
   dwidth  = parent_width  + rect.left - rect.right;
   dheight = parent_height + rect.top  - rect.bottom;

   // window
   d->window.hWndParent            = hTabParent;
   d->window.left                  = - rect.left;
   d->window.top                   = - rect.top;
   d->window.width                 = dwidth;
   d->window.height                = dheight;
   d->window.dwStyle               = WS_CHILD | WS_VISIBLE;
   d->window.dwExStyle             = 0;
   d->window.lpWindowName          = TEXT("");
   d->window.hMenu                 = NULL;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // set up some dynamic values
   for (i = 0; i < DLGBOX_SETUP_TAB_MPQ_NB_CHILDREN; i++)
   {
      if (    (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2)
           || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP)
           || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA)
           || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR) )
      {
         c[i].width = dwidth - 105 - 50 - (10 * 3) - 5;
      }
      else if (    (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2)
                || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP)
                || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA)
                || (c[i].iChildID == ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR) )
      {
         c[i].left = dwidth - 15 - c[i].width;
      }
   }

   // children
   d->nbChildren = DLGBOX_SETUP_TAB_MPQ_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   return 0;
}


// ===========================================================================
// create the dialog box window (with its children) : tab 2 (MOD Directory)
// return 0 on success
// ===========================================================================
int create_dlgbox_setup_tab_moddir(HWND hTabParent)
{
   ENUM_DLGBOX_ID       dialog_ID     = DLG_SETUP_TAB_MODDIR;
   int                  dwidth        = 0;
   int                  dheight       = 0;
   int                  parent_width  = 0;
   int                  parent_height = 0;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd          = NULL;
   CREATE_DLGBOX        * d           = NULL;
   RECT                 rect;
   int                  i             = 0;
   CREATE_DLGBOX_CHILD  c [DLGBOX_SETUP_TAB_MODDIR_NB_CHILDREN] =
   {
      // iChildID                       lpClassName left top  width height dwStyle                                                                      dwExStyle          lpWindowName             iFontID          lpParam pFuncInitChild           dwFuncInitChildParam    lpFuncInitChildParam handle
      // -----------------------------  ----------- ---- ---  ----- ------ ---------------------------------------------------------------------------  -----------------  -----------------------  ---------------  ------- -----------------------  ----------------------- -------------------- ------
      {ID_DLGBOX_SETUP_TABMODDIR_LABEL, WC_STATIC,  10,  20,  105,  23,    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,                            WS_EX_TRANSPARENT, STR_LABEL_MOD_DIRECTORY, FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMODDIR_EDIT,  WC_EDIT,    115, 20,  0,    23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER,   WS_EX_CLIENTEDGE,  TEXT(""),                FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_SETUP_TABMODDIR_BUTN,  WC_BUTTON,  0,   20,  50,   23,    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER, 0,                 TEXT("..."),             FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
   };


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODLESS;

   // window class
   memset( & cw, 0, sizeof(cw));
   cw.lpszClassName = STR_DLGBOX_SETUP_TAB_MPQ_CLASS;
   myglobals.class_registered[dialog_ID] = 1; // re-use the 1st tab class window, so avoid trying to create the same system window class again

   d->pWndClassEx = & cw;

   if (get_window_position_and_dimension(hTabParent, NULL, NULL, & parent_width, & parent_height) != 0)
      return 1;

   memset( & rect, 0, sizeof(rect));
   TabCtrl_AdjustRect(hTabParent, TRUE, & rect);
   dwidth  = parent_width  + rect.left - rect.right;
   dheight = parent_height + rect.top  - rect.bottom;

   // window
   d->window.hWndParent            = hTabParent;
   d->window.left                  = - rect.left;
   d->window.top                   = - rect.top;
   d->window.width                 = dwidth;
   d->window.height                = dheight;
   d->window.dwStyle               = WS_CHILD; // not yet visible by default
   d->window.dwExStyle             = 0;
   d->window.lpWindowName          = TEXT("");
   d->window.hMenu                 = NULL;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // set up some dynamic values
   for (i = 0; i < DLGBOX_SETUP_TAB_MODDIR_NB_CHILDREN; i++)
   {
      if (c[i].iChildID == ID_DLGBOX_SETUP_TABMODDIR_EDIT)
         c[i].width = dwidth - 105 - 50 - (10 * 3) - 5;
      else if (c[i].iChildID == ID_DLGBOX_SETUP_TABMODDIR_BUTN)
         c[i].left = dwidth - 15 - c[i].width;
   }

   // children
   d->nbChildren = DLGBOX_SETUP_TAB_MODDIR_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   return 0;
}


// ===========================================================================
// callback function for the dialog box : Setup, all tabs
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_setup_tab(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ENUM_DLGBOX_ID       dialog_ID       = DLG_NONE;
   CREATE_DLGBOX        * dlg           = NULL;
   HWND                 h               = NULL;
   ENUM_CTRL_IDENTIFIER iEdit           = ID_NULL;
   HWND                 hEdit           = NULL;
   WORD                 id              = ID_NULL;
   int                  child_idx       = -1;
   WCHAR                path [MAX_PATH] = TEXT("");
   int                  m               = 0;
   int                  n               = 0;


   switch(msg)
   {
      case WM_CTLCOLORSTATIC: // STATIC ---------------------------------------
         SetBkMode((HDC) wParam, TRANSPARENT);
         SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
         return (LRESULT) GetStockObject(HOLLOW_BRUSH);

      case WM_COMMAND : // BUTTON, EDIT ---------------------------------------------
         id = LOWORD(wParam);

         switch (id)
         {
            case ID_DLGBOX_SETUP_TABMODDIR_EDIT :
            case ID_DLGBOX_SETUP_TABMODDIR_BUTN :
               dialog_ID = DLG_SETUP_TAB_MODDIR;
               break;

            case ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2 :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2 :
            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP :
            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA :
            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR :
               dialog_ID = DLG_SETUP_TAB_MPQ;
               break;
         }

         switch (id)
         {
            case ID_DLGBOX_SETUP_TABMODDIR_EDIT :
            case ID_DLGBOX_SETUP_TABMODDIR_BUTN :
               iEdit = ID_DLGBOX_SETUP_TABMODDIR_EDIT;
               break;

            case ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2 :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2 :
               iEdit = ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2;
               break;

            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP :
               iEdit = ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP;
               break;

            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA :
               iEdit = ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA;
               break;

            case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR :
            case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR :
               iEdit = ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR;
               break;
         }

         if (dialog_ID != DLG_NONE)
         {
            dlg = & myglobals.dlgbox_datas[dialog_ID].dlg;
            if (get_dialog_child_index_from_ID(dialog_ID, (ENUM_CTRL_IDENTIFIER) id, & child_idx) == 0)
               h = dlg->pChild[child_idx].handle;

            switch (HIWORD(wParam))
            {
               case EN_KILLFOCUS :
                  // edit path had just been changed
                  GetWindowText(h, path, MAX_PATH);
                  if (wcslen(path) > 0)
                  {
                     if (dialog_ID == DLG_SETUP_TAB_MODDIR)
                     {
                        if (validate_new_mod_directory_path(path) != TRUE)
                        {
                           SetWindowText(h, TEXT(""));
                           path[0] = 0;
                        }

                        n = wcslen(path);
                        if (n > 0)
                        {
                           if (path[n - 1] != TEXT('\\'))
                           {
                              wcscat(path, TEXT("\\"));
                              SetWindowText(h, path);
                           }
                        }
                     }
                     else if (dialog_ID == DLG_SETUP_TAB_MPQ)
                     {
                        if (validate_file_path(path) != TRUE)
                           SetWindowText(h, TEXT(""));
                     }
                  }
                  break;

               case BN_CLICKED :
                  // browse button had just been pushed
                  if (iEdit != ID_NULL)
                  {
                     if (get_dialog_child_index_from_ID(dialog_ID, iEdit, & child_idx) == 0)
                        hEdit = dlg->pChild[child_idx].handle;

                     if (hEdit != NULL)
                     {
                        if (dialog_ID == DLG_SETUP_TAB_MODDIR)
                        {
                           if (select_new_mod_directory(NULL, path) == TRUE)
                              SetWindowText(hEdit, path);

                           n = wcslen(path);
                           if (n > 0)
                           {
                              if (path[n - 1] != TEXT('\\'))
                              {
                                 wcscat(path, TEXT("\\"));
                                 SetWindowText(hEdit, path);
                              }
                           }
                        }
                        else if (dialog_ID == DLG_SETUP_TAB_MPQ)
                        {
                           m = get_mpq_enum_from_id(id);
                           if (m != -1)
                           {
                              if (select_new_mpq_file(NULL, path, m) == TRUE)
                                 SetWindowText(hEdit, path);
                           }
                        }
                     }
                  }
                  break;
            }
         }
         break;
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// test if the path is a valid Mod directory
// return TRUE if it's valid
// ===========================================================================
int validate_new_mod_directory_path(WCHAR * path)
{
   int   length              = 0;
   WCHAR tmp_path [MAX_PATH] = TEXT("");


   length = wcslen(path);
   if (length <= 0)
      return FALSE;

   if (path[length - 1] == TEXT('\\'))
      path[length - 1] = 0;

   swprintf(tmp_path, MAX_PATH, TEXT("%s\\data"), path);
   return validate_directory_path(tmp_path);
}


// ===========================================================================
// open the operating system common dialog box for selecting a directory
// return TRUE if the user pushed the OK button, AND if it is a valid Mod directory
// ===========================================================================
int select_new_mod_directory(HWND hParent, WCHAR * path)
{
   BROWSEINFO       bi;
   PIDLIST_ABSOLUTE res;


   if (path == NULL)
      return FALSE;

   memset( & bi, 0, sizeof(bi));
   bi.hwndOwner      = hParent;
   bi.pidlRoot       = NULL;
   bi.pszDisplayName = path;
   bi.lpszTitle      = STR_SELECT_MOD_DIRECTORY_TITLE;
   bi.ulFlags        = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
   bi.lpfn           = NULL;
   bi.lParam         = 0;
   bi.iImage         = 0;

   res = SHBrowseForFolder( & bi); // http://msdn.microsoft.com/en-us/library/bb762115%28v=vs.85%29.aspx
   if (res == NULL)
      return FALSE;

   SHGetPathFromIDList(res, path);
   DESTROYME(res, CoTaskMemFree)

   if (validate_new_mod_directory_path(path) != TRUE)
   {
      path[0] = TEXT('\0');
      return FALSE;
   }

   return TRUE;
}


// ===========================================================================
// open the operating system common dialog box for selecting a file
// return TRUE if the user pushed the OK button
// ===========================================================================
int select_new_mpq_file(HWND hParent, WCHAR * path, int m)
{
   WCHAR        title   [300] = TEXT("");
   WCHAR        filters [300] = TEXT("");
   WCHAR        z             = TEXT('\0');
   WCHAR        * f           = NULL;
   OPENFILENAME ofn;
   int          has_changed   = FALSE;


   MYASSERT_RETURN((m == MPQ_PATCH_D2) || (m == MPQ_D2EXP) || (m == MPQ_D2DATA) || (m == MPQ_D2CHAR), FALSE, NULL);
   memset( & ofn, 0, sizeof(ofn));
   ofn.lStructSize       = sizeof (OPENFILENAME);
   ofn.hwndOwner         = hParent;
   ofn.hInstance         = NULL;
   ofn.lpstrFilter       = filters;
   ofn.lpstrCustomFilter = NULL;
   ofn.nMaxCustFilter    = 0;
   ofn.nFilterIndex      = 1;
   ofn.lpstrFile         = path;
   ofn.nMaxFile          = MAX_PATH;
   ofn.lpstrFileTitle    = NULL;
   ofn.nMaxFileTitle     = 0;
   ofn.lpstrInitialDir   = NULL;
   ofn.lpstrTitle        = title;
   ofn.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
   ofn.nFileOffset       = 0;
   ofn.nFileExtension    = 0;
   ofn.lpstrDefExt       = NULL;
   ofn.lCustData         = 0;
   ofn.lpfnHook          = NULL;
   ofn.lpTemplateName    = NULL;
   ofn.pvReserved        = 0;
   ofn.dwReserved        = 0;
   ofn.FlagsEx           = 0;

   f = myglobals.datas.mpq[m].filename;
   swprintf(filters, 300, TEXT("%s%c%s%cMPQ Files%c*.MPQ%cAll Files%c*.*%c%c"), f, z, f, z, z, z, z, z, z);
   swprintf(title, 300, STR_SELECT_MPQ_FILE_FORMAT, f);
   if (GetOpenFileName( & ofn) != 0) // http://msdn.microsoft.com/en-us/library/ms646927%28v=vs.85%29.aspx
      has_changed = TRUE;

   return has_changed;
}


// ===========================================================================
// the user may have choosed new mpq (and mod directory) paths, copy them
// while the dialogs datas are still available
// return 0 on succees
// ===========================================================================
int update_application_with_new_setup_paths(int * has_truly_changed)
{
   int                  i                     = 0;
   WCHAR                path [MAX_PATH]       = TEXT("");
   ENUM_CTRL_IDENTIFIER id_mpq_path [MPQ_MAX] = {ID_DLGBOX_SETUP_TABMODDIR_EDIT, ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR};
   HWND                 h_mpq_path  [MPQ_MAX] = {NULL, NULL, NULL, NULL, NULL};
   int                  child_idx             = 0;
   CREATE_DLGBOX        * dlg                 = NULL;
   ENUM_DLGBOX_ID       dialog_ID             = DLG_NONE;


   MYASSERT_RETURN(has_truly_changed != NULL, 1, NULL);
   (* has_truly_changed) = FALSE;
   for (i = 0; i < MPQ_MAX; i++)
   {
      if (i == MPQ_MOD_DIRECTORY)
         dialog_ID = DLG_SETUP_TAB_MODDIR;
      else
         dialog_ID = DLG_SETUP_TAB_MPQ;

      dlg = & myglobals.dlgbox_datas[dialog_ID].dlg;
      MYASSERT_RETURN(get_dialog_child_index_from_ID(dialog_ID, id_mpq_path[i], & child_idx) == 0, 1, NULL);
      h_mpq_path[i] = dlg->pChild[child_idx].handle;
      path[0] = 0;
      GetWindowText(h_mpq_path[i], path, MAX_PATH);
      {
         if (wcscmp(path, myglobals.datas.mpq[i].path) != 0)
            (* has_truly_changed) = TRUE;
      }
   }

   if ((* has_truly_changed) == TRUE)
   {
      // copy the new paths from the setup window
      for (i = 0; i < MPQ_MAX; i++)
      {
         path[0] = 0;
         GetWindowText(h_mpq_path[i], path, MAX_PATH);
         wcscpy(myglobals.datas.mpq[i].path, path);
      }
   }

   return 0;
}


// ===========================================================================
// initialise the mpq and Mod directory paths
// return 0 on succees
// ===========================================================================
int init_dlgbox_setup_paths(void)
{
   int                  i                     = 0;
   ENUM_CTRL_IDENTIFIER id_mpq_path [MPQ_MAX] = {ID_DLGBOX_SETUP_TABMODDIR_EDIT, ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA, ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR};
   HWND                 h                     = NULL;
   int                  child_idx             = 0;
   CREATE_DLGBOX        * dlg                 = NULL;
   ENUM_DLGBOX_ID       dialog_ID             = DLG_NONE;


   for (i = 0; i < MPQ_MAX; i++)
   {
      if (i == MPQ_MOD_DIRECTORY)
         dialog_ID = DLG_SETUP_TAB_MODDIR;
      else
         dialog_ID = DLG_SETUP_TAB_MPQ;

      dlg = & myglobals.dlgbox_datas[dialog_ID].dlg;
      MYASSERT_RETURN(get_dialog_child_index_from_ID(dialog_ID, id_mpq_path[i], & child_idx) == 0, 1, NULL);
      h = dlg->pChild[child_idx].handle;
      MYASSERT_RETURN(h != NULL, 1, "h = dlg->pChild[child_idx].handle");
      SetWindowText(h, myglobals.datas.mpq[i].path);
   }

   return 0;
}
