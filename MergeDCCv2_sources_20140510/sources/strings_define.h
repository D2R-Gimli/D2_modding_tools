#include "one_include_to_rule_them_all.h"

#define STR_D2ANMEXTR_CLASS_CACHE_DEBUG   TEXT("Diablo2AnimationsExtractorCacheDebug")
#define STR_D2ANMEXTR_TITLE_CACHE_DEBUG   TEXT("Diablo II animations extractor : cache debug informations")

#define STR_RES_DIRECTORY                 TEXT("ressources")
#define STR_RES_DEFAULT_FILE_DIRECTORY    TEXT("default_d2_files")
#define STR_RES_FONT_DIRECTORY            TEXT("fonts")
#define STR_DEBUG_DIRECTORY               TEXT("debug")
#define STR_DEBUG_FILES_DIRECTORY         TEXT("extracted_files")

#define STR_LISTFILES_FILE                TEXT("listfiles.txt")

#define STR_CONFIGURATION_FILENAME        TEXT("setup.bin")

#define FONT_NAME_VERDANA                 TEXT("Verdana")
#define FONT_NAME_CONSOLAS                TEXT("Consolas")

// dialog : Cof analyse in progress
#define STR_DLGBOX_COF_IN_P_CLASS         TEXT("DlgBoxCOFInProgress")
#define STR_DLGBOX_COF_IN_P_WINDOW_NAME   TEXT("Checking available files...")
#define STR_DLGBOX_COF_IN_P_TEXT          TEXT("Building the list of available files, given the\ncurrent Mod directory and MPQ paths configuration...")
#define STR_DLGBOX_COF_IN_P_EXISTS_L      TEXT("Total files found :\n    * Players :\n    * Monsters :\n    * Objects :\n    * Colormaps :")

// dialog : Setup
#define STR_DLGBOX_SETUP_CLASS            TEXT("DlgBoxSetup")
#define STR_DLGBOX_SETUP_WINDOW_NAME      TEXT("Preferences")

// children dialogs within the Setup dialog : MPQ tab, and Mod directory tab
#define STR_DLGBOX_SETUP_TAB_MPQ_CLASS    TEXT("DlgBoxSetupTabMPQ")
#define STR_DLGBOX_SETUP_TAB_MODDIR_CLASS TEXT("DlgBoxSetupTabMPQ")
#define STR_LABEL_MOD_DIRECTORY           TEXT("Mod Directory :")
#define STR_LABEL_PATCH_D2_MPQ            TEXT("patch_d2.mpq :")
#define STR_LABEL_D2EXP_MPQ               TEXT("d2exp.mpq :")
#define STR_LABEL_D2DATA_MPQ              TEXT("d2data.mpq :")
#define STR_LABEL_D2CHAR_MPQ              TEXT("d2char.mpq :")
#define STR_PATCH_D2_MPQ                  TEXT("patch_d2.mpq")
#define STR_D2EXP_MPQ                     TEXT("d2exp.mpq")
#define STR_D2DATA_MPQ                    TEXT("d2data.mpq")
#define STR_D2CHAR_MPQ                    TEXT("d2char.mpq")
#define STR_SELECT_MOD_DIRECTORY_TITLE    TEXT("Select a Diablo II Mod directory. It's the directory which contains a sub-directory \"data\". Usually it's simply your \"Diablo II\" game installation directory")
#define STR_SELECT_MPQ_FILE_FORMAT        TEXT("Select the file \"%s\" from your Diablo II game installation")

// dialog : Main
#define STR_DLGBOX_MAIN_CLASS             TEXT("DlgBoxMain")
#define STR_DLGBOX_MAIN_WINDOW_NAME       TEXT("Diablo II animations extractor")
#define STR_LABEL_ANIMATION_TYPE          TEXT("Animation\ntype")
#define STR_LABEL_TOKEN                   TEXT("Token")
#define STR_LABEL_WEAPON_CLASS            TEXT("Weapon\nclass")
#define STR_LABEL_MODE                    TEXT("Mode")

#define STR_DLGBOX_MAIN_NB_COLORS_FORMAT  TEXT("Unique colors used in the animation : %d")

#define D2_ROOT_PATH_GLOBAL               "data\\global\\"
#define D2_ROOT_PATH_PLAYERS              "data\\global\\chars\\"
#define D2_ROOT_PATH_MONSTERS             "data\\global\\monsters\\"
#define D2_ROOT_PATH_OBJECTS              "data\\global\\objects\\"
#define D2_ROOT_PATH_EXCEL                "data\\global\\excel\\"
#define D2_ROOT_PATH_PALETTE              "data\\global\\palette\\"

#define D2_ROOT_PATH_ITEM_PALETTE         "data\\global\\items\\palette\\"
#define D2_STR_COF_PALSHIFT               "\\cof\\palshift.dat"
#define D2_FIL_GREENBLOOD                 "data\\global\\monsters\\greenblood.dat"
#define D2_FIL_RANDTRANSFORMS             "data\\global\\monsters\\randtransforms.dat"

// dialog : Export
#define STR_DLGBOX_EXPORT_CLASS           TEXT("DlgBoxExport")
#define STR_DLGBOX_EXPORT_WINDOW_NAME     TEXT("Export")
#define STR_SELECT_EXPORT_DIRECTORY_TITLE TEXT("Select the directory where the animation will be exported")


#define STR_LAYER_VARIANT_NONE            TEXT("(none)")

#define STR_ERROR_SETUP_MPQ_PATHS         TEXT("Some Diablo II ressource files were not properly extracted or converted.\n\nTry to set the paths of all the Diablo II MPQ archives.")
