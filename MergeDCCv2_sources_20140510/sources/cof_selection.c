#include "one_include_to_rule_them_all.h"

#include <windows.h>
#include "enums.h"
#include "misc.h"
#include "tools.h"


// ===========================================================================
// a COF selection listbox have been selected, update all necessary listbox datas
// ===========================================================================
void cof_selection_listbox_clicked(ENUM_COFSELTYPE type, int * is_new_COF)
{
   int               reload_tokens         = FALSE;
   int               reload_weapon_classes = FALSE;
   int               reload_modes          = FALSE;
   int               i                     = 0;
   MAIN_DATAS        * md                  = NULL;
   HWND              hanimtype             = NULL;
   HWND              htoken                = NULL;
   HWND              hweapclass            = NULL;
   HWND              hmodes                = NULL;
   COF_SELECTION     * cs                  = NULL;
   LISTFILE_DATAS    * ld                  = NULL;
   LISTFILE_ANIMTYPE * la                  = NULL;
   TOKEN_DATAS       * td                  = NULL;
   WEAPCLASS_DATAS   * wd                  = NULL;
   WCHAR             wCode [10]            = TEXT("");
   int               pos                   = 0;
   int               selected              = 0;
   CREATE_DLGBOX     * dlg                 = NULL;
   int               child_idx             = 0;


   if (is_new_COF == NULL)
      return;

   (* is_new_COF) = FALSE;

   if (type != CST_NONE)
   {
      if ((type < 0) || (type >= CST_MAX))
         return;
   }

   md         = & myglobals.datas;
   ld         = & myglobals.listfile_datas;
   cs         = & myglobals.cof_selection;

   dlg = & myglobals.dlgbox_datas[DLG_MAIN].dlg;

   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_ANIMTYPE_LIST, & child_idx) == 0)
      hanimtype = dlg->pChild[child_idx].handle;

   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_TOKEN_LIST, & child_idx) == 0)
      htoken = dlg->pChild[child_idx].handle;

   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_WEAPCLASS_LIST, & child_idx) == 0)
      hweapclass = dlg->pChild[child_idx].handle;

   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_MODES_LIST, & child_idx) == 0)
      hmodes = dlg->pChild[child_idx].handle;

   // get listbox row currently selected
   i = LB_ERR;
   if (type == CST_NONE)
      i = 0;
   else
   {
      switch (type)
      {
         case CST_ANIMTYPE  : i = SendMessage(hanimtype,  LB_GETCURSEL, 0, (LPARAM) NULL); break;
         case CST_TOKEN     : i = SendMessage(htoken,     LB_GETCURSEL, 0, (LPARAM) NULL); break;
         case CST_WEAPCLASS : i = SendMessage(hweapclass, LB_GETCURSEL, 0, (LPARAM) NULL); break;
         case CST_MODES     : i = SendMessage(hmodes,     LB_GETCURSEL, 0, (LPARAM) NULL); break;
      }
   }

   if (i == LB_ERR)
      return;

   // decide what to do
   if ((type == CST_ANIMTYPE) || (type == CST_NONE))
   {
      if (type == CST_ANIMTYPE)
      {
         if (i == cs->animation_type_idx)
            return;
      }

      // new animation type, all tokens needs to be reloaded. reinitialise the prefered weapon class and mode
      cs->animation_type_idx = i;
      reload_tokens = TRUE;
      strcpy(cs->weapon_class_desired_code, "");
      strcpy(cs->mode_desired_code, "");
   }
   else if (type == CST_TOKEN)
   {
      if (i == cs->token_idx)
         return;

      // new token, all weapon classes needs to be reloaded
      cs->token_idx = i;
      reload_weapon_classes = TRUE;
   }
   else if (type == CST_WEAPCLASS)
   {
      if (i == cs->weapon_class_idx)
         return;

      // new weapon class, all modes needs to be reloaded, current weapon class become the new prefered one
      cs->weapon_class_idx = i;
      reload_modes = TRUE;
      la = & ld->at[cs->animation_type_idx];
      td = & la->tab_token[cs->token_idx];
      wd = & td->tab_weapon_class[i];
      strcpy(cs->weapon_class_desired_code, wd->code);
   }
   else if (type == CST_MODES)
   {
      if (i == cs->mode_idx)
         return;

      // new mode, current mode become the new prefered one
      cs->mode_idx = i;
      la = & ld->at[cs->animation_type_idx];
      td = & la->tab_token[cs->token_idx];
      wd = & td->tab_weapon_class[cs->weapon_class_idx];
      strcpy(cs->mode_desired_code, wd->tab_mode_code[i]);
   }

   // update listboxes in chain, where needed

   la = & ld->at[cs->animation_type_idx];
   if (reload_tokens == TRUE)
   {
      // reload tokens
      SendMessage(htoken, LB_RESETCONTENT, 0, (LPARAM) NULL);
      for (i = 0; i < la->nb_tokens; i++)
      {
         char_to_wide_char(la->tab_token[i].code, wCode, sizeof(wCode));
         pos = SendMessage(htoken, LB_ADDSTRING, (WPARAM) 0, (LPARAM) wCode);
         MYASSERT((pos != LB_ERR) && (pos != LB_ERRSPACE), "SendMessage() with LB_ADDSTRING");
      }

      cs->token_idx = 0;
      SendMessage(htoken, LB_SETCURSEL, (WPARAM) 0, (LPARAM) NULL);
      reload_weapon_classes = TRUE;
   }

   if (la->tab_token != NULL)
   {
      td = & la->tab_token[cs->token_idx];
      if (reload_weapon_classes == TRUE)
      {
         // reload weapon classes. select the prefered weapon class if it exists, else the first row
         SendMessage(hweapclass, LB_RESETCONTENT, 0, (LPARAM) NULL);
         selected = 0;
         for (i = 0; i < td->nb_weapon_class; i++)
         {
            char_to_wide_char(td->tab_weapon_class[i].code, wCode, sizeof(wCode));
            pos = SendMessage(hweapclass, LB_ADDSTRING, (WPARAM) 0, (LPARAM) wCode);
            MYASSERT((pos != LB_ERR) && (pos != LB_ERRSPACE), "SendMessage() with LB_ADDSTRING");
            if (stricmp(td->tab_weapon_class[i].code, cs->weapon_class_desired_code) == 0)
               selected = pos;
         }

         cs->weapon_class_idx = selected;
         SendMessage(hweapclass, LB_SETCURSEL, (WPARAM) selected, (LPARAM) NULL);
         reload_modes = TRUE;
      }

      if (td->tab_weapon_class != NULL)
      {
         wd = & td->tab_weapon_class[cs->weapon_class_idx];
         if (reload_modes == TRUE)
         {
            // reload modes. select the prefered mode if it exists, else the first row
            SendMessage(hmodes, LB_RESETCONTENT, 0, (LPARAM) NULL);
            selected = 0;
            for (i = 0; i < wd->nb_modes; i++)
            {
               char_to_wide_char(wd->tab_mode_code[i], wCode, sizeof(wCode));
               pos = SendMessage(hmodes, LB_ADDSTRING, (WPARAM) 0, (LPARAM) wCode);
               MYASSERT((pos != LB_ERR) && (pos != LB_ERRSPACE), "SendMessage() with LB_ADDSTRING");
               if (stricmp(wd->tab_mode_code[i], cs->mode_desired_code) == 0)
                  selected = pos;
            }

            cs->mode_idx = selected;
            SendMessage(hmodes, LB_SETCURSEL, (WPARAM) selected, (LPARAM) NULL);
         }
      }
      else
      {
         // no weapon class, so no mode. should never happen
         SendMessage(hmodes, LB_RESETCONTENT, 0, (LPARAM) NULL);
         SendMessage(hmodes, LB_SETCURSEL,    0, (LPARAM) NULL);
         cs->mode_idx = 0;
      }
   }
   else
   {
      // no token, so no weapon class nor mode. can happen with no MPQ and no Mod directory paths

      SendMessage(hweapclass, LB_RESETCONTENT, 0, (LPARAM) NULL);
      SendMessage(hweapclass, LB_SETCURSEL,    0, (LPARAM) NULL);
      cs->weapon_class_idx = 0;

      SendMessage(hmodes, LB_RESETCONTENT, 0, (LPARAM) NULL);
      SendMessage(hmodes, LB_SETCURSEL,    0, (LPARAM) NULL);
      cs->mode_idx = 0;
   }

   if (type == CST_NONE)
   {
      // total reinitialisation, select the 1st element of each listbox
      SendMessage(hanimtype,  LB_SETCURSEL, (WPARAM) 0, (LPARAM) NULL);
      SendMessage(htoken,     LB_SETCURSEL, (WPARAM) 0, (LPARAM) NULL);
      SendMessage(hweapclass, LB_SETCURSEL, (WPARAM) 0, (LPARAM) NULL);
      SendMessage(hmodes,     LB_SETCURSEL, (WPARAM) 0, (LPARAM) NULL);
   }

   (* is_new_COF) = TRUE;
}
