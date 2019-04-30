// Console.cpp : main source file for Console.exe
//

#include "stdafx.h"

#include "resource.h"

#include "ConsoleView.h"
#include "aboutdlg.h"
#include "MainFrame.h"
#include "Console.h"
#include "ConsoleException.h"
#include "WallPaper.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global variables

CAppModule					_Module;

std::shared_ptr<SettingsHandler>	g_settingsHandler;
std::shared_ptr<ImageHandler>	g_imageHandler;
_t_TranslateMessageEx TranslateMessageEx;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Invisible parent window class for the main window if no taskbar should be shown

class NoTaskbarParent
	: public CWindowImpl<NoTaskbarParent, CWindow, CWinTraits<WS_POPUP, WS_EX_TOOLWINDOW> >
{
	public:
		DECLARE_WND_CLASS(L"NoTaskbarParent")

		NoTaskbarParent() {}
		~NoTaskbarParent() {}

		BEGIN_MSG_MAP(NoTaskbarParent)
		END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

static std::wstring g_strInstanceName;

const std::wstring & GetInstanceName(void) { return g_strInstanceName; }

void ParseCommandLine
(
	LPTSTR lptstrCmdLine,
	std::wstring& strConfigFile,
	bool& bReuse,
	std::wstring& strSyncName,
	ShowHideWindowAction& visibility
)
{
	int argc = 0;
	std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(lptstrCmdLine, &argc));

	if (argc < 1) return;

	for (int i = 0; i < argc; ++i)
	{
		const std::wstring arg(argv[i]);
		if( arg == L"-c" )
		{
			// custom config file
			++i;
			if( i == argc ) break;
			strConfigFile = argv[i];
		}
		else if( arg == L"-reuse" )
		{
			bReuse = true;
		}
		else if( arg == L"-i" )
		{
			++i;
			if( i == argc ) break;
			g_strInstanceName = argv[i];
			bReuse = true;
		}
		else if( arg == L"-a" )
		{
			++i;
			if( i == argc ) break;
			strSyncName = argv[i];
		}
		else if( arg == L"-v" )
		{
			// ConsoleZ visibility
			++i;
			if( i == argc ) break;
			     if( _wcsicmp(L"Show", argv[i]) == 0 ) visibility = ShowHideWindowAction::SHWA_SHOW_ONLY;
			else if( _wcsicmp(L"Hide", argv[i]) == 0 ) visibility = ShowHideWindowAction::SHWA_HIDE_ONLY;
			else if( _wcsicmp(L"Switch", argv[i]) == 0 ) visibility = ShowHideWindowAction::SHWA_SWITCH;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

static bool HandleReuse(LPCTSTR lpstrCmdLine)
{
	SharedMemory<HWND> sharedInstance;
  try
  {
    sharedInstance.Open(std::wstring(L"ConsoleZ") + g_strInstanceName, syncObjNone);
  }
  catch(Win32Exception& ex)
  {
    if(ex.GetErrorCode() == ERROR_FILE_NOT_FOUND)
      return false;
    throw;
  }
	if (0 != sharedInstance.Get())
	{
		::SetForegroundWindow(*sharedInstance);

		ConsoleHandler::UpdateCurrentUserEnvironmentBlock();

		std::wstring commandLine = lpstrCmdLine;

		commandLine += L" -cwd ";
		commandLine += Helpers::EscapeCommandLineArg(Helpers::GetCurrentDirectory());

		commandLine += L" -ebx ";
		commandLine += ConsoleHandler::DumpCurrentUserEnvironmentBlock();

		COPYDATASTRUCT cds = {0};
		cds.dwData = 0;
		cds.lpData = (LPVOID)commandLine.c_str();
		cds.cbData = static_cast<DWORD>((commandLine.length() + 1) * sizeof(wchar_t));

		::SendMessage(*sharedInstance, WM_COPYDATA, 0, (LPARAM)&cds);

		return true;
	}

	return false;
}

void LoadLocalizedResources(const std::wstring& language);

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
  try
  {
		// parse command line
		std::wstring         strConfigFile(L"");
		bool                 bReuse = false;
		std::wstring         strSyncName;
		ShowHideWindowAction visibility = ShowHideWindowAction::SHWA_DONOTHING;

		ParseCommandLine(
			lpstrCmdLine,
			strConfigFile,
			bReuse,
			strSyncName,
			visibility);

    if (strConfigFile.length() == 0)
    {
      strConfigFile = std::wstring(L"console.xml");
    }

		// load settings
    if (!g_settingsHandler->LoadSettings(Helpers::ExpandEnvironmentStrings(strConfigFile)))
    {
      throw std::exception("Unable to load settings!");
    }

		if (!bReuse)
		{
			bReuse = !g_settingsHandler->GetBehaviorSettings2().instanceSettings.bAllowMultipleInstances;
		}

		LoadLocalizedResources(g_settingsHandler->GetLanguage());
		// check if console.exe has been hooked
		{
			HMODULE hModule;
			if( ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"ConsoleHook.dll", &hModule) ||
			    ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"ConsoleHook32.dll", &hModule) )
			{
				::MessageBox(
					nullptr,
					Helpers::LoadStringW(IDS_ERR_CONSOLEZ_HOOKED).c_str(),
					Helpers::LoadString(IDS_CAPTION_ERROR).c_str(),
					MB_OK | MB_ICONERROR);
				return 1;
			}
		}

		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);

		if (!strSyncName.empty())
		{
			CommandLineOptions commandLineOptions;

			MainFrame::ParseCommandLine
			(
				lpstrCmdLine,
				commandLineOptions
			);

			TabSettings& tabSettings = g_settingsHandler->GetTabSettings();

			// find tab with corresponding name...
			bool found = false;
			for (auto tabData = tabSettings.tabDataVector.begin(); tabData != tabSettings.tabDataVector.end(); ++tabData)
			{
				if (tabData->get()->strTitle == commandLineOptions.startupTabs[0])
				{
					ConsoleOptions consoleOptions;

					// startup directory choice (by descending order of priority):
					// 1 - ConsoleZ command line startup tab dir (-d)
					// 2 - Tab setting
					// 3 - Settings global initial dir
					// 4 - ConsoleZ command line working dir (-cwd)
					consoleOptions.strWorkingDir = commandLineOptions.strWorkingDir;

					if (commandLineOptions.startupDirs.size() > 0 && commandLineOptions.startupDirs[0].length() > 0)
					{
						consoleOptions.strInitialDir = commandLineOptions.startupDirs[0];
					}

					wstring	strShell(g_settingsHandler->GetConsoleSettings().strShell);

					if (tabData->get()->strShell.length() > 0)
					{
						strShell	= tabData->get()->strShell;
					}

					if (commandLineOptions.startupShellArgs.size() > 0 && commandLineOptions.startupShellArgs[0].length() > 0)
					{
						consoleOptions.strShellArguments = commandLineOptions.startupShellArgs[0];
					}

					if (commandLineOptions.basePriorities.size() > 0)
					{
						consoleOptions.dwBasePriority = commandLineOptions.basePriorities[0];
					}

					if (consoleOptions.dwBasePriority == ULONG_MAX)
						consoleOptions.dwBasePriority = tabData->get()->dwBasePriority;

					try
					{
						ConsoleHandler consoleHandler;
						consoleHandler.StartShellProcessAsAdministrator
						(
							consoleOptions,
							strSyncName,
							strShell,
							tabData->get()->environmentVariables
						);
					}
					catch (const ConsoleException& ex)
					{
						::MessageBox(nullptr, ex.GetMessage().c_str(), Helpers::LoadString(IDS_CAPTION_ERROR).c_str(), MB_ICONERROR|MB_OK);
						return 1;
					}

					found = true;
					break;
				}
			}

			if( !found )
				::MessageBox(
					nullptr,
					boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_UNDEFINED_TAB)) % commandLineOptions.startupTabs[0]).c_str(),
					Helpers::LoadString(IDS_CAPTION_ERROR).c_str(),
					MB_ICONERROR | MB_OK);

      return 0;
    }

    if (bReuse && HandleReuse(lpstrCmdLine))
      return 0;

#ifndef _USING_V110_SDK71_

     Helpers::SetProcessDpiAwareness(g_settingsHandler->GetAppearanceSettings().stylesSettings.bPerMonitorDpi ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_SYSTEM_DPI_AWARE);

#endif

		 // Set global value of splitbar size.
		 CMultiSplitPane::splitBarWidth    = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwSplitBarSize;
		 CMultiSplitPane::splitBarHeight   = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwSplitBarSize;
		 CMultiSplitPane::splitBarAutoSize = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwSplitBarSize != 0;

    // create main window
    NoTaskbarParent noTaskbarParent;
    MainFrame wndMain(lpstrCmdLine);

    if (!g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton)
    {
      noTaskbarParent.Create(nullptr);
    }

    if(wndMain.CreateEx(noTaskbarParent.m_hWnd) == nullptr)
    {
      ATLTRACE(_T("Main window creation failed!\n"));
      return 1;
    }

#ifdef _USE_AERO
    // restore the drop files message in elevated console
    ::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
    ::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
    ::ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
#endif

		switch( g_settingsHandler->GetAppearanceSettings().positionSettings.nState )
		{
			case WindowState::stateNormal:
			case WindowState::stateFullScreen:
				nCmdShow = SW_NORMAL;
				break;

			case WindowState::stateMinimized:
				nCmdShow = g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton ? SW_SHOWMINNOACTIVE : SW_HIDE;
				break;

			case WindowState::stateMaximized:
				nCmdShow = SW_MAXIMIZE;
				break;
		}

		if( visibility == ShowHideWindowAction::SHWA_HIDE_ONLY )
		{
			nCmdShow = g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton ? SW_SHOWMINNOACTIVE : SW_HIDE;
		}

		wndMain.ShowWindow(nCmdShow);
		// /!\ ShowWindow with "maximized" will resize the window, but views are not aware...
		wndMain.AdjustWindowSize(ADJUSTSIZE_WINDOW);

    SharedMemory<HWND> sharedInstance;
    if (bReuse)
    {
      sharedInstance.Create(std::wstring(L"ConsoleZ") + g_strInstanceName, 1, syncObjNone, _T(""));
      sharedInstance = wndMain.m_hWnd;
    }

    WallPaperThread wallPaperThread(wndMain);

    if (Helpers::CheckOSVersion(6, 1))
    {
      // Win7 or more, we use the wallpaper slideshow monitoring
      wallPaperThread.Start();
    }

		TranslateMessageEx = (_t_TranslateMessageEx)::GetProcAddress(::GetModuleHandle(L"user32.dll"), "TranslateMessageEx");
		if( !TranslateMessageEx )
			Win32Exception::ThrowFromLastError("TranslateMessageEx");

    int nRet = theLoop.Run();

    if (noTaskbarParent.m_hWnd != nullptr) noTaskbarParent.DestroyWindow();

    _Module.RemoveMessageLoop();

    return nRet;
  }
  catch(std::exception& e)
  {
    ::MessageBoxA(nullptr, e.what(), "exception", MB_OK);
    return 1;
  }
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HMODULE LoadLocalizedResourcesDll(const wchar_t * szLang)
{
	HMODULE hResources = nullptr;

	wstring dll (L"console_");
	dll += szLang;
	dll += L".dll";

	DWORD  verHandle = NULL;
	UINT   size      = 0;
	DWORD  verSize   = ::GetFileVersionInfoSize(dll.c_str(), &verHandle);
	std::unique_ptr<BYTE[]> verData(new BYTE[verSize]);

	if( verSize == 0 || !::GetFileVersionInfo(dll.c_str(), verHandle, verSize, verData.get()) )
	{
		Win32Exception ex("GetFileVersionInfo", ::GetLastError());
		TRACE(L"szLang=%s dll=%s hResources=%p (%S)\n", szLang, dll.c_str(), hResources, ex.what());

		return hResources;
	}

	void * lpBuffer  = nullptr;
	if( !::VerQueryValue(verData.get(), L"\\", &lpBuffer, &size) )
	{
		Win32Exception ex("VerQueryValue", ::GetLastError());
		TRACE(L"szLang=%s dll=%s hResources=%p (%S)\n", szLang, dll.c_str(), hResources, ex.what());

		return hResources;
	}

	VS_FIXEDFILEINFO *verInfo = static_cast<VS_FIXEDFILEINFO *>(lpBuffer);
	if( size == 0 || verInfo->dwSignature != 0xfeef04bd )
	{
		TRACE(L"szLang=%s dll=%s hResources=%p (bad file version)\n", szLang, dll.c_str(), hResources);

		return hResources;
	}

	TRACE(
		L"Resources File Version: %d.%d.%d.%d\n",
		(verInfo->dwFileVersionMS >> 16) & 0xffff,
		(verInfo->dwFileVersionMS >> 0) & 0xffff,
		(verInfo->dwFileVersionLS >> 16) & 0xffff,
		(verInfo->dwFileVersionLS >> 0) & 0xffff);

	ULONGLONG ullDllVer = MAKEDLLVERULL(
		(verInfo->dwFileVersionMS >> 16) & 0xffff,
		(verInfo->dwFileVersionMS >> 0) & 0xffff,
		(verInfo->dwFileVersionLS >> 16) & 0xffff,
		(verInfo->dwFileVersionLS >> 0) & 0xffff);

	TRACE(
		L"ConsoleZ File Version: %d.%d.%d.%d\n",
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_BUILD,
		VERSION_BUILD2);

	ULONGLONG ullExeVer = MAKEDLLVERULL(
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_BUILD,
		VERSION_BUILD2);

	if( ullExeVer == ullDllVer )
	{
		hResources = ::LoadLibraryEx(dll.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

		Win32Exception ex("LoadLibraryEx", ::GetLastError());
		TRACE(L"szLang=%s dll=%s hResources=%p (%S)\n", szLang, dll.c_str(), hResources, ex.what());
	}
	else
	{
		TRACE(L"szLang=%s dll=%s hResources=%p (file version mismatch)\n", szLang, dll.c_str(), hResources);
	}

	return hResources;
}

#ifdef _USING_V110_SDK71_

struct {
	LANGID language;
	wchar_t *iso639;
} languages[] = {
	{ LANG_GERMAN,   L"deu" },
	{ LANG_ENGLISH,  L"eng" },
	{ LANG_FRENCH,   L"fra" },
	{ LANG_JAPANESE, L"jap" },
	{ LANG_RUSSIAN,  L"rus" },
};

HMODULE LoadLocalizedResourcesXP(LANGID langid)
{
	HMODULE hResources = nullptr;

	for( size_t i = 0; i < ARRAYSIZE(languages); ++i )
	{
		if( languages[i].language == langid )
		{
			hResources = LoadLocalizedResourcesDll(languages[i].iso639);
			break;
		}
	}

	return hResources;
}
#endif

HMODULE LoadLocalizedResourcesAuto()
{
	HMODULE hResources = nullptr;

#ifdef _USING_V110_SDK71_
	// Windows XP ...
	if( !Helpers::CheckOSVersion(6, 0) )
	{
		hResources = LoadLocalizedResourcesXP(PRIMARYLANGID(::GetUserDefaultUILanguage()));

		if( !hResources )
			hResources = LoadLocalizedResourcesXP(PRIMARYLANGID(::GetSystemDefaultUILanguage()));
	}
	else
	{
#endif

		wchar_t szLang[9];

		// user language
		if( ::GetLocaleInfo(LOCALE_CUSTOM_UI_DEFAULT, LOCALE_SISO639LANGNAME2, szLang, 9) > 0 )
		{
			TRACE(L"LOCALE_CUSTOM_UI_DEFAULT ");
			hResources = LoadLocalizedResourcesDll(szLang);
		}
		else
		{
			Win32Exception ex("GetLocaleInfo", ::GetLastError());
			TRACE(L"LOCALE_CUSTOM_UI_DEFAULT fails %S\n", ex.what());
		}

		// default resources are in english
		if( !hResources && ::_wcsicmp(szLang, L"eng") == 0 ) return nullptr;

		// system language
		if( !hResources && ::GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME2, szLang, 9) > 0 )
		{
			TRACE(L"LOCALE_SYSTEM_DEFAULT ");
			hResources = LoadLocalizedResourcesDll(szLang);
		}
		else
		{
			Win32Exception ex("GetLocaleInfo", ::GetLastError());
			TRACE(L"LOCALE_SYSTEM_DEFAULT fails %S\n", ex.what());
		}

#ifdef _USING_V110_SDK71_
	}
#endif

	return hResources;
}

void LoadLocalizedResources(const std::wstring& language)
{
	HMODULE hResources = nullptr;

	if( language == std::wstring(L"auto") )
	{
		hResources = LoadLocalizedResourcesAuto();
	}
	else
	{
		hResources = LoadLocalizedResourcesDll(language.c_str());
	}

	if(hResources)
	{
		_Module.SetResourceInstance(hResources);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	// CoInitializeEx(COINIT_MULTITHREADED) can affect functions that rely on shell objects
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	ATLASSERT(SUCCEEDED(hRes));

#ifdef _USE_AERO
  // init GDI+
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

	g_settingsHandler.reset(new SettingsHandler());
	g_imageHandler.reset(new ImageHandler());

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	g_imageHandler.reset();

#ifdef _USE_AERO
  // shutdown GDI+;
  Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

	_Module.Term();
	g_settingsHandler.reset();

	::CoUninitialize();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////
