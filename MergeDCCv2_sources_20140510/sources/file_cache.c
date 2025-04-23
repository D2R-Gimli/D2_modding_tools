#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <direct.h>
#include "misc.h"
#include "file_cache.h"
#include "tools.h"


// ===========================================================================
// delete 1 file from the cache, given its filename
// ===========================================================================
void delete_filename_from_cache(char * filename)
{
   CACHE_ELEMENT * e = myglobals.datas.cache.first;
   CACHE_ELEMENT * n = NULL;


   if (e == NULL)
      return;

   do
   {
      n = e->next;
      if (_stricmp(e->filename, filename) == 0)
      {
         delete_element_from_cache(e);
         return;
      }
      e = n;
   } while (e != NULL);
}


// ===========================================================================
// put a file from the cache on top of the list
// ===========================================================================
void put_cache_element_on_top(CACHE_ELEMENT * e)
{
   CACHE_ELEMENT * f = myglobals.datas.cache.first;
   CACHE_ELEMENT * p = NULL;
   CACHE_ELEMENT * n = NULL;


   if ((e == NULL) || (f == NULL) || (e == f))
      return;

   p = e->prev;
   n = e->next;

   if (p != NULL)
      p->next = n;

   if (n != NULL)
      n->prev = p;

   e->prev = NULL;
   e->next = f;
   f->prev = e;

   myglobals.datas.cache.first = e;
}


// ===========================================================================
// extract a file from the cache. If found, set its length and return a
// pointer to the file. Return NULL if it isn't in the cache
// ===========================================================================
void * get_file_from_cache(char * filename, long * length, ENUM_MPQ * source)
{
   CACHE_ELEMENT * e = myglobals.datas.cache.first;


   if ((filename == NULL) || (length == NULL) || (source == NULL) || (e == NULL))
      return NULL;

   (* length) = 0;
   (* source) = (ENUM_MPQ) -1;

   do
   {
      if (_stricmp(filename, e->filename) == 0)
      {
         (* length) = e->length;
         (* source) = e->source;
         put_cache_element_on_top(e);
         e->nb_hits++;
         return e->file;
      }
      e = e->next;
   } while (e != NULL);

   return NULL;
}


// ===========================================================================
// free all previously allocated memory from the cache
// ===========================================================================
void destroy_cache(void)
{
   CACHE_ELEMENT * e = myglobals.datas.cache.first;
   CACHE_ELEMENT * n = NULL;


   while (e != NULL)
   {
      n = e->next;
      DESTROYME(e->file, free)
      DESTROYME(e,       free)
      e = n;
   }

   myglobals.datas.cache.first = NULL;
}


// ===========================================================================
// delete 1 file from the cache, given its cache element pointer
// ===========================================================================
void delete_element_from_cache(CACHE_ELEMENT * e)
{
   CACHE_ELEMENT * p = NULL;
   CACHE_ELEMENT * n = NULL;


   if (e == NULL)
      return;

   p = e->prev;
   n = e->next;

   DESTROYME(e->file, free)

   if (n != NULL)
      n->prev = p;

   if (p != NULL)
      p->next = n;

   if (myglobals.datas.cache.first == e)
       myglobals.datas.cache.first = n;

   DESTROYME(e, free)
}


// ===========================================================================
// delete all files at the end of the list that makes the cache bigger than
// its limit
// ===========================================================================
void keep_cache_under_limit(void)
{
   CACHE_ELEMENT * f    = myglobals.datas.cache.first;
   CACHE_ELEMENT * e    = f;
   CACHE_ELEMENT * p    = NULL;
   CACHE_ELEMENT * last = NULL;
   unsigned long length = 0;
   unsigned long max    = myglobals.datas.cache.max_length;


   if (f == NULL)
      return;

   if (f->next == NULL)
      return;

   do
   {
      length += e->length;
      last = e;
      e = e->next;
   } while (e != NULL);

   if (length < max)
      return;

   e = last;
   do
   {
      p = e->prev;
      length -= e->length;
      delete_element_from_cache(e);
      e = p;
   } while ((e != NULL) && (length >= max));
}
 

// ===========================================================================
// insert a file on top of the cache
// ===========================================================================
void insert_file_in_cache(char * filename, long length, char * datas, ENUM_MPQ source)
{
   CACHE_ELEMENT * f                       = myglobals.datas.cache.first;
   CACHE_ELEMENT * e                       = NULL;
   void          * file                    = NULL;
   FILE          * out                     = NULL;
   WCHAR         tmp_filename [MAX_PATH]   = TEXT("");
   WCHAR         tmp_buffer   [MAX_PATH]   = TEXT("");
   int           x                         = 0;
   int           xmax                      = strlen(filename);
   char          dirname [MAX_PATH]        = "";
   int           i                         = 0;
   char          local_filename [MAX_PATH] = "";


   if ((filename == NULL) || (datas == NULL))
      return;

   if ((source < MPQ_DEFAULT_D2_FILES) || (source >= MPQ_MAX))
      return;

   e = (CACHE_ELEMENT *) calloc(1, sizeof(CACHE_ELEMENT));
   MYASSERT(e != NULL, "can't allocate memory for a new cache element");
   if (e == NULL)
      return;

   file = (void *) calloc(1, length + 1); // +1 to ensure that text files will have a '\0' at the end, just for safety
   MYASSERT(file != NULL, "can't allocate memory to copy a file in the cache");
   if (file == NULL)
   {
      DESTROYME(e, free)
      return;
   }

   memcpy(file, datas, length);

   // replace '/' by '\', if any
   strcpy_s(local_filename, sizeof(local_filename), filename);
   for (i=0; i < xmax; i++)
   {
      if (local_filename[i] == '/')
         local_filename[i] = '\\';
   }

   // for debug\extracted_files\ : make all sub-directories as needed
   x = 0;
   for (;;)
   {
      while ((x < xmax) && (local_filename[x] != '\\') && (local_filename[x] != '/'))
         x++;
      if (x < xmax)
      {
         strncpy(dirname, local_filename, x);
         dirname[x] = 0;
         char_to_wide_char(dirname, tmp_filename, sizeof(tmp_filename));
         swprintf_s(tmp_buffer, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.debug_extracted_files, tmp_filename);
         _wmkdir(tmp_buffer);
         x++;
      }
      else
         break;
   }

   // save the file for debug
   char_to_wide_char(local_filename, tmp_filename, sizeof(tmp_filename));
   swprintf_s(tmp_buffer, MAX_PATH, TEXT("%s\\%s"), myglobals.datas.path.debug_extracted_files, tmp_filename);
   out = _wfopen(tmp_buffer, TEXT("wb"));
   MYASSERT(out != NULL, "can't create a file in the 'debug\\extracted_files' directory");
   if (out != NULL)
   {
      fwrite(datas, length, 1, out);
      fclose(out);
      out = NULL;
   }

   // save the file on top of the cache

   strcpy_s(e->filename, sizeof(e->filename), filename);
   e->source  = source;
   e->length  = length;
   e->file    = file;
   e->nb_hits = 0;

   e->prev = NULL;
   e->next = f;

   if (f != NULL)
      f->prev = e;

   myglobals.datas.cache.first = e;
}



// ===========================================================================
// popup a cache debug informations
// ===========================================================================
void debug_cache(void)
{
   CACHE_ELEMENT * f               = myglobals.datas.cache.first;
   CACHE_ELEMENT * e               = f;
   char          * buffer          = NULL;
   int           i                 = 0;
   WCHAR         * wchar_buffer    = NULL;
   char          * source[MPQ_MAX] = {"MOD_DIR", "PATCH_D2", "D2EXP", "D2DATA", "D2CHAR"};
   int           buffer_length     = 100000;
   unsigned long length            = 0;


   if (myglobals.hwnd_cache_debug == NULL)
      return;

   buffer = (char *) calloc(1, buffer_length);
   if (buffer == NULL)
      return;

   wchar_buffer = (WCHAR *) calloc(1, sizeof(WCHAR) * buffer_length);
   if (wchar_buffer == NULL)
   {
      DESTROYME(buffer, free)
      return;
   }

   if (f == NULL)
      sprintf_s(buffer, buffer_length, "cache is empty\n\n");
   else
   {
      sprintf_s(
         buffer + strlen(buffer),
         buffer_length - strlen(buffer),
         " idx prev     (this)   next        length file     nb_hits source   filename\n"
         "---- -------- -------- -------- --------- -------- ------- -------- -------------------------------------------------\n"
      );

      do
      {
         sprintf_s(
            buffer + strlen(buffer),
            buffer_length - strlen(buffer),
            "%4d %08lX %08lX %08lX %9lu %08lX %7d %-8s %s\n",
            i + 1,
            e->prev,
            e,
            e->next,
            e->length,
            e->file,
            e->nb_hits,
            (e->source == MPQ_DEFAULT_D2_FILES) ? "DEFAULT" : source[e->source],
            e->filename
         );
         i++;
         e = e->next;
      } while ((e != NULL) && (i < 29));
   }

   for (; i < 29; i++)
      sprintf_s(buffer + strlen(buffer), buffer_length - strlen(buffer), "\n");

   e = f;
   i = 0;
   length = 0;
   if (e != NULL)
   {
      do
      {
         length += e->length;
         i++;
         e = e->next;
      } while (e != NULL);
   }

   sprintf_s(
      buffer + strlen(buffer),
      buffer_length - strlen(buffer),
      "Cache statistics : %d file%s. Total = %ld KB. Usage = %0.2f %%. Limit = %ld KB.",
      i,
      (i >= 2) ? "s" : "",
      length >> 10,
      100.0 * (double) length / (double) myglobals.datas.cache.max_length,
      myglobals.datas.cache.max_length >> 10
   );

   if (MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), wchar_buffer, buffer_length) > 0)
      SendMessage(myglobals.hwnd_cache_static, WM_SETTEXT, 0, (LPARAM) wchar_buffer);

   DESTROYME(buffer,       free)
   DESTROYME(wchar_buffer, free)
}


// ===========================================================================
// delete all files of a certain MPQ type from the cache
// ===========================================================================
void delete_mpq_type_files_from_cache(ENUM_MPQ m)
{
   CACHE_ELEMENT * f = myglobals.datas.cache.first;
   CACHE_ELEMENT * e = f;
   CACHE_ELEMENT * n = NULL;


   if (f == NULL)
      return;

   do
   {
      n = e->next;
      if (e->source == m)
         delete_element_from_cache(e);
      e = n;
   } while (e != NULL);
}
