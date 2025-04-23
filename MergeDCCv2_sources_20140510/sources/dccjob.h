#ifndef __DCCJOB_H__
#define __DCCJOB_H__

#include "one_include_to_rule_them_all.h"

#include "allegro_animation.h"

typedef enum ENUM_TASK_STATE_N
{
   TASK_NULL,
   TASK_TODO,
   TASK_RUNNING,
   TASK_DONE
} ENUM_TASK_STATE;

typedef struct TASK_S
{
   ENUM_TASK_STATE state;
   int             thread_index;
   int             return_code;
   DCC_S           * dcc;
   DC6_S           * dc6;
   long            dir_bitfield;
} TASK;


// functions declaration
void         dccjobs_init              (void);
void         dccjob_exit               (void);
long         get_new_job_ID            (void);
TASK *       dccjob_get_new_task_table (int nb_tasks);
int          dccjob_add_task_table     (long job_ID, TASK * task_table, int nb_tasks);
TASK *       dccjob_get_next_task      (int thread_index);
DWORD WINAPI dccjob_decoding_thread    (LPVOID lpParam);
int          dccjob_create_threads     (int nb_threads);
void         dccjob_destroy_threads    (void);
void         dccjob_wait_for_tasks     (long job_ID);

#endif
