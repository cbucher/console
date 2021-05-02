#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct __case_insensitive_compare
{
	bool operator() (const std::wstring& a, const std::wstring& b) const
	{
		std::wstring a1 = a, b1 = b;

		boost::to_upper(a1);
		boost::to_upper(b1);
		return (wcscmp(a1.c_str( ), b1.c_str()) < 0);
	}
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class Helpers
{
	public:

		static std::wstring GetModulePath(HINSTANCE hInstance, bool boolTrailingPathDelimiter);
		static std::wstring GetModuleFileName(HINSTANCE hInstance);

		static std::wstring GetCurrentDirectory(void);
		static std::wstring EscapeCommandLineArg(const std::wstring& str);

		static std::wstring ExpandEnvironmentStrings(const std::wstring& str);
		static std::wstring ExpandEnvironmentStringsForUser(HANDLE userToken, const std::wstring& str);

		static std::wstring ExpandEnvironmentStrings(const wchar_t * envb, const std::wstring & str);
		static const wchar_t * GetEnvironmentVariable(const wchar_t * envb, const wchar_t * str, size_t len = SIZE_MAX);
		static std::wstring ExpandEnvironmentStrings(const std::map<std::wstring, std::wstring, __case_insensitive_compare> & dictionary, const std::wstring & str);

		static std::wstring GetComputerName(void);
		static std::wstring GetWindowsVersionString(void);

		static bool GetMonitorRect(HWND hWnd, CRect& rectMonitor);
		static bool GetDesktopRect(HWND hWnd, CRect& rectDesktop);
		static bool GetDesktopRect(const CPoint& point, CRect& rectDesktop);
		static bool GetDesktopRect(const CRect& rect, CRect& rectDesktop);

		static HBITMAP CreateBitmap(HDC dc, DWORD dwWidth, DWORD dwHeight, CBitmap& bitmap);

		static std::wstring LoadString(UINT uID);
		static std::wstring LoadFileFilter(UINT uID);
		static void LoadCombo(CComboBox& cb, UINT uID);

		static HICON LoadTabIcon(bool bBigIcon, bool bUseDefaultIcon, const std::wstring& strIcon, const std::wstring& strShell);
		static HICON LoadIcon(bool bBigIcon, const std::wstring& strIcon);

		static bool IsElevated(void);
		static bool CheckOSVersion(DWORD dwMinMajorVersion, DWORD dwMinMinorVersion);

		static int GetHighDefinitionResourceId(int nId);

		static std::string ToUtf8(const std::wstring& text);
		static void WriteLine(HANDLE hFile, const std::wstring& text);

		static std::wstring GetUACPrefix(void);

		static void GetCurrentUserAndDomain(std::wstring& strUser, std::wstring& strDomain);

#ifndef _USING_V110_SDK71_
		static bool GetDpiForMonitor(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);
		static bool SetProcessDpiAwareness(PROCESS_DPI_AWARENESS value);
#endif

		static bool GetUrlContent(const std::wstring& strUrl, std::vector<char>& content);

	private:

		static bool GetMonitorRect(HMONITOR hMonitor, bool bIgnoreTaskbar, CRect& rectDesktop);

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class CriticalSection
{
	public:

		CriticalSection()
		{
			::InitializeCriticalSection(&m_cs);
		}

		~CriticalSection()
		{
			::DeleteCriticalSection(&m_cs);
		}

	public:

		_Acquires_lock_(this->m_cs) void Enter()
		{
			::EnterCriticalSection(&m_cs);			
		}
		
		_Releases_lock_(this->m_cs) void Leave()
		{
			::LeaveCriticalSection(&m_cs);			
		}

	private:

		CRITICAL_SECTION m_cs;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class CriticalSectionLock
{
	public:

		explicit CriticalSectionLock(CriticalSection& critSection)
		: m_critSection(critSection)
		{
			m_critSection.Enter();
		}

		~CriticalSectionLock()
		{
			m_critSection.Leave();
		}

	private:

		CriticalSection& m_critSection;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class Mutex
{
	public:
		Mutex(SECURITY_ATTRIBUTES* pSecAttribs, BOOL bInitialOwner,	LPCTSTR pszName)
		: m_mutex(::CreateMutex(pSecAttribs, bInitialOwner, pszName), ::CloseHandle)
		{
		}

		HANDLE get()
		{
			return static_cast<HANDLE>(m_mutex.get());
		}

	private:

		std::shared_ptr<void>	m_mutex;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class MutexLock
{
	public:

		explicit MutexLock(Mutex& mutex)
		: m_mutex(mutex)
		{
			::WaitForSingleObject(m_mutex.get(), INFINITE);
		}

		~MutexLock()
		{
			::ReleaseMutex(m_mutex.get());
		}

	private:

		Mutex&	m_mutex;
};



//////////////////////////////////////////////////////////////////////////////
