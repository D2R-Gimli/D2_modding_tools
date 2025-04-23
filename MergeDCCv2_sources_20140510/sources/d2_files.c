#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include "d2_files.h"
#include "misc.h"
#include "loaders.h"
#include "strings_define.h"
#include "tools.h"

// TODO :
// D2R_MONSTATS : "Id" + "NameStr" [ + "MonType" ] + "Code"
// Actx Palette : special pl2 (table of pointers to either 256 or 256*256 bytes)
// cmncof.d2, for act 1 thru 5

static void load_specific_mpq_files (void);
static int  load_d2_text_datas      (ENUM_D2_RESSOURCES d2r);


// ===========================================================================
// destroy all the Diablo II ressources files needed by the application
// ===========================================================================
void destroy_d2_ressources(void)
{
   int                 i    = 0;
   D2_RESSOURCES_DATAS * r  = NULL;
   APPLICATION_DATAS   * ad = & myglobals.application_datas;


   for (i = 0; i < D2R_MAX; i++)
   {
      r = & myglobals.datas.d2_ressource[i];
      DESTROYME(r->buffer, free);
      switch(r->type)
      {
         case D2RT_TEXT :
            DESTROYME(r->txt.tab_cell, free);
            break;
      }

      memset(r, 0, sizeof(D2_RESSOURCES_DATAS));
   }

   DESTROYME(ad->tab_layer, free);

   memset(ad, 0, sizeof(APPLICATION_DATAS));
}


// ===========================================================================
// for a given Diablo II TXT ressources file, return the string of the desired cell
// X column and Y row are zero-based
// if any problem, or if the cell don't exists in the TXT file, return an empty string
// ===========================================================================
char * get_d2_ressource_cell_XY(int id, int x, int y)
{
   D2_RESSOURCES_DATAS * r    = NULL;
   unsigned long       offset = 0;
   char                * cptr = NULL;


   if ((id < 0) || (id >= D2R_MAX) || (x < 0) || (y < 0))
      return "";

   r = & myglobals.datas.d2_ressource[id];
   if ((r->buffer == NULL) || (r->type != D2RT_TEXT) || (r->txt.tab_cell == NULL))
      return "";

   if ((x >= r->txt.nb_cols) || (y >= r->txt.nb_rows))
      return "";

   offset = r->txt.tab_cell[(y * r->txt.nb_cols) + x];
   if (offset == (unsigned long) -1)
      return "";

   cptr = (char *) r->buffer;
   return cptr + offset;
}


// ===========================================================================
// for a given Diablo II TXT ressources file, return the string of the desired header
// X column is zero-based
// if any problem, or if the header cell don't exists in the TXT file, return an empty string
// ===========================================================================
char * get_d2_ressource_column_X_header(int id, int x)
{
   return get_d2_ressource_cell_XY(id, x, 0);
}


// ===========================================================================
// return the zero-based X column position of the desired header in a given
// Diablo II TXT ressources file.
// return -1 if any problem, or if the header was not found
// ===========================================================================
int get_d2_ressource_column_X_from_name(int id, char * column_name)
{
   D2_RESSOURCES_DATAS * r = NULL;
   int                 x   = 0;


   if ((id < 0) || (id >= D2R_MAX) || (column_name == NULL))
      return -1;

   if (strlen(column_name) == 0)
      return -1;

   r = & myglobals.datas.d2_ressource[id];

   if ((r->buffer == NULL) || (r->type != D2RT_TEXT) || (r->txt.tab_cell == NULL))
      return -1;

   for (x=0; x < r->txt.nb_cols; x++)
   {
      if (_stricmp(column_name, get_d2_ressource_column_X_header(id, x)) == 0)
         return x;
   }

   return -1;
}


// ===========================================================================
// load a Diablo II data\global\excel\(...) .txt 
// return 0 on success
// ===========================================================================
int load_d2_txt(int id)
{
   D2_RESSOURCES_DATAS * r = NULL;
   void                * voidptr        = NULL;
   unsigned char       * cptr           = NULL;
   int                 c                = 0;
   int                 nb_rows          = 0;
   int                 nb_cols          = 1;
   int                 nb_cols_max      = 1;
   unsigned long       * tab_cell       = NULL;
   char                message [500]    = "";
   unsigned long       nb_cells         = 0;
   unsigned long       size             = 0;
   int                 x                = 0;
   int                 y                = 0;
   unsigned long       offset           = 0;
   char                * cell           = NULL;
   int                 n                = 0;
   int                 c2               = 0;
   char                * str_expansion  = "Expansion";
   int                 len_expansion    = strlen(str_expansion);
   int                 k                = 0;
   int                 nb_expansion_row = 0;


   if ((id < 0) || (id >= D2R_MAX))
      return 1;

   r       = & myglobals.datas.d2_ressource[id];
   voidptr = r->buffer;
   cptr    = (unsigned char *) voidptr;

   sprintf(message, "The Diablo II ressource file \"%s\" (ID = %d) is empty.", r->filename, id);
   MYASSERT_RETURN(cptr[0] != 0x00, 1, message);

   // count rows, and max cols
   do
   {
      c = cptr[0];
      if ((c < 32) && (c != 0))
      {
         if ((c == 0x0D) || (c == 0x0A))
         {
            nb_rows++;
            if ((c == 0x0D) && (cptr[1] == 0x0A))
               cptr++;
            if (cptr[1] == 0)
               nb_rows--;
            if (nb_cols > nb_cols_max)
               nb_cols_max = nb_cols;
            nb_cols = 1;
         }
         else if (c == '\t')
            nb_cols++;
         else
            cptr[0] = ' '; // no unprintable characters, for safety
      }
      cptr++;
   } while (c != 0);
   nb_rows++;

   // allocate a table of nb_rows * nb_max_cols offsets
   nb_cells = nb_rows * nb_cols_max;
   size     = nb_cells * sizeof(unsigned long);
   tab_cell = (unsigned long *) malloc(size);
   sprintf(message, "The Diablo II ressource file \"%s\" (ID = %d) can't be loaded in memory, MY_MALLOC() error for %lu bytes.", r->filename, id, size);
   MYASSERT_RETURN(tab_cell != NULL, 1, message);
   memset(tab_cell, 0, size);

   // initialise the offsets table
   cptr = (unsigned char *) voidptr;
   do
   {
      // skip the "Expansion" row
      if (nb_cols == 1)
      {
         if(strnicmp((char *) cptr, str_expansion, len_expansion) == 0)
         {
            k = len_expansion;
            while ((cptr[k] <= 32) && (cptr[k] != 0) && (cptr[k] != 0x0D) && (cptr[k] != 0x0A)  && (cptr[k] != '\t'))
               k++;
            if (cptr[k] <= 32)
            {
               // the row has "Expansion" in its first column, and after a rtrim() of the cell we're sure its still the special "Expansion" row. Skip the entire row
               k = len_expansion; 
               while ((cptr[k] != 0) && (cptr[k] != 0x0D) && (cptr[k] != 0x0A))
                  k++;
               if ((cptr[k] == 0x0D) || (cptr[k] == 0x0A))
               {
                  k++;
                  if (((cptr[k] == 0x0D) || (cptr[k] == 0x0A)) && (cptr[k] != cptr[k - 1]))
                     k++;
               }

               cptr   += k;
               offset += k;
               tab_cell[(y * nb_cols_max) + x] = offset;
               nb_expansion_row++;
            }
         }
      }

      c = cptr[0];
      if ((c < 32) && (c != 0))
      {
         if ((c == 0x0D) || (c == 0x0A))
         {
            if ( (cptr[1] == 0)
                 ||
                 ( ((cptr[1] == 0x0D) || (cptr[1] == 0x0A)) && (cptr[2] == 0) )
               )
            {
               cptr[0] = 0;
               break;
            }

            x = 0;
            y++;
            if (y >= nb_rows)
            {
               sprintf(message, "BUG, after '\\n'. Ressource file \"%s\" (ID = %d). y = %d, nb_rows = %d, offset = %lu", r->filename, id, y, nb_rows, offset);
               DESTROYME(tab_cell, free);
               tab_cell = NULL;
               MYASSERT_RETURN(y < nb_rows, 1, message);
            }
            else
               tab_cell[(y * nb_cols_max) + x] = offset + 1;

            if ((c == 0x0D) && (cptr[1] == 0x0A))
            {
               cptr[0] = 0;
               cptr++;
               offset++;
               tab_cell[(y * nb_cols_max) + x] = offset + 1;
            }
         }
         else if (c == '\t')
         {
            cptr[0] = 0;
            x++;
            if (x >= nb_cols_max)
            {
               sprintf(message, "BUG, after '\\t'. Ressource file \"%s\" (ID = %d). x = %d, nb_cols_max = %d, offset = %lu", r->filename, id, x, nb_cols_max, offset);
               DESTROYME(tab_cell, free);
               MYASSERT_RETURN(x < nb_cols_max, 1, message);
            }
            else
               tab_cell[(y * nb_cols_max) + x] = (offset + 1);
         }
      }
      cptr++;
      offset++;
   } while (c != 0);

   // for all cells without effective datas (the row had less columns than expected), replace offset by -1
   for (y=0; y < nb_rows; y++)
   {
      for (x=0; x < nb_cols_max; x++)
      {
         if ((x == 0) && (y == 0))
            continue;

         offset = (y * nb_cols_max) + x;
         if (tab_cell[offset] == 0)
            tab_cell[offset] = (unsigned long) -1;
      }
   }

   // trim each cell
   cptr = (unsigned char *) voidptr;
   for (y=0; y < nb_rows; y++)
   {
      for (x=0; x < nb_cols_max; x++)
      {
         offset = tab_cell[(y * nb_cols_max) + x];
         if (offset == -1)
            continue;

         cell = (char *) (cptr + offset);
         n = strlen(cell);
         for (c = n - 1; (c >= 0) && (cell[c] == ' '); c--)
            cell[c] = 0;
         for (c=0; (c < n) && (cell[c] == ' '); c++)
         {}
         for (c2=0; c2 < n; c2++)
         {
            cell[c2] = cell[c + c2];
            if (cell[c2] == 0)
               break;
         }
      }
   }

   r->txt.tab_cell = tab_cell;
   r->txt.nb_cols  = nb_cols_max;
   r->txt.nb_rows  = nb_rows - nb_expansion_row;

   return 0;
}


// ===========================================================================
// load all Diablo II ressources files needed by the application
// return 0 on success
// ===========================================================================
int load_d2_ressources(void)
{
   char                message [300] = "";
   int                 i             = 0;
   D2_RESSOURCES_DATAS * r           = NULL;
   int                 load_ok       = FALSE;
   int                 error_code    = 0;
   struct
   {
      ENUM_D2_RESSOURCES_TYPE type;
      char                    * root;
      char                    * name;
   } data [D2R_MAX] = {
      // type        root                  name
      // ----------  --------------------  -----------------
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "Composit.txt"},      // D2R_COMPOSIT
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "WeaponClass.txt"},   // D2R_WEAPONCLASS
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "PlrType.txt"},       // D2R_PLRTYPE
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "PlrMode.txt"},       // D2R_PLRMODE
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "ObjMode.txt"},       // D2R_OBJMODE
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "Colors.txt"},        // D2R_COLORS
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "MonStats.txt"},      // D2R_MONSTATS
      {D2RT_TEXT,        D2_ROOT_PATH_EXCEL,   "Objects.txt"},       // D2R_OBJECTS
      {D2RT_PALETTE,     D2_ROOT_PATH_PALETTE, "Act1\\Pal.pl2"},     // D2R_ACT1PAL
      {D2RT_PALETTE,     D2_ROOT_PATH_PALETTE, "Act2\\Pal.pl2"},     // D2R_ACT2PAL
      {D2RT_PALETTE,     D2_ROOT_PATH_PALETTE, "Act3\\Pal.pl2"},     // D2R_ACT3PAL
      {D2RT_PALETTE,     D2_ROOT_PATH_PALETTE, "Act4\\Pal.pl2"},     // D2R_ACT4PAL
      {D2RT_PALETTE,     D2_ROOT_PATH_PALETTE, "Act5\\Pal.pl2"},     // D2R_ACT5PAL
      {D2RT_ANIMDATA_D2, D2_ROOT_PATH_GLOBAL,  "AnimData.d2"},       // D2R_ANIMDATA_D2
   };


   // destroy any current ressources
   destroy_d2_ressources();

   // load all ressources from current Mod Directory / MPQ paths configuration
   for (i=0; i < D2R_MAX; i++)
   {
      r = & myglobals.datas.d2_ressource[i];
      sprintf_s(r->filename, sizeof(r->filename), "%s%s", data[i].root, data[i].name);
      delete_filename_from_cache(r->filename);

      load_ok = FALSE;
      if (load_file(r->filename, & r->buffer, & r->length, FALSE) != 0)
      {
         if (load_default_d2_file(r->filename, & r->buffer, & r->length, FALSE) != 0)
         {
            sprintf_s(message, sizeof(message), "can't extract the Diablo II ressource file : %s", r->filename);
            MessageBoxA(NULL, message, "Error", MB_ICONERROR | MB_OK);
            error_code = 1;
         }
         else
            load_ok = TRUE;
      }
      else
         load_ok = TRUE;

      if (load_ok == TRUE)
      {
         r->type = data[i].type;

         switch (data[i].type)
         {
            case D2RT_TEXT :
               if (load_d2_txt(i) != 0)
                  error_code = 1;
               break;

            case D2RT_PALETTE :
               fix_PL2_colormaps((UBYTE *) r->buffer);
               break;

            case D2RT_ANIMDATA_D2 :
               break;
         }
      }
   }

   for (i = 0; i < D2R_MAX; i++)
   {
      if (data[i].type != D2RT_TEXT)
         continue;

      if (load_d2_text_datas((ENUM_D2_RESSOURCES) i) != 0)
      {
         if (i == D2R_PLRMODE)
         {
            r = & myglobals.datas.d2_ressource[i];
            delete_filename_from_cache(r->filename); // TODO : check if it's really necessary here, since ressources are asked to NOT be loaded in cache now
            if (load_default_d2_file(r->filename, & r->buffer, & r->length, FALSE) != 0)
            {
               sprintf_s(message, sizeof(message), "can't extract the (default) Diablo II ressource file : %s", r->filename);
               MessageBoxA(NULL, message, "Error", MB_ICONERROR | MB_OK);
               return 1;
            }

            if (load_d2_text_datas((ENUM_D2_RESSOURCES) i) == 0)
               continue;
         }

         sprintf_s(message, sizeof(message), "Can't load datas from the Diablo II ressource file : %s", myglobals.datas.d2_ressource[i].filename);
         MessageBoxA(NULL, message, "Error", MB_ICONERROR | MB_OK);
         return 1;
      }
   }

   return 0;
}


// ===========================================================================
// extract a listfile from a given MPQ, and copy it in cache under a new name
// return 0 if sucess
// ===========================================================================
int load_mpq_listfile(int m, char ** buffer, long * length)
{
   char * mpq [MPQ_MAX] = {NULL, "patch_d2", "d2exp", "d2data", "d2char"};
   char tmp   [100]     = "";
   char * lf            = "(listfile)";


   if ((m < 0) || (m >= MPQ_MAX))
      return 1;

   if (mpq[m] == NULL)
      return 1;

   if ((buffer == NULL) || (length == NULL))
      return 1;

   if (load_internal_mpq_file((ENUM_MPQ) m, lf, buffer, length) == 0)
   {
      sprintf(tmp, "mpq\\%s.%s.txt", mpq[m], lf);
      insert_file_in_cache(tmp, (* length), (* buffer), (ENUM_MPQ) m);
   }

   return 0;
}


// ===========================================================================
// from the Diablo II ressource file data\global\excel\(...) .txt, make the
// datas the application will use
// return 0 on succees
// ===========================================================================
int load_d2_text_datas(ENUM_D2_RESSOURCES d2r)
{
   D2_RESSOURCES_DATAS * r          = NULL;
   APPLICATION_DATAS   * ad         = & myglobals.application_datas;
   int                 col_Name     = -1;
   int                 col_Code     = -1;
   int                 y            = 0;
   char                * cell_Name  = NULL;
   char                * cell_Code  = NULL;
   int                 n            = 0;
   int                 * nb_datas   = NULL;
   void                ** tab_datas = NULL;
   char                * name       = "";
   char                * code       = "";
   int                 sizeof_data  = 0;


   switch (d2r)
   {
      case D2R_COMPOSIT    : nb_datas = & ad->nb_layers;    tab_datas = (void **) & ad->tab_layer;     name = "Name";            code = "Token"; sizeof_data = sizeof(APP_LAYER);     break;
      case D2R_WEAPONCLASS : nb_datas = & ad->nb_weapclass; tab_datas = (void **) & ad->tab_weapclass; name = "Weapon Class";    code = "Code";  sizeof_data = sizeof(APP_WEAPCLASS); break;
      case D2R_PLRTYPE     : nb_datas = & ad->nb_plrtype;   tab_datas = (void **) & ad->tab_plrtype;   name = "Name";            code = "Token"; sizeof_data = sizeof(APP_PLRTYPE);   break;
      case D2R_PLRMODE     : nb_datas = & ad->nb_plrmode;   tab_datas = (void **) & ad->tab_plrmode;   name = "Name";            code = "Code";  sizeof_data = sizeof(APP_PLRMODE);   break;
      case D2R_OBJMODE     : nb_datas = & ad->nb_objmode;   tab_datas = (void **) & ad->tab_objmode;   name = "Name";            code = "Token"; sizeof_data = sizeof(APP_OBJMODE);   break;
      case D2R_COLORS      : nb_datas = & ad->nb_colors;    tab_datas = (void **) & ad->tab_colors;    name = "Transform Color"; code = "Code";  sizeof_data = sizeof(APP_COLORS);    break;
      case D2R_MONSTATS    : nb_datas = & ad->nb_monstats;  tab_datas = (void **) & ad->tab_monstats;  name = "NameStr";         code = "Code";  sizeof_data = sizeof(APP_MONSTATS);  break;
      case D2R_OBJECTS     : nb_datas = & ad->nb_objects;   tab_datas = (void **) & ad->tab_objects;   name = "Name";            code = "Token"; sizeof_data = sizeof(APP_OBJECTS);   break;
      default : return 1;
   }

   r = & myglobals.datas.d2_ressource[d2r];

   if ((r->buffer == NULL) || (r->txt.tab_cell == NULL))
      return 1;

   DESTROYME(* tab_datas, free);
   (* nb_datas) = 0;

   col_Name = get_d2_ressource_column_X_from_name(d2r, name);
   col_Code = get_d2_ressource_column_X_from_name(d2r, code);

   if ((col_Code == -1) && (d2r == D2R_PLRMODE))
      col_Code = get_d2_ressource_column_X_from_name(d2r, "Token");

   if ((col_Name == -1) && (d2r == D2R_MONSTATS))
      col_Name = get_d2_ressource_column_X_from_name(d2r, "Class");

   if ((col_Name == -1) || (col_Code == -1))
      return 1;

   for (y = 1; y < r->txt.nb_rows; y++)
   {
      cell_Name = get_d2_ressource_cell_XY(d2r, col_Name, y);
      cell_Code = get_d2_ressource_cell_XY(d2r, col_Code, y);
      if ((strlen(cell_Name) > 0) && (strlen(cell_Code) > 0))
         (* nb_datas)++;
   }

   (* tab_datas) = calloc((* nb_datas), sizeof_data);
   if ((* tab_datas) == NULL)
      return 1;

   n = 0;
   for (y = 1; y < r->txt.nb_rows; y++)
   {
      cell_Name = get_d2_ressource_cell_XY(d2r, col_Name, y);
      cell_Code = get_d2_ressource_cell_XY(d2r, col_Code, y);
      if ((strlen(cell_Name) > 0) && (strlen(cell_Code) > 0))
      {
         if (n >= (* nb_datas))
         {
            DESTROYME(* tab_datas, free);
            return 1;
         }
         else
         {
            make_string_uppercase(cell_Code);
            switch (d2r)
            {
               case D2R_COMPOSIT :
                  strcpy(ad->tab_layer[n].code_char, cell_Code);
                  char_to_wide_char(cell_Code, ad->tab_layer[n].code, sizeof(ad->tab_layer[n].code));
                  char_to_wide_char(cell_Name, ad->tab_layer[n].name, sizeof(ad->tab_layer[n].name));
                  break;

               case D2R_WEAPONCLASS :
                  char_to_wide_char(cell_Code, ad->tab_weapclass[n].code, sizeof(ad->tab_weapclass[n].code));
                  char_to_wide_char(cell_Name, ad->tab_weapclass[n].name, sizeof(ad->tab_weapclass[n].name));
                  break;

               case D2R_PLRTYPE :
                  char_to_wide_char(cell_Code, ad->tab_plrtype[n].code, sizeof(ad->tab_plrtype[n].code));
                  char_to_wide_char(cell_Name, ad->tab_plrtype[n].name, sizeof(ad->tab_plrtype[n].name));
                  break;

               case D2R_PLRMODE :
                  char_to_wide_char(cell_Code, ad->tab_plrmode[n].code, sizeof(ad->tab_plrmode[n].code));
                  char_to_wide_char(cell_Name, ad->tab_plrmode[n].name, sizeof(ad->tab_plrmode[n].name));
                  break;

               case D2R_OBJMODE :
                  char_to_wide_char(cell_Code, ad->tab_objmode[n].code, sizeof(ad->tab_objmode[n].code));
                  char_to_wide_char(cell_Name, ad->tab_objmode[n].name, sizeof(ad->tab_objmode[n].name));
                  break;

               case D2R_COLORS :
                  char_to_wide_char(cell_Code, ad->tab_colors[n].code, sizeof(ad->tab_colors[n].code));
                  char_to_wide_char(cell_Name, ad->tab_colors[n].name, sizeof(ad->tab_colors[n].name));
                  break;

               case D2R_MONSTATS :
                  char_to_wide_char(cell_Code, ad->tab_monstats[n].code, sizeof(ad->tab_monstats[n].code));
                  char_to_wide_char(cell_Name, ad->tab_monstats[n].name, sizeof(ad->tab_monstats[n].name));
                  break;

               case D2R_OBJECTS :
                  char_to_wide_char(cell_Code, ad->tab_objects[n].code, sizeof(ad->tab_objects[n].code));
                  char_to_wide_char(cell_Name, ad->tab_objects[n].name, sizeof(ad->tab_objects[n].name));
                  break;

               default :
                  return 1;
            }
            n++;
         }
      }
   }

   return 0;
}


// ===========================================================================
// real Objects speed is in Objects.txt, columns FrameDelta0 to
// FrameDelta7, 0 = "NU" ... 7 = "S5", according to ObjMode.txt
// return 0 if problem, or else value found in Objects.txt for this Token and Mode
// ===========================================================================
long get_ObjectsTXT_speed(char * token, char * mode)
{
   int  col_speed           = -1;
   int  col_token           = -1;
   long y                   = 0;
   char * value             = NULL;
   long speed               = 0;
   char col_speed_name [30] = "";
   char * objmodes     [8]  = {"NU", "OP", "ON", "S1", "S2", "S3", "S4", "S5"};
   int  i                   = 0;


   if ((token == NULL) || (mode == NULL))
      return 0;

   for (i = 0; i < 8; i++)
   {
      if (strcmp(mode, objmodes[i]) == 0)
      {
         sprintf(col_speed_name, "FrameDelta%d", i);
         col_speed = get_d2_ressource_column_X_from_name(D2R_OBJECTS, col_speed_name);
         break;
      }
   }

   col_token = get_d2_ressource_column_X_from_name(D2R_OBJECTS, "Token");

   if ((col_speed == -1) || (col_token == -1))
      return 0;

   // search Token row
   for (y = 1; y < myglobals.application_datas.nb_objects; y++)
   {
      if (strcmp(token, get_d2_ressource_cell_XY(D2R_OBJECTS, col_token, y)) == 0)
      {
         // get FrameDeltaX value for this row
         value = get_d2_ressource_cell_XY(D2R_OBJECTS, col_speed, y);
         speed = atol(value);
         if (speed < 0)
            return 0;
         if (speed > 512)
            return 512;
         return speed;
      }
   }

   return 0;
}


// ===========================================================================
// return a pointer to the record in AnimData.d2 of a given COF
// return NULL if not found
// ===========================================================================
ANIMDATA_D2_RECORD * get_animdata_d2_record(COF_ROW_EXISTS * cof)
{
   char               * d2file        = myglobals.datas.d2_ressource[D2R_ANIMDATA_D2].buffer;
   long               length          = myglobals.datas.d2_ressource[D2R_ANIMDATA_D2].length;
   char               cof_name[7 + 1] = "";
   int                i               = 0;
   int                hash            = 0;
   long               cursor          = 0;
   long               * nb_records    = 0;
   ANIMDATA_D2_RECORD * ptr_record    = NULL;


   if (d2file == NULL)
      return NULL;

   if (cof == NULL)
      return NULL;

   if ((cof->token[2] != 0) || (cof->mode[2] != 0) || (cof->weapon_class[3] != 0))
      return NULL;

   sprintf(cof_name, "%s%s%s", cof->token, cof->mode, cof->weapon_class);
   hash = 0;
   for (i = 0; i < 7; i++)
      hash += toupper(cof_name[i]);

   hash &= 0xFF;

   // get to the desired set of records
   for (i = 0; i < hash; i++)
   {
      // get the number of records in this set

      if ((cursor + 4) >= length)
         return NULL;

      nb_records = (long *) (d2file + cursor);
      cursor += 4;
      cursor += (* nb_records) * sizeof(ANIMDATA_D2_RECORD);
   }

   // get the number of records in this set
   if ((cursor + 4) >= length)
      return NULL;

   nb_records = (long *) (d2file + cursor);
   cursor += 4;

   // scan all the records in this set until we find the desired COF
   for (i = 0; i < (* nb_records); i++)
   {
      if ((cursor + (long) sizeof(ANIMDATA_D2_RECORD)) >= length)
         return NULL;

      ptr_record = (ANIMDATA_D2_RECORD *) (d2file + cursor);
      if (stricmp(cof_name, ptr_record->cof_name) == 0)
         return ptr_record;

      cursor += sizeof(ANIMDATA_D2_RECORD);
   }

   return NULL;
}
