#include "one_include_to_rule_them_all.h"

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "misc.h"
#include "dlgbox_main.h"
#include "d2_files.h"
#include "strings_define.h"
#include "loaders.h"
#include "mpq_handler.h"
#include "tools.h"
#include "allegro_animation.h"
#include "dccjob.h"


// TODO
// in setup dlgbox, create the 3rd tab (cache) : cache size limit (KB), current cache size + add them to save_current_configuration() and load_last_configuration()
// in 1st tab (mpq), create a "local" control (see MPQ_LOCALE)
// transform the cache debug window into a modless dialogbox that can be open and close at wish

/*
   preferences, tab "options"
      * shadows : (*) like Diablo II + controls for hight % and skew %
                  (*) enhanced (no overlaping layers)
*/


/* LAYERS :

code is a dropbox, showing all the other layer codes. This is to reorder the layer rows. This has no effect on the animation, this is just a toy for users

tabs :
* adjust
   - position (offsets) : for 1 frame of 1 direction (2D table, each cell having "x,y", like "-1,+3"), in blue the non-standard from the COF
* colormaps
   - item (greybrown.dat)
   - 'this' monster (palshift.dat)
   - another monster (palshift.dat of another code)
   - 127 actN.pl2 variations
   - greenblood.dat
   - user specific (application .bin : collection of user colormaps)
   - colormap index (+ name if available)
* drawing effects : none, Diablo II translucency, % pixel deletion... or application % translucency, brightness, contrast, gamma correction, colorize ...
* options : cast shadows.

*/


// starting setup
#define  MPQ_LOCALE       0x0      /* 0x0 = Default (English) ; 0x409 = English. Note : seems to not be used in standard Diablo II MPQ */
#define  CACHE_LIMIT      20971520 /* 20 MB   */

// function declarations
void             my_atexit                 (void);
int              main_initialisations      (HINSTANCE hInst);
LRESULT CALLBACK callback_cache_debug      (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void             init_window_cache_debug   (void);
void             create_cache_debug_window (void);

// global variables
MYGLOBALS myglobals;


// ===========================================================================
// start of the program
// ===========================================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
   MSG                 msg;
   int                 i             = 0;
   HWND                h             = NULL;
   int                 child_idx     = -1;
   CREATE_DLGBOX_CHILD * pChild      = NULL;
   char                str[200]      = "";
   int                 ret           = 0;
   BOOL                bRet          = 0;
   HWND                hMain_window  = NULL;


   // dcc library test, should always work
   if (test_dcc_library_integrity() != 0)
   {
      MYSHOWERROR("test_dcc_library_integrity() != 0", "Bug, 'dcctypes.h' source code need to be fixed");
      return 1;
   }

   // initialisations
   szCmdLine     = szCmdLine;     // just to avoid a warning "unreferenced formal parameter ...
   hPrevInstance = hPrevInstance; // same here
   if (main_initialisations(hInst) != 0)
   {
      MYSHOWERROR("global_datas_initialisation(hInst) != 0", "Failed to initialize the application");
      return 1;
   }

   // create the application main window
   MYASSERT_RETURN(create_dlgbox_main(hInst, iCmdShow) == 0, 1, NULL);

   // main window created, we can now initialise the Allegro library
   if (get_dialog_child_index_from_ID(DLG_MAIN, ID_DLGBOX_MAIN_ANIMATION, & child_idx) == 0)
   {
      pChild = & myglobals.dlgbox_datas[DLG_MAIN].dlg.pChild[child_idx];
      ret = my_allegro_init(pChild->handle, pChild->width, pChild->height);
      if (ret != 0)
      {
         sprintf(str, "Can't initialise the Allegro library.\nmy_allegro_init() returned %d", ret);
         MYSHOWERROR("my_allegro_init() != 0", str);
         return 1;
      }
      myglobals.datas.allegro_initialized = TRUE;      
      // test_animation(pChild->handle, pChild->width, pChild->height);
   }

   // create the cache window
   create_cache_debug_window();
   myglobals.datas.cache.max_length = CACHE_LIMIT;

   // load Diablo II ressources the application needs
   if (load_d2_ressources() != 0)
      MessageBox(NULL, STR_ERROR_SETUP_MPQ_PATHS, TEXT("Diablo II Ressource file(s) not found"), MB_ICONWARNING | MB_OK);

   set_new_palette_from_PL2(D2R_ACT1PAL);
   debug_cache();

   // create the initial DCC decoding threads
   dccjobs_init();
   dccjob_create_threads(4); // FIXME : need to be the user choice
   myglobals.datas.dccjob_threads_initialized = TRUE;

   // load the current selected animation
   load_current_cof();

   // to force animation even when the main window is not selected. 1000 ms / 50 ticks per second = 20 ms between each tick
   SetTimer(myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle, ID_DLGBOX_MAIN_APP_TIMER, 1000/50, NULL);

   // get the main dialog window handle
   hMain_window = myglobals.dlgbox_datas[DLG_MAIN].dlg.window.handle;

   // messages loop
   while ((bRet = GetMessage( & msg, NULL, 0, 0)) != 0)
   {

      if (hMain_window != NULL)
      {
         if (TranslateAccelerator(hMain_window, myglobals.datas.hAccel, & msg) != 0) // handle keyboard accelerators
            continue;
      }

      // inform the application if there's a new frame to draw
      if (myglobals.update_animation_message_posted == FALSE)
      {
         if (animation_must_be_updated() == 1)
         {
               myglobals.update_animation_message_posted = TRUE;
               PostMessage(hMain_window, WM_APP_UPDATE_ANIMATION, 0, 0);
         }
      }

      for (i = 0; i < DLG_MAX; i++)
      {
         h = myglobals.dlgbox_datas[i].dlg.window.handle;
         if (h != NULL)
         {
            if (IsDialogMessage(h, & msg)) // handle tabulations from keyboard to jump to another control, among other things
               break;
         }
      }

      if (i >= DLG_MAX)
      {
         TranslateMessage( & msg);
         DispatchMessage ( & msg);
      }
   }

   // stop the animations
   stop_tick_25fps(1, 1);

   // close ressources previously opened by CoInitializeEx()
   CoUninitialize(); 

   return msg.wParam;
}


// ===========================================================================
// automaticaly called at the end of the application
// ===========================================================================
void my_atexit(void)
{
   int m = 0;


   // Allegro
   if (myglobals.datas.allegro_initialized == TRUE)
   {
      DESTROYME(myglobals.animation, destroy_animation)
      my_allegro_exit();
      myglobals.datas.allegro_initialized = FALSE;
   }

   // Application
   destroy_my_fonts();
   destroy_cache();
   destroy_d2_ressources();
   destroy_listfiles();
   destroy_all_dialogs();

   // Storm.dll
   for (m = 0; m < MPQ_MAX; m++)
      close_mpq((ENUM_MPQ) m);

   // Export dialog datas
   DESTROYME(myglobals.dlgbox_export_datas.filename_format_list, free)

   // miscelaneous
   if (myglobals.datas.hAccel != NULL)
   {
      DestroyAcceleratorTable(myglobals.datas.hAccel);
      myglobals.datas.hAccel = NULL;
   }

   // DCC decoding threads
   if (myglobals.datas.dccjob_threads_initialized == TRUE)
   {
      dccjob_destroy_threads();
      dccjob_exit();
      myglobals.datas.dccjob_threads_initialized = FALSE;
   }
}


// ===========================================================================
// initialize the global variables, and other important stuff
// ===========================================================================
int main_initialisations(HINSTANCE hInst)
{
   HCURSOR              hcursor_wait  = LoadCursor(NULL, IDC_APPSTARTING);
   WCHAR                message [500] = TEXT("");
   int                  m             = 0;
   HRESULT              hres          = 0;
   INITCOMMONCONTROLSEX InitCommonControlsEx_datas =
   {
      sizeof(INITCOMMONCONTROLSEX),
      ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_USEREX_CLASSES
   };

   
   memset( & myglobals, 0, sizeof(myglobals));
   myglobals.datas.allegro_initialized = FALSE;
   atexit(my_atexit);

   // temp directory
   myglobals.temp_directory = getenv("TEMP");
   if (myglobals.temp_directory == NULL)
      myglobals.temp_directory = getenv("TMP");

   // initialisation for common controls
   if (InitCommonControlsEx( & InitCommonControlsEx_datas) != TRUE)
   {
      swprintf(message, sizeof(message) / 2, TEXT("InitCommonControlsEx() failed."));
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return 1;
   }

   // needed before we can use SHBrowseForFolder()
   hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); 
   if((hres != S_OK) && (hres != S_FALSE))
   {
      swprintf(message, sizeof(message) / 2, TEXT("CoInitializeEx() failed."));
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return 1;
   }

   // inform the user this may take some time from now on
   SetCursor(hcursor_wait);

   // from the full path of the application executable, extract the directory and its derivated paths
   if (get_my_ressources_directories() != 0)
      return 1;

   // load the fonts needed by the application
   if (create_my_fonts() != 0)
   {
      swprintf(message, sizeof(message) / 2, TEXT("create_my_fonts() failed."));
      MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR | MB_OK);
      return 1;
   }

   myglobals.main_instance = hInst;

   myglobals.datas.mpq[MPQ_PATCH_D2].filename = STR_PATCH_D2_MPQ;
   myglobals.datas.mpq[MPQ_D2EXP   ].filename = STR_D2EXP_MPQ;
   myglobals.datas.mpq[MPQ_D2DATA  ].filename = STR_D2DATA_MPQ;
   myglobals.datas.mpq[MPQ_D2CHAR  ].filename = STR_D2CHAR_MPQ;

   myglobals.datas.root_path[AT_PLAYERS]  = D2_ROOT_PATH_PLAYERS;
   myglobals.datas.root_path[AT_MONSTERS] = D2_ROOT_PATH_MONSTERS;
   myglobals.datas.root_path[AT_OBJECTS ] = D2_ROOT_PATH_OBJECTS;

   // load the last Mod directory, MPQ paths configuration, and dialog Export datas
   load_last_configuration();

   // open the MPQ archives
   set_mpq_locale(MPQ_LOCALE);
   for (m = 0; m < MPQ_MAX; m++)
   {
      if (m == MPQ_MOD_DIRECTORY)
         continue;

      if (wcslen(myglobals.datas.mpq[m].path) > 0)
      {
         if (open_mpq((ENUM_MPQ) m) != 0)
            wcscpy_s(myglobals.datas.mpq[m].path, sizeof(myglobals.datas.mpq[m].path) / 2, TEXT(""));
      }
   }

   // user default settings
   myglobals.user_preview_settings.zoom = 1;

   return 0;
}


// ===========================================================================
// callback function called on each cache debug information window's message
// ===========================================================================
LRESULT CALLBACK callback_cache_debug(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg)
   {
      case WM_CTLCOLORSTATIC: // STATIC ---------------------------------------
         SetBkMode((HDC) wParam, OPAQUE);
         SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
         SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
         SetDCBrushColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
         return (LRESULT) GetStockObject(DC_BRUSH);
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}


// ===========================================================================
// init the cache debug information window and its child controls
// ===========================================================================
void init_window_cache_debug(void)
{
   HWND h = NULL;


   h = myglobals.hwnd_cache_static = CreateWindowEx(
      0,
      WC_STATIC,
      TEXT(""),
      WS_CHILD | WS_VISIBLE | SS_LEFT,
      10,
      10,
      900 - 20,
      500 - 20,
      myglobals.hwnd_cache_debug,
      (HMENU) ID_CACHE_DEBUG_TEXT,
      myglobals.main_instance,
      NULL
   );
   MYASSERT(h != NULL, "CreateWindowEx() with WC_STATIC for cache debug informations");
   if (myglobals.datas.my_font[FONT_CONSOLAS_14].hfont != NULL)
      SendMessage(h, WM_SETFONT, (WPARAM) myglobals.datas.my_font[FONT_CONSOLAS_14].hfont, TRUE);
}


// ===========================================================================
// create the cache debug information window
// ===========================================================================
void create_cache_debug_window(void)
{
   HCURSOR    hcursor_arrow = LoadCursor(NULL, IDC_ARROW);
   WNDCLASSEX window_cache;


   memset( & window_cache, 0, sizeof(window_cache));
   window_cache.cbSize        = sizeof(WNDCLASSEX);
   window_cache.style         = CS_HREDRAW | CS_VREDRAW;
   window_cache.lpfnWndProc   = callback_cache_debug;
   window_cache.cbClsExtra    = 0;
   window_cache.cbWndExtra    = 0;
   window_cache.hInstance     = myglobals.main_instance;
   window_cache.hIcon         = NULL;
   window_cache.hCursor       = hcursor_arrow;
   window_cache.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   window_cache.lpszMenuName  = NULL;
   window_cache.lpszClassName = STR_D2ANMEXTR_CLASS_CACHE_DEBUG;
   window_cache.hIconSm       = NULL;

   if (RegisterClassEx( & window_cache) == 0)
   {
      MYSHOWERROR("RegisterClassEx( & window_cache) == 0", "can't create the cache debug informations window");
      return;
   }

   myglobals.hwnd_cache_debug = CreateWindowEx(
      WS_EX_NOACTIVATE,
      STR_D2ANMEXTR_CLASS_CACHE_DEBUG,
      STR_D2ANMEXTR_TITLE_CACHE_DEBUG,
      WS_OVERLAPPED | WS_VISIBLE,
      760,
      240,
      900,
      500,
      NULL,
      NULL,
      myglobals.main_instance,
      NULL
   );
   MYASSERT(myglobals.hwnd_cache_debug != NULL, "CreateWindowEx() for cache debug informations");

   init_window_cache_debug();

   ShowWindow  (myglobals.hwnd_cache_debug, SW_SHOWNORMAL);
   UpdateWindow(myglobals.hwnd_cache_debug);

   debug_cache();
}
