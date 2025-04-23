#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include "enums.h"
#include "strings_define.h"
#include "tools.h"
#include "misc.h"


// ===========================================================================
// get the screen position and/or dimensions (in pixels) of the window given in parameter
// return 0 on success, 1 on failure
// ===========================================================================
int get_window_position_and_dimension(HWND h, int * x, int * y, int * width, int * height)
{
   RECT r;


   if (h == NULL)
      return 1;

   memset( & r, 0, sizeof(r));
   if (GetWindowRect(h, & r) == 0)
      return 1;

   if (x      != NULL) (* x)      = r.left;
   if (y      != NULL) (* y)      = r.top;
   if (width  != NULL) (* width)  = r.right  - r.left;
   if (height != NULL) (* height) = r.bottom - r.top;

   return 0;
}


// ===========================================================================
// return the desired x (left) pixel position to center a window on screen, given its width
// ===========================================================================
int get_x_to_center_into_screen(int width)
{
   return (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
}


// ===========================================================================
// return the desired y (top) pixel position to center a window on screen, given its height
// ===========================================================================
int get_y_to_center_into_screen(int height)
{
   return (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
}


// ===========================================================================
// return the desired x (left) pixel position to center a window into another window
// ===========================================================================
int get_x_to_center_into_parent(HWND h, int width)
{
   int parent_x     = 0;
   int parent_width = 0;


   if (get_window_position_and_dimension(h, & parent_x, NULL, & parent_width, NULL) == 0)
      return parent_x + ((parent_width - width) / 2);

   return 0;
}


// ===========================================================================
// return the desired y (top) pixel position to center a window into another window
// ===========================================================================
int get_y_to_center_into_parent(HWND h, int height)
{
   int parent_y      = 0;
   int parent_height = 0;


   if (get_window_position_and_dimension(h, NULL, & parent_y, NULL, & parent_height) == 0)
      return parent_y + ((parent_height - height) / 2);

   return 0;
}


// ===========================================================================
// convert a string to unicode into a buffer
// ===========================================================================
void char_to_wide_char(char * szSource, WCHAR * wszDest, int iDestByteSize)
{
   int iDestCount = (iDestByteSize / 2) - 1;
   int length     = 0;
   int min        = 0;


   if ((szSource == NULL) || (wszDest == NULL) || (iDestCount <= 1))
      return;

   min = length = strlen(szSource);
   if (iDestCount < length)
      min = iDestCount;

   wcscpy(wszDest, TEXT(""));
   MultiByteToWideChar(CP_ACP, 0, szSource, length, wszDest, iDestCount);
   wszDest[min] = TEXT('\0');

   return;
}


// ===========================================================================
// test if the path is a valid directory
// return TRUE if it's valid
// ===========================================================================
int validate_directory_path(WCHAR * path)
{
   DWORD attr                      = 0;
   WCHAR message  [MAX_PATH + 100] = TEXT("");


   MYASSERT_RETURN(path != NULL, FALSE, NULL);
   attr = GetFileAttributes(path);
   if (attr == INVALID_FILE_ATTRIBUTES)
   {
      swprintf(message, sizeof(message) / 2, TEXT("The directory \"%s\" can't be read, or it don't exists."), path);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return FALSE;
   }

   if ( ! (attr & FILE_ATTRIBUTE_DIRECTORY) )
   {
      swprintf(message, sizeof(message) / 2, TEXT("The path \"%s\" is not a directory."), path);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return FALSE;
   }

   return TRUE;
}


// ===========================================================================
// test if the path is a valid file
// return TRUE if it's valid
// ===========================================================================
int validate_file_path(WCHAR * path)
{
   DWORD attr                      = 0;
   WCHAR message  [MAX_PATH + 100] = TEXT("");


   MYASSERT_RETURN(path != NULL, FALSE, NULL);
   attr = GetFileAttributes(path);
   if (attr == INVALID_FILE_ATTRIBUTES)
   {
      swprintf(message, sizeof(message) / 2, TEXT("The directory \"%s\" can't be read, or it don't exists."), path);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return FALSE;
   }

   if (attr & FILE_ATTRIBUTE_DIRECTORY)
   {
      swprintf(message, sizeof(message) / 2, TEXT("The path \"%s\" is a directory."), path);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return FALSE;
   }

   return TRUE;
}


// ===========================================================================
// simple "checksum" for validation of the config file
// ===========================================================================
unsigned long get_jenkins_one_at_a_time_hash(unsigned char * key, long size)
{
   unsigned long hash = 0;
   long          i    = 0;


   for(i = 0; i < size; i++)
   {
      hash += key[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }

   hash += (hash << 3);
   hash ^= (hash >> 11);
   hash += (hash << 15);

   return hash;
}


// ===========================================================================
// write the current user configuration : mpq paths, Mod directory, dialog Export datas
// ===========================================================================
void save_current_configuration(void)
{
   MY_CONFIGURATION    config;
   int                 m                = 0;
   FILE                * out            = NULL;
   WCHAR               buffer[MAX_PATH] = TEXT("");
   DLGBOX_EXPORT_DATAS * e              = & myglobals.dlgbox_export_datas;
   unsigned long       checksum         = 0;


   memset( & config, 0, sizeof(MY_CONFIGURATION));

   strcpy(config.version, MY_CONFIGURATION_EXPECTED_VERSION);

   for (m = 0; m < MPQ_MAX; m++)
      wcscpy(config.mpq_path[m], myglobals.datas.mpq[m].path);

   if (e->datas_state == EDS_ALL_READY)
   {
      config.have_export_datas = 1;

      wcscpy(config.last_export_directory, e->directory);
      wcscpy(config.filename_format, e->filename_format);

      config.enum_format = e->enum_format;
      config.enum_usebox = e->enum_usebox;

      config.background_index = e->palette_background.index;
      config.background_red   = e->palette_background.red;
      config.background_green = e->palette_background.green;
      config.background_blue  = e->palette_background.blue;

      config.shadow_index = e->palette_shadow.index;
      config.shadow_red   = e->palette_shadow.red;
      config.shadow_green = e->palette_shadow.green;
      config.shadow_blue  = e->palette_shadow.blue;

      config.mirror_sprite         = e->mirror_sprite;
      config.shadow_height_percent = e->shadow_height_percent;
      config.shadow_skew_percent   = e->shadow_skew_percent;

      config.userbox_left   = e->user_box.left;
      config.userbox_right  = e->user_box.right;
      config.userbox_top    = e->user_box.top;
      config.userbox_bottom = e->user_box.bottom;

      config.shadow_offset_x = e->shadow_offset_x;
      config.shadow_offset_y = e->shadow_offset_y;
   }

   checksum = get_jenkins_one_at_a_time_hash((unsigned char *) & config, sizeof(MY_CONFIGURATION));
   config.checksum = checksum;

   swprintf(buffer, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources, STR_CONFIGURATION_FILENAME);
   out = _wfopen(buffer, TEXT("wb"));
   MYASSERT(out != NULL, "can't create 'ressources\\setup.bin'");
   if (out == NULL)
      return;

   fwrite( & config, sizeof(config), 1, out);
   fclose(out);
   out = NULL;
}


// ===========================================================================
// read the last user configuration (mpq paths and options)
// ===========================================================================
void load_last_configuration(void)
{
   char                version [MY_CONFIGURATION_VERSION_SIZE] = "";
   MY_CONFIGURATION    config;
   int                 m                = 0;
   FILE                * in             = NULL;
   WCHAR               buffer[MAX_PATH] = TEXT("");
   DLGBOX_EXPORT_DATAS * e              = & myglobals.dlgbox_export_datas;
   unsigned long       checksum         = 0;
   unsigned long       new_checksum     = 0;


   memset( & config, 0, sizeof(MY_CONFIGURATION));

   swprintf(buffer, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.ressources, STR_CONFIGURATION_FILENAME);
   in = _wfopen(buffer, TEXT("rb"));
   if (in == NULL)
      return;

   fread(version, MY_CONFIGURATION_VERSION_SIZE, 1, in);
   if (strcmp(version, MY_CONFIGURATION_EXPECTED_VERSION) != 0)
   {
      fclose(in);
      return;
   }

   fseek(in, 0, SEEK_SET);
   fread( & config, sizeof(config), 1, in);
   fclose(in);
   in = NULL;

   checksum = config.checksum;
   config.checksum = 0;
   new_checksum = get_jenkins_one_at_a_time_hash((unsigned char *) & config, sizeof(MY_CONFIGURATION));
   if (checksum != new_checksum)
      return;

   // MPQ paths + MOD Directory
   for (m = 0; m < MPQ_MAX; m++)
      wcscpy(myglobals.datas.mpq[m].path, config.mpq_path[m]);

   // dialog Export datas
   if (config.have_export_datas == 1)
   {
      wcscpy(e->directory, config.last_export_directory);
      wcscpy(e->filename_format, config.filename_format);

      e->enum_format = (EXPORT_FORMAT) config.enum_format;
      e->enum_usebox = (EXPORT_USEBOX) config.enum_usebox;

      e->palette_background.index = config.background_index;
      e->palette_background.red   = config.background_red;
      e->palette_background.green = config.background_green;
      e->palette_background.blue  = config.background_blue;

      e->palette_shadow.index = config.shadow_index;
      e->palette_shadow.red   = config.shadow_red;
      e->palette_shadow.green = config.shadow_green;
      e->palette_shadow.blue  = config.shadow_blue;

      e->mirror_sprite         = config.mirror_sprite;
      e->shadow_height_percent = config.shadow_height_percent;
      e->shadow_skew_percent   = config.shadow_skew_percent;

      e->datas_state = EDS_JUST_LOADED;

      e->user_box.left   = config.userbox_left;
      e->user_box.top    = config.userbox_top;
      e->user_box.right  = config.userbox_right;
      e->user_box.bottom = config.userbox_bottom;
      e->user_box.width  = config.userbox_right  - config.userbox_left + 1;
      e->user_box.height = config.userbox_bottom - config.userbox_top  + 1;

      e->shadow_offset_x = config.shadow_offset_x;
      e->shadow_offset_y = config.shadow_offset_y;
   }
}


// ===========================================================================
// get the byte value of a WCHAR string
// return 0 to 255 if success
// return -1 if error
// ===========================================================================
int get_byte_value_from_wchar_string(WCHAR * string)
{
   int   max   = 0;
   WCHAR c     = 0;
   int   value = 0;
   int   n     = 0;


   max = wcslen(string);
   if ((max == 0) || (max > 3))
      return -1;

   // get its numerical value
   value = 0;
   for (n = 0; n < max; n++)
   {
      c = string[n];
      value *= 10;
      switch (c)
      {
         case TEXT('0') : break;
         case TEXT('1') : value += 1; break;
         case TEXT('2') : value += 2; break;
         case TEXT('3') : value += 3; break;
         case TEXT('4') : value += 4; break;
         case TEXT('5') : value += 5; break;
         case TEXT('6') : value += 6; break;
         case TEXT('7') : value += 7; break;
         case TEXT('8') : value += 8; break;
         case TEXT('9') : value += 9; break;
         default :
            return -1;
      }
   }

   return value;
}
