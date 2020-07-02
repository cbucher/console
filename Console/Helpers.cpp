#include "StdAfx.h"

#include "Console.h"
extern int g_nIconSize;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModulePath(HINSTANCE hInstance, bool boolTrailingPathDelimiter)
{
	wchar_t szModulePath[MAX_PATH] = L"";

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath.substr(0, strPath.rfind(L'\\') + (boolTrailingPathDelimiter? 1 : 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModuleFileName(HINSTANCE hInstance)
{
	wchar_t szModulePath[MAX_PATH] = L"";

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetCurrentDirectory(void)
{
	wchar_t szCD[MAX_PATH] = L"";

	if( ::GetCurrentDirectory(MAX_PATH, szCD) == 0 )
		return wstring();
	else
		return wstring(szCD);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::EscapeCommandLineArg(const wstring& str)
{
	wstring result(L"\"");
	result += str;
	if( str.back() == L'\\' )
		result += L"\\";
	result += L"\"";

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStrings(const wstring& str)
{
	wchar_t szExpanded[0x8000] = L"";

	::ExpandEnvironmentStrings(str.c_str(), szExpanded, ARRAYSIZE(szExpanded));

	return wstring(szExpanded);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStringsForUser(HANDLE userToken, const wstring& str)
{
	wchar_t szExpanded[0x8000] = L"";

	::ExpandEnvironmentStringsForUser(userToken, str.c_str(), szExpanded, ARRAYSIZE(szExpanded));

	return wstring(szExpanded);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////


const wchar_t * Helpers::GetEnvironmentVariable(const wchar_t * envb, const wchar_t * str, size_t len /*= SIZE_MAX*/)
{
	const wchar_t * ptr = envb;

	if( len == SIZE_MAX ) len = wcslen(str);

	while ((ptr[0] != L'\x00') && !(_wcsnicmp(ptr, str, len) == 0 && ptr[len] == L'=')) ptr += wcslen(ptr)+1;

	if( ptr[0] != L'\x00' )
		return ptr + len + 1;

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::ExpandEnvironmentStrings(const wchar_t * envb, const std::wstring & str)
{
	std::wstring result;

	for(size_t i = 0; i < str.length(); ++i)
	{
		if(str[i] != L'%')
		{
			result += str[i];
		}
		else
		{
			const wchar_t * value = nullptr;
			size_t len = 0;

			for(size_t j = i + 1; j < str.length(); ++j, ++len)
			{
				if(str[j] == L'%')
				{
					if(len > 0)
					{
						value = Helpers::GetEnvironmentVariable(
							envb,
							str.data() + i + 1,
							len);
					}
					break;
				}
			}

			if(value)
			{
				result.append(value);
				i += len + 1;
			}
			else
			{
				result.append(str.data() + i, len + 1);
				i += len;
			}
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::ExpandEnvironmentStrings(const std::map<std::wstring, std::wstring, __case_insensitive_compare> & dictionary, const std::wstring & str)
{
	std::wstring result;

	for(size_t i = 0; i < str.length(); ++i)
	{
		if(str[i] != L'%')
		{
			result += str[i];
		}
		else
		{
			auto value = dictionary.end();
			size_t len = 0;

			for(size_t j = i + 1; j < str.length(); ++j, ++len)
			{
				if(str[j] == L'%')
				{
					if(len > 0)
					{
						value = dictionary.find(std::wstring(str.data() + i + 1, len));
					}
					break;
				}
			}

			if(value != dictionary.end())
			{
				result.append(value->second);
				i += len + 1;
			}
			else
			{
				result.append(str.data() + i, len + 1);
				i += len;
			}
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::GetComputerName(void)
{
	std::wstring strComputerName;

	wchar_t szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD   dwComputerNameLen = ARRAYSIZE(szComputerName);
	if(::GetComputerName(szComputerName, &dwComputerNameLen))
		strComputerName = szComputerName;

	return strComputerName;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::GetWindowsVersionString(void)
{
	__pragma(warning(push))
	__pragma(warning(disable:4996))

	std::wstring    winver;
	OSVERSIONINFOEX osver;
	SYSTEM_INFO     sysInfo;

	memset(&osver, 0, sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	if( ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osver)) )
	{
		::GetSystemInfo(&sysInfo);

		if( osver.dwMajorVersion == 10 && osver.dwMinorVersion >= 0 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows 10 Server";
		if( osver.dwMajorVersion == 10 && osver.dwMinorVersion >= 0 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows 10";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 3 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows Server 2012 R2";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 3 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows 8.1";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 2 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows Server 2012";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 2 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows 8";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 1 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows Server 2008 R2";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 1 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows 7";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 0 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows Server 2008";
		if( osver.dwMajorVersion == 6  && osver.dwMinorVersion == 0 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows Vista";
		if( osver.dwMajorVersion == 5  && osver.dwMinorVersion == 2 && osver.wProductType != VER_NT_WORKSTATION )  winver = L"Windows Server 2003";
		if( osver.dwMajorVersion == 5  && osver.dwMinorVersion == 2 && osver.wProductType == VER_NT_WORKSTATION )  winver = L"Windows XP x64";
		if( osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1 )   winver = L"Windows XP";
		if( osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0 )   winver = L"Windows 2000";
		if( osver.dwMajorVersion < 5 )   winver = L"unknown";

		if( osver.wServicePackMajor != 0 )
		{
			winver += L" Service Pack ";
			winver += std::to_wstring(osver.wServicePackMajor);
		}

		winver += boost::str(
			boost::wformat(L" (%1% bits) [%2%.%3%.%4%]")
			% (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 64 : 32)
			% osver.dwMajorVersion
			% osver.dwMinorVersion
			% osver.dwBuildNumber);
	}

	return winver;

	__pragma(warning(pop))
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetMonitorRect(HWND hWnd, CRect& rectMonitor)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, true, rectMonitor);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetDesktopRect(HWND hWnd, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetDesktopRect(const CPoint& point, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetDesktopRect(const CRect& rect, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetMonitorRect(HMONITOR hMonitor, bool bIgnoreTaskbar, CRect& rectDesktop)
{
  ::ZeroMemory(&rectDesktop, sizeof(CRect));

  MONITORINFO		mi;

  ::ZeroMemory(&mi, sizeof(MONITORINFO));
  mi.cbSize = sizeof(MONITORINFO);

  if( ::GetMonitorInfo(hMonitor, &mi) )
  {
    rectDesktop = bIgnoreTaskbar? mi.rcMonitor : mi.rcWork;
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HBITMAP Helpers::CreateBitmap(HDC dc, DWORD dwWidth, DWORD dwHeight, CBitmap& bitmap)
{
//	HBITMAP hBmp = bitmap.CreateCompatibleBitmap(dc, dwWidth, dwHeight);
//	if (hBmp != NULL) return hBmp;

	// failed to create compatible bitmap, fall back to DIB section...
	BITMAPINFO	bmpInfo;
	void*		pBits = NULL;
	
	::ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));

//	DWORD dwBytesPerLine =   (((32 * bkImage->dwImageWidth) + 31) / 32 * 4); 
	bmpInfo.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth		= dwWidth;
	bmpInfo.bmiHeader.biHeight		= dwHeight;
	bmpInfo.bmiHeader.biPlanes		= static_cast<WORD>(::GetDeviceCaps(dc, PLANES));
	bmpInfo.bmiHeader.biBitCount	= static_cast<WORD>(::GetDeviceCaps(dc, BITSPIXEL));
	bmpInfo.bmiHeader.biCompression	= BI_RGB;
	bmpInfo.bmiHeader.biSizeImage	= 0;//dwBytesPerLine*bkImage->dwImageHeight;


	return bitmap.CreateDIBSection(dc, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::LoadString(UINT uID)
{
	CAtlString localized;

	if(localized.LoadStringW(uID))
		return std::wstring(localized);
	else
		return std::wstring(L"LoadString failed");
}

std::wstring Helpers::LoadFileFilter(UINT uID)
{
	// The OPENFILENAME struct (used by the CFileDialog) expects
	// the filter string components to be delimited using '\0' chars.
	// Unfortunately, strings containing \0 can't be embed in applications string table,
	// so a pipe symbol is used instead
	std::wstring str = Helpers::LoadStringW(uID);

	std::replace(str.begin(), str.end(), L'|', L'\0');

	return str;
}

void Helpers::LoadCombo(CComboBox& cb, UINT uID)
{
	std::wstring combo = Helpers::LoadStringW(uID);
	std::vector<std::wstring> tok;
	boost::algorithm::split(tok, combo, /*boost::is_any_of(L";")*/std::bind(std::equal_to<wchar_t>(), placeholders::_1, L';'));

	for(auto tok_iter = tok.begin(); tok_iter != tok.end(); ++tok_iter)
	{
		if(!tok_iter->empty())
			cb.AddString(tok_iter->c_str());
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HICON Helpers::LoadIcon(bool bBigIcon, const wstring& strIcon)
{
	int index = 0;

	// check strIcon ends with ,<integer>
	bool ok = false;

	size_t pos = strIcon.find_last_of(L',');
	if( pos != wstring::npos )
	{
		bool negative = false;
		size_t i = pos + 1;
		if( i < strIcon.length() && strIcon.at(i) == L'-' )
		{
			i++;
			negative = true;
		}
		for( ; i < strIcon.length(); ++i )
		{
			if( strIcon.at(i) >= L'0' && strIcon.at(i) <= L'9' )
			{
				ok = true;
				index = index * 10 + (strIcon.at(i) - L'0');
			}
			else
			{
				ok = false;
				break;
			}
		}
		if( negative )
			index = -index;
	}

	wstring strIconPath = ok ? strIcon.substr(0, pos) : strIcon;

	HICON hIcon = nullptr;

	if( bBigIcon )
	{
		::ExtractIconEx(
			Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
			index,
			&hIcon,
			nullptr,
			1);
	}
	else
	{
		if( g_nIconSize == 0 )
		{
			::ExtractIconEx(
				Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
				index,
				nullptr,
				&hIcon,
				1);
		}
		else
		{
			::SHDefExtractIcon(
				Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
				index,
				0,
				&hIcon,
				nullptr,
				g_nIconSize);
		}
	}

	return hIcon;
}

HICON Helpers::LoadTabIcon(bool bBigIcon, bool bUseDefaultIcon, const wstring& strIcon, const wstring& strShell)
{
	if( bUseDefaultIcon )
	{
			std::wstring strCommandLine = Helpers::ExpandEnvironmentStrings(strShell);
			int argc = 0;
			std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(strCommandLine.c_str(), &argc));

			if( argv && argc > 0 )
			{
				SHFILEINFO info;
				memset(&info, 0, sizeof(info));
				if( bBigIcon || g_nIconSize == 0 )
				{
					if( ::SHGetFileInfo(
						argv[0],
						0,
						&info,
						sizeof(info),
						SHGFI_ICON | ((bBigIcon) ? SHGFI_LARGEICON : SHGFI_SMALLICON)) != 0 )
					{
						return info.hIcon;
					}
				}
				else
				{
					if( ::SHGetFileInfo(
						argv[0],
						0,
						&info,
						sizeof(info),
						SHGFI_SYSICONINDEX) != 0 )
					{
						// Retrieve the system image list.
						// To get the 48x48 icons, use SHIL_EXTRALARGE
						// To get the 256x256 icons (Vista only), use SHIL_JUMBO
						HIMAGELIST* imageList;

						HRESULT hResult = ::SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&imageList);

						if( hResult == S_OK )
						{
							// Get the icon we need from the list. Note that the HIMAGELIST we retrieved
							// earlier needs to be casted to the IImageList interface before use.

							HICON hIcon;
							hResult = ((IImageList*)imageList)->GetIcon(info.iIcon, ILD_TRANSPARENT, &hIcon);

							if( hResult == S_OK )
							{
								return hIcon;
							}
						}
					}
				}
		}
	}
	else
	{
		if( !strIcon.empty() )
		{
			HICON hIcon = Helpers::LoadIcon(bBigIcon, strIcon);

			if( hIcon )
				return hIcon;
		}
	}

	// window icon
	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;
	if( !windowSettings.strIcon.empty() )
	{
		HICON hIcon = Helpers::LoadIcon(bBigIcon, windowSettings.strIcon);

		if( hIcon )
			return hIcon;
	}

	// icon in ConsoleZ resources
	if( bBigIcon )
	{
		return static_cast<HICON>(
			::LoadImage(
				::GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDR_MAINFRAME),
				IMAGE_ICON,
				g_nIconSize ? g_nIconSize : ::GetSystemMetrics(SM_CXICON),
				g_nIconSize ? g_nIconSize : ::GetSystemMetrics(SM_CYICON),
				LR_DEFAULTCOLOR));
	}
	else
	{
		return static_cast<HICON>(
			::LoadImage(
				::GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDR_MAINFRAME),
				IMAGE_ICON,
				g_nIconSize ? g_nIconSize : ::GetSystemMetrics(SM_CXSMICON),
				g_nIconSize ? g_nIconSize : ::GetSystemMetrics(SM_CYSMICON),
				LR_DEFAULTCOLOR));
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::IsElevated(void)
{
	std::unique_ptr<void, CloseHandleHelper> hToken(nullptr);

	{
		HANDLE _hToken = nullptr;

		if ( !::OpenProcessToken(
			::GetCurrentProcess(),
			TOKEN_QUERY,
			&_hToken) )
		{
			Win32Exception::ThrowFromLastError("OpenProcessToken");
		}

		hToken.reset(_hToken);
	}

	TOKEN_ELEVATION_TYPE tet;
	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
		hToken.get(),
		TokenElevationType,
		&tet,
		sizeof(TOKEN_ELEVATION_TYPE),
		&dwReturnLength ) )
	{
		Win32Exception::ThrowFromLastError("GetTokenInformation");
	}

	return tet == TokenElevationTypeFull;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::CheckOSVersion(DWORD dwMinMajorVersion, DWORD dwMinMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	BYTE op = VER_GREATER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.

	::ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = dwMinMajorVersion;
	osvi.dwMinorVersion = dwMinMinorVersion;

	// Initialize the condition mask.

	VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
	VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

	// Perform the test.

	if( ::VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask) )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int g_nIconSize = 0;

int Helpers::GetHighDefinitionResourceId(int nId)
{
	int size = g_nIconSize ? g_nIconSize : ::GetSystemMetrics(SM_CYSMICON);
	if( size <= 16 ) return nId;
	if( size <= 20 ) return nId + 10;
	if( size <= 24 ) return nId + 20;
	if( size <= 32 ) return nId + 30;
	if( size <= 40 ) return nId + 40;
	if( size <= 48 ) return nId + 50;
	if( size <= 64 ) return nId + 60;
	return nId + 70;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::string Helpers::ToUtf8(const std::wstring& text)
{
	std::string result;
	int rc = ::WideCharToMultiByte(
		CP_UTF8,
		0,
		text.c_str(), static_cast<int>(text.length()),
		nullptr, 0,
		nullptr, nullptr);

	if(rc > 0)
	{
		result.resize(rc);
		::WideCharToMultiByte(
			CP_UTF8,
			0,
			text.c_str(), static_cast<int>(text.length()),
			&result[0], rc,
			nullptr, nullptr);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::WriteLine(HANDLE hFile, const std::wstring& text)
{
	std::string utf8 = Helpers::ToUtf8(text);

	DWORD dwNumberOfBytesWritten;

	if(!::WriteFile(
		hFile,
		utf8.data(),
		static_cast<DWORD>(utf8.size()),
		&dwNumberOfBytesWritten, /* This parameter can be NULL only when the lpOverlapped parameter is not NULL. */
		NULL))
		Win32Exception::ThrowFromLastError("WriteFile");

	if(!::WriteFile(
		hFile,
		"\r\n",
		2,
		&dwNumberOfBytesWritten, /* This parameter can be NULL only when the lpOverlapped parameter is not NULL. */
		NULL))
		Win32Exception::ThrowFromLastError("WriteFile");
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::GetUACPrefix(void)
{
	std::wstring result;

	// read the prefix in default language
	std::unique_ptr<HINSTANCE__, FreeMUILibraryHelper> mui(::LoadMUILibrary(L"cmd.exe", MUI_LANGUAGE_ID, LOCALE_USER_DEFAULT));
	if(mui.get())
	{
		LPWSTR lpBuffer = nullptr;
		if(FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_HMODULE |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			mui.get(),
			1073751880,
			0,
			reinterpret_cast<LPWSTR>(&lpBuffer),
			0,
			nullptr))
		{
			result = lpBuffer;
			::LocalFree(lpBuffer);
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

void Helpers::GetCurrentUserAndDomain(std::wstring& strUser, std::wstring& strDomain)
{
	std::unique_ptr<void, CloseHandleHelper> hToken(nullptr);

	{
		HANDLE _hToken = nullptr;

		if ( !::OpenProcessToken(
			::GetCurrentProcess(),
			TOKEN_QUERY,
			&_hToken) )
		{
			Win32Exception::ThrowFromLastError("OpenProcessToken");
		}

		hToken.reset(_hToken);
	}

	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
		hToken.get(),
		TokenUser,
		nullptr,
		0,
		&dwReturnLength ) )
	{
		if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
			Win32Exception::ThrowFromLastError("GetTokenInformation");
	}

	std::unique_ptr<BYTE[]> tu(new BYTE[dwReturnLength]);

	if ( !::GetTokenInformation(
		hToken.get(),
		TokenUser,
		tu.get(),
		dwReturnLength,
		&dwReturnLength ) )
	{
			Win32Exception::ThrowFromLastError("GetTokenInformation");
	}

	wchar_t szUser[CRED_MAX_STRING_LENGTH + 1] = L"";
	wchar_t szDomain[CRED_MAX_STRING_LENGTH + 1] = L"";
	DWORD dwUserLen = static_cast<DWORD>(ARRAYSIZE(szUser));
	DWORD dwDomainLen = static_cast<DWORD>(ARRAYSIZE(szDomain));
	SID_NAME_USE snu;

	// Retrieve user name and domain name based on user's SID.
	if( !LookupAccountSid(NULL, reinterpret_cast<TOKEN_USER*>(tu.get())->User.Sid, szUser, &dwUserLen, szDomain, &dwDomainLen, &snu) )
	{
		Win32Exception::ThrowFromLastError("LookupAccountSid");
	}

	strUser = szUser;
	strDomain = szDomain;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#ifndef _USING_V110_SDK71_

bool Helpers::GetDpiForMonitor(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY)
{
	HMODULE hmodule = ::LoadLibrary(L"Shcore.dll");
	if (hmodule == nullptr) return false;

	typedef HRESULT(WINAPI* GetDPIForMonitor_t)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

	GetDPIForMonitor_t fnGetDPIForMonitor = (GetDPIForMonitor_t)::GetProcAddress(hmodule, "GetDpiForMonitor");
	bool res = fnGetDPIForMonitor && fnGetDPIForMonitor(hmonitor, dpiType, dpiX, dpiY) == S_OK;

	::FreeLibrary(hmodule);

	return res;
}

bool Helpers::SetProcessDpiAwareness(PROCESS_DPI_AWARENESS value)
{
	bool res = false;

	HMODULE hmodule = ::LoadLibrary(L"Shcore.dll");
	if( hmodule )
	{
		typedef HRESULT(WINAPI* SetProcessDpiAwareness_t)(PROCESS_DPI_AWARENESS value);

		SetProcessDpiAwareness_t fnSetProcessDpiAwareness = (SetProcessDpiAwareness_t)::GetProcAddress(hmodule, "SetProcessDpiAwareness");
		res = fnSetProcessDpiAwareness && fnSetProcessDpiAwareness(value) == S_OK;

		::FreeLibrary(hmodule);
	}

	if( !res )
	{
		// function SetProcessDpiAwareness does not exist
		// we use SetProcessDpiAware
		res = ::SetProcessDPIAware() != FALSE;
	}

	return res;
}

#endif

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetUrlContent(const std::wstring& strUrl, std::vector<char>& content)
{
	content.resize(0);

	wchar_t szHostName[256];
	wchar_t szUrlPath[1024];
	wchar_t szUserName[256];
	wchar_t szPassword[256];

	URL_COMPONENTS urlComponents = { sizeof(URL_COMPONENTS) };
	urlComponents.lpszHostName = szHostName;
	urlComponents.dwHostNameLength = static_cast<DWORD>(_countof(szHostName));
	urlComponents.lpszUrlPath = szUrlPath;
	urlComponents.dwUrlPathLength = static_cast<DWORD>(_countof(szUrlPath));
	urlComponents.lpszUserName = szUserName;
	urlComponents.dwUserNameLength = static_cast<DWORD>(_countof(szUserName));
	urlComponents.lpszPassword = szPassword;
	urlComponents.dwPasswordLength = static_cast<DWORD>(_countof(szPassword));

	if( !::InternetCrackUrl(strUrl.c_str(), static_cast<DWORD>(strUrl.length()),
	                        0,
	                        &urlComponents) )
		Win32Exception::ThrowFromLastError("InternetCrackUrl");

	std::unique_ptr<void, InternetCloseHandleHelper> hSession;
	std::unique_ptr<void, InternetCloseHandleHelper> hConnect;
	std::unique_ptr<void, InternetCloseHandleHelper> hRequest;

	hSession.reset(::InternetOpen(L"ConsoleZ",
	                              INTERNET_OPEN_TYPE_PRECONFIG,
	                              nullptr,
	                              nullptr,
	                              0));
	if( hSession.get() == nullptr )
		Win32Exception::ThrowFromLastError("InternetOpen");

	hConnect.reset(::InternetConnect(hSession.get(),
	                                 urlComponents.lpszHostName,
	                                 urlComponents.nPort,
	                                 urlComponents.lpszUserName,
	                                 urlComponents.lpszPassword,
	                                 INTERNET_SERVICE_HTTP,
	                                 urlComponents.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0,
	                                 0));
	if( hConnect.get() == nullptr )
		Win32Exception::ThrowFromLastError("InternetConnect");

	PCWSTR pszAcceptTypes[] = { L"*/*", nullptr };
	hRequest.reset(::HttpOpenRequest(hConnect.get(),
	                                 L"GET",
	                                 urlComponents.lpszUrlPath,
	                                 nullptr,
	                                 nullptr,
	                                 pszAcceptTypes,
	                                 INTERNET_FLAG_NO_UI,
	                                 0));
	if( hRequest.get() == nullptr )
		Win32Exception::ThrowFromLastError("HttpOpenRequest");

	if( !::HttpSendRequest(hRequest.get(),
	                       nullptr, 0,
	                       nullptr, 0) )
		Win32Exception::ThrowFromLastError("HttpSendRequest");

	for( ; ; )
	{
		DWORD dwAvailable = 0;
		if( !::InternetQueryDataAvailable(hRequest.get(),
		                                  &dwAvailable,
		                                  0,
		                                  0) )
			Win32Exception::ThrowFromLastError("InternetQueryDataAvailable");

		if( dwAvailable == 0 ) break;

		size_t offset = content.size();
		content.resize(offset + dwAvailable);

		DWORD dwDownloaded = 0;
		if( !::InternetReadFile(hRequest.get(),
		                        &content[offset],
		                        dwAvailable,
		                        &dwDownloaded) )
			Win32Exception::ThrowFromLastError("InternetReadFile");
	}

  return true;
}

//////////////////////////////////////////////////////////////////////////////
