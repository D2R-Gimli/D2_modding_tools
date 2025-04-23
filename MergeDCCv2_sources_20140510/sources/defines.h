#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "one_include_to_rule_them_all.h"

// application private messages
#define WM_APP_MPQ_PATHS_CHANGED (WM_APP + 1)
#define WM_APP_UPDATE_ANIMATION  (WM_APP + 2)
#define WM_APP_EXPORT_IMAGES     (WM_APP + 3)
#define WM_APP_EXPORT_UPDATE_BOX (WM_APP + 4)

// macros for debug
#define  MYSHOWERROR(expression, comment)                    {my_show_error(expression, comment, __FILE__, __FUNCTION__, __LINE__);}
#define  MYASSERT(expression, comment)                       {if ( ! (expression)) {MYSHOWERROR(#expression, comment); PostQuitMessage(WM_QUIT);}}
#define  MYASSERT_RETURN(expression, return_value, comment)  {if ( ! (expression)) {MYSHOWERROR(#expression, comment); return (return_value);}}

// macro for cleaner memory liberation
#define  DESTROYME(ptr, function)                            {if ((ptr) != NULL) {(function)((ptr)); (ptr) = NULL;}}

// static text, without border
#define  HEIGHT_TXT_12  12
#define  HEIGHT_TXT_13  13
#define  HEIGHT_TXT_14  14

// edit text
#define  HEIGHT_EDT_14_CLIENTEDGE  (HEIGHT_TXT_13 + 4)

// pushed button
#define  HEIGHT_BUT_13  (HEIGHT_TXT_13 + 10)
#define  HEIGHT_BUT_14  (HEIGHT_TXT_14 + 10)

// specific to dlgbox_main.c, but need to be in sync also in enums.h
#define  DLGBOX_MAIN_CTRL_PER_ROW  7

#endif
