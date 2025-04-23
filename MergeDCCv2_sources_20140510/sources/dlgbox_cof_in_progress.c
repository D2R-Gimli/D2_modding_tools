#include "one_include_to_rule_them_all.h"

#include <windows.h>
#include <commctrl.h>
#include "defines.h"
#include "dlgbox_cof_in_progress.h"
#include "dialog_maker.h"
#include "misc.h"
#include "strings_define.h"
#include "tools.h"


#define  DLGBOX_COFIP_NB_CHILDREN  7

LRESULT CALLBACK callback_dlgbox_cof_in_progress (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// ===========================================================================
// create the dialog box window (with its children) : COF in progress
// return 0 on success
// ===========================================================================
int create_dlgbox_COF_in_progress(void)
{
   ENUM_DLGBOX_ID       dialog_ID = DLG_COF_IN_PROGRESS;
   int                  dwidth    = 420;
   int                  dheight   = 240;
   WNDCLASSEX           cw;
   DLGBOX_DATAS         * dd      = NULL;
   CREATE_DLGBOX        * d       = NULL;
   int                  ret       = 0;
   CREATE_DLGBOX_CHILD  c [DLGBOX_COFIP_NB_CHILDREN] =
   {
      // iChildID                   lpClassName     left top  width height             dwStyle                                                                                                       dwExStyle lpWindowName                  iFontID          lpParam pFuncInitChild           dwFuncInitChildParam    lpFuncInitChildParam handle
      // -------------------------  --------------  ---- ---  ----- -----------------  ------------------------------------------------------------------------------------------------------------  --------- ----------------------------  ---------------  ------- -----------------------  ----------------------- -------------------- ------
      {ID_DLGBOX_COF_IN_P_ICON,     WC_STATIC,      20,  20,  32,   32,                WS_CHILD | WS_VISIBLE | SS_ICON,                                                                              0,        TEXT(""),                     FONT_NONE,       NULL,   dlgbox_init_static_icon, 0,                      IDI_INFORMATION,     NULL},
      {ID_DLGBOX_COF_IN_P_LABEL,    WC_STATIC,      60,  22,  340,  HEIGHT_TXT_13 * 2, WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                              0,        STR_DLGBOX_COF_IN_P_TEXT,     FONT_VERDANA_13, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_COF_IN_P_PBAR,     PROGRESS_CLASS, 20,  80,  270,  HEIGHT_TXT_12 * 2, WS_CHILD | WS_VISIBLE,                                                                                        0,        TEXT(""),                     FONT_NONE,       NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_COF_IN_P_NBFILES,  WC_STATIC,      300, 85,  100,  HEIGHT_TXT_12,     WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE,                                                            0,        TEXT(""),                     FONT_VERDANA_12, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_COF_IN_P_EXISTS_L, WC_STATIC,      20,  130, 165,  HEIGHT_TXT_13 * 5, WS_CHILD | WS_VISIBLE | SS_LEFT,                                                                              0,        STR_DLGBOX_COF_IN_P_EXISTS_L, FONT_VERDANA_13, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_COF_IN_P_EXISTS_R, WC_STATIC,      187, 130, 50,   HEIGHT_TXT_13 * 5, WS_CHILD | WS_VISIBLE | SS_RIGHT,                                                                             0,        TEXT(""),                     FONT_VERDANA_13, NULL,   NULL,                    0,                      NULL,                NULL},
      {ID_DLGBOX_COF_IN_P_OK,       WC_BUTTON,      317, 174, 86,   HEIGHT_BUT_14,     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | WS_DISABLED | BS_DEFPUSHBUTTON, 0,        TEXT("OK"),                   FONT_VERDANA_14, NULL,   NULL,                    0,                      NULL,                NULL},
   };


   dd = & myglobals.dlgbox_datas[dialog_ID];
   d  = & dd->dlg;

   if (dd->is_active == TRUE)
      return 1;

   dd->type_modal = DLGMT_MODAL;

   // window class
   cw.cbSize        = sizeof(WNDCLASSEX);
   cw.style         = CS_HREDRAW | CS_VREDRAW;
   cw.lpfnWndProc   = callback_dlgbox_cof_in_progress;
   cw.cbClsExtra    = 0;
   cw.cbWndExtra    = 0;
   cw.hInstance     = myglobals.main_instance;
   cw.hIcon         = NULL;
   cw.hCursor       = LoadCursor(NULL, IDC_ARROW);
   cw.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   cw.lpszMenuName  = NULL;
   cw.lpszClassName = STR_DLGBOX_COF_IN_P_CLASS;
   cw.hIconSm       = NULL;

   d->pWndClassEx = & cw;

   // window
   d->window.hWndParent            = myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle;
   d->window.left                  = get_x_to_center_into_screen(dwidth);
   d->window.top                   = get_y_to_center_into_screen(dheight);
   d->window.width                 = dwidth;
   d->window.height                = dheight;
   d->window.dwStyle               = WS_POPUP | WS_BORDER | WS_CAPTION | WS_VISIBLE, // | WS_SYSMENU;
   d->window.dwExStyle             = 0;
   d->window.lpWindowName          = STR_DLGBOX_COF_IN_P_WINDOW_NAME;
   d->window.hMenu                 = NULL;
   d->window.lpParam               = NULL;
   d->window.pFuncInitDialog       = NULL;
   d->window.dwFuncInitDialogParam = 0;
   d->window.lpFuncInitDialogParam = NULL;
   d->window.handle                = NULL;

   // children
   d->nbChildren = DLGBOX_COFIP_NB_CHILDREN;
   d->pChild = (CREATE_DLGBOX_CHILD *) calloc(d->nbChildren + 1, sizeof(CREATE_DLGBOX_CHILD));
   MYASSERT_RETURN(d->pChild != NULL, 1, "calloc() error");
   memcpy(d->pChild, c, sizeof(c));

   // create the dialog
   MYASSERT_RETURN(create_dialog(dialog_ID) == 0, 1, NULL);

   // show and draw it
   ShowWindow(d->window.handle, SW_SHOWNORMAL);
   UpdateWindow(d->window.handle);
   SetFocus(d->window.handle);

   // force all children to be draw right now
   MYASSERT_RETURN(UpdateWindow_for_all_children(dialog_ID) == 0, 1, NULL);

   return ret;
}


// ===========================================================================
// callback function for the dialog box : COF in progress
// ===========================================================================
LRESULT CALLBACK callback_dlgbox_cof_in_progress(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ENUM_DLGBOX_ID  dialog_ID = DLG_COF_IN_PROGRESS;
   DLGBOX_DATAS    * dd      = NULL;
   int             id        = ID_NULL;
   WORD            w         = 0;


   dd = & myglobals.dlgbox_datas[dialog_ID];
   switch(msg)
   {

      case WM_SYSCOMMAND : // user selected the Exit Sysmenu command ------------------------
         w = (wParam & 0xFFF0);
         if (w == SC_CLOSE)
            dd->is_closing = TRUE;
         break;

      case WM_CTLCOLORSTATIC: // STATIC -----------------------------------------------------
         SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
         SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
         return (LRESULT) GetSysColorBrush(COLOR_BTNFACE);

      case WM_COMMAND : // BUTTON, LISTBOX, EDIT ---------------------------------------------
         id  = LOWORD(wParam);
         if (id == IDOK)
         {
            dd->is_closing = TRUE;
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
         }
         else if (HIWORD(wParam) == BN_CLICKED)
         {
            switch (id)
            {
               case ID_DLGBOX_COF_IN_P_OK : 
                  dd->is_closing = TRUE;
                  SendMessage(hwnd, WM_CLOSE, 0, 0);
                  break;
            }
         }
         break;

      case WM_DESTROY: // close the dialog box ----------------------------------------------
         if (dd->is_closing == TRUE)
            close_dialog(dialog_ID);
         break;
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}
