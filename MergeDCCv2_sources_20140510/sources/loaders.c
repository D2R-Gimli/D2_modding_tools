#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include "loaders.h"
#include "misc.h"
#include "file_cache.h"
#include "mpq_handler.h"
#include "strings_define.h"
#include "tools.h"
#include "listfiles.h"
#include "dlgbox_main.h"
#include "d2_files.h"

// ----------------------------
//#define DEBUG_DCC_DECODE_TIME
// ----------------------------


// ===========================================================================
// decode the selected COF, unless it was alreay extracted and decoded
// return 0 on succees
// ===========================================================================
int decode_COF(long iat, char * token, char * weaponclass, char * mode, char * buffer, long length)
{
   LISTFILE_ANIMTYPE * pAt    = NULL;
   COF_ROW_EXISTS    * pCof   = NULL;
   DCC_ROW_EXISTS    * pDcc   = NULL;
   long              c        = 0;
   unsigned char     * b      = NULL;
   int               i        = 0;
   long              nb       = 0;
   long              d        = 0;
   LAYER_DATAS       * pLayer = NULL;


   myglobals.cof_selection.current_cof = NULL;

   pAt = & myglobals.listfile_datas.at[iat];
   for (c = 0; c < pAt->nb_cofs; c++)
   {
      pCof = & pAt->tab_COF[c];
      if (pCof->exists != TRUE)
         continue;

      if (    (strcmp(pCof->token,        token      ) == 0)
           && (strcmp(pCof->weapon_class, weaponclass) == 0)
           && (strcmp(pCof->mode,         mode       ) == 0) )
      {
         break;
      }
   }

   if (c >= pAt->nb_cofs)
      return 1;

   if (pCof->already_decoded == TRUE)
   {
      myglobals.cof_selection.current_cof = pCof;
      return 0;
   }

   if (length < 0x1C)
      return 1;

   b = (unsigned char *) buffer;

   pCof->nb_layers               = b[0];
   pCof->nb_frames_per_direction = b[1];
   pCof->nb_directions           = b[2];
   pCof->speed                   = * ((long *) (b + 0x18));

   nb = pCof->nb_directions * pCof->nb_layers * pCof->nb_frames_per_direction;
   if (length < 0x1C + (pCof->nb_layers * 9) + pCof->nb_frames_per_direction + nb)
      return 1;

   pCof->layer_datas = (LAYER_DATAS *) calloc(pCof->nb_layers, sizeof(LAYER_DATAS));
   MYASSERT_RETURN(pCof->layer_datas != NULL, 1, NULL);

   b += 0x1C;
   for (i = 0; i < pCof->nb_layers; i++)
   {
      if (b[0] >= 16)
         return 1;

      pCof->layer_datas[i].code_idx               = b[0];
      strcpy(pCof->layer_datas[i].code_char,  myglobals.application_datas.tab_layer[ b[0] ].code_char);
      wcscpy(pCof->layer_datas[i].code_wchar, myglobals.application_datas.tab_layer[ b[0] ].code);
      pCof->layer_datas[i].shadow                 = b[1];
      pCof->layer_datas[i].selectable             = b[2];
      pCof->layer_datas[i].transparency           = b[3];
      pCof->layer_datas[i].transparency_type      = (ENUM_LAYER_EFFECT_TYPE) b[4];
      pCof->layer_datas[i].transparency_type_user = (ENUM_LAYER_EFFECT_TYPE) LAYEREFFECT_NOT_INITIALIZED;
      pCof->layer_datas[i].special_effect_level   = -1; // -1 = "not initialised yet"
      strncpy(pCof->layer_datas[i].weapon_class_in_cof, (char *) (b + 5), 3);
      pCof->layer_datas[i].weapon_class_in_cof[3] = 0;
      make_string_uppercase(pCof->layer_datas[i].weapon_class_in_cof);

      b += 9;
   }

   memcpy(pCof->frame_event, b, pCof->nb_frames_per_direction);
   b += pCof->nb_frames_per_direction;

   pCof->priority = (unsigned char *) malloc(nb);
   MYASSERT_RETURN(pCof->priority != NULL, 1, NULL);
   memcpy(pCof->priority, b, nb);

   // search the start of the list of variants for each COF layer, using the weapon class in the COF, NOT the weapon class from the COF filename
   for (i = 0; i < pCof->nb_layers; i++)
   {
      pLayer = & pCof->layer_datas[i];
      for (d = 0; d < pAt->nb_dcc; d++)
      {
        pDcc = & pAt->tab_DCC[d];
        if (pDcc->exists != TRUE)
            continue;

        if ( (strcmp(pDcc->token,        token                      ) == 0) &&
             (strcmp(pDcc->weapon_class, pLayer->weapon_class_in_cof) == 0) &&
             (strcmp(pDcc->mode,         mode                       ) == 0) &&
             (strcmp(pDcc->layer,        pLayer->code_char          ) == 0) )
        {
           pLayer->first_variant = pDcc;
           strcpy(pLayer->variant, pDcc->variant);
           break;
        }
      }
   }

   // done
   pCof->already_decoded = TRUE;
   myglobals.cof_selection.current_cof = pCof;

   return 0;
}


// ===========================================================================
// load the current DCC of a layer
// ===========================================================================
void load_layer_DCC(int i, ANIMATION_DCC * dcc, int * update_cache)
{
   char           filename [MAX_PATH] = "";
   long           iat                 = myglobals.cof_selection.animation_type_idx;
   long           it                  = myglobals.cof_selection.token_idx;
   long           iwc                 = myglobals.cof_selection.weapon_class_idx;
   long           im                  = myglobals.cof_selection.mode_idx;
   char           * root [AT_MAX]     = {D2_ROOT_PATH_PLAYERS, D2_ROOT_PATH_MONSTERS, D2_ROOT_PATH_OBJECTS};
   char           * token             = NULL;
   char           * weaponclass       = NULL;
   char           * mode              = NULL;
   char           * buffer            = NULL;
   long           length              = 0;
   COF_ROW_EXISTS * pCof              = myglobals.cof_selection.current_cof;
   int            layer_idx           = 0;
   int            found               = FALSE;
   int            is_dc6              = 0;
   int            n                   = 0;


   token       = myglobals.listfile_datas.at[iat].tab_token[it].code;
   weaponclass = myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class[iwc].code;
   mode        = myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class[iwc].tab_mode_code[im];

   sprintf_s(
      filename,
      sizeof(filename),
      "%s%s\\%s\\%s%s%s%s%s.dcc",
      root[iat],
      token,
      pCof->layer_datas[i].code_char,
      token,
      pCof->layer_datas[i].code_char,
      pCof->layer_datas[i].variant,
      mode,
      pCof->layer_datas[i].weapon_class_in_cof
   );

   if (load_file(filename, & buffer, & length, TRUE) == 0)
      found = TRUE;
   else
   {
      n = strlen(filename);
      if (n > 1)
      {
         filename[n - 1] = '6';
         if (load_file(filename, & buffer, & length, TRUE) == 0)
         {
            found  = TRUE;
            is_dc6 = 1;
         }
      }
   }

   if (found == TRUE)
   {
      (* update_cache) = TRUE;
      layer_idx = pCof->layer_datas[i].code_idx;
      if ((layer_idx >= 0) && (layer_idx < 16))
      {
         if (dcc->file != NULL)
            DESTROYME(dcc->file, free) // should never happen, but just in case

         dcc->is_dc6            = is_dc6;
         dcc->file              = buffer;
         dcc->length            = length;
         dcc->layer_idx         = pCof->layer_datas[i].code_idx;
         dcc->shadows           = pCof->layer_datas[i].shadow;
         dcc->transparency      = pCof->layer_datas[i].transparency;
         dcc->transparency_type = pCof->layer_datas[i].transparency_type;

         if (pCof->layer_datas[i].transparency_type_user == LAYEREFFECT_NOT_INITIALIZED)
         {
            pCof->layer_datas[i].transparency_type_user = (ENUM_LAYER_EFFECT_TYPE) LAYEREFFECT_NONE;
            if (dcc->transparency != 0)
               pCof->layer_datas[i].transparency_type_user = pCof->layer_datas[i].transparency_type;
         }

         dcc->transparency_type_user = pCof->layer_datas[i].transparency_type_user;

         if (pCof->layer_datas[i].special_effect_level == -1)
            pCof->layer_datas[i].special_effect_level = 45;

         dcc->special_effect_level = pCof->layer_datas[i].special_effect_level;
         if ((dcc->special_effect_level < 0) || (dcc->special_effect_level > 255))
            dcc->special_effect_level = 0;
      }
   }
}


// ===========================================================================
// for the COF currently selected, load all necessary files
// ===========================================================================
void load_current_cof(void)
{
   char                    filename [MAX_PATH] = "";
   long                    iat                 = myglobals.cof_selection.animation_type_idx;
   long                    it                  = myglobals.cof_selection.token_idx;
   long                    iwc                 = myglobals.cof_selection.weapon_class_idx;
   long                    im                  = myglobals.cof_selection.mode_idx;
   char                    * root [AT_MAX]     = {D2_ROOT_PATH_PLAYERS, D2_ROOT_PATH_MONSTERS, D2_ROOT_PATH_OBJECTS};
   char                    * token             = NULL;
   char                    * weaponclass       = NULL;
   char                    * mode              = NULL;
   char                    * buffer            = NULL;
   long                    length              = 0;
   int                     i                   = 0;
   COF_ROW_EXISTS          * pCof              = NULL;
   int                     update_cache        = FALSE;
   ANIMATION_LAYER_LIST    dcc_list;
   ANIMATION_DCC           * dcc               = NULL;
   int                     layer_idx           = 0;
   ANIMDATA_D2_RECORD      * record            = NULL;
   long                    animdatad2_speed    = 0;
   CREATE_ANIMATION_PARAMS anim_params;


   memset( & dcc_list, 0, sizeof(dcc_list));

   if (myglobals.animation != NULL)
   {
      // destroy the current animation
      stop_tick_25fps(1, 0);
      reset_tick_25fps();
      DESTROYME(myglobals.animation, destroy_animation)
   }

   if (myglobals.listfile_datas.at[iat].tab_token == NULL)
   {
      // no token available for this animation type
      myglobals.cof_selection.current_cof = NULL;
      init_layer_controls();
      clear_animation();
      update_animation_nb_colors_control();
      return;
   }

   if (myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class == NULL)
   {
      // no weapon class available for this token
      myglobals.cof_selection.current_cof = NULL;
      init_layer_controls();
      clear_animation();
      update_animation_nb_colors_control();
      return;
   }

   if (myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class[iwc].tab_mode_code == NULL)
   {
      // no mode available for this weapon class
      myglobals.cof_selection.current_cof = NULL;
      init_layer_controls();
      clear_animation();
      update_animation_nb_colors_control();
      return;
   }

   // extract the COF

   token       = myglobals.listfile_datas.at[iat].tab_token[it].code;
   weaponclass = myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class[iwc].code;
   mode        = myglobals.listfile_datas.at[iat].tab_token[it].tab_weapon_class[iwc].tab_mode_code[im];

   sprintf_s(
      filename,
      sizeof(filename),
      "%s%s\\cof\\%s%s%s.cof",
      root[iat],
      token,
      token,
      mode,
      weaponclass
   );

   if (load_file(filename, & buffer, & length, TRUE) == 0)
      update_cache = TRUE;

   // decode the COF datas, if not previously decoded
   if (decode_COF(iat, token, weaponclass, mode, buffer, length) != 0)
   {
      // error
      DESTROYME(buffer, free)
      myglobals.cof_selection.current_cof = NULL;
      init_layer_controls();
      if (update_cache == TRUE)
         debug_cache();
      clear_animation();
      update_animation_nb_colors_control();
      return;
   }

   pCof = myglobals.cof_selection.current_cof;

   DESTROYME(buffer, free)
   length = 0;

   // load all DCC
   if (pCof != NULL)
   {
      for (i = 0; i < pCof->nb_layers; i++)
      {
         layer_idx = pCof->layer_datas[i].code_idx;
         if ((layer_idx >= 0) && (layer_idx < 16))
         {
            dcc = & dcc_list.layer[layer_idx];
            load_layer_DCC(i, dcc, & update_cache);
            // TODO : this seems the best moment to decode this DCC (or DC6) using dcc jobs... it needs the rewriting of some codes tough
            //        especially : an ANIMATION_DCC memory (and ->dcc->file) should NOT be free() before the decoding is finished !
         }
      }
   }

   // animation speed in AnimData.d2

   record = get_animdata_d2_record(pCof);
   if (record != NULL)
      animdatad2_speed = record->animation_speed;

   if (animdatad2_speed != 0)
      pCof->speed = animdatad2_speed;

   // adjust speed for Objects
   if (iat == AT_OBJECTS)
   {
      long tmp_speed = 0;

      tmp_speed = get_ObjectsTXT_speed(pCof->token, pCof->mode);
      // FIXME : it seems we also need to override the pCof->nb_frames_per_direction too, by FrameCnt0 to FrameCnt7 from Objects.txt
      //         also ... MAYBE Start0 to Start7 are also necessary ?
      //         Xoffset and Yoffset ARE both necessary
      // FIXME : or use values in AnimData.d2 for speed ? all of these speeds should be in the hand of the user anyway

      if (tmp_speed != 0)
         pCof->speed = tmp_speed;
   }

   memset( & anim_params, 0, sizeof(anim_params));

   anim_params.layer_list              = & dcc_list;
   anim_params.nb_directions           = pCof->nb_directions;
   anim_params.nb_frames_per_direction = pCof->nb_frames_per_direction;
   anim_params.nb_layers               = pCof->nb_layers;
   anim_params.speed                   = pCof->speed;
   anim_params.animdatad2_speed        = animdatad2_speed;
   anim_params.priority                = pCof->priority;
   anim_params.pl2                     = (UBYTE *) myglobals.datas.d2_ressource[D2R_ACT1PAL].buffer; // FIXME use current PL2, not only act 1

   // decode all DCC
   myglobals.animation = create_animation( & anim_params);

   // update layers control with current selected COF datas
   init_layer_controls();

   // update cache debug window if needed
   if (update_cache == TRUE)
      debug_cache();

   // if there's nothing to animate, stop the animation timer, else start it
   if (myglobals.animation != NULL)
      start_tick_25fps(1, 0);
   else
   {
      stop_tick_25fps(1, 0);
      reset_tick_25fps();
      clear_animation();
   }

#ifdef DEBUG_DCC_DECODE_TIME
   // debug of dcc_decode() bench
   {
      char   buffer[2000] = "";
      double duration     = anim_params.dcc_decode_nb_clocks;

      sprintf_s(buffer, sizeof(buffer), "decoding DCC took %0.2f seconds.\n\nThere are %ld system ticks per second, and there was %ld ticks during decoding.", duration / CLOCKS_PER_SEC, CLOCKS_PER_SEC, anim_params.dcc_decode_nb_clocks);
      MessageBoxA(NULL, buffer, "Debug information", MB_ICONINFORMATION | MB_OK);
   }
#endif

   // DCC have been decoded, we can free their memory
   for (i = 0; i < pCof->nb_layers; i++)
   {
      layer_idx = pCof->layer_datas[i].code_idx;
      dcc = & dcc_list.layer[layer_idx];
      DESTROYME(dcc->file, free)
   }

   // show how many distinct colors are used in the animation
   update_animation_nb_colors_control();
}


// ===========================================================================
// update the control displaying the number of colors in the entire animation
// ===========================================================================
void update_animation_nb_colors_control(void)
{
   WCHAR wchar_buffer [200] = TEXT("");
   int   child_idx          = -1;
   HWND  h                  = NULL;
   int   nb_anim_colors     = 0;


   nb_anim_colors = get_anim_nb_colors(myglobals.animation);
   swprintf(wchar_buffer, 200, STR_DLGBOX_MAIN_NB_COLORS_FORMAT, nb_anim_colors);
   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_NB_ANIM_COLORS, & child_idx) == 0)
   {
      h = myglobals.dlgbox_datas[DLG_MAIN].dlg.pChild[child_idx].handle;
      SendMessage(h, WM_SETTEXT, 0, (LPARAM) wchar_buffer);
   }
}


// ===========================================================================
// for a given MPQ internal full path file, load it from several places until
// it found it, or all places have been tried
// note : either from cache or mod directory or from an MPQ, the caller is
// responsible for freeing the memory when it don't needs it anymore
// ===========================================================================
int load_file(char * filename, char ** buffer, long * length, int save_in_cache)
{
   int  d             = 0;
   void * file_cached = NULL;


   if ((filename == NULL) || (buffer == NULL) || (length == NULL))
      return 1;

   (* buffer) = NULL;
   (* length) = 0;

   // try to extract from cache first
   file_cached = get_file_from_cache(filename, length, (ENUM_MPQ *) & d);
   if (file_cached != NULL)
   {
      // found from cache
      (* buffer) = (char *) calloc(1, (* length) + 1);
      if ((* buffer) == NULL)
         return 1;

      memcpy((* buffer), file_cached, (* length));
      return 0;
   }

   for (d = 0; d < MPQ_MAX; d++)
   {
      if (d == MPQ_MOD_DIRECTORY)
      {
         if (load_file_from_mod_directory((ENUM_MPQ) d, filename, buffer, length) == 0)
         {
            // found
            if (save_in_cache == TRUE)
            {
               insert_file_in_cache(filename, (* length), (* buffer), (ENUM_MPQ) d);
               keep_cache_under_limit();
            }
            return 0;
         }
      }
      else
      {
         if (load_file_from_mpq((ENUM_MPQ) d, filename, buffer, length) == 0)
         {
            // found
            if (save_in_cache == TRUE)
            {
               insert_file_in_cache(filename, (* length), (* buffer), (ENUM_MPQ) d);
               keep_cache_under_limit();
            }
            return 0;
         }
      }
   }

   return 1;
}


// ===========================================================================
// load a file from the Mod Directory
// ===========================================================================
int load_file_from_mod_directory(ENUM_MPQ m, char * filename, char ** buffer, long * length)
{
   WCHAR wcfile [MAX_PATH] = TEXT("");
   WCHAR path   [MAX_PATH] = TEXT("");
   DWORD attr              = 0;
   FILE  * in              = NULL;


   if ((filename == NULL) || (buffer == NULL) || (length == NULL))
      return 1;

   (* buffer) = NULL;
   (* length) = 0;

   if (m != MPQ_MOD_DIRECTORY)
      return 1;

   if (myglobals.datas.mpq[m].path[0] == 0)
      return 1;

   memset(wcfile, 0, sizeof(wcfile));
   char_to_wide_char(filename, wcfile, sizeof(wcfile));
   swprintf(path, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.mpq[m].path, wcfile);
   attr = GetFileAttributes(path);
   if (attr == INVALID_FILE_ATTRIBUTES)
      return 1;
   else if (attr & FILE_ATTRIBUTE_DIRECTORY)
      return 1;

   if (_wfopen_s( & in, path, TEXT("rb")) != 0)
      return 1;

   fseek(in, 0, SEEK_END);
   (* length) = ftell(in);
   fseek(in, 0, SEEK_SET);

   (* buffer) = calloc(1, (* length) + 1);
   if ((* buffer) == NULL)
   {
      (* length) = 0;
      fclose(in);
      return 1;
   }

   if (fread((* buffer), (* length), 1, in) < 1)
   {
      DESTROYME(* buffer, free)
      (* length) = 0;
      fclose(in);
      return 1;
   }

   fclose(in);
   return 0;
}


// ===========================================================================
// load a file from the ressources\default_d2_files directory
// ===========================================================================
int load_default_d2_file(char * filename, char ** buffer, long * length, int save_in_cache)
{
   WCHAR wcfile [MAX_PATH] = TEXT("");
   WCHAR path   [MAX_PATH] = TEXT("");
   DWORD attr              = 0;
   FILE  * in              = NULL;


   if ((filename == NULL) || (buffer == NULL) || (length == NULL))
      return 1;

   (* buffer) = NULL;
   (* length) = 0;

   memset(wcfile, 0, sizeof(wcfile));
   char_to_wide_char(filename, wcfile, sizeof(wcfile));

   swprintf(path, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources_default_files, wcfile);
   attr = GetFileAttributes(path);
   if (attr == INVALID_FILE_ATTRIBUTES)
      return 1;
   else if (attr & FILE_ATTRIBUTE_DIRECTORY)
      return 1;

   if (_wfopen_s( & in, path, TEXT("rb")) != 0)
      return 1;

   fseek(in, 0, SEEK_END);
   (* length) = ftell(in);
   fseek(in, 0, SEEK_SET);

   (* buffer) = calloc(1, (* length) + 1);
   if ((* buffer) == NULL)
   {
      (* length) = 0;
      fclose(in);
      return 1;
   }

   if (fread((* buffer), (* length), 1, in) < 1)
   {
      DESTROYME(* buffer, free)
      (* length) = 0;
      fclose(in);
      return 1;
   }

   fclose(in);

   if (save_in_cache == TRUE)
   {
      insert_file_in_cache(filename, (* length), (* buffer), MPQ_DEFAULT_D2_FILES);
      keep_cache_under_limit();
   }

   return 0;
}


// ===========================================================================
// load the internal "(listfile)" file from a given mpq
// return 0 on success
// ===========================================================================
int load_internal_mpq_file(ENUM_MPQ mpq, char * filename, char ** buffer, long * length)
{
   if ((filename == NULL) || (buffer == NULL) || (length == NULL))
      return 1;

   if ((mpq != MPQ_PATCH_D2) && (mpq != MPQ_D2EXP) &&(mpq != MPQ_D2DATA) &&(mpq != MPQ_D2CHAR))
      return 1;

   if (load_file_from_mpq((ENUM_MPQ) mpq, filename, buffer, length) != 0)
      return 1;

   return 0;
}


// ===========================================================================
// apply the palette contained in a given PL2 file
// ===========================================================================
void set_new_palette_from_PL2(ENUM_D2_RESSOURCES d2r)
{
   APP_PALETTE pal;
   UBYTE       * b = NULL;
   int         i   = 0;


   switch (d2r)
   {
      case D2R_ACT1PAL :
      case D2R_ACT2PAL :
      case D2R_ACT3PAL :
      case D2R_ACT4PAL :
      case D2R_ACT5PAL :
         memset( & pal, 0, sizeof(pal));
         b = (UBYTE *) myglobals.datas.d2_ressource[d2r].buffer;
         if (b == NULL)
            return;

         for (i = 0; i < 256; i++)
         {
            pal[i].r = b[0];
            pal[i].g = b[1];
            pal[i].b = b[2];
            pal[i].luminance = (0.2126 * pal[i].r) + (0.7152 * pal[i].g) + (0.0722 * pal[i].b);
            // FIXME (must be in user options) : luminance = 0.3333 * r + 0.3333 * g + 0.3333 * b;
            b += 4;
         }

         set_animation_palette(pal);
         break;
   }
}
