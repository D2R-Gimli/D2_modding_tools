#ifndef __FILE_CACHE_H__
#define __FILE_CACHE_H__

#include "one_include_to_rule_them_all.h"

#include <windows.h>
#include "enums.h"

typedef struct CACHE_ELEMENT_S CACHE_ELEMENT;
   
typedef struct CACHE_ELEMENT_S
{
   CACHE_ELEMENT * prev;
   CACHE_ELEMENT * next;
   char          filename [MAX_PATH];
   ENUM_MPQ      source;
   unsigned long length;
   void          * file;
   int           nb_hits;
} CACHE_ELEMENT;

typedef struct CACHE_DATAS_S
{
   CACHE_ELEMENT * first;
   unsigned long max_length;
} CACHE_DATAS;

void   put_cache_element_on_top         (CACHE_ELEMENT * e);
void * get_file_from_cache              (char * filename, long * length, ENUM_MPQ * source);
void   destroy_cache                    (void);
void   delete_filename_from_cache       (char * filename);
void   delete_element_from_cache        (CACHE_ELEMENT * e);
void   keep_cache_under_limit           (void);
void   insert_file_in_cache             (char * filename, long length, void * datas, ENUM_MPQ source);
void   debug_cache                      (void);
void   delete_mpq_type_files_from_cache (ENUM_MPQ m);

#endif
