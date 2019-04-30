#pragma once

struct GlobalFreeHelper
{
	void operator()(void * toFree)
	{
		::GlobalFree(static_cast<HGLOBAL>(toFree));
	};
};

struct CloseHandleHelper
{
  void operator()(void * toFree)
  {
    if( toFree && toFree != INVALID_HANDLE_VALUE )
      ::CloseHandle(static_cast<HANDLE>(toFree));
  };
};

struct FindCloseHelper
{
  void operator()(void * toFree)
  {
    if( toFree && toFree != INVALID_HANDLE_VALUE )
      ::FindClose(static_cast<HANDLE>(toFree));
  };
};

struct GlobalUnlockHelper
{
	void operator()(void * toFree)
	{
		if( toFree )
			::GlobalUnlock(toFree);
	};
};

class RevertToSelfHelper
{
	bool bIsLoggedOn;
public:
	RevertToSelfHelper()
		: bIsLoggedOn(false)
	{
	}
	~RevertToSelfHelper()
	{
		off();
	}
	void on(void)
	{
		bIsLoggedOn = true;
	}
	void off(void)
	{
		if( bIsLoggedOn )
			::RevertToSelf();
		bIsLoggedOn = false;
	}
};

class ClipboardHelper
{
	bool bIsClipboardOpened;
public:
	ClipboardHelper()
		: bIsClipboardOpened(false)
	{
		// to workaround OpenClipboard failure when Clipboard is busy
		// we mimic the .net API
		// the .net API retries for 10 times with a 100ms delay
		for( int retries = 0; !this->bIsClipboardOpened && retries < 10; ++retries )
		{
			if( retries > 0 ) ::Sleep(100);
			this->bIsClipboardOpened = ::OpenClipboard(nullptr) != 0;
		}
		if( !this->bIsClipboardOpened )
			Win32Exception::ThrowFromLastError("OpenClipboard");
	}

	~ClipboardHelper()
	{
		if( this->bIsClipboardOpened )
		{
			if( !::CloseClipboard() )
				Win32Exception::ThrowFromLastError("CloseClipboard");
		}
	}

	void empty()
	{
		if( !::EmptyClipboard() )
			Win32Exception::ThrowFromLastError("EmptyClipboard");
	}

	HANDLE getData()
	{
		HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
		if( hData == nullptr )
			Win32Exception::ThrowFromLastError("GetClipboardData");

		return hData;
	}

	void setData(UINT uFormat, const void* p, size_t size)
	{
		std::unique_ptr<void, GlobalFreeHelper> global(::GlobalAlloc(GMEM_MOVEABLE, size));
		if( global.get() == nullptr )
			Win32Exception::ThrowFromLastError("GlobalAlloc");

		{
			std::unique_ptr<void, GlobalUnlockHelper> lock(::GlobalLock(global.get()));
			if( lock.get() == nullptr )
				Win32Exception::ThrowFromLastError("GlobalLock");

			::CopyMemory(lock.get(), p, size);
		}

		if( ::SetClipboardData(uFormat, global.get()) == nullptr )
			Win32Exception::ThrowFromLastError("SetClipboardData");

		// !!! No call to GlobalFree here. Next app that uses clipboard will call EmptyClipboard to free the data
		global.release();
	}
};

#ifdef _LMAPIBUF_
struct NetApiBufferFreeHelper
{
  void operator()(void * toFree)
  {
    if( toFree )
      ::NetApiBufferFree(static_cast<HANDLE>(toFree));
  };
};
#endif

#ifdef _COMBASEAPI_H_
struct CoTaskMemFreeHelper
{
  void operator()(void * toFree)
  {
    ::CoTaskMemFree(toFree);
  };
};
#endif

#ifdef _INC_USERENV
struct DestroyEnvironmentBlockHelper
{
  void operator()(void * toFree)
  {
    if( toFree )
      ::DestroyEnvironmentBlock(toFree);
  };
};
#endif

struct FindCloseChangeNotificationHelper
{
  void operator()(void * toFree)
  {
    if( toFree != INVALID_HANDLE_VALUE )
      ::FindCloseChangeNotification(static_cast<HANDLE>(toFree));
  };
};

struct RegCloseKeyHelper
{
  void operator()(void * toFree)
  {
    ::RegCloseKey(static_cast<HKEY>(toFree));
  };
};

#ifdef _MUILOAD_H_INCLUDED_
struct FreeMUILibraryHelper
{
	void operator()(HINSTANCE toFree)
	{
		if( toFree != NULL )
			::FreeMUILibrary(toFree);
	};
};
#endif

#ifdef _WININET_
struct InternetCloseHandleHelper
{
	void operator()(HINTERNET toFree)
	{
		if( toFree != NULL )
			::InternetCloseHandle(toFree);
	};
};
#endif
