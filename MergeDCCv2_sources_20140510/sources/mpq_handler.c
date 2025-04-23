#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <direct.h>
#include "misc.h"
#include "mpq_handler.h"
#include "storm.h"


// ===========================================================================
// set Storm.dll current language preference when extracting files
// ===========================================================================
void set_mpq_locale(DWORD id)
{
   SFileSetLocale(id);
}


// ===========================================================================
// close an MPQ, if it was previously opened
// ===========================================================================
void close_mpq(ENUM_MPQ m)
{
   if ((m < 0) || (m >= MPQ_MAX))
      return;

   if (m == MPQ_MOD_DIRECTORY)
      return;

   if (myglobals.datas.mpq[m].storm_handle == NULL)
      return;

   SFileCloseArchive(myglobals.datas.mpq[m].storm_handle);
   myglobals.datas.mpq[m].storm_handle = NULL;
}


// ===========================================================================
// open an MPQ.
// since SFileOpenArchive() does NOT handle unicode strings in paths, we
// change the current working directory to where the mpq is, and gives only the
// file to SFileOpenArchive()
// return 0 on succees
// ===========================================================================
int open_mpq(ENUM_MPQ m)
{
   BOOL   b                        = 0;
   DWORD  e                        = 0;
   char   buffer[MAX_PATH]         = "";
   WCHAR  message[500]             = TEXT("");
   WCHAR  * s                      = NULL;
   BOOL   UsedDefaultChar          = 0;
   int    r                        = 0;
   WCHAR  old_directory [MAX_PATH] = TEXT("");
   WCHAR  directory     [MAX_PATH] = TEXT("");
   WCHAR  file          [MAX_PATH] = TEXT("");
   int    directory_changed        = FALSE;


   if ((m < 0) || (m >= MPQ_MAX) || (m == MPQ_MOD_DIRECTORY))
      return 1;

   if (myglobals.datas.mpq[m].storm_handle != NULL)
      return 1;

   s = myglobals.datas.mpq[m].path;
   if (wcslen(s) == 0)
      return 1;

   wcscpy(directory, s);
   r = wcslen(directory) - 1;
   while ((r > 0) && (directory[r] != TEXT('\\')))
      r--;

   if (directory[r] == TEXT('\\'))
   {
      directory[r] = 0;
      MYASSERT_RETURN(_wgetcwd(old_directory, MAX_PATH) != NULL, 1, NULL);
      MYASSERT_RETURN(_wchdir(directory) == 0, 1, NULL);
      directory_changed = TRUE;
      wcscpy(file, directory + r + 1);
   }
   else
      wcscpy(file, s);

   r = WideCharToMultiByte(CP_ACP, 0, file, wcslen(s), buffer, MAX_PATH, " ", & UsedDefaultChar);
   if (r == 0)
   {
      swprintf(message, 500, TEXT("WideCharToMultiByte() failed to convert the MPQ filename \"%s\" from UNICODE to ASCII."), s);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      if (directory_changed == TRUE)
         _wchdir(old_directory);
      return 1;
   }

   b = SFileOpenArchive((LPCSTR) buffer, m, 0, & myglobals.datas.mpq[m].storm_handle);
   e = GetLastError();
   if (b == 0)
   {
      swprintf(message, 500, TEXT("can't open the MPQ \"%s\" with SFileOpenArchive().\nGetLastError() = %d"), s, e);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      if (directory_changed == TRUE)
         _wchdir(old_directory);
      return 1;
   }

   if (myglobals.datas.mpq[m].storm_handle == NULL)
   {
      swprintf(message, 500, TEXT("SFileOpenArchive() has returned a NULL handle when opening the MPQ \"%s\""), s);
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      if (directory_changed == TRUE)
         _wchdir(old_directory);
      return 1;
   }

   if (directory_changed == TRUE)
      _wchdir(old_directory);
   return 0;
}


// ===========================================================================
// try to load a file from an MPQ
// return 1 if not found or error
// ===========================================================================
int load_file_from_mpq(ENUM_MPQ m, char * filename, char ** buffer, long * length)
{
   HANDLE h = NULL;
   long   n = 0;
   DWORD  r = 0;
   int    e = 0;


   if ((filename == NULL) || (buffer == NULL) || (length == NULL))
      return 1;

   (* buffer) = NULL;
   (* length) = 0;

   if ((m < 0) || (m >= MPQ_MAX))
      return 1;

   if (m == MPQ_MOD_DIRECTORY)
      return 1;

   if (myglobals.datas.mpq[m].storm_handle == NULL)
      return 1;

   if (SFileOpenFileEx(myglobals.datas.mpq[m].storm_handle, (LPCSTR) filename, 0, & h) == 0)
   {
      e = GetLastError();
      return 1;
   }

   n = SFileGetFileSize(h, NULL);
   if (n == -1)
   {
      SFileCloseFile(h);
      return 1;
   }

   if (n < 0) // we refuse to support animations files (COF, DCC ...) larger than 2GB, there shouldn't be any in the Diablo II MPQ archives
   {
      SFileCloseFile(h);
      return 1;
   }

   (* length) = n;
   if (n == 0)
      return 0;

   (* buffer) = (char *) calloc(1, n + 1);
   if ((* buffer) == NULL)
   {
      (* length) = 0;
      SFileCloseFile(h);
      return 1;
   }

   if (SFileReadFile(h, (* buffer), n, & r, NULL) == 0)
   {
      free (* buffer);
      (* buffer) = NULL;
      (* length) = 0;
      SFileCloseFile(h);
      return 1;
   }

   if (n != (long) r)
   {
      free (* buffer);
      (* buffer) = NULL;
      (* length) = 0;
      SFileCloseFile(h);
      return 1;
   }

   SFileCloseFile(h);
   return 0;
}


// ===========================================================================
// test if a given file exists in a given MPQ
// return TRUE if it was found, FALSE otherwise
// ===========================================================================
int test_file_existence_from_mpq(int m, char * filename)
{
   HANDLE h = NULL;
   long   n = 0;
   int    e = 0;


   if (filename == NULL)
      return FALSE;

   if ((m < 0) || (m >= MPQ_MAX))
      return FALSE;

   if (m == MPQ_MOD_DIRECTORY)
      return FALSE;

   if (myglobals.datas.mpq[m].storm_handle == NULL)
      return FALSE;

   if (SFileOpenFileEx(myglobals.datas.mpq[m].storm_handle, (LPCSTR) filename, 0, & h) == 0)
   {
      e = GetLastError();
      return FALSE;
   }

   n = SFileGetFileSize(h, NULL);
   if (n == -1)
   {
      SFileCloseFile(h);
      return FALSE;
   }

   SFileCloseFile(h);
   return TRUE;
}
