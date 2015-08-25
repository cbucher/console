// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
/*
#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
*/
#ifdef _USE_AERO
#define _WIN32_IE     0x0700
#define _RICHEDIT_VER 0x0200
#else
#define _WIN32_IE     0x0501
#define _RICHEDIT_VER 0x0100
#endif

//////////////////////////////////////////////////////////////////////////////

#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atltypes.h>

#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES

#pragma warning(push)
#pragma warning(disable: 4996)
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlddx.h>
#include <atlmisc.h>

#include <atltheme.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#pragma warning(pop)

#include <userenv.h>
#include <Lm.h>

#pragma warning(push)
#pragma warning(disable: 4189 4267)
#include <atlgdix.h>

#ifdef _USE_AERO
#include <dwmapi.h>
#include <gdiplus.h>
#include "wtlaero.h"
#pragma comment (lib, "gdiplus.lib")
#endif

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h"
#ifdef _USE_AERO
#include "resource.h"
#include "AeroTabCtrl.h"
#endif
#include "TabbedFrame.h"

#pragma warning(pop)
#include "multisplit.h"

#include <HtmlHelp.h>
#include <ShellApi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#ifdef _USE_AERO
#include <WinCred.h>
#include <winuser.h>
#endif
#include <Muiload.h>

#include <shobjidl.h>

#pragma warning(disable: 4503) // disables 'name truncated' warnings

#pragma warning(push)
#pragma warning(disable: 4702)
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <array>
#include <regex>
#include <chrono>
using namespace std;
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244 4267 4511 4512 701 4702)
#define BOOST_ALL_NO_LIB
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4510 4610)
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
using namespace boost::multi_index;
#pragma warning(pop)

#include "../shared/SharedMemory.h"
#include "../shared/Structures.h"

#include "../shared/Cpp11Helpers.h"
#include "../shared/Win32Exception.h"
#include "../shared/NamedPipe.h"
#include "Helpers.h"
#include "ImageHandler.h"
#include "SettingsHandler.h"
#include "ConsoleHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Version numbers

#include "../shared/version.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// WinXP defines

#define SPI_GETFONTSMOOTHINGTYPE	0x200A
#define FE_FONTSMOOTHINGCLEARTYPE	0x0002

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User-defined messages

#define UM_UPDATE_CONSOLE_VIEW	WM_USER + 0x1000
#define UM_CONSOLE_RESIZED		WM_USER + 0x1001
#define UM_CONSOLE_CLOSED		WM_USER + 0x1002
#define UM_UPDATE_TITLES		WM_USER + 0x1003
#define UM_SHOW_POPUP_MENU		WM_USER + 0x1004
#define UM_START_MOUSE_DRAG		WM_USER + 0x1005
#define UM_TRAY_NOTIFY			WM_USER + 0x1006

#define UPDATE_CONSOLE_RESIZE        0x0001
#define UPDATE_CONSOLE_TEXT_CHANGED  0x0002
#define UPDATE_CONSOLE_TITLE_CHANGED 0x0004
#define UPDATE_CONSOLE_CSBI_CHANGED  0x0008

#define IDC_TRAY_ICON		0x0001

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global hotkeys

#define IDC_GLOBAL_ACTIVATE	0xB000

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Windows' console window menu commands

#define SC_CONSOLE_COPY			0xFFF0
#define SC_CONSOLE_PASTE		0xFFF1
#define SC_CONSOLE_MARK			0xFFF2
#define SC_CONSOLE_SCROLL		0xFFF3
#define SC_CONSOLE_FIND			0xFFF4
#define SC_CONSOLE_SELECT_ALL	0xFFF5

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Console limits
#define MIN_BUFFER_ROWS     0
#define MAX_BUFFER_ROWS     32766
#define MIN_BUFFER_COLUMNS  0
#define MAX_BUFFER_COLUMNS  9999
#define MIN_WINDOW_ROWS     10
#define MAX_WINDOW_ROWS     200
#define MIN_WINDOW_COLUMNS  10
#define MAX_WINDOW_COLUMNS  300

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
#define EXTERNAL_COMMANDS_COUNT 10
#define SEARCH_MRU_COUNT        10

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef enum
{
  ADJUSTSIZE_NONE,
  ADJUSTSIZE_WINDOW,
  ADJUSTSIZE_FONT
}
ADJUSTSIZE;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#define TM_INMENUMODE     0x0001
#define TM_POSTCHARBREAKS 0x0002

typedef
BOOL
(WINAPI * _t_TranslateMessageEx)(
    __in CONST MSG *lpMsg,
    __in UINT flags);

extern _t_TranslateMessageEx TranslateMessageEx;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Memory allocation tracking

#ifdef _DEBUG
#include <crtdbg.h>
void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(__FILE__, __LINE__)

void __cdecl operator delete(void* p, LPCSTR lpszFileName, int nLine);
#endif

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// trace function and TRACE macro

#ifdef _DEBUG

void Trace(const wchar_t* pszFormat, ...);

#define TRACE		::Trace
#define TRACE_KEY	__noop
#define TRACE_PERF	__noop

//#define CONSOLEZ_CHRONOS

#else

#define TRACE		__noop
#define TRACE_KEY	__noop
#define TRACE_PERF	__noop

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////

