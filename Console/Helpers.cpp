#include "StdAfx.h"
#include "Helpers.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModulePath(HINSTANCE hInstance)
{
	wchar_t szModulePath[MAX_PATH] = L"";

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath.substr(0, strPath.rfind(L'\\')+1);
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

wstring Helpers::LoadString(UINT uID)
{
  wchar_t str[0x800];

  if( ::LoadString(::GetModuleHandle(NULL), uID, str, ARRAYSIZE(str)) )
    return wstring(str);
  else
    return wstring(L"LoadString failed");
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HICON Helpers::LoadTabIcon(bool bBigIcon, bool bUseDefaultIcon, const wstring& strIcon, const wstring& strShell)
{
  if (bUseDefaultIcon)
  {
    if ( !strShell.empty() )
    {
      wstring strCommandLine = Helpers::ExpandEnvironmentStrings(strShell);
      int argc = 0;
      std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(strCommandLine.c_str(), &argc));

      if ( argv && argc > 0 )
      {
        SHFILEINFO info;
        memset(&info, 0, sizeof(info));
        if( ::SHGetFileInfo(
          argv[0],
          0,
          &info,
          sizeof(info),
          SHGFI_ICON | (( bBigIcon )? SHGFI_LARGEICON : SHGFI_SMALLICON)) != 0 )
        {
          return info.hIcon;
        }
      }
    }
  }
  else
  {
    if (!strIcon.empty())
    {
      int index = 0;

      // check strIcon ends with ,<integer>
      bool ok = false;

      size_t pos = strIcon.find_last_of(L',');
      if( pos != wstring::npos )
      {
        for(size_t i = pos + 1; i < strIcon.length(); ++i)
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
      }

      wstring strIconPath = ok ? strIcon.substr(0, pos) : strIcon;

      HICON hIcon = nullptr;

      if ( bBigIcon )
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
        ::ExtractIconEx(
          Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
          index,
          nullptr,
          &hIcon,
          1);
      }

      if( hIcon )
        return hIcon;
    }
  }

  if ( bBigIcon )
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTCOLOR | LR_DEFAULTSIZE));
  }
  else
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        16,
        16,
        LR_DEFAULTCOLOR));
  }
}

//////////////////////////////////////////////////////////////////////////////

