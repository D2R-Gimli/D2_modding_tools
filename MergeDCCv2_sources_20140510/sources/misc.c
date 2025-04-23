#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <direct.h>
#include "misc.h"
#include "strings_define.h"


// ===========================================================================
// pop up a useful debug window, used mainly by MYASSERT() macro
// ===========================================================================
void my_show_error(char * expression, char * comment, char * file, char * function, long line)
{
   char buffer[2000] = "";


   if (comment == NULL)
      sprintf_s(buffer, sizeof(buffer), "FILE : %s\n\nFUNCTION : %s()\n\nLINE : %ld\n\nEXPRESSION : %s", file, function, line, expression);
   else
      sprintf_s(buffer, sizeof(buffer), "FILE : %s\n\nFUNCTION : %s()\n\nLINE : %ld\n\nEXPRESSION : %s\n\nCOMMENT : %s", file, function, line, expression, comment);

   MessageBoxA(NULL, buffer, "Error", MB_ICONERROR | MB_OK);
}


// ===========================================================================
// create all the logical fonts needed for the application
// ===========================================================================
int create_my_fonts(void)
{
   int i = 0;
   struct
   {
      LPCTSTR  face;
      WCHAR    * filename; // from 'ressources\fonts' directory, itself from where the main application .exe was launched
      int      height;
      int      bold;
   } data[FONT_MAX] =
   {
      // face              filename             height bold
      // ----------------  -------------------  ------ ----
      {FONT_NAME_VERDANA,  TEXT("verdana.ttf"), 12,    0},  // FONT_VERDANA_12
      {FONT_NAME_VERDANA,  TEXT("verdana.ttf"), 13,    0},  // FONT_VERDANA_13
      {FONT_NAME_VERDANA,  NULL,                13,    1},  // FONT_VERDANA_13_BOLD
      {FONT_NAME_VERDANA,  TEXT("verdana.ttf"), 14,    0},  // FONT_VERDANA_14
      {FONT_NAME_VERDANA,  NULL,                14,    1},  // FONT_VERDANA_14_BOLD
      {FONT_NAME_CONSOLAS, TEXT("consola.ttf"), 14,    0},  // FONT_CONSOLAS_14
      {FONT_NAME_CONSOLAS, TEXT("consola.ttf"), 15,    0},  // FONT_CONSOLAS_15
   };
   WCHAR path [MAX_PATH];
   int   n = 0;


   for (i=0; i < FONT_MAX; i++)
   {
      if (data[i].filename != NULL)
      {
         swprintf(path, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources_fonts, data[i].filename);
         MYASSERT((n=AddFontResourceEx(path, FR_PRIVATE, 0)) == 1, "error while reading a 'ressources\\fonts' .ttf file");
         if (n == 0)
            return 1;
      }

      myglobals.datas.my_font[i].hfont = CreateFont(
         data[i].height,
         0,
         0,
         0,
         (data[i].bold == 1) ? FW_BOLD : FW_NORMAL,
         FALSE,
         FALSE,
         FALSE,
         ANSI_CHARSET,
         OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS,
         PROOF_QUALITY,
         DEFAULT_PITCH | FF_DONTCARE,
         data[i].face
      );
   }

   return 0;
}


// ===========================================================================
// destroy all the logical fonts previously created
// ===========================================================================
void destroy_my_fonts(void)
{
   int i = 0;


   for (i=0; i < FONT_MAX; i++)
   {
      if (myglobals.datas.my_font[i].hfont != NULL)
      {
         DeleteObject(myglobals.datas.my_font[i].hfont);
         myglobals.datas.my_font[i].hfont = NULL;
      }
   }
}


// ===========================================================================
// from the full path of the application executable, extract the directory
// and its derivated paths
// ===========================================================================
int get_my_ressources_directories(void)
{
   WCHAR  * ptr_direct_separator = TEXT("\\");
   WCHAR  * ptr_drive_separator  = TEXT(":");
   WCHAR  path[MAX_PATH];
   DWORD  path_length        = 0;
   int    i                  = 0;
   WCHAR  c                  = 0;
   WCHAR  direct_separator   = (* ptr_direct_separator);
   WCHAR  drive_separator    = (* ptr_drive_separator);
   DWORD attr                = 0;


   memset( & path, 0, sizeof(path));
   path_length = GetModuleFileName(NULL, path, MAX_PATH);
   
   i = path_length - 1;
   while (i >= 0)
   {
      c = path[i];
      if ((c == direct_separator) || (c == drive_separator))
         break;
      i--;
   }

   if ((c == direct_separator) || (c == drive_separator))
      path[i + 1] = 0;

   // ressources

   swprintf(myglobals.datas.path.ressources, MAX_PATH, TEXT("%s%s"), path, STR_RES_DIRECTORY);
   attr = GetFileAttributes(myglobals.datas.path.ressources);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'ressources' directory can't be read (or it don't exists)");
   MYASSERT_RETURN(attr & FILE_ATTRIBUTE_DIRECTORY, 1, "'ressources' must be a directory");

   swprintf(myglobals.datas.path.ressources_default_files, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources, STR_RES_DEFAULT_FILE_DIRECTORY);
   attr = GetFileAttributes(myglobals.datas.path.ressources_default_files);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'ressources\\default_d2_files' directory can't be read (or it don't exists)");
   MYASSERT_RETURN(attr & FILE_ATTRIBUTE_DIRECTORY, 1, "'ressources\\default_d2_files' must be a directory");

   swprintf(myglobals.datas.path.ressources_fonts, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources, STR_RES_FONT_DIRECTORY);
   attr = GetFileAttributes(myglobals.datas.path.ressources_fonts);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'ressources\\font' directory can't be read (or it don't exists)");
   MYASSERT_RETURN(attr & FILE_ATTRIBUTE_DIRECTORY, 1, "'ressources\\font' must be a directory");

   swprintf(myglobals.datas.path.listfile, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources, STR_LISTFILES_FILE);
   attr = GetFileAttributes(myglobals.datas.path.listfile);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'ressources\\listfiles.txt' file can't be read (or it don't exists)");
   MYASSERT_RETURN( ! (attr & FILE_ATTRIBUTE_DIRECTORY), 1, "'ressources\\listfiles.txt' must be a file");

   // debug

   swprintf(myglobals.datas.path.debug, MAX_PATH, TEXT("%s%s"), path, STR_DEBUG_DIRECTORY);
   attr = GetFileAttributes(myglobals.datas.path.debug);
   if (attr == INVALID_FILE_ATTRIBUTES)
      _wmkdir(myglobals.datas.path.debug);
   attr = GetFileAttributes(myglobals.datas.path.debug);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'debug' directory can't be read (or it don't exists)");
   MYASSERT_RETURN(attr & FILE_ATTRIBUTE_DIRECTORY, 1, "'debug' must be a directory");

   swprintf(myglobals.datas.path.debug_extracted_files, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.debug, STR_DEBUG_FILES_DIRECTORY);
   attr = GetFileAttributes(myglobals.datas.path.debug_extracted_files);
   if (attr == INVALID_FILE_ATTRIBUTES)
      _wmkdir(myglobals.datas.path.debug_extracted_files);
   attr = GetFileAttributes(myglobals.datas.path.debug_extracted_files);
   MYASSERT_RETURN(attr != INVALID_FILE_ATTRIBUTES, 1, "the 'debug\\extracted_files' directory can't be read (or it don't exists)");
   MYASSERT_RETURN(attr & FILE_ATTRIBUTE_DIRECTORY, 1, "'debug\\extracted_files' must be a directory");

   return 0;
}


// ===========================================================================
// make the letter uppercase letter
// ===========================================================================
void make_letter_uppercase(int * c)
{
   if (c == NULL)
      return;

   if ((*c >= 'a') && (*c <= 'z'))
      (*c) += 'A' - 'a';
}


// ===========================================================================
// make the string uppercase (just the letters)
// ===========================================================================
void make_string_uppercase(char * s)
{
   int n = 0;
   int x = 0;
   int c = 0;


   if (s == NULL)
      return;

   n = strlen(s);
   for (x = 0; x < n; x++)
   {
      c = s[x];
      make_letter_uppercase( & c);
      s[x] = (c & 0xFF);
   }
}


// ===========================================================================
// return the ENUM_MPQ index of a mpq control, given its ID
// return -1 if ID is not from a mpq control
// ===========================================================================
ENUM_MPQ get_mpq_enum_from_id(int id)
{
   switch (id)
   {
      case ID_DLGBOX_SETUP_TABMODDIR_EDIT :
      case ID_DLGBOX_SETUP_TABMODDIR_BUTN :
         return MPQ_MOD_DIRECTORY;

      case ID_DLGBOX_SETUP_TABMPQ_EDIT_PATCH_D2 :
      case ID_DLGBOX_SETUP_TABMPQ_BUTN_PATCH_D2 :
         return MPQ_PATCH_D2;

      case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2EXP :
      case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2EXP :
         return MPQ_D2EXP;

      case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2DATA :
      case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2DATA :
         return MPQ_D2DATA;

      case ID_DLGBOX_SETUP_TABMPQ_EDIT_D2CHAR :
      case ID_DLGBOX_SETUP_TABMPQ_BUTN_D2CHAR :
         return MPQ_D2CHAR;
   }

   return (ENUM_MPQ) -1;
}


// ===========================================================================
// return the ENUM_COFSELTYPE index of a cof selection control, given its ID
// return -1 if ID is not from a cof selection control
// ===========================================================================
ENUM_COFSELTYPE get_cst_enum_from_id(int id)
{
   switch (id)
   {
      case ID_DLGBOX_MAIN_ANIMTYPE_LABEL :
      case ID_DLGBOX_MAIN_ANIMTYPE_LIST  :
         return CST_ANIMTYPE;

      case ID_DLGBOX_MAIN_TOKEN_LABEL :
      case ID_DLGBOX_MAIN_TOKEN_LIST  :
         return CST_TOKEN;

      case ID_DLGBOX_MAIN_WEAPCLASS_LABEL :
      case ID_DLGBOX_MAIN_WEAPCLASS_LIST  :
         return CST_WEAPCLASS;

      case ID_DLGBOX_MAIN_MODES_LABEL :
      case ID_DLGBOX_MAIN_MODES_LIST  :
         return CST_MODES;
   }

   return (ENUM_COFSELTYPE) -1;
}
