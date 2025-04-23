#include "one_include_to_rule_them_all.h"

// to avoid all mention of : warning C4055: 'type cast' : from data pointer 'void *' to function pointer '_BMP_BANK_SWITCHER'
#pragma warning(disable:4055)

#include "allegro.h"
#include "winalleg.h"

#include "defines.h"
#include "misc.h"
#include "dccjob.h"


#define MAX_THREADS  20
#define SLEEP_DELAY  50


typedef enum ENUM_JOB_CRITICAL_SECTION_N
{
   SECTION_JOB_ID = 0,
   SECTION_TASKS_LINK,
   SECTION_THREAD_PARAMS,
   SECTION_EXIT_THREADS,
   SECTION_MAX
} ENUM_JOB_CRITICAL_SECTION;

typedef struct TASKS_LINK_S
{
   long                job_ID;
   TASK                * task_table;
   int                 nb_tasks;
   int                 tasks_all_done;
   struct TASKS_LINK_S * prev;
   struct TASKS_LINK_S * next;
} TASKS_LINK;

typedef struct THREADS_PARAMS_S
{
   int   thread_index;
   DWORD nb_tasks;
   int   has_returned;
} THREADS_PARAMS;


CRITICAL_SECTION      GLOBAL_critical_section [SECTION_MAX];
HANDLE                GLOBAL_hThreadArray     [MAX_THREADS];

// critical section : SECTION_JOB_ID
long                  GLOBAL_job_ID = 0;

// critical section : SECTION_TASKS_LINK
TASKS_LINK            * GLOBAL_first_link = NULL;

// critical section : SECTION_THREAD_PARAMS
THREADS_PARAMS        GLOBAL_thread_params[MAX_THREADS];

// critical section : SECTION_EXIT_THREADS
long                  GLOBAL_threads_must_exit = 0;


#define INIT_SECTION(x)   (InitializeCriticalSection( & GLOBAL_critical_section[(x)]))
#define LOCK_SECTION(x)   (EnterCriticalSection( & GLOBAL_critical_section[(x)]))
#define UNLOCK_SECTION(x) (LeaveCriticalSection( & GLOBAL_critical_section[(x)]))
#define CLOSE_SECTION(x)  (DeleteCriticalSection( & GLOBAL_critical_section[(x)]))


// ===========================================================================
// ===========================================================================
void dccjobs_init(void)
{
   int i = 0;


   for (i = 0; i < SECTION_MAX; i++)
      INIT_SECTION(i);
}


// ===========================================================================
// ===========================================================================
void dccjob_exit(void)
{
   int i = 0;


   for (i = 0; i < SECTION_MAX; i++)
      CLOSE_SECTION(i);
}



// ===========================================================================
// ===========================================================================
long get_new_job_ID(void)
{
   long job_ID = 0;


   LOCK_SECTION(SECTION_JOB_ID);
   job_ID = GLOBAL_job_ID;
   GLOBAL_job_ID++;
   UNLOCK_SECTION(SECTION_JOB_ID);

   return job_ID;
}


// ===========================================================================
// caller will have to free() the table
// ===========================================================================
TASK * dccjob_get_new_task_table(int nb_tasks)
{
   TASK * task_table = NULL;
   int  i            = 0;


   if (nb_tasks <= 0)
      return NULL;

   task_table = (TASK *) calloc(nb_tasks, sizeof(TASK));
   if (task_table != NULL)
   {
      for(i = 0; i < nb_tasks; i++)
      {
         task_table[i].state        = TASK_TODO;
         task_table[i].thread_index = -1;
         task_table[i].return_code  = -1;
      }
   }

   return task_table;
}


// ===========================================================================
// ===========================================================================
int dccjob_add_task_table(long job_ID, TASK * task_table, int nb_tasks)
{
   TASKS_LINK * datas = NULL;
   TASKS_LINK * next  = NULL;
   TASKS_LINK * p     = NULL;


   if ((task_table == NULL) || (nb_tasks <= 0))
      return -1;

   datas = (TASKS_LINK *) calloc(1, sizeof(TASKS_LINK));
   if (datas == NULL)
      return -2;

   datas->job_ID         = job_ID;
   datas->task_table     = task_table;
   datas->nb_tasks       = nb_tasks;
   datas->tasks_all_done = FALSE;
   datas->prev           = NULL;
   datas->next           = NULL;

   LOCK_SECTION(SECTION_TASKS_LINK);
   p = GLOBAL_first_link;
   if (p == NULL)
      GLOBAL_first_link = datas;
   else
   {
      next = p->next;
      while (next != NULL)
      {
         p    = next;
         next = p->next;
      }

      p->next     = datas;
      datas->prev = p;
   }
   UNLOCK_SECTION(SECTION_TASKS_LINK);

   return 0;
}


// ===========================================================================
// ===========================================================================
TASK * dccjob_get_next_task(int thread_index)
{
   TASKS_LINK * p = NULL;
   int        i   = 0;


   LOCK_SECTION(SECTION_TASKS_LINK);
   p = GLOBAL_first_link;
   while (p != NULL)
   {
      if (p->tasks_all_done == FALSE)
      {
         for (i = 0; i < p->nb_tasks; i++)
         {
            if (p->task_table[i].state == TASK_TODO)
            {
               p->task_table[i].state        = TASK_RUNNING;
               p->task_table[i].thread_index = thread_index;
               UNLOCK_SECTION(SECTION_TASKS_LINK);
               return & p->task_table[i];
            }
         }
      }

      p = p->next;
   }
   UNLOCK_SECTION(SECTION_TASKS_LINK);
   return NULL;
}


// ===========================================================================
// DCC decoding thread, checking for jobs to work on.
// should never return
// ===========================================================================
DWORD WINAPI dccjob_decoding_thread(LPVOID lpParam)
{
   THREADS_PARAMS * params     = (THREADS_PARAMS *) lpParam;
   TASK           * task       = NULL;
   DWORD          thread_index = 0;
   int            ret          = 0;
   DCC_S          * dcc        = NULL;
   DC6_S          * dc6        = NULL;
   long           dir_bitfield = 0;
   int            must_exit    = 0;


   if (lpParam == NULL)
      return (DWORD) -1;

   LOCK_SECTION(SECTION_THREAD_PARAMS);
   thread_index = params->thread_index;
   UNLOCK_SECTION(SECTION_THREAD_PARAMS);

   for(;;)
   {
      LOCK_SECTION(SECTION_EXIT_THREADS);
      must_exit = GLOBAL_threads_must_exit;
      UNLOCK_SECTION(SECTION_EXIT_THREADS);

      if (must_exit != 0)
      {
         LOCK_SECTION(SECTION_THREAD_PARAMS);
         GLOBAL_thread_params[thread_index].has_returned = 1;
         UNLOCK_SECTION(SECTION_THREAD_PARAMS);
         return 0;
      }

      task = dccjob_get_next_task(thread_index);
      if (task == NULL)
         Sleep(SLEEP_DELAY); // FIXME : should be user choice
      else
      {
         LOCK_SECTION(SECTION_TASKS_LINK);
         dcc          = task->dcc;
         dc6          = task->dc6;
         dir_bitfield = task->dir_bitfield;
         UNLOCK_SECTION(SECTION_TASKS_LINK);

         ret = 0;
         if (dcc != NULL)
            ret = dcc_decode(dcc, dir_bitfield);
         else if (dc6 != NULL)
            ret = dc6_decode(dc6, dir_bitfield);

         LOCK_SECTION(SECTION_TASKS_LINK);
         task->state       = TASK_DONE;
         task->return_code = ret;
         UNLOCK_SECTION(SECTION_TASKS_LINK);

         LOCK_SECTION(SECTION_THREAD_PARAMS);
         params->nb_tasks++;
         UNLOCK_SECTION(SECTION_THREAD_PARAMS);
      }
   }
}


// ===========================================================================
// create as many dccjob threads as required
// ===========================================================================
int dccjob_create_threads(int nb_threads)
{
   int i = 0;


   memset( & GLOBAL_hThreadArray,  0, sizeof(GLOBAL_hThreadArray));
   memset( & GLOBAL_thread_params, 0, sizeof(GLOBAL_thread_params));

   if (nb_threads < 0)
      nb_threads = 1;

   if (nb_threads > MAX_THREADS)
      nb_threads = MAX_THREADS;

   LOCK_SECTION(SECTION_EXIT_THREADS);
   GLOBAL_threads_must_exit = 0;
   UNLOCK_SECTION(SECTION_EXIT_THREADS);

   for (i = 0; i < nb_threads; i++)
   {
      GLOBAL_thread_params[i].thread_index = i;
      GLOBAL_hThreadArray[i] = CreateThread(NULL, 0, dccjob_decoding_thread, & GLOBAL_thread_params[i], 0, NULL);
      if (GLOBAL_hThreadArray[i] == NULL)
         return -1;
   }

   return 0;
}


// ===========================================================================
// ===========================================================================
void dccjob_destroy_threads(void)
{
   int   i      = 0;
   DWORD status = 0;
   int   done   = FALSE;
   BOOL  b      = 0;


   // inform threads to stop themselves
   LOCK_SECTION(SECTION_EXIT_THREADS);
   GLOBAL_threads_must_exit = 1;
   UNLOCK_SECTION(SECTION_EXIT_THREADS);

   // wait for all threads to finish
   while (done == FALSE)
   {
      done = TRUE;
      for (i = 0; i < MAX_THREADS; i++)
      {
         if (GLOBAL_hThreadArray[i] != NULL)
         {
            status = 0;
            b = GetExitCodeThread(GLOBAL_hThreadArray[i], & status);
            if (b == 0)
            {
               // function did not succeed, big problem
               // use another way to know
               LOCK_SECTION(SECTION_THREAD_PARAMS);
               if (GLOBAL_thread_params[i].has_returned == 0)
                  status = STILL_ACTIVE;
               UNLOCK_SECTION(SECTION_THREAD_PARAMS);
            }

            if (status == STILL_ACTIVE)
            {
               // at least 1 thread is still running
               done = FALSE;
               break;
            }
            else
            {
               // we can forget that thread now
               CloseHandle(GLOBAL_hThreadArray[i]);
               GLOBAL_hThreadArray[i] = NULL;
            }
         }
      }

      if (done == FALSE)
         Sleep(100);
   }
}


// ===========================================================================
// ===========================================================================
void dccjob_wait_for_tasks(long job_ID)
{
   TASKS_LINK * next = NULL;
   TASKS_LINK * p    = NULL;
   int        i      = 0;
   int        done   = FALSE;


   for(;;)
   {
      done = TRUE;

      LOCK_SECTION(SECTION_TASKS_LINK);
      p = GLOBAL_first_link;
      while ((p != NULL) && (done == TRUE))
      {
         if (p->job_ID == job_ID)
         {
            if (p->tasks_all_done == FALSE)
            {
               for (i = 0; i < p->nb_tasks; i++)
               {
                  if (p->task_table[i].state != TASK_DONE)
                  {
                     done = FALSE;
                     break;
                  }
               }

               if (done == TRUE)
                  p->tasks_all_done = TRUE;
            }
         }

         p = p->next;
      }

      if (done == TRUE)
      {
         // remove all tasks for this job
         p = GLOBAL_first_link;
         while (p != NULL)
         {
            next = p->next;
            if (p->job_ID == job_ID)
            {
               if (next != NULL)
                  next->prev = p->prev;

               if (p->prev != NULL)
                  p->prev->next = next;

               if (GLOBAL_first_link == p)
                  GLOBAL_first_link = next;

               DESTROYME(p->task_table, free)
               DESTROYME(p, free)
            }

            p = next;
         }
         UNLOCK_SECTION(SECTION_TASKS_LINK);
         return;
      }

      UNLOCK_SECTION(SECTION_TASKS_LINK);
      Sleep(SLEEP_DELAY); // FIXME : should be user choice
   }
}
