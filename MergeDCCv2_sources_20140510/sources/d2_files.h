#ifndef __D2_FILES_H__
#define __D2_FILES_H__

#include "one_include_to_rule_them_all.h"

#include "enums.h"
#include "misc.h"


#pragma pack(1)
typedef struct ANIMDATA_D2_RECORD_S
{
   char          cof_name [7 + 1];
   long          frames_per_direction;
   long          animation_speed;
   unsigned char event_table [144];
} ANIMDATA_D2_RECORD; // sizeof(ANIMDATA_D2_RECORD) should be 160 bytes, no more, no less
#pragma pack()

void                 destroy_d2_ressources    (void);
int                  load_d2_ressources       (void);
int                  load_mpq_listfile        (int m, char ** buffer, long * length);
long                 get_ObjectsTXT_speed     (char * token, char * mode);
ANIMDATA_D2_RECORD * get_animdata_d2_record (COF_ROW_EXISTS * cof);

#endif
