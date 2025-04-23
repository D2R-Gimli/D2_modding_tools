#ifndef __DIALOG_MAKER_H__
#define __DIALOG_MAKER_H__

#include "one_include_to_rule_them_all.h"

#include <windows.h>
#include "enums.h"


typedef struct CREATE_DLGBOX_WINDOW_S CREATE_DLGBOX_WINDOW;
typedef struct CREATE_DLGBOX_CHILD_S  CREATE_DLGBOX_CHILD;
typedef struct CREATE_DLGBOX_S        CREATE_DLGBOX;

typedef struct CREATE_DLGBOX_WINDOW_S
{
   HWND           hWndParent;
   int            left;
   int            top;
   int            width;
   int            height;
   DWORD          dwStyle;
   DWORD          dwExStyle;
   LPCTSTR        lpWindowName;
   HMENU          hMenu;
   LPVOID         lpParam;
   int            (* pFuncInitDialog)(ENUM_DLGBOX_ID dialog_ID, DWORD dwParam, LPVOID lpParam); // must return 0 on success
   DWORD          dwFuncInitDialogParam;
   LPVOID         lpFuncInitDialogParam;
   HWND           handle; // initial value is ignored (should be NULL anyway). Will be set only at run-time when the control will be created by CreateWindowEx()
   void           * window_datas;
} CREATE_DLGBOX_WINDOW;

typedef struct CREATE_DLGBOX_CHILD_S
{
   int          iChildID;
   LPCTSTR      lpClassName;
   int          left;
   int          top;
   int          width;
   int          height;
   DWORD        dwStyle;
   DWORD        dwExStyle;
   LPCTSTR      lpWindowName;
   ENUM_MY_FONT iFontID;
   LPVOID       lpParam;
   int          (* pFuncInitChild)(ENUM_DLGBOX_ID dialog_ID, int iChildIdx, DWORD dwParam, LPVOID lpParam); // must return 0 on success
   DWORD        dwFuncInitChildParam;
   LPVOID       lpFuncInitChildParam;
   HWND         handle; // initial value is ignored (should be NULL anyway). Will be set only at run-time when the control will be created by CreateWindowEx()
   void         * child_datas;
} CREATE_DLGBOX_CHILD;

typedef struct CREATE_DLGBOX_S
{
   WNDCLASSEX           * pWndClassEx;
   CREATE_DLGBOX_WINDOW window;
   int                  nbChildren;
   CREATE_DLGBOX_CHILD  * pChild; // pointer to a table of CREATE_DLGBOX_CHILD, having nbChildren elements
   void                 * dlg_datas;
} CREATE_DLGBOX;

int  create_dialog                   (ENUM_DLGBOX_ID dialog_ID);
int  close_dialog                    (ENUM_DLGBOX_ID dialog_ID);
int  UpdateWindow_for_all_children   (ENUM_DLGBOX_ID dialog_ID);
int  create_dialog_window            (ENUM_DLGBOX_ID dialog_ID);
int  create_dialog_children          (ENUM_DLGBOX_ID dialog_ID);
int  dlgbox_init_static_icon         (ENUM_DLGBOX_ID dialog_ID, int child_idx, DWORD dw, LPVOID lpIconName);
int  modify_enable_state_by_child_ID (ENUM_DLGBOX_ID dialog_ID, ENUM_CTRL_IDENTIFIER child_ID, int is_enable);
void destroy_all_dialogs             (void);
int  get_dialog_child_index_from_ID  (ENUM_DLGBOX_ID dialog_ID, ENUM_CTRL_IDENTIFIER child_ID, int * child_idx);

#endif
