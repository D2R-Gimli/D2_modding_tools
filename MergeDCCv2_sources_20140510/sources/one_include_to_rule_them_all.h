#ifndef UNICODE
#define UNICODE
#endif

#define _WIN32_IE 0x0600

// Enabling Visual Styles : "To enable your application to use visual styles, you must use ComCtl32.dll version 6 or later."
// "For common controls, no further action is necessary to ensure that the controls are displayed in the user's preferred visual style. "
// from : http://msdn.microsoft.com/en-us/library/bb773175%28v=vs.85%29.aspx
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// to avoid all the warnings like : warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#pragma warning(disable:4996)

// the linker should have these additional dependencies :
//    * Storm.lib
//    * Comctl32.lib
//    * allegro-4.4.2-md-debug.lib (in Debug), or allegro-4.4.2-md.lib (in Release)
