#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include "listfiles.h"
#include "misc.h"
#include "strings_define.h"
#include "d2_files.h"
#include "tools.h"
#include "mpq_handler.h"
#include "dialog_maker.h"
#include "dlgbox_cof_in_progress.h"
#include "loaders.h"


struct test_existence_list_PARAMS_S
{
   int  r;
   HWND hpbar;
   HWND hpnct;
   HWND hexists;
   long nb_files;
   long curr_pos;
   long curr_step;
   long curr_exists;
   long nb_tokens_players;
   long nb_tokens_monsters;
   long nb_tokens_objects;
   long nb_colormaps;
   FILE * debug_exists_file;
};


static int                build_COF_existence_list         (int r);
static int                build_DCC_existence_list         (int r);
static int                create_tab_cmap_file             (void);
static void               destroy_colormap_file            (COLORMAP_FILE * cmap);
static void               destroy_colormaps_linked_list    (void);
static void               destroy_tab_cmap_file            (void);
static int                extract_COF_row_elements         (ENUM_ANIMTYPE r, char * row, STR_2_LETTERS * token, STR_3_LETTERS * weapon_class, STR_2_LETTERS * mode);
static int                extract_DCC_row_elements         (ENUM_ANIMTYPE r, char * row, STR_2_LETTERS * token, STR_3_LETTERS * weapon_class, STR_2_LETTERS * mode, STR_2_LETTERS * layer, STR_3_LETTERS * variant);
static int                get_colormap_name_from_row_type  (CMAP_ROW * pt_cmap_row, WCHAR * name);
static ENUM_ANIMTYPE      get_root_path_type               (char * row);
static ENUM_ROW_EXTENSION get_row_type                     (char * row);
static int                insert_colormap_link             (COLORMAP_FILE * pt_cmap);
static int                prepare_CMAP_selection_listboxes (void);
static int                prepare_COF_selection_listboxes  (void);
static void               process_DCC_row                  (char * row);
static void               process_CMAP_row                 (char * row, ENUM_ROW_EXTENSION num_row_ext);
static void               process_COF_row                  (char * row);
static int                qsort_helper_COF                 (const void * a, const void * b);
static int                qsort_helper_DCC                 (const void * a, const void * b);
static int                qsort_helper_tab_cmap_file       (const void * e1, const void * e2);
static void               rtrim_row                        (char * row);
static int                test_existence_list              (struct test_existence_list_PARAMS_S * params);
static int                test_existence_list_colormaps    (struct test_existence_list_PARAMS_S * params);
static int                validate_row                     (ENUM_ANIMTYPE r, char * row, char * format);


// ===========================================================================
// open listfiles, and analyse them
// ===========================================================================
int load_all_listfiles(void)
{
   ENUM_DLGBOX_ID                      dialog_ID           = DLG_COF_IN_PROGRESS;
   DLGBOX_DATAS                        * dd                = NULL;
   CREATE_DLGBOX                       * dlg               = NULL;
   CREATE_DLGBOX_WINDOW                * w                 = NULL;
   FILE                                * in                = NULL;
   long                                length              = 0;
   char                                * buffer            = NULL;
   int                                 m                   = 0;
   int                                 pbar_idx            = -1;
   int                                 pnct_idx            = -1;
   int                                 exists_idx          = -1;
   int                                 i                   = 0;
   WCHAR                               filename [MAX_PATH] = TEXT("");
   int                                 ret                 = 0;
   CMAP_ROW                            * pt_cmap_row       = NULL;
   struct test_existence_list_PARAMS_S params              = {0};
   WCHAR                               * at_name [AT_MAX]  = {TEXT("players"), TEXT("monsters"), TEXT("objects")};


   MYASSERT_RETURN(((dialog_ID >= 0) && (dialog_ID < DLG_MAX)), 1, NULL);
   MYASSERT_RETURN(create_dlgbox_COF_in_progress() == 0, 1, NULL);
   dd  = & myglobals.dlgbox_datas[dialog_ID];
   dlg = & dd->dlg;
   w  = & dlg->window;

   if (dlg == NULL)
      return 1;

   memset( & params, 0, sizeof(params));

   // search which child index is the progress bar in d->pChild[]
   for (pbar_idx = 0; pbar_idx < dlg->nbChildren; pbar_idx++)
   {
      if (dlg->pChild[pbar_idx].iChildID == ID_DLGBOX_COF_IN_P_PBAR)
         break;
   }

   if (pbar_idx >= dlg->nbChildren)
      return 1;

   if (dlg->pChild[pbar_idx].handle == NULL)
      return 1;

   // search which child index is the "percentage" in d->pChild[], right of the progress bar
   for (pnct_idx = 0; pnct_idx < dlg->nbChildren; pnct_idx++)
   {
      if (dlg->pChild[pnct_idx].iChildID == ID_DLGBOX_COF_IN_P_NBFILES)
         break;
   }

   if (pnct_idx >= dlg->nbChildren)
      return 1;

   if (dlg->pChild[pnct_idx].handle == NULL)
      return 1;

   // search which child index in d->pChild[] display the number of COF existing in MPQ
   for (exists_idx = 0; exists_idx < dlg->nbChildren; exists_idx++)
   {
      if (dlg->pChild[exists_idx].iChildID == ID_DLGBOX_COF_IN_P_EXISTS_R)
         break;
   }

   if (exists_idx >= dlg->nbChildren)
      return 1;

   if (dlg->pChild[exists_idx].handle == NULL)
      return 1;

   // load the application listfile

   MYASSERT((_wfopen_s( & in, myglobals.datas.path.listfile, TEXT("rb")) == 0), "can't open 'ressources\\listfiles.txt'");
   if (in == NULL)
      return 1;

   fseek(in, 0, SEEK_END);
   length = ftell(in);
   fseek(in, 0, SEEK_SET);

   buffer = (char *) calloc(1, length + 1);
   MYASSERT(buffer != NULL, NULL);
   if (buffer == NULL)
   {
      fclose(in);
      return 1;
   }

   fread(buffer, length, 1, in);
   fclose(in);

   analyse_listfile(buffer, length);
   DESTROYME(buffer, free)
   length = 0;

   // load the listfiles of all opened MPQ
   for (m = 0; m < MPQ_MAX; m++)
   {
      if (load_mpq_listfile(m,  & buffer, & length) == 0)
      {
         analyse_listfile(buffer, length);
         DESTROYME(buffer, free)
         length = 0;
      }
   }

   // prepare the search of existing files
   for (i = 0; i < AT_MAX; i++)
   {
      if (build_COF_existence_list(i) != 0)
         return 1;

      if (build_DCC_existence_list(i) != 0)
         return 1;
   }

   // search the total number of files that'll be tested

   params.nb_files = 0;
   for (i = 0; i < AT_MAX; i++)
   {
      params.nb_files += myglobals.listfile_datas.at[i].nb_cofs;
      params.nb_files += myglobals.listfile_datas.at[i].nb_dcc;
   }

   pt_cmap_row = myglobals.listfile_datas.first_CMAP;
   while (pt_cmap_row != NULL)
   {
      params.nb_files++;
      pt_cmap_row = pt_cmap_row->next;
   }

   if (params.nb_files <= 0)
      return 1;

   // progress bar : set the range and position, and to be safe set the current state to normal
   SendMessage(dlg->pChild[pbar_idx].handle, PBM_SETSTATE,   (WPARAM) PBST_NORMAL, (LPARAM) 0);
   SendMessage(dlg->pChild[pbar_idx].handle, PBM_SETRANGE32, (WPARAM) 0,           (LPARAM) params.nb_files);
   SendMessage(dlg->pChild[pbar_idx].handle, PBM_SETPOS,     (WPARAM) 0,           (LPARAM) 0);

   // ===============================================
   // test the existence of files for COF / DCC / DC6
   // ===============================================
   params.hpbar   = dlg->pChild[pbar_idx].handle;
   params.hpnct   = dlg->pChild[pnct_idx].handle;
   params.hexists = dlg->pChild[exists_idx].handle;
   for (i = 0; i < AT_MAX; i++)
   {
      // create a debug file to know the existence state of all searched files
      swprintf(filename, sizeof(filename) / 2, TEXT("%s\\mpq_file_existence_%s.txt"),  myglobals.datas.path.debug, at_name[i]);
      params.debug_exists_file = _wfopen(filename, TEXT("wt"));
      params.r                 = i;

      // test the existence of all the current animation type files
      ret = test_existence_list( & params);

      // close the debug file
      if (params.debug_exists_file != NULL)
      {
         fclose(params.debug_exists_file);
         params.debug_exists_file = NULL;
      }

      if (ret != 0)
         return 1;
   }

   // =====================================
   // test the existence of colormaps files
   // =====================================

   // create a debug file to know the existence state of all searched files
   swprintf(filename, sizeof(filename) / 2, TEXT("%s\\mpq_file_existence_colormaps.txt"),  myglobals.datas.path.debug);
   params.debug_exists_file = _wfopen(filename, TEXT("wt"));
   params.r                 = i;

   // test the existence of all the colormaps
   ret = test_existence_list_colormaps( & params);

   // close the debug file
   DESTROYME(params.debug_exists_file, fclose)

   if (ret != 0)
      return 1;

   // prepare selection listboxes

   if (prepare_COF_selection_listboxes() != 0)
      return 1;

   if (prepare_CMAP_selection_listboxes() != 0)
      return 1;

   return 0;
}


// ===========================================================================
// parse the entire listfile in memory, and process the desired types of row
// ===========================================================================
void analyse_listfile(char * _cptr, long length)
{
   long               i           = 0;
   int                c           = 0;
   char               * row       = NULL;
   UBYTE              * cptr      = (UBYTE *) _cptr;
   ENUM_ROW_EXTENSION num_row_ext = REXT_NULL;


   if ((cptr == NULL) || (length <= 0))
      return;

   // pre-format the entire text : EOL are replaced by 0, and unprintable characters by spaces
   for (i = 0; i < length; i++)
   {
      c = cptr[i];
      if ((c == 0x0D) || (c == 0x0A))
         cptr[i] = 0;
      else if ((c < 32) && (c != 0))
         cptr[i] = 32;
   }

   // for each row, get its extension, and call its corresponding processing function
   for (i = 0; i < length;)
   {
      row  = _cptr + i;
      i   += (strlen(row) + 1);
      rtrim_row(row);
      if (row[0] == 0)
         continue;

      num_row_ext = get_row_type(row);
      switch (num_row_ext)
      {
         case REXT_COF :
            process_COF_row(row);
            break;

         case REXT_DCC :
         case REXT_DC6 :
            process_DCC_row(row);
            break;

         case REXT_DAT_CMAP_ITM                : // Player
         case REXT_DAT_CMAP_MON_PALSHIFT       : // Monster
         case REXT_DAT_CMAP_MON_GREENBLOOD     : // Monster
         case REXT_DAT_CMAP_MON_RANDTRANSFORMS : // Monster
            process_CMAP_row(row, num_row_ext);
            break;

         default :
            break;
      }
   }
}


// ===========================================================================
// keep track of this colormap, for later analyse
// ===========================================================================
void process_CMAP_row(char * row, ENUM_ROW_EXTENSION num_row_ext)
{
   CMAP_ROW * pt_cmap_row = myglobals.listfile_datas.first_CMAP;


   // scan the linked list of colormaps names, to check if it already exists or not
   while (pt_cmap_row != NULL)
   {
      if (pt_cmap_row->filename != NULL)
      {
         if (stricmp(pt_cmap_row->filename, row) == 0)
            return; // already in the list
      }

      pt_cmap_row = pt_cmap_row->next;
   }

   // create a new colormap element
   pt_cmap_row = (CMAP_ROW *) calloc(1, sizeof(CMAP_ROW));
   if (pt_cmap_row == NULL)
      return;

   // insert this colormap in the list
   pt_cmap_row->next              = myglobals.listfile_datas.first_CMAP;
   pt_cmap_row->num_row_extension = num_row_ext;
   pt_cmap_row->filename          = strdup(row);
   pt_cmap_row->exists            = FALSE;
   if (pt_cmap_row->filename == NULL)
   {
      DESTROYME(pt_cmap_row, free)
      return;
   }
   myglobals.listfile_datas.first_CMAP = pt_cmap_row;
}


// ===========================================================================
// delete unprintable ending characters/spaces of the row
// ===========================================================================
void rtrim_row(char * row)
{
   long i = 0;


   if (row == NULL)
      return;

   i = strlen(row) - 1;
   if (i < 0)
      return;

   while ((i >= 0) && (row[i] <= 32))
   {
      row[i] = 0;
      i--;
   }
}


// ===========================================================================
// return the type of the row, given its path and file extension
// ===========================================================================
ENUM_ROW_EXTENSION get_row_type(char * row)
{
   char        * ext                = NULL;
   long        i                    = 0;
   char        * tab_ext [REXT_MAX] = {".cof", ".dcc", ".dc6", ".dat", ".dat", ".dat", ".dat"}; // must be in the same order as ENUM_ROW_EXTENSION
   int         length               = 0;
   static char buffer [1000]        = "";
   static int  error_throw          = FALSE;


   if (row == NULL)
      return REXT_NULL;

   length = strlen(row) - 1;
   if (length < 2)
      return REXT_NULL; // there must be at least 3 characters

   i = length;
   while ((row[i] != '.') && (i > 0))
      i--;

   if (row[i] != '.') // the extension must start by a '.'
      return REXT_NULL;

   ext = row + i;

   for (i = 0; i < REXT_MAX; i++)
   {
      if (error_throw == TRUE)
         return REXT_NULL;
      else
      {
         if (tab_ext[i] == NULL)
            error_throw = TRUE;

         MYASSERT_RETURN(tab_ext[i] != NULL, REXT_NULL, "tab_ext[] lacks at least a string");
      }

      if (_stricmp(ext, tab_ext[i]) == 0)
      {
         // extension is ok, but now maybe we need also to check the path
         switch (i)
         {
            case REXT_DAT_CMAP_ITM : // ------------------------------------------------------------------------
               // data/global/items/palette/*.dat
               if (_strnicmp(row, D2_ROOT_PATH_ITEM_PALETTE, sizeof(D2_ROOT_PATH_ITEM_PALETTE) - 1) == 0)
                  return (ENUM_ROW_EXTENSION) i;
               break;

            case REXT_DAT_CMAP_MON_PALSHIFT : // ---------------------------------------------------------------
               // data/global/monsters/ (...)
               if (_strnicmp(row, D2_ROOT_PATH_MONSTERS, sizeof(D2_ROOT_PATH_MONSTERS) - 1) == 0)
               {
                  // (...) ??/cof/palshift.dat
                  if (length >= sizeof(buffer))
                     return REXT_NULL;

                  strcpy(buffer, row + sizeof(D2_ROOT_PATH_MONSTERS) + 1);
                  if (_strnicmp(buffer, D2_STR_COF_PALSHIFT, sizeof(D2_STR_COF_PALSHIFT) - 1) == 0)
                     return (ENUM_ROW_EXTENSION) i;
               }
               break;

            case REXT_DAT_CMAP_MON_GREENBLOOD : // -------------------------------------------------------------
               // data/global/monsters/greenblood.dat
               if (_stricmp(row, D2_FIL_GREENBLOOD) == 0)
                  return (ENUM_ROW_EXTENSION) i;
               break;

            case REXT_DAT_CMAP_MON_RANDTRANSFORMS : // ---------------------------------------------------------
               // data/global/monsters/randtransforms.dat
               if (_stricmp(row, D2_FIL_RANDTRANSFORMS) == 0)
                  return (ENUM_ROW_EXTENSION) i;
               break;

            default : // ---------------------------------------------------------------------------------------
               return (ENUM_ROW_EXTENSION) i;
         }
      }
   }

   return REXT_NULL;
}


// ===========================================================================
// extract the needed elements of a COF row, and save it for later in a structure
// ===========================================================================
void process_COF_row(char * row)
{
   ENUM_ANIMTYPE r     = AT_NULL;
   COF_ROW       * pcf = NULL;
   COF_ROW       cf    = {0};


   if (row == NULL)
      return;

   r = get_root_path_type(row);
   if (r == AT_NULL)
      return;

   memset ( & cf, 0, sizeof(cf));
   if (extract_COF_row_elements(r, row, & cf.token, & cf.weapon_class, & cf.mode) == 0)
   {
      pcf = (COF_ROW *) malloc(sizeof(cf));
      if (pcf == NULL)
         return;

      memcpy(pcf, & cf, sizeof(cf));
      pcf->next = myglobals.listfile_datas.at[r].first_COF;
      myglobals.listfile_datas.at[r].first_COF = pcf;
   }

   return;
}


// ===========================================================================
// from a COF row, extract token, weapon class and mode, in uppercase
// return 0 if success
// ===========================================================================
int extract_COF_row_elements(ENUM_ANIMTYPE r, char * row, STR_2_LETTERS * token, STR_3_LETTERS * weapon_class, STR_2_LETTERS * mode)
{
   int           n      = 0;
   char          * cptr = NULL;
   STR_2_LETTERS token2 = "";


   if (r == AT_NULL)
      return 1;

   if ((r < 0) || (r >= AT_MAX))
      return 1;

   if ((row == NULL) || (token == NULL) || (weapon_class == NULL) || (mode == NULL))
      return 1;

   if (validate_row(r, row, "$$\\cof\\$$$$$$$.cof") != 0)
      return 1;

   n = strlen(myglobals.datas.root_path[r]);

   cptr = (char *) token;
   cptr[0] = row[n];
   cptr[1] = row[n + 1];
   cptr[2] = 0;

   cptr = (char *) token2;
   cptr[0] = row[n + 7];
   cptr[1] = row[n + 8];
   cptr[2] = 0;

   cptr = (char *) mode;
   cptr[0] = row[n +  9];
   cptr[1] = row[n + 10];
   cptr[2] = 0;

   cptr = (char *) weapon_class;
   cptr[0] = row[n + 11];
   cptr[1] = row[n + 12];
   cptr[2] = row[n + 13];
   cptr[3] = 0;

   make_string_uppercase((char *) token);
   make_string_uppercase((char *) token2);
   make_string_uppercase((char *) mode);
   make_string_uppercase((char *) weapon_class);

   if (strcmp((char *) token, (char *) token2) != 0)
      return 1;

   return 0;
}


// ===========================================================================
// validate the row from a given format
// special format code : '\\' = must be '\\', '$' = any letter > 32 except '\\'
// other format code   : must be the same letter (case insensitive)
// return 0 if the row is of the expected format
// ===========================================================================
int validate_row(ENUM_ANIMTYPE r, char * row, char * format)
{
   int n  = 0;
   int x  = 0;
   int c  = 0;
   int cf = 0;


   if ((r < 0) || (r >= AT_MAX))
      return 1;

   if ((row == NULL) || (format == NULL))
      return 1;

   n = strlen(myglobals.datas.root_path[r]);
   if (row[n] == 0)
      return 1;

   for (x = 0; format[x] != 0; x++)
   {
      if (format[x] < 32)
         return 1;

      c = row[n + x];
      if (c == 0)
         return 1;

      if (format[x] == '$')
      {
         if ((c <= 32) || (c == '\\'))
            return 1;
      }
      else
      {
         cf = format[x];
         make_letter_uppercase( & c);
         make_letter_uppercase( & cf);
         if (c != cf)
            return 1;
      }
   }

   if (row[n + x] != 0)
      return 1;

   return 0;
}


// ===========================================================================
// return the type of the row, or AT_NULL if error
// ===========================================================================
ENUM_ANIMTYPE get_root_path_type(char * row)
{
   int  i      = 0;
   char * root = NULL;


   if (row == NULL)
      return AT_NULL;

   for (i = 0; i < AT_MAX; i++)
   {
      root = myglobals.datas.root_path[i];
      if (root == NULL)
         continue;

      if (_strnicmp(row, root, strlen(root)) == 0)
         return (ENUM_ANIMTYPE) i;
   }

   return AT_NULL;
}


// ===========================================================================
// destroy all the listfiles datas
// ===========================================================================
void destroy_listfiles(void)
{
   LISTFILE_ANIMTYPE * a                = NULL;
   TOKEN_DATAS       * td               = NULL;
   WEAPCLASS_DATAS   * wd               = NULL;
   COF_ROW           * f                = NULL;
   COF_ROW           * n                = NULL;
   CMAP_ROW          * pt_cmap_row      = NULL;
   CMAP_ROW          * pt_cmap_row_next = NULL;
   int               r                  = 0;
   long              t                  = 0;
   long              w                  = 0;
   long              c                  = 0;


   for (r = 0; r < AT_MAX; r++)
   {
      a = & myglobals.listfile_datas.at[r];

      f = a->first_COF;
      while (f != NULL)
      {
         n = f->next;
         DESTROYME(f, free)
         f = n;
      }
      a->first_COF = NULL;

      if (a->tab_COF != NULL)
      {
         for (c = 0; c < a->nb_cofs; c++)
         {
            DESTROYME(a->tab_COF[c].priority,    free)
            DESTROYME(a->tab_COF[c].layer_datas, free)
            a->tab_COF[c].nb_layers = 0;
         }
         DESTROYME(a->tab_COF, free)
         a->nb_cofs = 0;
      }

      if (a->tab_token != NULL)
      {
         for (t = 0; t < a->nb_tokens; t++)
         {
            td = & a->tab_token[t];
            if (td->tab_weapon_class != NULL)
            {
               for (w = 0; w < td->nb_weapon_class; w++)
               {
                  wd = & td->tab_weapon_class[w]; 
                  DESTROYME(wd->tab_mode_code, free)
               }
               DESTROYME(td->tab_weapon_class, free)
               td->nb_weapon_class = 0;
            }
         }
         DESTROYME(a->tab_token, free)
         a->nb_tokens = 0;
      }

      DESTROYME(a->tab_DCC, free)
      a->nb_dcc = 0;
   }

   // colormaps rows from listfiles
   pt_cmap_row = myglobals.listfile_datas.first_CMAP;
   while (pt_cmap_row != NULL)
   {
      pt_cmap_row_next = pt_cmap_row->next;
      DESTROYME(pt_cmap_row->filename, free)
      DESTROYME(pt_cmap_row, free)
      pt_cmap_row = pt_cmap_row_next;
   }
   myglobals.listfile_datas.first_CMAP = NULL;

   // colormaps linked list and table of pointers
   destroy_colormaps_linked_list();
   destroy_tab_cmap_file();
}


// ===========================================================================
// qsort helper function for comparing 2 COF rows (case insensitive)
// order : token, weapon class, mode
// ===========================================================================
int qsort_helper_COF(const void * a, const void * b)
{
   COF_ROW_EXISTS * ca = (COF_ROW_EXISTS *) a;
   COF_ROW_EXISTS * cb = (COF_ROW_EXISTS *) b;
   int            r    = 0;


   if ((a == NULL) || (b == NULL))
      return 0;

   r = stricmp(ca->token, cb->token);
   if (r == 0)
   {
      r = stricmp(ca->weapon_class, cb->weapon_class);
      if (r == 0)
         return stricmp(ca->mode, cb->mode);
      else
         return r;
   }
   else
      return r;
}
 

// ===========================================================================
// build the table of unique COF datas for this animation type
// return 0 on success
// ===========================================================================
int build_COF_existence_list(int r)
{
   LISTFILE_DATAS    * d        = NULL;
   LISTFILE_ANIMTYPE * at       = NULL;
   COF_ROW           * f        = NULL;
   long              nb_cofs    = 0;
   COF_ROW           * c        = NULL;
   COF_ROW           * n        = NULL;
   COF_ROW_EXISTS    * tab      = NULL;
   long              i          = 0;
   long              nb_uniques = 0;
   long              idx        = 0;


   d = & myglobals.listfile_datas;

   if ((r < 0) || (r >= AT_MAX))
      return 1;

   at = & d->at[r];
   f  = at->first_COF;

   // count number of COF found in the different listfiles (some are probably here multiple times)
   c = f;
   while (c != NULL)
   {
      nb_cofs++;
      c = c->next;
   }

   if (nb_cofs <= 1)
      return 1;

   // prepare a table to put them all
   tab = (COF_ROW_EXISTS *) calloc(nb_cofs, sizeof(COF_ROW_EXISTS));
   if (tab == NULL)
      return 1;

   // copy them
   c = f;
   i = 0;
   while (c != NULL)
   {
      strncpy(tab[i].token,        c->token,        sizeof(STR_2_LETTERS));
      strncpy(tab[i].weapon_class, c->weapon_class, sizeof(STR_3_LETTERS));
      strncpy(tab[i].mode,         c->mode,         sizeof(STR_2_LETTERS));
      tab[i].exists          = FALSE;
      tab[i].already_decoded = FALSE;
      i++;

      c = c->next;
   }

   // no need for the COF linked list anymore
   c = f;
   while (c != NULL)
   {
      n = c->next;
      DESTROYME(c, free)
      c = n;
   }
   at->first_COF = NULL;
   f             = NULL;

   // sort the table
   qsort(tab, nb_cofs, sizeof(COF_ROW_EXISTS), qsort_helper_COF);

   // identify each first unique element, count them along the way
   tab[0].exists = 1;
   nb_uniques = 1;
   for (i = 1; i < nb_cofs; i++)
   {
      if (qsort_helper_COF((const void *) & tab[i - 1], (const void *) & tab[i]) != 0)
      {
         tab[i].exists = 1;
         nb_uniques++;
      }
   }

   // create the table of unique COF to check for existence in MPQ
   at->nb_cofs = nb_uniques;
   at->tab_COF = (COF_ROW_EXISTS *) calloc(nb_uniques, sizeof(COF_ROW_EXISTS));
   if (at->tab_COF == NULL)
      return 1;

   // fill it
   idx = 0;
   for (i = 0; i < nb_cofs; i++)
   {
      if (tab[i].exists == 1)
      {
         if (idx >= nb_uniques)
         {
            DESTROYME(tab, free)
            DESTROYME(at->tab_COF, free)
            at->nb_cofs = 0;
            return 1;
         }
         else
         {
            memcpy( & at->tab_COF[idx], & tab[i], sizeof(COF_ROW_EXISTS));
            at->tab_COF[idx].exists = FALSE;
            idx++;         
         }
      }
   }

   return 0;
}


// ===========================================================================
// check the existence of all COF and DCC for this animation type
// update the progress bar along the way when needed
// return 0 on success
// ===========================================================================
int test_existence_list(struct test_existence_list_PARAMS_S * params)
{
   LISTFILE_DATAS    * d                 = NULL;
   LISTFILE_ANIMTYPE * at                = NULL;
   long              i                   = 0;
   static char       file    [MAX_PATH]  = "";
   static WCHAR      wc_file [MAX_PATH]  = TEXT("");
   static WCHAR      wc_path [MAX_PATH]  = TEXT("");
   DWORD             attr                = 0;
   int               m                   = 0;
   int               mpq_order [MPQ_MAX] = {MPQ_MOD_DIRECTORY, MPQ_D2DATA, MPQ_D2EXP, MPQ_D2CHAR, MPQ_PATCH_D2}; // prefered order, less total tests in MPQ that way
   long              tmp_step            = 0;
   static WCHAR      wcString [300]      = TEXT("");
   int               found               = FALSE;


   memset(file,     0, sizeof(file));
   memset(wc_file,  0, sizeof(wc_file));
   memset(wc_path,  0, sizeof(wc_path));
   memset(wcString, 0, sizeof(wcString));

   if (params == NULL)
      return 1;

   d = & myglobals.listfile_datas;

   if ((params->r < 0) || (params->r >= AT_MAX) || (params->hpbar == NULL) || (params->hpnct == NULL))
      return 1;

   at = & d->at[params->r];

   // test all COF one by one
   for (i = 0; i < at->nb_cofs; i++)
   {
      sprintf(file, "%s%s\\cof\\%s%s%s.cof", myglobals.datas.root_path[params->r], at->tab_COF[i].token, at->tab_COF[i].token, at->tab_COF[i].mode, at->tab_COF[i].weapon_class);
      found = FALSE;
      for (m = 0; (m < MPQ_MAX) && (found == FALSE); m++)
      {
         if (mpq_order[m] == MPQ_MOD_DIRECTORY)
         {
            char_to_wide_char(file, wc_file, sizeof(wc_file));
            swprintf(wc_path, MAX_PATH, TEXT("%s%s"), myglobals.datas.mpq[MPQ_MOD_DIRECTORY].path, wc_file);
            attr = GetFileAttributes(wc_path);
            if ((attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0))
            {
               found = TRUE;
               break;
            }
         }
         else if (test_file_existence_from_mpq(mpq_order[m], file) == TRUE)
         {
            found = TRUE;
            break;
         }
      }

      // debug
      if (params->debug_exists_file != NULL)
      {
         fwprintf(params->debug_exists_file, TEXT("%s\t"), wc_file);
         if (found == FALSE)
            fwprintf(params->debug_exists_file, TEXT("NOT found\n"));
         else
         {
            switch(mpq_order[m])
            {
               case MPQ_MOD_DIRECTORY : fwprintf(params->debug_exists_file, TEXT("Mod directory\n")); break;
               case MPQ_D2DATA        : fwprintf(params->debug_exists_file, TEXT("d2data.mpq\n"));    break;
               case MPQ_D2EXP         : fwprintf(params->debug_exists_file, TEXT("d2exp.mpq\n"));     break;
               case MPQ_D2CHAR        : fwprintf(params->debug_exists_file, TEXT("d2char.mpq\n"));    break;
               case MPQ_PATCH_D2      : fwprintf(params->debug_exists_file, TEXT("patch_d2.mpq\n"));  break;
               default : fwprintf(params->debug_exists_file, TEXT("? (%d)\n"), m);  break;
            }
         }
      }

      // if found, keep a trace and update the existence counter
      if (found == TRUE)
      {
         at->tab_COF[i].exists = TRUE;
         params->curr_exists++;
         switch (params->r)
         {
            case AT_PLAYERS  : params->nb_tokens_players++;  break;
            case AT_MONSTERS : params->nb_tokens_monsters++; break;
            case AT_OBJECTS  : params->nb_tokens_objects++;  break;
         }
      }

      // update the progress bar, if needed
      params->curr_pos++;
      tmp_step = 100 * params->curr_pos / params->nb_files; // update the progress bar only 100 times
      if (tmp_step != params->curr_step)
      {
         params->curr_step = tmp_step;
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos + 1, (LPARAM) 0);
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos, (LPARAM) 0); // if you wonder why we do this, check http://stackoverflow.com/questions/1061715/how-do-i-make-tprogressbar-stop-lagging

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld / %ld"), params->curr_pos, params->nb_files);
         SendMessage(params->hpnct, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld\n%ld\n%ld\n%ld"), params->curr_exists, params->nb_tokens_players, params->nb_tokens_monsters, params->nb_tokens_objects);
         SendMessage(params->hexists, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);
      }
   }

   // test all DCC one by one
   for (i = 0; i < at->nb_dcc; i++)
   {
      sprintf(
         file,
         "%s%s\\%s\\%s%s%s%s%s.dcc",
         myglobals.datas.root_path[params->r],
         at->tab_DCC[i].token,
         at->tab_DCC[i].layer,
         at->tab_DCC[i].token,
         at->tab_DCC[i].layer,
         at->tab_DCC[i].variant,
         at->tab_DCC[i].mode,
         at->tab_DCC[i].weapon_class
      );

      found = FALSE;
      for (m = 0; (m < MPQ_MAX) && (found == FALSE); m++)
      {
         if (mpq_order[m] == MPQ_MOD_DIRECTORY)
         {
            char_to_wide_char(file, wc_file, sizeof(wc_file));
            swprintf(wc_path, MAX_PATH, TEXT("%s%s"), myglobals.datas.mpq[MPQ_MOD_DIRECTORY].path, wc_file);
            attr = GetFileAttributes(wc_path);
            if ((attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0))
            {
               found = TRUE;
               break;
            }
         }
         else if (test_file_existence_from_mpq(mpq_order[m], file) == TRUE)
         {
            found = TRUE;
            break;
         }
      }

      // debug
      if (params->debug_exists_file != NULL)
      {
         fwprintf(params->debug_exists_file, TEXT("%s\t"), wc_file);
         if (found == FALSE)
            fwprintf(params->debug_exists_file, TEXT("NOT found\n"));
         else
         {
            switch(mpq_order[m])
            {
               case MPQ_MOD_DIRECTORY : fwprintf(params->debug_exists_file, TEXT("Mod directory\n")); break;
               case MPQ_D2DATA        : fwprintf(params->debug_exists_file, TEXT("d2data.mpq\n"));    break;
               case MPQ_D2EXP         : fwprintf(params->debug_exists_file, TEXT("d2exp.mpq\n"));     break;
               case MPQ_D2CHAR        : fwprintf(params->debug_exists_file, TEXT("d2char.mpq\n"));    break;
               case MPQ_PATCH_D2      : fwprintf(params->debug_exists_file, TEXT("patch_d2.mpq\n"));  break;
               default : fwprintf(params->debug_exists_file, TEXT("? (%d)\n"), m);  break;
            }
         }
      }

      // DC6
      if (found == FALSE)
      {
         m = strlen(file);
         if (m > 1)
         {
            file[m - 1] = '6';
            for (m = 0; (m < MPQ_MAX) && (found == FALSE); m++)
            {
               if (mpq_order[m] == MPQ_MOD_DIRECTORY)
               {
                  char_to_wide_char(file, wc_file, sizeof(wc_file));
                  swprintf(wc_path, MAX_PATH, TEXT("%s%s"), myglobals.datas.mpq[MPQ_MOD_DIRECTORY].path, wc_file);
                  attr = GetFileAttributes(wc_path);
                  if ((attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0))
                  {
                     found = TRUE;
                     break;
                  }
               }
               else if (test_file_existence_from_mpq(mpq_order[m], file) == TRUE)
               {
                  found = TRUE;
                  break;
               }
            }
         }

         // debug
         if (params->debug_exists_file != NULL) 
         {
            fwprintf(params->debug_exists_file, TEXT("%s\t"), wc_file);
            if (found == FALSE)
               fwprintf(params->debug_exists_file, TEXT("NOT found\n"));
            else
            {
               switch(mpq_order[m])
               {
                  case MPQ_MOD_DIRECTORY : fwprintf(params->debug_exists_file, TEXT("Mod directory\n")); break;
                  case MPQ_D2DATA        : fwprintf(params->debug_exists_file, TEXT("d2data.mpq\n"));    break;
                  case MPQ_D2EXP         : fwprintf(params->debug_exists_file, TEXT("d2exp.mpq\n"));     break;
                  case MPQ_D2CHAR        : fwprintf(params->debug_exists_file, TEXT("d2char.mpq\n"));    break;
                  case MPQ_PATCH_D2      : fwprintf(params->debug_exists_file, TEXT("patch_d2.mpq\n"));  break;
                  default : fwprintf(params->debug_exists_file, TEXT("? (%d)\n"), m);  break;
               }
            }
         }
      }

      // if found, keep a trace and update the existence counter
      if (found == TRUE)
      {
         at->tab_DCC[i].exists = TRUE;
         params->curr_exists++;
         switch (params->r)
         {
            case AT_PLAYERS  : params->nb_tokens_players++;  break;
            case AT_MONSTERS : params->nb_tokens_monsters++; break;
            case AT_OBJECTS  : params->nb_tokens_objects++;  break;
         }
      }

      // update the progress bar, if needed
      params->curr_pos++;
      tmp_step = 100 * params->curr_pos / params->nb_files; // update the progress bar only 100 times
      if (tmp_step != params->curr_step)
      {
         params->curr_step = tmp_step;
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos + 1, (LPARAM) 0);
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos, (LPARAM) 0); // if you wonder why we do this, check http://stackoverflow.com/questions/1061715/how-do-i-make-tprogressbar-stop-lagging

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld / %ld"), params->curr_pos, params->nb_files);
         SendMessage(params->hpnct, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld\n%ld\n%ld\n%ld"), params->curr_exists, params->nb_tokens_players, params->nb_tokens_monsters, params->nb_tokens_objects);
         SendMessage(params->hexists, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);
      }
   }

   return 0;
}


// ===========================================================================
// check the existence of all colormaps
// update the progress bar along the way when needed
// return 0 on success
// ===========================================================================
int test_existence_list_colormaps(struct test_existence_list_PARAMS_S * params)
{
   LISTFILE_DATAS    * d                 = NULL;
   static WCHAR      wc_file [MAX_PATH]  = TEXT("");
   static WCHAR      wc_path [MAX_PATH]  = TEXT("");
   DWORD             attr                = 0;
   int               m                   = 0;
   int               mpq_order [MPQ_MAX] = {MPQ_MOD_DIRECTORY, MPQ_D2DATA, MPQ_D2EXP, MPQ_D2CHAR, MPQ_PATCH_D2}; // prefered order, less total tests in MPQ that way
   long              tmp_step            = 0;
   static WCHAR      wcString [300]      = TEXT("");
   int               found               = FALSE;
   CMAP_ROW          * pt_cmap_row       = NULL;


   memset(wc_file,  0, sizeof(wc_file));
   memset(wc_path,  0, sizeof(wc_path));
   memset(wcString, 0, sizeof(wcString));

   if (params == NULL)
      return 1;

   d = & myglobals.listfile_datas;

   if ((params->hpbar == NULL) || (params->hpnct == NULL))
      return 1;

   // test all colormaps one by one
   pt_cmap_row = d->first_CMAP;
   while (pt_cmap_row != NULL)
   {
      found = FALSE;
      for (m = 0; (m < MPQ_MAX) && (found == FALSE); m++)
      {
         if (mpq_order[m] == MPQ_MOD_DIRECTORY)
         {
            char_to_wide_char(pt_cmap_row->filename, wc_file, sizeof(wc_file));
            swprintf(wc_path, MAX_PATH, TEXT("%s%s"), myglobals.datas.mpq[MPQ_MOD_DIRECTORY].path, wc_file);
            attr = GetFileAttributes(wc_path);
            if ((attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0))
            {
               found = TRUE;
               break;
            }
         }
         else if (test_file_existence_from_mpq(mpq_order[m], pt_cmap_row->filename) == TRUE)
         {
            found = TRUE;
            break;
         }
      }

      // debug
      if (params->debug_exists_file != NULL)
      {
         fwprintf(params->debug_exists_file, TEXT("%s\t"), wc_file);
         if (found == FALSE)
            fwprintf(params->debug_exists_file, TEXT("NOT found\n"));
         else
         {
            switch(mpq_order[m])
            {
               case MPQ_MOD_DIRECTORY : fwprintf(params->debug_exists_file, TEXT("Mod directory\n")); break;
               case MPQ_D2DATA        : fwprintf(params->debug_exists_file, TEXT("d2data.mpq\n"));    break;
               case MPQ_D2EXP         : fwprintf(params->debug_exists_file, TEXT("d2exp.mpq\n"));     break;
               case MPQ_D2CHAR        : fwprintf(params->debug_exists_file, TEXT("d2char.mpq\n"));    break;
               case MPQ_PATCH_D2      : fwprintf(params->debug_exists_file, TEXT("patch_d2.mpq\n"));  break;
               default : fwprintf(params->debug_exists_file, TEXT("? (%d)\n"), m);  break;
            }
         }
      }

      // if found, keep a trace and update the existence counter
      if (found == TRUE)
      {
         pt_cmap_row->exists = TRUE;
         params->curr_exists++;
         params->nb_colormaps++;
      }

      // update the progress bar, if needed
      params->curr_pos++;
      tmp_step = 100 * params->curr_pos / params->nb_files; // update the progress bar only 100 times
      if (tmp_step != params->curr_step)
      {
         params->curr_step = tmp_step;
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos + 1, (LPARAM) 0);
         SendMessage(params->hpbar, PBM_SETPOS, (WPARAM) params->curr_pos, (LPARAM) 0); // if you wonder why we do this, check http://stackoverflow.com/questions/1061715/how-do-i-make-tprogressbar-stop-lagging

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld / %ld"), params->curr_pos, params->nb_files);
         SendMessage(params->hpnct, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);

         swprintf(wcString, sizeof(wcString) / 2, TEXT("%ld\n%ld\n%ld\n%ld\n%ld"), params->curr_exists, params->nb_tokens_players, params->nb_tokens_monsters, params->nb_tokens_objects, params->nb_colormaps);
         SendMessage(params->hexists, WM_SETTEXT, (WPARAM) 0, (LPARAM) wcString);
      }

      pt_cmap_row = pt_cmap_row->next;
   }

   return 0;
}


// ===========================================================================
// scan all COF of all animation types, and prepare tokens + weapon classes +
//   modes structures for all the COF selection listboxes
// return 0 on success
// ===========================================================================
int prepare_COF_selection_listboxes(void)
{
   LISTFILE_ANIMTYPE * d      = NULL;
   COF_ROW_EXISTS    * r      = NULL;
   TOKEN_DATAS       * td     = NULL;
   WEAPCLASS_DATAS   * wd     = NULL;
   int               a        = 0;
   long              c        = 0;
   long              last_c   = 0;
   STR_3_LETTERS     old_code = "";
   long              idx      = 0;
   long              t        = 0;
   long              w        = 0;


   for (a = 0; a < AT_MAX; a++)
   {
      d = & myglobals.listfile_datas.at[a];

      // count number of tokens
      d->nb_tokens = 0;
      strcpy(old_code, "");
      for (c = 0; c < d->nb_cofs; c++)
      {
         r = & d->tab_COF[c];
         if (r->exists != TRUE)
            continue;

         if (_stricmp(old_code, r->token) != 0)
         {
            d->nb_tokens++;
            strcpy(old_code, r->token);
         }
      }

      if (d->nb_tokens <= 0)
         continue;

      // allocate memory for the tokens table
      d->tab_token = (TOKEN_DATAS *) calloc(d->nb_tokens, sizeof(TOKEN_DATAS));
      if (d->tab_token == NULL)
         return 1;

      // copy tokens
      strcpy(old_code, "");
      idx = 0;
      for (c = 0; c < d->nb_cofs; c++)
      {
         r = & d->tab_COF[c];
         if (r->exists != TRUE)
            continue;

         if (_stricmp(old_code, r->token) != 0)
         {
            if (idx >= d->nb_tokens)
               return 1;

            strcpy(old_code, r->token);
            strcpy(d->tab_token[idx].code, r->token);
            d->tab_token[idx].first_COF_idx = c;
            if (idx > 0)
               d->tab_token[idx - 1].nb_cofs_for_token = c - d->tab_token[idx - 1].first_COF_idx;
            idx++;
         }
      }

      if (idx > 0)
         d->tab_token[idx - 1].nb_cofs_for_token = c - d->tab_token[idx - 1].first_COF_idx;

      // for each token
      for (t = 0; t < d->nb_tokens; t++)
      {
         td = & d->tab_token[t];
         last_c = td->first_COF_idx + td->nb_cofs_for_token;
         td->nb_weapon_class = 0;
         strcpy(old_code, "");

         // count number of weapon classes
         for (c = td->first_COF_idx; c < last_c; c++)
         {
            r = & d->tab_COF[c];
            if (r->exists != TRUE)
               continue;

            if (_stricmp(old_code, r->weapon_class) != 0)
            {
               td->nb_weapon_class++;
               strcpy(old_code, r->weapon_class);
            }
         }

         if (td->nb_weapon_class <= 0)
            return 1;

         // allocate memory for the weapon classes table
         td->tab_weapon_class = (WEAPCLASS_DATAS *) calloc(td->nb_weapon_class, sizeof(WEAPCLASS_DATAS));
         if (td->tab_weapon_class == NULL)
            return 1;

         // copy weapon classes
         strcpy(old_code, "");
         idx = 0;
         for (c = td->first_COF_idx; c < last_c; c++)
         {
            r = & d->tab_COF[c];
            if (r->exists != TRUE)
               continue;

            if (_stricmp(old_code, r->weapon_class) != 0)
            {
               if (idx >= td->nb_weapon_class)
                  return 1;

               strcpy(old_code, r->weapon_class);
               strcpy(td->tab_weapon_class[idx].code, r->weapon_class);
               td->tab_weapon_class[idx].first_COF_idx = c;
               if (idx > 0)
                  td->tab_weapon_class[idx - 1].nb_cofs_for_weapon_class = c - td->tab_weapon_class[idx - 1].first_COF_idx;
               idx++;
            }
         }

         if (idx > 0)
            td->tab_weapon_class[idx - 1].nb_cofs_for_weapon_class = c - td->tab_weapon_class[idx - 1].first_COF_idx;

         // for each weapon class
         for (w = 0; w < td->nb_weapon_class; w++)
         {
            wd = & td->tab_weapon_class[w];
            last_c = wd->first_COF_idx + wd->nb_cofs_for_weapon_class;

            // allocate memory for the modes table
            wd->tab_mode_code = (STR_2_LETTERS *) calloc(wd->nb_cofs_for_weapon_class, sizeof(STR_2_LETTERS));
            if (wd->tab_mode_code == NULL)
               return 1;

            // copy modes
            idx = 0;
            for (c = wd->first_COF_idx; c < last_c; c++)
            {
               r = & d->tab_COF[c];
               if (r->exists != TRUE)
                  continue;

               if (idx >= wd->nb_cofs_for_weapon_class)
                  return 1;

               strcpy(wd->tab_mode_code[idx], r->mode);
               idx++;
            }

            wd->nb_modes = idx;
         }
      }
   }

   return 0;
}


// ===========================================================================
// extract the needed elements of a COF row, and save it for later in a structure
// ===========================================================================
void process_DCC_row(char * row)
{
   ENUM_ANIMTYPE r   = AT_NULL;
   DCC_ROW       * p = NULL;
   DCC_ROW       d   = {0};


   if (row == NULL)
      return;

   r = get_root_path_type(row);
   if (r == AT_NULL)
      return;

   memset ( & d, 0, sizeof(d));
   if (extract_DCC_row_elements(r, row, & d.token, & d.weapon_class, & d.mode, & d.layer, & d.variant) == 0)
   {
      p = (DCC_ROW *) malloc(sizeof(d));
      if (p == NULL)
         return;

      memcpy(p, & d, sizeof(d));
      p->next = myglobals.listfile_datas.at[r].first_DCC;
      myglobals.listfile_datas.at[r].first_DCC = p;
   }

   return;
}


// ===========================================================================
// from a DCC row, extract token, weapon class, mode, layer and variant, in uppercase
// return 0 if success
// ===========================================================================
int extract_DCC_row_elements(ENUM_ANIMTYPE r, char * row, STR_2_LETTERS * token, STR_3_LETTERS * weapon_class, STR_2_LETTERS * mode, STR_2_LETTERS * layer, STR_3_LETTERS * variant)
{
   int           n      = 0;
   char          * cptr = NULL;
   STR_2_LETTERS token2 = "";
   STR_2_LETTERS layer2 = "";


   if (r == AT_NULL)
      return 1;

   if ((r < 0) || (r >= AT_MAX))
      return 1;

   if ((row == NULL) || (token == NULL) || (weapon_class == NULL) || (mode == NULL) || (layer == NULL) || (variant == NULL))
      return 1;

   if (validate_row(r, row, "$$\\$$\\$$$$$$$$$$$$.dcc") != 0)
   {
      if (validate_row(r, row, "$$\\$$\\$$$$$$$$$$$$.dc6") != 0)
         return 1;
   }

   n = strlen(myglobals.datas.root_path[r]);

   cptr = (char *) token;
   cptr[0] = row[n];
   cptr[1] = row[n + 1];
   cptr[2] = 0;

   cptr = (char *) layer;
   cptr[0] = row[n + 3];
   cptr[1] = row[n + 4];
   cptr[2] = 0;

   cptr = (char *) token2;
   cptr[0] = row[n + 6];
   cptr[1] = row[n + 7];
   cptr[2] = 0;

   cptr = (char *) layer2;
   cptr[0] = row[n + 8];
   cptr[1] = row[n + 9];
   cptr[2] = 0;

   cptr = (char *) variant;
   cptr[0] = row[n + 10];
   cptr[1] = row[n + 11];
   cptr[2] = row[n + 12];
   cptr[3] = 0;

   cptr = (char *) mode;
   cptr[0] = row[n + 13];
   cptr[1] = row[n + 14];
   cptr[2] = 0;

   cptr = (char *) weapon_class;
   cptr[0] = row[n + 15];
   cptr[1] = row[n + 16];
   cptr[2] = row[n + 17];
   cptr[3] = 0;

   make_string_uppercase((char *) token);
   make_string_uppercase((char *) token2);
   make_string_uppercase((char *) layer);
   make_string_uppercase((char *) layer2);
   make_string_uppercase((char *) mode);
   make_string_uppercase((char *) weapon_class);
   make_string_uppercase((char *) variant);

   if (strcmp((char *) token, (char *) token2) != 0)
      return 1;

   if (strcmp((char *) layer, (char *) layer2) != 0)
      return 1;

   return 0;
}


// ===========================================================================
// qsort helper function for comparing 2 DCC rows (case insensitive)
// order : token, weapon class, mode, layer, variant
// ===========================================================================
int qsort_helper_DCC(const void * a, const void * b)
{
    DCC_ROW_EXISTS * ca = (DCC_ROW_EXISTS *) a;
    DCC_ROW_EXISTS * cb = (DCC_ROW_EXISTS *) b;
    int            r    = 0;


    if ((a == NULL) || (b == NULL))
        return 0;

    r = stricmp(ca->token, cb->token);
    if (r == 0)
    {
        r = stricmp(ca->weapon_class, cb->weapon_class);
        if (r == 0)
        {
            r = stricmp(ca->mode, cb->mode);
            if (r == 0)
            {
                r = stricmp(ca->layer, cb->layer);
                if (r == 0)
                    return stricmp(ca->variant, cb->variant);
                else
                    return r;
            }
            else
                return r;
        }
        else
            return r;
    }
    else
        return r;
}


// ===========================================================================
// build the table of unique DCC datas for this animation type
// return 0 on success
// ===========================================================================
int build_DCC_existence_list(int r)
{
   LISTFILE_DATAS    * d        = NULL;
   LISTFILE_ANIMTYPE * at       = NULL;
   DCC_ROW           * f        = NULL;
   long              nb_dcc     = 0;
   DCC_ROW           * c        = NULL;
   DCC_ROW           * n        = NULL;
   DCC_ROW_EXISTS    * tab      = NULL;
   long              i          = 0;
   long              nb_uniques = 0;
   long              idx        = 0;


   d = & myglobals.listfile_datas;

   if ((r < 0) || (r >= AT_MAX))
      return 1;

   at = & d->at[r];
   f  = at->first_DCC;

   // count number of DCC found in the different listfiles (some are probably here multiple times)
   c = f;
   while (c != NULL)
   {
      nb_dcc++;
      c = c->next;
   }

   if (nb_dcc <= 1)
      return 1;

   // prepare a table to put them all
   tab = (DCC_ROW_EXISTS *) calloc(nb_dcc, sizeof(DCC_ROW_EXISTS));
   if (tab == NULL)
      return 1;

   // copy them
   c = f;
   for (i = 0; i < nb_dcc; i++)
   {
      if (c == NULL)
      {
         DESTROYME(tab, free)
         return 1;
      }

      strncpy(tab[i].token,        c->token,        sizeof(STR_2_LETTERS));
      strncpy(tab[i].weapon_class, c->weapon_class, sizeof(STR_3_LETTERS));
      strncpy(tab[i].mode,         c->mode,         sizeof(STR_2_LETTERS));
      strncpy(tab[i].layer,        c->layer,        sizeof(STR_2_LETTERS));
      strncpy(tab[i].variant,      c->variant,      sizeof(STR_3_LETTERS));
      tab[i].exists = FALSE;

      c = c->next;
   }

   if (c != NULL)
   {
      DESTROYME(tab, free)
      return 1;
   }

   // no need for the DCC linked list anymore
   c = f;
   while (c != NULL)
   {
      n = c->next;
      DESTROYME(c, free)
      c = n;
   }
   at->first_DCC = NULL;

   // sort the table
   qsort(tab, nb_dcc, sizeof(DCC_ROW_EXISTS), qsort_helper_DCC);

   // identify each first unique element, count them along the way
   tab[0].exists = 1;
   nb_uniques = 1;
   for (i = 1; i < nb_dcc; i++)
   {
      if (qsort_helper_DCC((const void *) & tab[i - 1], (const void *) & tab[i]) != 0)
      {
         tab[i].exists = 1;
         nb_uniques++;
      }
   }

   // create the table of unique DCC to check for existence in MPQ
   at->nb_dcc = nb_uniques;
   at->tab_DCC = (DCC_ROW_EXISTS *) calloc(nb_uniques, sizeof(DCC_ROW_EXISTS));
   if (at->tab_DCC == NULL)
      return 1;

   // fill it
   idx = 0;
   for (i = 0; i < nb_dcc; i++)
   {
      if (tab[i].exists == 1)
      {
         if (idx >= nb_uniques)
         {
            DESTROYME(tab, free)
            DESTROYME(at->tab_DCC, free)
            return 1;
         }
         else
         {
            memcpy(at->tab_DCC + idx, tab + i, sizeof(DCC_ROW_EXISTS));
            at->tab_DCC[idx].exists = FALSE;
            idx++;         
         }
      }
   }

   return 0;
}


// ===========================================================================
// 
// ===========================================================================
int prepare_CMAP_selection_listboxes(void)
{

   int            i             = 0;
   ENUM_CMAP_TYPE type          = 0;
   CMAP_ROW       * pt_cmap_row = NULL;
   char           * buffer      = NULL;
   long           length        = 0;
   COLORMAP_FILE  * cmap        = NULL;
   int            n             = 0;
   int            * usable      = NULL;
   int            k             = 0;
   UBYTE          * color       = NULL;
   int            nb_colormaps  = 0;


   // load all colormaps
   pt_cmap_row = myglobals.listfile_datas.first_CMAP;
   while (pt_cmap_row != NULL)
   {
      if (pt_cmap_row->exists == TRUE)
      {
         // load the file
         buffer = NULL;
         length = 0;
         if (load_file(pt_cmap_row->filename, & buffer, & length, FALSE) == 0)
         {
            // analyse the file
            switch (pt_cmap_row->num_row_extension)
            {
               case REXT_DAT_CMAP_ITM :
                  type = CMAPTYPE_PLAYER;
                  break;

               case REXT_DAT_CMAP_MON_PALSHIFT       :
               case REXT_DAT_CMAP_MON_GREENBLOOD     :
               case REXT_DAT_CMAP_MON_RANDTRANSFORMS :
                  type = CMAPTYPE_MONSTER;
                  break;

               default :
                  type = CMAPTYPE_NONE;
                  break;
            }

            if ((type == CMAPTYPE_PLAYER) || (type == CMAPTYPE_MONSTER))
            {
               // =================================
               // standard colormap file, not a PL2
               // =================================

               if ((length >= 256) && ((length % 256) == 0))
               {
                  // ok, file with N colormaps exactly, and at least 1
                  n      = length / 256;
                  usable = (int *) calloc(n, sizeof(int));
                  if (usable == NULL)
                     return 1;

                  // check all colormaps, put away 'identity' colormaps (that changes nothing)
                  nb_colormaps = 0;
                  for (i = 0; i < n; i++)
                  {
                     color = (UBYTE *) & buffer[i * 256];
                     for (k = 0; k < 256; k++)
                     {
                        if (color[k] != k)
                           break;
                     }
                     if (k < 256)
                     {
                        usable[i] = 1;
                        nb_colormaps++;
                     }
                  }

                  // if some colormaps are identical, keeps only the first
                  if (nb_colormaps >= 2)
                  {
                     for (i = 0; i < n; i++)
                     {
                        if (usable[i] == 1)
                        {
                           for (k = i + 1; k < n; k++)
                           {
                              if (usable[k] == 1)
                              {
                                 if (memcmp(& buffer[i * 256], & buffer[k * 256], 256) == 0)
                                 {
                                    usable[k] = 0;
                                    nb_colormaps--;
                                 }
                              }
                           }
                        }
                     }
                  }

                  if (nb_colormaps > 0)
                  {
                     // at least 1 colormap is usable

                     // prepare a new colormap file element in memory
                     cmap = (COLORMAP_FILE *) calloc(1, sizeof(COLORMAP_FILE));
                     if (cmap == NULL)
                     {
                        DESTROYME(usable, free)
                        return 2;
                     }
                     cmap->tab_index = (int  *) calloc(nb_colormaps, sizeof(int));
                     cmap->tab_cmap  = (CMAP *) calloc(nb_colormaps, sizeof(CMAP));
                     if ((cmap->tab_index == NULL) || (cmap->tab_cmap == NULL))
                     {
                        DESTROYME(cmap->tab_index, free)
                        DESTROYME(cmap->tab_cmap,  free)
                        DESTROYME(cmap,            free)
                        DESTROYME(usable,          free)
                        return 3;
                     }

                     // fill it
                     cmap->nb_colormaps      = nb_colormaps;
                     cmap->num_cmap_type     = (ENUM_CMAP_TYPE) type;
                     cmap->num_row_extension = pt_cmap_row->num_row_extension;
                     k = 0;
                     for (i = 0; i < n; i++)
                     {
                        if (usable[i] == 1)
                        {
                           cmap->tab_index[k] = i;
                           memcpy(cmap->tab_cmap[k], & buffer[i * 256], 256);
                           k++;
                        }
                     }

                     // cmap->name
                     if (get_colormap_name_from_row_type(pt_cmap_row, cmap->name) != 0)
                     {
                        DESTROYME(cmap->tab_index, free)
                        DESTROYME(cmap->tab_cmap,  free)
                        DESTROYME(cmap,            free)
                        DESTROYME(usable,          free)
                        return 4;
                     }

                     // add this colormap to the temporary linked list
                     if (insert_colormap_link(cmap) != 0)
                     {
                        DESTROYME(cmap->tab_index, free)
                        DESTROYME(cmap->tab_cmap,  free)
                        DESTROYME(cmap,            free)
                        DESTROYME(usable,          free)
                        return 5;
                     }
                  }

                  DESTROYME(usable, free)
               }
            }
            else if (type == CMAPTYPE_PL2)
            {
               // =======================================
               // PL2 file, huge collections of colormaps
               // =======================================

               // TODO
            }

            // no need of the original file any longer
            DESTROYME(buffer, free)
         }
      }

      // next colormap row
      pt_cmap_row = pt_cmap_row->next;
   }

   // ========================================
   // add virtual files for the "Same as" type
   // ========================================

   for (i = 0; i < 16; i++)
   {
      // prepare a new colormap file element in memory
      cmap = (COLORMAP_FILE *) calloc(1, sizeof(COLORMAP_FILE));
      if (cmap == NULL)
         return 6;

      // fill it
      cmap->num_cmap_type     = CMAPTYPE_SAMEAS;
      cmap->num_row_extension = REXT_NULL;
      cmap->sameas_layer_code = i;

      // add this colormap to the temporary linked list
      if (insert_colormap_link(cmap) != 0)
      {
         DESTROYME(cmap, free)
         return 7;
      }
   }

   // ============================================================================================
   // linked list of colormaps is done, now use it to build a more handy table of pointers instead
   // ============================================================================================
   if (create_tab_cmap_file() != 0)
      return 8;

   return 0;
}


// ===========================================================================
// 
// ===========================================================================
int get_colormap_name_from_row_type(CMAP_ROW * pt_cmap_row, WCHAR * name)
{
   int           filename_size   = 0;
   int           start           = 0;
   int           length          = 0;
   char          tmp_name [100]  = "";
   COLORMAP_FILE * dummy_cmap    = NULL;


   if ((pt_cmap_row == NULL) || (name == NULL))
      return 1;

   filename_size = strlen(pt_cmap_row->filename);
   switch (pt_cmap_row->num_row_extension)
   {
      case REXT_DAT_CMAP_ITM                : // data/global/items/palette/*.dat
         start = strlen(D2_ROOT_PATH_ITEM_PALETTE);
         if ((start + 5) > filename_size)
            return 2;
         length = filename_size - start - 4;
         if ((length < 1) || (length >= sizeof(tmp_name)))
            return 3;
         if (length >= sizeof(dummy_cmap->name))
            return 4;
         break;

      case REXT_DAT_CMAP_MON_PALSHIFT       : // data/global/monsters/??/cof/palshift.dat
         start = strlen(D2_ROOT_PATH_MONSTERS);
         if ((start + (int) strlen(D2_STR_COF_PALSHIFT)) > filename_size)
            return 5;
         length = 2;
         break;

      case REXT_DAT_CMAP_MON_GREENBLOOD     : // data/global/monsters/greenblood.dat
      case REXT_DAT_CMAP_MON_RANDTRANSFORMS : // data/global/monsters/randtransforms.dat
         start  = strlen(D2_ROOT_PATH_MONSTERS);
         if ((start + 5) > filename_size)
            return 6;
         length = filename_size - start - 4;
         break;

      default :
         return 7;
   }

   if (length > 0)
   {
      strncpy_s(tmp_name, sizeof(tmp_name), & pt_cmap_row->filename[start], length);
      if (pt_cmap_row->num_row_extension == REXT_DAT_CMAP_MON_PALSHIFT)
         make_string_uppercase(tmp_name);
      char_to_wide_char(tmp_name, name, sizeof(dummy_cmap->name));
   }

   return 0;
}


// ===========================================================================
// 
// ===========================================================================
int insert_colormap_link(COLORMAP_FILE * pt_cmap)
{
   LISTFILE_DATAS     * lfd  = & myglobals.listfile_datas;
   COLORMAP_FILE_LINK * link = NULL;


   if (pt_cmap == NULL)
      return 1;

   link = (COLORMAP_FILE_LINK *) calloc(1, sizeof(COLORMAP_FILE_LINK));
   if (link == NULL)
      return 2;

   link->next    = lfd->first_colormap_link;
   link->pt_cmap = pt_cmap;

   lfd->first_colormap_link = link;

   return 0;
}


// ===========================================================================
// 
// ===========================================================================
void destroy_colormap_file(COLORMAP_FILE * cmap)
{
   if (cmap != NULL)
   {
      DESTROYME(cmap->tab_index, free)
      DESTROYME(cmap->tab_cmap,  free)
      cmap->nb_colormaps = 0;
      DESTROYME(cmap, free);
   }
}


// ===========================================================================
// 
// ===========================================================================
void destroy_colormaps_linked_list(void)
{
   LISTFILE_DATAS     * lfd  = & myglobals.listfile_datas;
   COLORMAP_FILE_LINK * link = NULL;
   COLORMAP_FILE_LINK * next = NULL;


   link = lfd->first_colormap_link;
   while (link != NULL)
   {
      next = link->next;

      DESTROYME(link->pt_cmap, destroy_colormap_file)
      link->next = NULL;
      DESTROYME(link, free)

      link = next;
   }

   lfd->first_colormap_link = NULL;
}


// ===========================================================================
// 
// ===========================================================================
void destroy_tab_cmap_file(void)
{
   LISTFILE_DATAS * lfd = & myglobals.listfile_datas;
   int            i     = 0;
   int            n     = lfd->nb_cmap_files;


   lfd->nb_cmap_files = 0;

   if (lfd->tab_cmap_file == NULL)
      return;

   for (i = 0; i < n; i++)
      DESTROYME(lfd->tab_cmap_file[i], destroy_colormap_file)

   DESTROYME(lfd->tab_cmap_file, free)
}


// ===========================================================================
// 
// ===========================================================================
int create_tab_cmap_file(void)
{
   LISTFILE_DATAS     * lfd            = & myglobals.listfile_datas;
   COLORMAP_FILE_LINK * link           = NULL;
   COLORMAP_FILE_LINK * next           = NULL;
   int                t                = 0;
   int                i                = 0;
   WCHAR              * file_to_search = NULL;
   COLORMAP_FILE      * cmapfile       = NULL;


   lfd->nb_cmap_files = 1; // first entry 0 is always present, and is reserved for "(none)"

   // count how many colormaps are in the temporary linked list
   link = lfd->first_colormap_link;
   while (link != NULL)
   {
      lfd->nb_cmap_files++;
      link = link->next;
   }

   // create the table of pointers
   lfd->tab_cmap_file = (COLORMAP_FILE **) calloc(lfd->nb_cmap_files, sizeof(COLORMAP_FILE *));
   if (lfd->tab_cmap_file == NULL)
      return 1;

   // fill it
   i    = 1;
   link = lfd->first_colormap_link;
   while (link != NULL)
   {
      if (i >= lfd->nb_cmap_files)
         return 2;

      lfd->tab_cmap_file[i] = link->pt_cmap;
      i++;

      next = link->next;
      lfd->first_colormap_link = next;
      link->next    = NULL;
      link->pt_cmap = NULL;
      DESTROYME(link, free)

      link = next;
   }

   qsort(lfd->tab_cmap_file, lfd->nb_cmap_files, sizeof(COLORMAP_FILE *), qsort_helper_tab_cmap_file);

   for (t = 0; t < AT_MAX; t++)
   {
      lfd->at[t].default_cmap_file_id = 0;

      switch (t)
      {
         case AT_PLAYERS  : file_to_search = TEXT("greybrown");      break;
         case AT_MONSTERS : file_to_search = TEXT("randtransforms"); break;
         default          : file_to_search = NULL;                   break;
      }

      if (file_to_search != NULL)
      {
         for (i = 0; i < lfd->nb_cmap_files; i++)
         {
            cmapfile = lfd->tab_cmap_file[i];
            if (cmapfile != NULL)
            {
               if (wcsicmp(cmapfile->name, file_to_search) == 0)
               {
                  lfd->at[t].default_cmap_file_id = i;
                  break;
               }
            }
         }
      }
   }

   return 0;
}


// ===========================================================================
// 
// ===========================================================================
int qsort_helper_tab_cmap_file(const void * e1, const void * e2)
{
   COLORMAP_FILE * c1 = NULL;
   COLORMAP_FILE * c2 = NULL;


   if (e1 != NULL) c1 = * (COLORMAP_FILE **) e1;
   if (e2 != NULL) c2 = * (COLORMAP_FILE **) e2;

   if (c1 == NULL) return -1;
   if (c2 == NULL) return 1;

   if (c1->num_cmap_type != c2->num_cmap_type)
      return c1->num_cmap_type - c2->num_cmap_type;

   switch (c1->num_cmap_type)
   {
      case CMAPTYPE_PLAYER :
         return wcscmp(c1->name, c2->name);
         break;

      case CMAPTYPE_MONSTER :
         if (c1->num_row_extension != c2->num_row_extension)
            return c1->num_row_extension - c2->num_row_extension;
         else
            return wcscmp(c1->name, c2->name);
         break;

      case CMAPTYPE_SAMEAS :
         return c1->sameas_layer_code - c2->sameas_layer_code;
         break;

      default :
         return wcscmp(c1->name, c2->name);
         break;
   }
}
