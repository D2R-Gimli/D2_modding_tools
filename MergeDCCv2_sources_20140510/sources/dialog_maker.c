#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include "dialog_maker.h"
#include "misc.h"
#include "strings_define.h"


static void destroy_dialog (int i);


// ===========================================================================
// create a dialog box main window and its children, given a dialog ID
// return 0 on success
// ===========================================================================
int create_dialog(ENUM_DLGBOX_ID dialog_ID)
{
   DLGBOX_DATAS         * dd = NULL;
   CREATE_DLGBOX        * d  = NULL;
   CREATE_DLGBOX_WINDOW * w  = NULL;
   int                  i    = 0;
   HWND                 h    = NULL;


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   MYASSERT_RETURN(create_dialog_window(dialog_ID) == 0, 1, NULL);
   MYASSERT_RETURN(create_dialog_children(dialog_ID) == 0, 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   dd->prev_modal_window = DLG_NONE;
   dd->next_modal_window = DLG_NONE;

   dd = & myglobals.dlgbox_datas[dialog_ID];
   if (dd->type_modal == DLGMT_MODAL)
   {
      // handle the worst case when there's already (at least) another modal window
      for (i = 0; i < DLG_MAX; i++)
      {
         if (i == dialog_ID)
            continue;

         if (myglobals.dlgbox_datas[i].dlg.window.handle != NULL)
         {
            if ((myglobals.dlgbox_datas[i].type_modal == DLGMT_MODAL) && (myglobals.dlgbox_datas[i].next_modal_window == DLG_NONE))
            {
               // found current modal window, update the links
               dd->prev_modal_window = (ENUM_DLGBOX_ID) i;
               myglobals.dlgbox_datas[i].next_modal_window = dialog_ID;
               break;
            }
         }
      }

      // disable all other windows
      for (i = 0; i < DLG_MAX; i++)
      {
         if (i == dialog_ID)
            continue;

         h = myglobals.dlgbox_datas[i].dlg.window.handle;
         if (h != NULL)
            EnableWindow(h, FALSE);
      }
   }

   dd->is_active = TRUE;
   return 0;
}


// ===========================================================================
// a modal dialog window is closing. reactivate a previous modal dialog, or
// all windows if no modal dialog is in the linked list
// return 0 on success
// ===========================================================================
int close_dialog(ENUM_DLGBOX_ID dialog_ID)
{
   DLGBOX_DATAS * dd      = NULL;
   DLGBOX_DATAS * prev_dd = NULL;
   int          i         = 0;
   HWND         h         = NULL;


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];

   if (dd->type_modal == DLGMT_MODAL)
   {
      if (dd->prev_modal_window != DLG_NONE)
      {
         // a previous modal window exists, reactivate only this one
         prev_dd = & myglobals.dlgbox_datas[dd->prev_modal_window];
         prev_dd->next_modal_window = DLG_NONE;
         dd->prev_modal_window = DLG_NONE;
         h = prev_dd->dlg.window.handle;
         if (h != NULL)
         {
            EnableWindow(h, TRUE);
            BringWindowToTop(h);
            UpdateWindow(h);
            SetFocus(h);
         }
      }
      else
      {
         // there was no previously modal dialog before, so enable all dialogs right now
         for (i = 0; i < DLG_MAX; i++)
         {
            if (i == dialog_ID)
               continue;

            if (myglobals.dlgbox_datas[i].is_active == TRUE)
            {
               h = myglobals.dlgbox_datas[i].dlg.window.handle;
               if (h != NULL)
                  EnableWindow(h, TRUE);
            }
         }

         h = dd->dlg.window.hWndParent;
         if (h != NULL)
         {
            EnableWindow(h, TRUE);
            BringWindowToTop(h);
            UpdateWindow(h);
            SetFocus(h);
         }
      }
   }

   // delete all datas of this dialog
   destroy_dialog(dialog_ID);

   return 0;
}


// ===========================================================================
// send an UpdateWindow() call to all children of a dialog box (force PAINT)
// return 0 on success
// ===========================================================================
int UpdateWindow_for_all_children(ENUM_DLGBOX_ID dialog_ID)
{
   DLGBOX_DATAS         * dd = NULL;
   CREATE_DLGBOX        * d  = NULL;
   CREATE_DLGBOX_WINDOW * w  = NULL;
   CREATE_DLGBOX_CHILD  * pc = NULL;
   int                  i    = 0;


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   MYASSERT_RETURN(w->handle != NULL, 1, NULL);
   pc = d->pChild;
   MYASSERT_RETURN(pc != NULL, 1, NULL);
   for (i = 0; i < d->nbChildren; i++)
   {
      if (pc[i].handle != NULL)
         UpdateWindow(pc[i].handle);
   }

   return 0;
}


// ===========================================================================
// create the dialog box main window
// return 0 on success (in this case, myglobals.dlgbox_datas[dialog_ID].dlg.window.handle contains the dialog box window handle)
// ===========================================================================
int create_dialog_window(ENUM_DLGBOX_ID dialog_ID)
{
   DLGBOX_DATAS         * dd = NULL;
   CREATE_DLGBOX        * d  = NULL;
   CREATE_DLGBOX_WINDOW * w  = NULL;


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   MYASSERT_RETURN(w->handle == NULL, 1, NULL);

   if (myglobals.class_registered[dialog_ID] == 0)
   {
      MYASSERT_RETURN(d->pWndClassEx != NULL, 1, NULL);
      MYASSERT_RETURN(RegisterClassEx(d->pWndClassEx) != 0, 1, NULL);
      myglobals.class_registered[dialog_ID] = 1;
   }

   w->handle = CreateWindowEx(
      w->dwExStyle, // http://msdn.microsoft.com/en-us/library/ff700543%28v=vs.85%29.aspx
      d->pWndClassEx->lpszClassName,
      w->lpWindowName,
      w->dwStyle, // http://msdn.microsoft.com/en-us/library/ms632600%28v=vs.85%29.aspx
      w->left,
      w->top,
      w->width,
      w->height,
      w->hWndParent,
      w->hMenu,
      myglobals.main_instance,
      w->lpParam
   );
   MYASSERT_RETURN(w->handle != NULL, 1, NULL);

   if (w->pFuncInitDialog != NULL)
      MYASSERT_RETURN(w->pFuncInitDialog(dialog_ID, w->dwFuncInitDialogParam, w->lpFuncInitDialogParam) == 0, 1, NULL);

   return 0;
}


// ===========================================================================
// create all dialog box children controls
// return 0 on success
// ===========================================================================
int create_dialog_children(ENUM_DLGBOX_ID dialog_ID)
{
   DLGBOX_DATAS         * dd      = NULL;
   CREATE_DLGBOX        * d       = NULL;
   CREATE_DLGBOX_WINDOW * w       = NULL;
   CREATE_DLGBOX_CHILD  * pc      = NULL;
   CREATE_DLGBOX_CHILD  * c       = NULL;
   HFONT                hf        = NULL;
   int                  i         = 0;
   char                 str[1000] = "";


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   MYASSERT_RETURN(w->handle != NULL, 1, NULL);
   pc = d->pChild;
   MYASSERT_RETURN(pc != NULL, 1, NULL);
   for (i = 0; i < d->nbChildren; i++)
   {
      c = & pc[i];
      MYASSERT_RETURN(c->handle == NULL, 1, NULL);
      c->handle = CreateWindowEx(
         c->dwExStyle, // http://msdn.microsoft.com/en-us/library/ff700543%28v=vs.85%29.aspx
         c->lpClassName,
         c->lpWindowName,
         c->dwStyle, // http://msdn.microsoft.com/en-us/library/ms632600%28v=vs.85%29.aspx
         c->left,
         c->top,
         c->width,
         c->height,
         w->handle,
         (HMENU) c->iChildID,
         myglobals.main_instance,
         c->lpParam
      );
      sprintf(str, "dialog_ID=%d, pChild[%d].iChildID=%d, .lpClassName=%s, .left=%d, .top=%d", dialog_ID, i, pc[i].iChildID, pc[i].lpClassName, pc[i].left, pc[i].top);
      MYASSERT_RETURN(c->handle != NULL, 1, str);
      if ((c->iFontID >= 0) && (c->iFontID < FONT_MAX))
      {
         hf = myglobals.datas.my_font[c->iFontID].hfont;
         if (hf != NULL)
            SendMessage(c->handle, WM_SETFONT, (WPARAM) hf, FALSE);
      }

      if (c->pFuncInitChild != NULL)
         MYASSERT_RETURN(c->pFuncInitChild(dialog_ID, i, c->dwFuncInitChildParam, c->lpFuncInitChildParam) == 0, 1, NULL);
   }

   UpdateWindow(c->handle); // FIXME : really necessary ? not sure...

   return 0;
}


// ===========================================================================
// load the sysem icon into the static control
// return 0 on success
// ===========================================================================
int dlgbox_init_static_icon(ENUM_DLGBOX_ID dialog_ID, int child_idx, DWORD dw, LPVOID * lpIconName)
{
   DLGBOX_DATAS         * dd       = NULL;
   CREATE_DLGBOX        * d        = NULL;
   CREATE_DLGBOX_WINDOW * w        = NULL;
   CREATE_DLGBOX_CHILD  * pc       = NULL;
   CREATE_DLGBOX_CHILD  * c        = NULL;
   HICON                hicon_info = LoadIcon(NULL, (LPCWSTR) lpIconName);


   MYASSERT_RETURN(hicon_info != NULL, 1, NULL);
   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
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
   SendMessage(d->pChild[child_idx].handle, STM_SETICON, (WPARAM) hicon_info, (LPARAM) 0);

   return 0;
}


// ===========================================================================
// for a given dialog box ID, 
// enable or disable the child (or children) with the given child ID
// return 0 on success (if the child ID was found in this dialog)
// ===========================================================================
int modify_enable_state_by_child_ID(ENUM_DLGBOX_ID dialog_ID, ENUM_CTRL_IDENTIFIER child_ID, int is_enable)
{
   DLGBOX_DATAS         * dd       = NULL;
   CREATE_DLGBOX        * d        = NULL;
   CREATE_DLGBOX_WINDOW * w        = NULL;
   CREATE_DLGBOX_CHILD  * pc       = NULL;
   CREATE_DLGBOX_CHILD  * c        = NULL;
   int                  i          = 0;


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   dd = & myglobals.dlgbox_datas[dialog_ID];
   d = & dd->dlg;
   w = & d->window;
   MYASSERT_RETURN(w->handle != NULL, 1, NULL);
   pc = d->pChild;
   MYASSERT_RETURN(pc != NULL, 1, NULL);

   for (i = 0; i < d->nbChildren; i++)
   {
      c = & pc[i];
      MYASSERT_RETURN(c->handle != NULL, 1, NULL);
      if (c->iChildID == child_ID)
      {
         EnableWindow(c->handle, (is_enable == TRUE) ? TRUE : FALSE);
         return 0;
      }
   }

   return 1;
}


// ===========================================================================
// destroy all the dialogs datas
// ===========================================================================
void destroy_all_dialogs(void)
{
   int i = 0;


   for (i = 0; i < DLG_MAX; i++)
     destroy_dialog(i);
}


// ===========================================================================
// destroy 1 dialog datas
// ===========================================================================
void destroy_dialog(int i)
{
   int                  c    = 0;
   DLGBOX_DATAS         * dd = NULL;
   CREATE_DLGBOX        * cd = NULL;
   CREATE_DLGBOX_WINDOW * w  = NULL;


   if ((i < 0) || (i >= DLG_MAX))
      return;

   dd = & myglobals.dlgbox_datas[i];
   cd = & dd->dlg;
   w  = & cd->window;

   DESTROYME(w->window_datas, free);

   if (cd->pChild != NULL)
   {
      for (c = 0; c < cd->nbChildren; c++)
         DESTROYME(cd->pChild[c].child_datas, free);
   }

   DESTROYME(cd->dlg_datas, free)

   memset( & dd->dlg.window, 0, sizeof(CREATE_DLGBOX_WINDOW));
   memset( & dd->dlg,        0, sizeof(CREATE_DLGBOX));
   memset(dd,                0, sizeof(DLGBOX_DATAS));
}


// ===========================================================================
// for a given dialog ID, and a given child control ID of this dialog, find
// the child index in the table of children control of this dialog
// return 0 if succees
// ===========================================================================
int get_dialog_child_index_from_ID(ENUM_DLGBOX_ID dialog_ID, ENUM_CTRL_IDENTIFIER child_ID, int * child_idx)
{
   CREATE_DLGBOX * d = NULL;
   int           i   = -1;


   MYASSERT_RETURN(child_idx != NULL, 1, NULL);
   (* child_idx) = -1;
   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   d = & myglobals.dlgbox_datas[dialog_ID].dlg;
   MYASSERT_RETURN(d->pChild != NULL, 1, NULL);
   for (i = 0; i < d->nbChildren; i++)
   {
      if (d->pChild[i].iChildID == child_ID)
      {
         (* child_idx) = i;
         return 0;
      }
   }

   return 1;
}
