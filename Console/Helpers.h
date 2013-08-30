#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class Helpers
{
	public:

		static wstring GetModulePath(HINSTANCE hInstance);
		static wstring GetModuleFileName(HINSTANCE hInstance);

		static wstring ExpandEnvironmentStrings(const wstring& str);
		static wstring ExpandEnvironmentStringsForUser(HANDLE userToken, const wstring& str);

		static bool GetMonitorRect(HWND hWnd, CRect& rectMonitor);
		static bool GetDesktopRect(HWND hWnd, CRect& rectDesktop);
		static bool GetDesktopRect(const CPoint& point, CRect& rectDesktop);

		static HBITMAP CreateBitmap(HDC dc, DWORD dwWidth, DWORD dwHeight, CBitmap& bitmap);

		static wstring LoadString(UINT uID);
		static HICON LoadTabIcon(bool bBigIcon, bool bUseDefaultIcon, const wstring& strIcon, const wstring& strShell);

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
