#include "stdafx.h"

#include "../shared/SharedMemNames.h"
#include "ConsoleHandler.h"

#define GET_STD_OUT_READ_ONLY  StdOutHandle hStdOut(true); if( hStdOut == INVALID_HANDLE_VALUE ) return;

#define GET_STD_OUT_READ_WRITE StdOutHandle hStdOut(false); if( hStdOut == INVALID_HANDLE_VALUE ) return;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_consoleParams()
, m_consoleInfo()
, m_cursorInfo()
, m_consoleBuffer()
, m_consoleCopyInfo()
, m_consoleMouseEvent()
, m_newConsoleSize()
, m_newScrollPos()
, m_hMonitorThread()
, m_hMonitorThreadExit(std::shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
, m_dwScreenBufferSize(0)
, m_dwLastAttachedPID(0)
, m_dwWaitingTime(INFINITE)
, m_timePoint(std::chrono::system_clock::now())
, m_hStdOut(true)
{
	m_szConsoleTitle[0] = 0;
	m_szCurrentDirectory[0] = 0;
}

ConsoleHandler::~ConsoleHandler()
{
	StopMonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::StartMonitorThread()
{
	DWORD dwThreadId = 0;
	m_hMonitorThread = std::shared_ptr<void>(
							::CreateThread(
								NULL,
								0, 
								MonitorThreadStatic, 
								reinterpret_cast<void*>(this), 
								0, 
								&dwThreadId),
							::CloseHandle);

	return dwThreadId;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::StopMonitorThread()
{
	::SetEvent(m_hMonitorThreadExit.get());
	::WaitForSingleObject(m_hMonitorThread.get(), 10000);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::OpenSharedObjects()
{
  try
  {
    // open startup params  memory object
    DWORD dwProcessId = ::GetCurrentProcessId();

    // TODO: error handling
    m_consoleParams.Open((SharedMemNames::formatConsoleParams % dwProcessId).str(), syncObjRequest);

    // open console info shared memory object
    m_consoleInfo.Open((SharedMemNames::formatInfo % dwProcessId).str(), syncObjRequest);

    // open console info shared memory object
    m_cursorInfo.Open((SharedMemNames::formatCursorInfo % dwProcessId).str(), syncObjRequest);

    // open console buffer shared memory object
    m_consoleBuffer.Open((SharedMemNames::formatBuffer % dwProcessId).str(), syncObjRequest);

    // copy info
    m_consoleCopyInfo.Open((SharedMemNames::formatCopyInfo % dwProcessId).str(), syncObjBoth);

    // mouse event
    m_consoleMouseEvent.Open((SharedMemNames::formatMouseEvent % dwProcessId).str(), syncObjBoth);

    // open new console size shared memory object
    m_newConsoleSize.Open((SharedMemNames::formatNewConsoleSize % dwProcessId).str(), syncObjRequest);

    // new scroll position
    m_newScrollPos.Open((SharedMemNames::formatNewScrollPos % dwProcessId).str(), syncObjRequest);

    // message pipe (workaround for User Interface Privilege Isolation messages filtering)
    m_consoleMsgPipe.Open((SharedMemNames::formatPipeName % dwProcessId).str());

		// multiple info :
		//  current directory
		//  process list
		m_multipleInfo.Open((SharedMemNames::formatMultipleInfo % dwProcessId).str(), syncObjBoth);
  }
  catch(Win32Exception& ex)
  {
    fprintf(stderr, "/!\\ ConsoleZ: can't open shared objects (reason: %s)\n", ex.what());
    return false;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ReadConsoleBuffer()
{
	auto timePoint2 = std::chrono::system_clock::now();
	DWORD dwElapsedTime = static_cast<DWORD>(
		std::chrono::duration_cast<std::chrono::milliseconds>(timePoint2 - m_timePoint).count());
	DWORD dwNotificationTimeout = m_consoleParams->dwNotificationTimeout;

	if(dwElapsedTime >= dwNotificationTimeout)
	{
		// notification timeout expired
		ForceReadConsoleBuffer();
	}
	else
	{
		// wait more
		m_dwWaitingTime = dwNotificationTimeout - dwElapsedTime;
	}
}

void ConsoleHandler::ForceReadConsoleBuffer()
{
	RealReadConsoleBuffer();
	m_dwWaitingTime = INFINITE;
	m_timePoint = std::chrono::system_clock::now();
}

void ConsoleHandler::RealReadConsoleBuffer()
{
	// we take a fresh STDOUT handle - seems to work better (in case a program
	// has opened a new screen output buffer)
	GET_STD_OUT_READ_ONLY

	CONSOLE_SCREEN_BUFFER_INFO csbiConsole;
	COORD                      coordConsoleSize;

	if(!::GetConsoleScreenBufferInfo(hStdOut, &csbiConsole))
	{
		Win32Exception err("GetConsoleScreenBufferInfo", ::GetLastError());
		TRACE(L"GetConsoleScreenBufferInfo(%p) returns error (%lu) : %S\n", hStdOut, err.GetErrorCode(), err.what());
		return;
	}

	// check for inconsistent values
	if(csbiConsole.srWindow.Right < csbiConsole.srWindow.Left || csbiConsole.srWindow.Bottom < csbiConsole.srWindow.Top)
		return;

	coordConsoleSize.X = csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordConsoleSize.Y = csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top + 1;

	/*
	TRACE(L"ReadConsoleBuffer console buffer size: %ix%i\n", csbiConsole.dwSize.X, csbiConsole.dwSize.Y);
	TRACE(L"ReadConsoleBuffer console rect: %ix%i - %ix%i\n", csbiConsole.srWindow.Left, csbiConsole.srWindow.Top, csbiConsole.srWindow.Right, csbiConsole.srWindow.Bottom);
	TRACE(L"console window rect: (%i, %i) - (%i, %i)\n", csbiConsole.srWindow.Top, csbiConsole.srWindow.Left, csbiConsole.srWindow.Bottom, csbiConsole.srWindow.Right);
	*/

	// do console output buffer reading
	DWORD       dwScreenBufferSize = coordConsoleSize.X * coordConsoleSize.Y;
	DWORD       dwScreenBufferOffset = 0;

	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[dwScreenBufferSize]);

	COORD      coordBufferSize;
	// start coordinates for the buffer are always (0, 0) - we use offset
	COORD      coordStart = {0, 0};
	SMALL_RECT srBuffer;

	//TRACE(L"===================================================================\n");

	// ReadConsoleOutput seems to fail for large (around 6k CHAR_INFO's) buffers
	// here we calculate max buffer size (row count) for safe reading
	coordBufferSize.X = csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordBufferSize.Y = 6144 / coordBufferSize.X;

	// initialize reading rectangle
	srBuffer.Top      = csbiConsole.srWindow.Top;
	srBuffer.Bottom   = csbiConsole.srWindow.Top + coordBufferSize.Y - 1;
	srBuffer.Left     = csbiConsole.srWindow.Left;
	srBuffer.Right    = csbiConsole.srWindow.Left + csbiConsole.srWindow.Right - csbiConsole.srWindow.Left;

	/*
	TRACE(L"Buffer size for loop reads: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
	*/

	// read rows 'chunks'
	SHORT i = 0;
	for(; i < coordConsoleSize.Y / coordBufferSize.Y; ++i)
	{
		//TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get() + dwScreenBufferOffset,
			coordBufferSize,
			coordStart,
			&srBuffer);

		srBuffer.Top    = srBuffer.Top + coordBufferSize.Y;
		srBuffer.Bottom = srBuffer.Bottom + coordBufferSize.Y;

		dwScreenBufferOffset += coordBufferSize.X * coordBufferSize.Y;
	}

	// read the last 'chunk', we need to calculate the number of rows in the
	// last chunk and update bottom coordinate for the region
	coordBufferSize.Y = coordConsoleSize.Y - i * coordBufferSize.Y;
	srBuffer.Bottom   = csbiConsole.srWindow.Bottom;

	/*
	TRACE(L"Buffer size for last read: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
	TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);
	*/

	::ReadConsoleOutput(
		hStdOut,
		pScreenBuffer.get() + dwScreenBufferOffset,
		coordBufferSize,
		coordStart,
		&srBuffer);


	//TRACE(L"===================================================================\n");

	unsigned long long ullProgressCompleted = 0ULL;
	unsigned long long ullProgressTotal     = 0ULL;
	GetPowerShellProgress(hStdOut, csbiConsole, ullProgressCompleted, ullProgressTotal);

	// compare previous buffer, and if different notify ConsoleZ
	SharedMemoryLock consoleInfoLock(m_consoleInfo);
	SharedMemoryLock bufferLock(m_consoleBuffer);

	::GetConsoleCursorInfo(hStdOut, m_cursorInfo.Get());

	bool somethingChanged = false;

	// compare text
	if(m_dwScreenBufferSize != dwScreenBufferSize
	   ||
	   ::memcmp(m_consoleBuffer.Get(), pScreenBuffer.get(), dwScreenBufferSize * sizeof(CHAR_INFO)) != 0)
	{
		::CopyMemory(m_consoleBuffer.Get(), pScreenBuffer.get(), dwScreenBufferSize * sizeof(CHAR_INFO));

		somethingChanged = true;

		// only ConsoleZ sets the flags to false
		m_consoleInfo->textChanged  = true;
	}

	// compare previous console title
	wchar_t szNewConsoleTitle[ARRAYSIZE(m_szConsoleTitle)] = L"";
	if(::GetConsoleTitle(szNewConsoleTitle, ARRAYSIZE(m_szConsoleTitle))
	   &&
	   wcscmp(szNewConsoleTitle, m_szConsoleTitle))
	{
		wcscpy_s<ARRAYSIZE(m_szConsoleTitle)>(m_szConsoleTitle, szNewConsoleTitle);

		somethingChanged = true;

		// only ConsoleZ sets the flags to false
		m_consoleInfo->titleChanged = true;
	}

	// compare console buffer information
	if(::memcmp(&m_consoleInfo->csbi, &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO))
	   ||
	   m_dwScreenBufferSize != dwScreenBufferSize)
	{
		// update screen buffer variables
		m_dwScreenBufferSize = dwScreenBufferSize;
		::CopyMemory(&m_consoleInfo->csbi, &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO));

		somethingChanged = true;

		// only ConsoleZ sets the flags to false
		m_consoleInfo->csbiChanged  = true;
	}

	// compare progress
	if(ullProgressCompleted != m_consoleInfo->ullProgressCompleted || ullProgressTotal != m_consoleInfo->ullProgressTotal)
	{
		m_consoleInfo->ullProgressCompleted = ullProgressCompleted;
		m_consoleInfo->ullProgressTotal     = ullProgressTotal;

		somethingChanged = true;

		// only ConsoleZ sets the flags to false
		m_consoleInfo->progressChanged  = true;
	}

	if (!m_consoleInfo->titleChanged)
	{
		// compare current directory
		wchar_t szCurrentDirectory[_MAX_PATH];
		szCurrentDirectory[0] = 0;
		DWORD len = ::GetCurrentDirectory(_MAX_PATH, szCurrentDirectory);
		if (len >= _MAX_PATH)
			// truncated
			szCurrentDirectory[0] = 0;
		if (wcscmp(szCurrentDirectory, m_szCurrentDirectory))
		{
			::memcpy(m_szCurrentDirectory, szCurrentDirectory, _MAX_PATH);
			somethingChanged = true;

			// only ConsoleZ sets the flags to false
			m_consoleInfo->titleChanged = true;
		}
	}

	if (!m_consoleInfo->titleChanged)
	{
		// compare last process ID attached to the console
		DWORD lpdwProcessList[256];
		DWORD dwProcessCount = ::GetConsoleProcessList(lpdwProcessList, 256);
		if (dwProcessCount > 0 && dwProcessCount < 256)
		{
			if (lpdwProcessList[dwProcessCount - 1] != m_dwLastAttachedPID)
			{
				m_dwLastAttachedPID = lpdwProcessList[dwProcessCount - 1];
				somethingChanged = true;

				// only ConsoleZ sets the flags to false
				m_consoleInfo->titleChanged = true;
			}
		}
	}


	// if something changed then event is alerted
	if(somethingChanged)
	{
		m_consoleBuffer.SetReqEvent();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetConsoleFont()
{
#ifndef _USING_V110_SDK71_
	// SetCurrentConsoleFontEx is available only since Vista

	// We set "Lucidia Console" font beacuse:
	// - this font is availbale everywhere
	// - we need a True Type font to use Unicode console
	//   (with RASTER font, chars are restricted to the current code page)
	// we set the smallest font size because:
	// - max cols and rows are computed from the primary monitor size and the console font size
	CONSOLE_FONT_INFOEX cfi = {sizeof(cfi)};
	cfi.dwFontSize.X = 0x0000;
	cfi.dwFontSize.Y = 0x0005;
	::wcscpy_s(cfi.FaceName, ARRAYSIZE(cfi.FaceName), L"Lucida Console");
	::SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
#endif
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::GetPowerShellProgress(HANDLE hStdOut, CONSOLE_SCREEN_BUFFER_INFO& csbiConsole, unsigned long long & ullProgressCompleted, unsigned long long & ullProgressTotal)
{
	// Bug in KernelBase appears from Windows 8 and still alive in Windows 8.1 console!
	// when application calls ReadConsoleOutput function to read region out of screen buffer
	// access violation occurs (application crashes)
	// behavior has been fixed in Windows 10
	if( !m_consoleParams->bWin10 ) return false;

	SHORT sBufferColumns = csbiConsole.dwSize.X;
	SHORT sBufferRows    = csbiConsole.dwSize.Y;

	// buffer width is too small to display a progress bar!
	if( sBufferColumns <= 12 )
		return false;

	// we search the empty line of color 3e
	// this line is in third row
	// of the window containing the cursor
	SHORT sMinRow = csbiConsole.dwCursorPosition.Y
	              - (csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top)
	              + 2;
	if( sMinRow < 2 ) sMinRow = 2;

	SHORT sMaxRow = csbiConsole.dwCursorPosition.Y + 2;
	if( sMaxRow >= sBufferRows ) sMaxRow = sBufferRows - 1;

	COORD coordFrom       = {0, 0};
	COORD coordBufferSize = {sBufferColumns, 1};

	SMALL_RECT srBuffer;
	srBuffer.Left  = 0;
	srBuffer.Right = sBufferColumns - 1;
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[sBufferColumns]);

	//TRACE(L"GetPowerShellProgress from %hd to %hd\n", sMinRow, sMaxRow);
	for( SHORT sRow = sMinRow; sRow <= sMaxRow; ++sRow )
	{
		srBuffer.Top    = sRow;
		srBuffer.Bottom = sRow;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		bool bEmpty = true;
		for( SHORT i = srBuffer.Left; i < srBuffer.Right; ++i )
		{
			if( pScreenBuffer[i].Attributes != 0x3e ||  pScreenBuffer[i].Char.UnicodeChar != L' ' )
			{
				bEmpty = false;
				break;
			}
		}

		if( bEmpty )
		{
			// we search "    [ooo...ooo   ...   ]"
			for( SHORT sRow2 = sRow + 3; sRow2 < min(sMaxRow + 10, sBufferRows); ++sRow2 )
			{
				srBuffer.Top    = sRow2;
				srBuffer.Bottom = sRow2;

				::ReadConsoleOutput(
					hStdOut,
					pScreenBuffer.get(),
					coordBufferSize,
					coordFrom,
					&srBuffer);

				if( pScreenBuffer[0].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[1].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[2].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[3].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[4].Char.UnicodeChar != L'[' ) continue;

				if( pScreenBuffer[sBufferColumns - 1].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 2].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 3].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 4].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 5].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 6].Char.UnicodeChar != L' ' ) continue;
				if( pScreenBuffer[sBufferColumns - 7].Char.UnicodeChar != L']' ) continue;

				int nBarLength = 0;
				for(SHORT sCol = 5; sCol < sBufferColumns - 7; ++sCol)
				{
					if( pScreenBuffer[sCol].Char.UnicodeChar == L'o' ) nBarLength++;
					else if ( pScreenBuffer[sCol].Char.UnicodeChar != L' ' ) return false;
				}

				TRACE(L"GetPowerShellProgress progress = %i/%i\n", 
							nBarLength,
							sBufferColumns - 12);

				ullProgressCompleted = nBarLength;
				ullProgressTotal     = sBufferColumns - 12;
				return true;
			}

			return false;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResizeConsoleWindow(DWORD& dwColumns, DWORD& dwRows, DWORD dwResizeWindowEdge)
{
	GET_STD_OUT_READ_WRITE

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);
	TRACE(L"Console size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
	TRACE(L"Old win pos: %ix%i - %ix%i\n", csbi.srWindow.Left, csbi.srWindow.Top, csbi.srWindow.Right, csbi.srWindow.Bottom);

  bool boolCursorVisible =
    csbi.dwCursorPosition.X >= csbi.srWindow.Left  &&
    csbi.dwCursorPosition.X <= csbi.srWindow.Right &&
    csbi.dwCursorPosition.Y >= csbi.srWindow.Top   &&
    csbi.dwCursorPosition.Y <= csbi.srWindow.Bottom;

	// check against max size
	TRACE(L"Columns: %i\n", dwColumns);
	TRACE(L"Max columns: %i\n", m_consoleParams->dwMaxColumns);

	TRACE(L"Rows: %i\n", dwRows);
	TRACE(L"Max rows: %i\n", m_consoleParams->dwMaxRows);

  TRACE(L"Cursor X: %i\n", csbi.dwCursorPosition.X);
  TRACE(L"Cursor Y: %i\n", csbi.dwCursorPosition.Y);
  TRACE(L"boolCursorVisible: %s\n", boolCursorVisible? L"true" : L"false");

	if (dwColumns > m_consoleParams->dwMaxColumns) dwColumns = m_consoleParams->dwMaxColumns;
	if (dwRows > m_consoleParams->dwMaxRows) dwRows = m_consoleParams->dwMaxRows;

	COORD		coordBufferSize;
	SMALL_RECT	srConsoleRect;

	TRACE(L"Screen buffer: %ix%i\n", m_consoleParams->dwBufferRows, m_consoleParams->dwBufferColumns);

	// determine new buffer size
	if (m_consoleParams->dwBufferColumns == 0)
	{
		coordBufferSize.X = static_cast<SHORT>(dwColumns);
	}
	else
	{
		coordBufferSize.X = static_cast<SHORT>(m_consoleParams->dwBufferColumns);
	}

	if (m_consoleParams->dwBufferRows == 0)
	{
		coordBufferSize.Y = static_cast<SHORT>(dwRows);
	}
	else
	{
		coordBufferSize.Y = static_cast<SHORT>(m_consoleParams->dwBufferRows);
	}


	// determine new window size
	// vertical size
	switch (dwResizeWindowEdge)
	{
		case WMSZ_TOP :
		case WMSZ_TOPLEFT :
		case WMSZ_TOPRIGHT :
		{
			if ((csbi.srWindow.Top == 0) || (csbi.srWindow.Bottom - static_cast<SHORT>(dwRows - 1) <= 0))
			{
				srConsoleRect.Top    = 0;
				srConsoleRect.Bottom = static_cast<SHORT>(dwRows - 1);
			}
			else
			{
				srConsoleRect.Top    = csbi.srWindow.Bottom - static_cast<SHORT>(dwRows - 1);
				srConsoleRect.Bottom = csbi.srWindow.Bottom;
			}

			break;
		}

		case WMSZ_BOTTOM :
		case WMSZ_BOTTOMLEFT :
		case WMSZ_BOTTOMRIGHT :
		{
			if ((m_consoleParams->dwBufferRows > 0) && (csbi.srWindow.Top + static_cast<SHORT>(dwRows) > static_cast<SHORT>(m_consoleParams->dwBufferRows)))
			{
				srConsoleRect.Top    = static_cast<SHORT>(m_consoleParams->dwBufferRows - dwRows);
				srConsoleRect.Bottom = static_cast<SHORT>(m_consoleParams->dwBufferRows - 1);
			}
			else
			{
				srConsoleRect.Top    = csbi.srWindow.Top;
				srConsoleRect.Bottom = csbi.srWindow.Top + static_cast<SHORT>(dwRows - 1);
			}

			break;
		}

		default :
		{
			srConsoleRect.Top    = csbi.srWindow.Top;
			srConsoleRect.Bottom = csbi.srWindow.Top + static_cast<SHORT>(dwRows - 1);
		}
	}

  if( boolCursorVisible &&
      ( csbi.dwCursorPosition.Y < srConsoleRect.Top ||
        csbi.dwCursorPosition.Y > srConsoleRect.Bottom ) )
  {
    if( csbi.dwCursorPosition.Y < static_cast<SHORT>(dwRows) )
    {
      srConsoleRect.Top    = 0;
      srConsoleRect.Bottom = static_cast<SHORT>(dwRows - 1);
    }
    else
    {
      srConsoleRect.Top    = csbi.dwCursorPosition.Y - static_cast<SHORT>(dwRows - 1);
      srConsoleRect.Bottom = csbi.dwCursorPosition.Y;
    }
  }

	// horizontal size
	switch (dwResizeWindowEdge)
	{
		case WMSZ_LEFT :
		case WMSZ_TOPLEFT :
		case WMSZ_BOTTOMLEFT :
		{
			if ((csbi.srWindow.Left == 0) || (csbi.srWindow.Right - static_cast<SHORT>(dwColumns - 1) <= 0))
			{
				srConsoleRect.Left  = 0;
				srConsoleRect.Right = static_cast<SHORT>(dwColumns - 1);
			}
			else
			{
				srConsoleRect.Left  = csbi.srWindow.Right - static_cast<SHORT>(dwColumns - 1);
				srConsoleRect.Right = csbi.srWindow.Right;
			}

			break;
		}

		case WMSZ_RIGHT :
		case WMSZ_TOPRIGHT :
		case WMSZ_BOTTOMRIGHT :
		{
			if ((m_consoleParams->dwBufferColumns != 0) && (csbi.srWindow.Left + static_cast<SHORT>(dwColumns) > static_cast<SHORT>(m_consoleParams->dwBufferColumns)))
			{
				srConsoleRect.Left  = static_cast<SHORT>(m_consoleParams->dwBufferColumns - dwColumns);
				srConsoleRect.Right = static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1);
			}
			else
			{
				srConsoleRect.Left  = csbi.srWindow.Left;
				srConsoleRect.Right = csbi.srWindow.Left + static_cast<SHORT>(dwColumns - 1);
			}

			break;
		}

		default :
		{
			srConsoleRect.Left  = csbi.srWindow.Left;
			srConsoleRect.Right = csbi.srWindow.Left + static_cast<SHORT>(dwColumns - 1);
		}
	}

  if( boolCursorVisible &&
      ( csbi.dwCursorPosition.X < srConsoleRect.Left ||
        csbi.dwCursorPosition.X > srConsoleRect.Right ) )
  {
    if( csbi.dwCursorPosition.X < static_cast<SHORT>(dwColumns) )
    {
      srConsoleRect.Left  = 0;
      srConsoleRect.Right = static_cast<SHORT>(dwColumns - 1);
    }
    else
    {
      srConsoleRect.Left  = csbi.dwCursorPosition.X - static_cast<SHORT>(dwColumns - 1);
      srConsoleRect.Right = csbi.dwCursorPosition.X;
    }
  }

	TRACE(L"New win pos: %ix%i - %ix%i\n", srConsoleRect.Left, srConsoleRect.Top, srConsoleRect.Right, srConsoleRect.Bottom);
	TRACE(L"Buffer size: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);

	COORD		finalCoordBufferSize;
	SMALL_RECT	finalConsoleRect;

	// first, resize rows
	finalCoordBufferSize.X	= csbi.dwSize.X;
	finalCoordBufferSize.Y	= coordBufferSize.Y;

	finalConsoleRect.Left	= csbi.srWindow.Left;
	finalConsoleRect.Right	= csbi.srWindow.Right;

	finalConsoleRect.Top	= srConsoleRect.Top;
	finalConsoleRect.Bottom	= srConsoleRect.Bottom;

	if (coordBufferSize.Y > csbi.dwSize.Y)
	{
		// if new buffer size is > than old one, we need to resize the buffer first
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
	}
	else
	{
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
	}

	// then, resize columns
	finalCoordBufferSize.X	= coordBufferSize.X;

	finalConsoleRect.Left	= srConsoleRect.Left;
	finalConsoleRect.Right	= srConsoleRect.Right;

	if (coordBufferSize.X > csbi.dwSize.X)
	{
		// if new buffer size is > than old one, we need to resize the buffer first
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
	}
	else
	{
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
	}

	TRACE(L"console buffer size: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"console rect: %ix%i - %ix%i\n", finalConsoleRect.Left, finalConsoleRect.Top, finalConsoleRect.Right, finalConsoleRect.Bottom);

	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	dwColumns = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
	dwRows    = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;

	TRACE(L"Columns: %i\n", dwColumns);
	TRACE(L"Rows: %i\n", dwRows);

//	TRACE(L"console buffer size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
//	TRACE(L"console rect: %ix%i - %ix%i\n", csbi.srWindow.Left, csbi.srWindow.Top, csbi.srWindow.Right, csbi.srWindow.Bottom);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ClipboardData
{
public:
  ClipboardData(void) {}
  virtual ~ClipboardData(void) {}
  virtual void StartRow(void) = 0;
  virtual void EndRow(void) = 0;
  virtual void AddChar(PCHAR_INFO) = 0;
  virtual bool IsEOL(DWORD dwEOLSpaces) = 0;
  virtual size_t GetRowLength(void) = 0;
  virtual void TrimRight(void) = 0;
  virtual void Wrap(CopyNewlineChar) = 0;
  virtual void Publish(ClipboardHelper&) = 0;
};

class ClipboardDataUnicode : public ClipboardData
{
public:
  ClipboardDataUnicode(void):strText(L"") {}
  virtual ~ClipboardDataUnicode(void) {}
  virtual void StartRow(void)
  {
    strRow = L"";
  }
  virtual void EndRow(void)
  {
    strText += strRow;
  }
  virtual void AddChar(PCHAR_INFO p)
  {
    strRow += p->Char.UnicodeChar;
  }
  virtual bool IsEOL(DWORD dwEOLSpaces)
  {
		size_t len = strRow.length();
		if( len < dwEOLSpaces ) return false;

		for(size_t i = (len - dwEOLSpaces); i < len; ++i)
			if( strRow[i] != L' ' )
				return false;

		return true;
  }
  size_t GetRowLength(void)
  {
    return strRow.length();
  }
  virtual void TrimRight(void)
  {
    boost::trim_right(strRow);
  }
  virtual void Wrap(CopyNewlineChar copyNewlineChar)
  {
    switch(copyNewlineChar)
    {
      case newlineCRLF: strRow += wstring(L"\r\n"); break;
      case newlineLF:   strRow += wstring(L"\n");   break;
      default:          strRow += wstring(L"\r\n"); break;
    }
  }
  virtual void Publish(ClipboardHelper& clipboard)
  {
		clipboard.setData(CF_UNICODETEXT, strText.c_str(), (strText.length() + 1) * sizeof(wchar_t));
  }

  const wchar_t * GetText(void) const
  {
	  return strText.c_str();
  }

  size_t GetTextLength(void) const
  {
	  return strText.length();
  }

private:
  wstring strText;
  wstring strRow;
};

class ClipboardDataRtf : public ClipboardData
{
public:
  ClipboardDataRtf(ConsoleCopy* pconsoleCopy):sizeRtfLen(0)
  {
    strRtf = "{\\rtf\\ansi\\deff0";

    strRtf += "{\\fonttbl{\\f0\\fnil ";
    strRtf += pconsoleCopy->szFontName;
    strRtf += ";}}";

    strRtf += "{\\colortbl\n";
    for(int i = 0; i < 16; i ++)
    {
      char szColor[64];
      _snprintf_s(
        szColor, sizeof(szColor),
        _TRUNCATE,
        "\\red%lu\\green%lu\\blue%lu;\n",
        GetRValue(pconsoleCopy->consoleColors[i]),
        GetGValue(pconsoleCopy->consoleColors[i]),
        GetBValue(pconsoleCopy->consoleColors[i]));
      strRtf += szColor;
    }
    strRtf += "}";

    char szFont[64];
    _snprintf_s(
      szFont, sizeof(szFont),
      _TRUNCATE,
      "\\f0\\fs%lu%s%s\n",
      pconsoleCopy->dwSize,
      pconsoleCopy->bBold ? "\\b" : "",
      pconsoleCopy->bItalic ? "\\i" : "");

    strRtf += szFont;
  }
  virtual ~ClipboardDataRtf(void) {}
  virtual void StartRow(void)
  {
    strRowRtf.clear();
    sizeRowLen = 0;
  }
  virtual void EndRow(void)
  {
    strRtf += strRowRtf;
    strRtf += strTrimRowRtf;
    strTrimRowRtf.clear();
  }
  virtual void AddChar(PCHAR_INFO p)
  {
    char szDummy[32];

    WORD wCharForegroundAttributes = p->Attributes & 0x000f;
    WORD wCharBackgroundAttributes = (p->Attributes >> 4) & 0x000f;
    if( sizeRtfLen == 0 )
    {
      wLastCharForegroundAttributes = ~wCharForegroundAttributes;
      wLastCharBackgroundAttributes = ~wCharBackgroundAttributes;
    }

    bool trim = std::isspace<wchar_t>(p->Char.UnicodeChar, std::locale());

    std::string& strRowRtfRef = (trim)?strTrimRowRtf:strRowRtf;
    if( trim )
    {
      if( strTrimRowRtf.empty() )
      {
        wLastTrimCharForegroundAttributes = wLastCharForegroundAttributes;
        wLastTrimCharBackgroundAttributes = wLastCharBackgroundAttributes;
      }
    }
    else
    {
      strRowRtf += strTrimRowRtf;
      strTrimRowRtf.clear();
    }

    if( wLastCharBackgroundAttributes != wCharBackgroundAttributes )
    {
      _snprintf_s(
        szDummy, sizeof(szDummy),
        _TRUNCATE,
        "\\highlight%hu ",
        wCharBackgroundAttributes);
      strRowRtfRef += szDummy;
    }
    if( wLastCharForegroundAttributes != wCharForegroundAttributes )
    {
      _snprintf_s(
        szDummy, sizeof(szDummy),
        _TRUNCATE,
        "\\cf%hu ",
        wCharForegroundAttributes);
      strRowRtfRef += szDummy;
    }
    wLastCharForegroundAttributes = wCharForegroundAttributes;
    wLastCharBackgroundAttributes = wCharBackgroundAttributes;

    WCHAR wc = p->Char.UnicodeChar;
         if( wc == L'\\' ) strRowRtfRef += "\\\\";
    else if( wc == L'{' )  strRowRtfRef += "\\{";
    else if( wc == L'}' )  strRowRtfRef += "\\}";
    else if( wc <= 0x7f )  strRowRtfRef += p->Char.AsciiChar;
    else
    {
      _snprintf_s(szDummy, sizeof(szDummy), _TRUNCATE, "\\u%u?", wc);
      strRowRtfRef += szDummy;
    }
    sizeRowLen ++;
    sizeRtfLen ++;
  }
  virtual bool IsEOL(DWORD /*dwEOLSpaces*/)
  {
    return !strTrimRowRtf.empty();
  }
  size_t GetRowLength(void)
  {
    return sizeRowLen;
  }
  virtual void TrimRight(void)
  {
    if( !strTrimRowRtf.empty() )
    {
      strTrimRowRtf.clear();
      wLastCharForegroundAttributes = wLastTrimCharForegroundAttributes;
      wLastCharBackgroundAttributes = wLastTrimCharBackgroundAttributes;
    }
  }
  virtual void Wrap(CopyNewlineChar /*copyNewlineChar*/)
  {
    strTrimRowRtf += "\\line\n";
  }
  virtual void Publish(ClipboardHelper& clipboard)
  {
    strRtf += "}";

		clipboard.setData(::RegisterClipboardFormat(L"Rich Text Format"), strRtf.c_str(), strRtf.length() + 1);
  }

private:
  string strRtf;
  string strRowRtf;
  string strTrimRowRtf;
  size_t sizeRtfLen;
  size_t sizeRowLen;
  WORD   wLastCharForegroundAttributes;
  WORD   wLastCharBackgroundAttributes;
  WORD   wLastTrimCharForegroundAttributes;
  WORD   wLastTrimCharBackgroundAttributes;
};

void ConsoleHandler::CopyConsoleTextLine(HANDLE hStdOut, std::unique_ptr<ClipboardData> clipboardDataPtr[], size_t clipboardDataCount)
{
	COORD& coordStart = m_consoleCopyInfo->coordStart;
	COORD& coordEnd   = m_consoleCopyInfo->coordEnd;

	COORD                        coordFrom       = {0, 0};
	COORD                        coordBufferSize = {(m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns) : static_cast<SHORT>(m_consoleParams->dwColumns), 1};
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[coordBufferSize.X]);

	// suppress end empty lines
	bool emptyLine = true;
	for (SHORT i = coordEnd.Y; i > coordStart.Y && emptyLine; --i)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = 0;
		srBuffer.Top    = i;
		srBuffer.Right  = (i == coordEnd.Y) ? coordEnd.X : (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) : static_cast<SHORT>(m_consoleParams->dwColumns - 1);
		srBuffer.Bottom = i;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = 0; x <= srBuffer.Right - srBuffer.Left && emptyLine; ++x)
		{
			if( pScreenBuffer[x].Char.UnicodeChar != L' ' )
				emptyLine = false;
		}

		if( emptyLine )
		{
			coordEnd.Y --;
			coordEnd.X = coordBufferSize.X - 1;
		}
	}

	for (SHORT i = coordStart.Y; i <= coordEnd.Y; ++i)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left	= (i == coordStart.Y) ? coordStart.X : 0;
		srBuffer.Top	= i;
		srBuffer.Right	= (i == coordEnd.Y) ? coordEnd.X : (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) : static_cast<SHORT>(m_consoleParams->dwColumns - 1);
		srBuffer.Bottom	= i;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
			clipboardDataPtr[clipboardDataIndex]->StartRow();

		bool bWrap       = true;
		bool bTrimSpaces = m_consoleCopyInfo->bTrimSpaces;

		for (SHORT x = 0; x <= srBuffer.Right - srBuffer.Left; ++x)
		{
			if (pScreenBuffer[x].Attributes & COMMON_LVB_TRAILING_BYTE) continue;
			for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
				clipboardDataPtr[clipboardDataIndex]->AddChar(&(pScreenBuffer[x]));
		}

		// handle trim/wrap settings
		if (coordStart.Y == coordEnd.Y)
		{
			// only one line
			bWrap = false;
		}
		if (i == coordEnd.Y)
		{
			// last row
			if (clipboardDataPtr[0]->GetRowLength() < static_cast<size_t>(coordBufferSize.X))
			{
				bWrap = false;
			}
		}
		else
		{
			// rows between first and (last - 1)
			if (m_consoleCopyInfo->bNoWrap && (!clipboardDataPtr[0]->IsEOL(m_consoleCopyInfo->dwEOLSpaces)))
			{
				bWrap       = false;
				bTrimSpaces = false;
			}
		}

		for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
		{
			if (bTrimSpaces)
				clipboardDataPtr[clipboardDataIndex]->TrimRight();

			if (bWrap)
				clipboardDataPtr[clipboardDataIndex]->Wrap(m_consoleCopyInfo->copyNewlineChar);

			clipboardDataPtr[clipboardDataIndex]->EndRow();
		}
	}
}

void ConsoleHandler::CopyConsoleTextColumn(HANDLE hStdOut, std::unique_ptr<ClipboardData> clipboardDataPtr[], size_t clipboardDataCount)
{
	COORD& coordStart = m_consoleCopyInfo->coordStart;
	COORD& coordEnd   = m_consoleCopyInfo->coordEnd;

	if( m_consoleCopyInfo->selectionType == seltypeColumn )
	{
		SHORT nLeft  = min(coordStart.X, coordEnd.X);
		SHORT nRight = max(coordStart.X, coordEnd.X);

		coordStart.X = nLeft;
		coordEnd.X   = nRight;
	}

	COORD                        coordFrom       = {0, 0};
	COORD                        coordBufferSize = {coordEnd.X - coordStart.X + 1, 1};
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[coordBufferSize.X]);

	// suppress end empty lines
	bool emptyLine = true;
	for (SHORT i = coordEnd.Y; i > coordStart.Y && emptyLine; --i)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = coordStart.X;
		srBuffer.Top    = i;
		srBuffer.Right  = coordEnd.X;
		srBuffer.Bottom = i;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = 0; x <= srBuffer.Right - srBuffer.Left && emptyLine; ++x)
		{
			if( pScreenBuffer[x].Char.UnicodeChar != L' ' )
				emptyLine = false;
		}

		if( emptyLine )
		{
			coordEnd.Y --;
		}
	}

	for (SHORT i = coordStart.Y; i <= coordEnd.Y; ++i)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = coordStart.X;
		srBuffer.Top    = i;
		srBuffer.Right  = coordEnd.X;
		srBuffer.Bottom = i;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
			clipboardDataPtr[clipboardDataIndex]->StartRow();

		bool bWrap       = true;
		bool bTrimSpaces = false;

		for (SHORT x = 0; x <= srBuffer.Right - srBuffer.Left; ++x)
		{
			if (pScreenBuffer[x].Attributes & COMMON_LVB_TRAILING_BYTE) continue;
			for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
				clipboardDataPtr[clipboardDataIndex]->AddChar(&(pScreenBuffer[x]));
		}

		for(size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex ++)
		{
			if (bTrimSpaces)
				clipboardDataPtr[clipboardDataIndex]->TrimRight();

			if (bWrap)
				clipboardDataPtr[clipboardDataIndex]->Wrap(m_consoleCopyInfo->copyNewlineChar);

			clipboardDataPtr[clipboardDataIndex]->EndRow();
		}
	}
}

void ConsoleHandler::CopyConsoleText()
{
	try
	{
		GET_STD_OUT_READ_ONLY

		std::unique_ptr<ClipboardData> clipboardDataPtr[2];
		size_t clipboardDataCount = 0;
		clipboardDataPtr[clipboardDataCount++].reset(new ClipboardDataUnicode());
		if( m_consoleCopyInfo->bRTF )
			clipboardDataPtr[clipboardDataCount++].reset(new ClipboardDataRtf(m_consoleCopyInfo.Get()));

		if( m_consoleCopyInfo->selectionType == seltypeColumn )
			CopyConsoleTextColumn(hStdOut, clipboardDataPtr, clipboardDataCount);
		else
			CopyConsoleTextLine(hStdOut, clipboardDataPtr, clipboardDataCount);

		{
			ClipboardHelper clipboard;

			clipboard.empty();

			for( size_t clipboardDataIndex = 0; clipboardDataIndex < clipboardDataCount; clipboardDataIndex++ )
				clipboardDataPtr[clipboardDataIndex]->Publish(clipboard);
		}
	}
	catch( std::exception& e )
	{
		::MessageBoxA(0, e.what(), "exception", MB_ICONERROR | MB_OK);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::GetSelectionPart()
{
	m_consoleCopyInfo->szSelectionPart[0] = 0;

	if( m_consoleCopyInfo->dwSelectionPartOffset == 0 )
	{
		GET_STD_OUT_READ_ONLY

		m_selectionFullText.reset(new ClipboardDataUnicode());

		if( m_consoleCopyInfo->selectionType == seltypeColumn )
			CopyConsoleTextColumn(hStdOut, &m_selectionFullText, 1);
		else
			CopyConsoleTextLine(hStdOut, &m_selectionFullText, 1);
	}

	if( m_selectionFullText.get() )
	{
		ClipboardDataUnicode * p = reinterpret_cast<ClipboardDataUnicode *>(m_selectionFullText.get());
		if( p->GetTextLength() > m_consoleCopyInfo->dwSelectionPartOffset )
		{
			size_t part_len = p->GetTextLength() - m_consoleCopyInfo->dwSelectionPartOffset;
			if( part_len > MAX_SELECTION_PART ) part_len = MAX_SELECTION_PART;

			wcsncpy_s(
				m_consoleCopyInfo->szSelectionPart,
				MAX_SELECTION_PART + 1,
				p->GetText() + m_consoleCopyInfo->dwSelectionPartOffset,
				part_len);

			m_consoleCopyInfo->szSelectionPart[part_len] = 0;

			m_consoleCopyInfo->dwSelectionPartOffset += static_cast<DWORD>(part_len);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SelectWord()
{
	GET_STD_OUT_READ_ONLY

	SHORT                        sBufferColumns  = (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns) : static_cast<SHORT>(m_consoleParams->dwColumns);
	SHORT                        sBufferRows     = (m_consoleParams->dwBufferRows    > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferRows)    : static_cast<SHORT>(m_consoleParams->dwRows);
	COORD                        coordFrom       = {0, 0};
	COORD                        coordBufferSize = {sBufferColumns, 1};
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[sBufferColumns]);

	m_multipleInfo->coordLeft.X  = 0;
	m_multipleInfo->coordLeft.Y  = 0;
	m_multipleInfo->coordRight.X = sBufferColumns - 1;
	m_multipleInfo->coordRight.Y = sBufferRows - 1;

	SHORT x0 = m_multipleInfo->coordCurrent.X;
	SHORT y0 = m_multipleInfo->coordCurrent.Y;

	for (SHORT y = m_multipleInfo->coordCurrent.Y; y >= 0; --y)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = 0;
		srBuffer.Top    = y;
		srBuffer.Right  = (y == m_multipleInfo->coordCurrent.Y)? m_multipleInfo->coordCurrent.X : (sBufferColumns - 1);
		srBuffer.Bottom = y;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = srBuffer.Right; x >= srBuffer.Left; --x)
		{
			for (size_t d = 0; m_multipleInfo->u.select_word.szLeftDelimiters[d]; ++d)
			{
				if( pScreenBuffer[x - srBuffer.Left].Char.UnicodeChar == m_multipleInfo->u.select_word.szLeftDelimiters[d] )
				{
					if( m_multipleInfo->u.select_word.bIncludeLeftDelimiter )
					{
						m_multipleInfo->coordLeft.X = x;
						m_multipleInfo->coordLeft.Y = y;
					}
					else
					{
						m_multipleInfo->coordLeft.X = x0;
						m_multipleInfo->coordLeft.Y = y0;
					}
					goto right;
				}
			}
			x0 = x;
		}
		y0 = y;
	}

right:

	x0 = m_multipleInfo->coordCurrent.X;
	y0 = m_multipleInfo->coordCurrent.Y;

	for (SHORT y = m_multipleInfo->coordCurrent.Y; y < sBufferRows; ++y)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = (y == m_multipleInfo->coordCurrent.Y)? m_multipleInfo->coordCurrent.X : 0;
		srBuffer.Top    = y;
		srBuffer.Right  = sBufferColumns - 1;
		srBuffer.Bottom = y;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = srBuffer.Left; x <= srBuffer.Right; ++x)
		{
			for (size_t d = 0; m_multipleInfo->u.select_word.szRightDelimiters[d]; ++d)
			{
				if( pScreenBuffer[x - srBuffer.Left].Char.UnicodeChar == m_multipleInfo->u.select_word.szRightDelimiters[d] )
				{
					if( m_multipleInfo->u.select_word.bIncludeRightDelimiter )
					{
						m_multipleInfo->coordRight.X = x;
						m_multipleInfo->coordRight.Y = y;
					}
					else
					{
						m_multipleInfo->coordRight.X = x0;
						m_multipleInfo->coordRight.Y = y0;
					}
					return;
				}
			}
			x0 = x;
		}
		y0 = y;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ClickLink()
{
	GET_STD_OUT_READ_ONLY

	SHORT                        sBufferColumns  = (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns) : static_cast<SHORT>(m_consoleParams->dwColumns);
	SHORT                        sBufferRows     = (m_consoleParams->dwBufferRows    > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferRows)    : static_cast<SHORT>(m_consoleParams->dwRows);
	COORD                        coordFrom       = {0, 0};
	COORD                        coordBufferSize = {sBufferColumns, 1};
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[sBufferColumns]);

	std::wstring strLink;

	for (SHORT y = m_multipleInfo->coordCurrent.Y; y >= 0; --y)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = 0;
		srBuffer.Top    = y;
		srBuffer.Right  = (y == m_multipleInfo->coordCurrent.Y)? m_multipleInfo->coordCurrent.X : (sBufferColumns - 1);
		srBuffer.Bottom = y;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = srBuffer.Right; x >= srBuffer.Left; --x)
		{
			wchar_t c = pScreenBuffer[x - srBuffer.Left].Char.UnicodeChar;
			for (size_t d = 0; m_multipleInfo->u.select_word.szLeftDelimiters[d]; ++d)
			{
				if( c == m_multipleInfo->u.select_word.szLeftDelimiters[d] )
				{
					goto right;
				}
			}
			strLink.insert(0, &c, 1);
		}
	}

right:

	for (SHORT y = m_multipleInfo->coordCurrent.Y; y < sBufferRows; ++y)
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = (y == m_multipleInfo->coordCurrent.Y)? (m_multipleInfo->coordCurrent.X + 1) : 0;
		srBuffer.Top    = y;
		srBuffer.Right  = sBufferColumns - 1;
		srBuffer.Bottom = y;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		for (SHORT x = srBuffer.Left; x <= srBuffer.Right; ++x)
		{
			wchar_t c = pScreenBuffer[x - srBuffer.Left].Char.UnicodeChar;
			for (size_t d = 0; m_multipleInfo->u.select_word.szRightDelimiters[d]; ++d)
			{
				if( c == m_multipleInfo->u.select_word.szRightDelimiters[d] )
				{
					goto link;
				}
			}
			strLink += c;
		}
	}

link:

	// remove spaces on either end of a string
	boost::trim(strLink);

	// don't execute an empty string (= nothing found)
	// that launches explorer in the current working directory
	if( !strLink.empty() )
		ShellExecute(NULL, L"open", strLink.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI * _t_GetCurrentConsoleFontEx)(
	_In_ HANDLE hConsoleOutput,
	_In_ BOOL bMaximumWindow,
	_Out_ PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);

void ConsoleHandler::GetFontInfo()
{
	GET_STD_OUT_READ_ONLY

	::ZeroMemory(&m_multipleInfo->consoleFontInfo, sizeof(CONSOLE_FONT_INFOEX));
	m_multipleInfo->consoleFontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);

	_t_GetCurrentConsoleFontEx GetCurrentConsoleFontEx = (_t_GetCurrentConsoleFontEx)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetCurrentConsoleFontEx");

	if(GetCurrentConsoleFontEx)
	{
		GetCurrentConsoleFontEx(hStdOut, TRUE, &m_multipleInfo->consoleFontInfo);
		m_multipleInfo->coordFontSize = ::GetConsoleFontSize(hStdOut, m_multipleInfo->consoleFontInfo.nFont);
	}
	else
	{
		CONSOLE_FONT_INFO consoleFontInfo;
		::ZeroMemory(&consoleFontInfo, sizeof(CONSOLE_FONT_INFO));

		::GetCurrentConsoleFont(hStdOut, TRUE, &consoleFontInfo);
		m_multipleInfo->coordFontSize = ::GetConsoleFontSize(hStdOut, consoleFontInfo.nFont);
		m_multipleInfo->consoleFontInfo.nFont = consoleFontInfo.nFont;
		m_multipleInfo->consoleFontInfo.dwFontSize = consoleFontInfo.dwFontSize;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::Clear()
{
	GET_STD_OUT_READ_WRITE

	COORD coordScreen = { 0, 0 };    /* here's where we'll home the cursor */
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
	DWORD dwConSize;                 /* number of character cells in the current buffer */

	/* get the number of character cells in the current buffer */
	::GetConsoleScreenBufferInfo( hStdOut, &csbi );
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	/* fill the entire screen with blanks */
	::FillConsoleOutputCharacter( hStdOut, L' ', dwConSize, coordScreen, &cCharsWritten );

	/* get the current text attribute */
	::GetConsoleScreenBufferInfo( hStdOut, &csbi );

	/* now set the buffer's attributes accordingly */
	::FillConsoleOutputAttribute( hStdOut, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten );

	/* put the cursor at (0, 0) */
	::SetConsoleCursorPosition( hStdOut, coordScreen );
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SearchText()
{
	GET_STD_OUT_READ_ONLY

	SHORT                        sBufferColumns  = (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns) : static_cast<SHORT>(m_consoleParams->dwColumns);
	SHORT                        sBufferRows     = (m_consoleParams->dwBufferRows    > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferRows)    : static_cast<SHORT>(m_consoleParams->dwRows);
	COORD                        coordFrom       = {0, 0};
	COORD                        coordBufferSize = {sBufferColumns, 1};
	std::unique_ptr<CHAR_INFO[]> pScreenBuffer(new CHAR_INFO[sBufferColumns]);

	m_multipleInfo->coordLeft.X  = -1;
	m_multipleInfo->coordLeft.Y  = -1;
	m_multipleInfo->coordRight.X = -1;
	m_multipleInfo->coordRight.Y = -1;

	wchar_t text[MAX_SEARCH_TEXT + 2];
	int index          = 0;
	int len            = static_cast<int>(wcslen(m_multipleInfo->u.search_text.szText));
	int current_offset = static_cast<int>(m_multipleInfo->coordCurrent.Y) * static_cast<int>(sBufferColumns) + static_cast<int>(m_multipleInfo->coordCurrent.X);
	int offset         = 0;
	int row_offset     = 0;
	int last_offset    = INT_MIN;

	if( m_multipleInfo->u.search_text.bMatchWholeWord )
	{
		text[0] = 0;
		for(int i = 0, j = 1; i < len; ++i, ++j)
			text[j] = m_multipleInfo->u.search_text.bMatchCase? m_multipleInfo->u.search_text.szText[i] : towupper(m_multipleInfo->u.search_text.szText[i]);
		text[len + 1] = 0;

		index = 1;
		len += 2;
		current_offset --;
	}
	else
	{
		for(int i = 0; i < len; ++i)
			text[i] = m_multipleInfo->u.search_text.bMatchCase? m_multipleInfo->u.search_text.szText[i] : towupper(m_multipleInfo->u.search_text.szText[i]);
	}

	for (SHORT y = 0; y < sBufferRows - 1; ++y, row_offset += static_cast<int>(sBufferColumns))
	{
		SMALL_RECT srBuffer;

		srBuffer.Left   = 0;
		srBuffer.Top    = y;
		srBuffer.Right  = sBufferColumns - 1;
		srBuffer.Bottom = y;

		::ReadConsoleOutput(
			hStdOut,
			pScreenBuffer.get(),
			coordBufferSize,
			coordFrom,
			&srBuffer);

		int prev_index = index;

		for(int i = (-prev_index); i < static_cast<int>(sBufferColumns); ++i)
		{
			offset = row_offset + i;

			if( !m_multipleInfo->u.search_text.bNext && offset >= current_offset )
			{
				if( last_offset != INT_MIN )
				{
					if( m_multipleInfo->u.search_text.bMatchWholeWord )
					{
						last_offset ++;
						len -= 2;
					}
					m_multipleInfo->coordLeft.X  = static_cast<SHORT>(last_offset % static_cast<int>(sBufferColumns));
					m_multipleInfo->coordLeft.Y  = static_cast<SHORT>(last_offset / static_cast<int>(sBufferColumns));
					last_offset += (len - 1);
					m_multipleInfo->coordRight.X = static_cast<SHORT>(last_offset % static_cast<int>(sBufferColumns));
					m_multipleInfo->coordRight.Y = static_cast<SHORT>(last_offset / static_cast<int>(sBufferColumns));
				}
				return;
			}

			for(int j = (i + index); j < static_cast<int>(sBufferColumns); ++j)
			{
				wchar_t c = ( j < 0 )? text[j + prev_index] : pScreenBuffer[j].Char.UnicodeChar;

				if( text[index] == 0 )
				{
					if( !iswspace(c) && !iswpunct(c) )
					{
						index = 0;
						break;
					}
				}
				else
				{
					if( !m_multipleInfo->u.search_text.bMatchCase )
						c = towupper(c);

					if( c != text[index] )
					{
						index = 0;
						break;
					}
				}

				index++;

				if( index == len )
				{
					if( m_multipleInfo->u.search_text.bNext && offset > current_offset )
					{
						if( m_multipleInfo->u.search_text.bMatchWholeWord )
						{
							offset ++;
							len -= 2;
						}
						m_multipleInfo->coordLeft.X  = static_cast<SHORT>(offset % static_cast<int>(sBufferColumns));
						m_multipleInfo->coordLeft.Y  = static_cast<SHORT>(offset / static_cast<int>(sBufferColumns));
						offset += (len - 1);
						m_multipleInfo->coordRight.X = static_cast<SHORT>(offset % static_cast<int>(sBufferColumns));
						m_multipleInfo->coordRight.Y = static_cast<SHORT>(offset / static_cast<int>(sBufferColumns));
						return;
					}

					last_offset = offset;

					index = 0;
					break;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SendConsoleText(HANDLE hStdIn, const wchar_t*	pszText, size_t	textLen)
{
	const size_t partMaxLen    = min(512, textLen);
	size_t       keyEventCount = 0;
	DWORD        dwTextWritten;

	std::unique_ptr<INPUT_RECORD[]> pKeyEvents(new INPUT_RECORD[partMaxLen]);

	for(size_t i = 0; i < textLen; ++i)
	{
		if( keyEventCount == partMaxLen )
		{
			::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(keyEventCount), &dwTextWritten);

			keyEventCount = 0;
		}

		if( (pszText[i] == L'\r') || (pszText[i] == L'\n') )
		{
			if((pszText[i] == L'\r') && (i + 1) < textLen && (pszText[i + 1] == L'\n')) ++i;

			pKeyEvents[keyEventCount].EventType                        = KEY_EVENT;
			pKeyEvents[keyEventCount].Event.KeyEvent.bKeyDown          = TRUE;
			pKeyEvents[keyEventCount].Event.KeyEvent.wRepeatCount      = 1;
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualKeyCode   = VK_RETURN;
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualScanCode  = 0;
			pKeyEvents[keyEventCount].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
			pKeyEvents[keyEventCount].Event.KeyEvent.dwControlKeyState = 0;

			keyEventCount++;

			if( keyEventCount == partMaxLen )
			{
				::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(keyEventCount), &dwTextWritten);

				keyEventCount = 0;
			}

			pKeyEvents[keyEventCount].EventType                        = KEY_EVENT;
			pKeyEvents[keyEventCount].Event.KeyEvent.bKeyDown          = FALSE;
			pKeyEvents[keyEventCount].Event.KeyEvent.wRepeatCount      = 1;
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualKeyCode   = VK_RETURN;
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualScanCode  = 0;
			pKeyEvents[keyEventCount].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
			pKeyEvents[keyEventCount].Event.KeyEvent.dwControlKeyState = 0;

			keyEventCount++;
		}
		else
		{
			pKeyEvents[keyEventCount].EventType                        = KEY_EVENT;
			pKeyEvents[keyEventCount].Event.KeyEvent.bKeyDown          = TRUE;
			pKeyEvents[keyEventCount].Event.KeyEvent.wRepeatCount      = 1;
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualKeyCode   = LOBYTE(::VkKeyScan(pszText[i]));
			pKeyEvents[keyEventCount].Event.KeyEvent.wVirtualScanCode  = 0;
			pKeyEvents[keyEventCount].Event.KeyEvent.uChar.UnicodeChar = pszText[i];
			pKeyEvents[keyEventCount].Event.KeyEvent.dwControlKeyState = 0;

			keyEventCount++;
		}
	}

	if( keyEventCount > 0 )
		::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(keyEventCount), &dwTextWritten);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SendMouseEvent(HANDLE hStdIn)
{
	DWORD	dwEvents = 0;

	INPUT_RECORD mouseEvent;
	::ZeroMemory(&mouseEvent, sizeof(INPUT_RECORD));

	mouseEvent.EventType	= MOUSE_EVENT;

	::CopyMemory(&mouseEvent.Event.MouseEvent, m_consoleMouseEvent.Get(), sizeof(MOUSE_EVENT_RECORD));
	::WriteConsoleInput(hStdIn, &mouseEvent, 1, &dwEvents);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ScrollConsole(int nXDelta, int nYDelta)
{
	GET_STD_OUT_READ_WRITE

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	int nXCurrentPos = csbi.srWindow.Right - m_consoleParams->dwColumns + 1;
	int nYCurrentPos = csbi.srWindow.Bottom - m_consoleParams->dwRows + 1;

	// limit deltas
	nXDelta = max(-nXCurrentPos, min(nXDelta, (int)(m_consoleParams->dwBufferColumns-m_consoleParams->dwColumns) - nXCurrentPos));
	nYDelta = max(-nYCurrentPos, min(nYDelta, (int)(m_consoleParams->dwBufferRows-m_consoleParams->dwRows) - nYCurrentPos));

	SMALL_RECT sr;
	sr.Top		= static_cast<SHORT>(nYDelta);
	sr.Bottom	= static_cast<SHORT>(nYDelta);
	sr.Left		= static_cast<SHORT>(nXDelta);
	sr.Right	= static_cast<SHORT>(nXDelta);

	::SetConsoleWindowInfo(hStdOut, FALSE, &sr);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetConsoleParams()
{
	GET_STD_OUT_READ_WRITE

	// get max console size
	// /!\ GetLargestConsoleWindowSize requires a HANDLE with write access
	COORD coordMaxSize = ::GetLargestConsoleWindowSize(hStdOut);

	//if(coordMaxSize.X == 0 && coordMaxSize.Y == 0)
	//	::MessageBoxA(NULL, "GetLargestConsoleWindowSize", "error", MB_ICONERROR | MB_OK);

	m_consoleParams->dwMaxRows		= coordMaxSize.Y;
	m_consoleParams->dwMaxColumns	= coordMaxSize.X;

	// check rows and columns
	if (m_consoleParams->dwRows > static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwRows = coordMaxSize.Y;
	if (m_consoleParams->dwColumns > static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwColumns = coordMaxSize.X;

	// buffer rows cannot be less than max console size
//	if (m_consoleParams->dwBufferRows < static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwBufferRows = coordMaxSize.Y;
//	if (m_consoleParams->dwBufferColumns < static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwBufferColumns = coordMaxSize.X;

	if ((m_consoleParams->dwBufferRows != 0) && (m_consoleParams->dwMaxRows > m_consoleParams->dwBufferRows)) m_consoleParams->dwMaxRows = m_consoleParams->dwBufferRows;
	if ((m_consoleParams->dwBufferColumns != 0) && (m_consoleParams->dwMaxColumns > m_consoleParams->dwBufferColumns)) m_consoleParams->dwMaxColumns = m_consoleParams->dwBufferColumns;

	// set console window handle and hook monitor thread id
	m_consoleParams->hwndConsoleWindow = ::GetConsoleWindow();
	m_consoleParams->dwHookThreadId    = ::GetCurrentThreadId();

	TRACE(
		L"columns: %i, rows: %i\n",
		L"Max columns: %i, max rows: %i\n",
		m_consoleParams->dwRows, m_consoleParams->dwColumns,
		m_consoleParams->dwMaxColumns, m_consoleParams->dwMaxRows);

	// get initial window and cursor info
	::GetConsoleScreenBufferInfo(hStdOut, &m_consoleInfo->csbi);
	::GetConsoleCursorInfo(hStdOut, m_cursorInfo.Get());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ConsoleHandler::MonitorThreadStatic(LPVOID lpParameter)
{
	ConsoleHandler* pConsoleHandler = reinterpret_cast<ConsoleHandler*>(lpParameter);
	return pConsoleHandler->MonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::MonitorThread()
{
	TRACE(L"Hook!\n");

	// TODO: error handling
	// open shared objects (shared memory, events, etc)
	if (!OpenSharedObjects()) return 0;

	HANDLE hStdIn = ::CreateFile(
						L"CONIN$",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						0);

	// TODO: handle specfic font for DBCS code page
	//SetConsoleFont();
	SetConsoleParams();

	m_consoleParams.SetReqEvent();

	// if console window is null
	// then the current process in not a Win32 console application
	if( m_consoleParams->hwndConsoleWindow == nullptr ) return 0;

	if (::WaitForSingleObject(m_consoleParams.GetRespEvent(), 10000) == WAIT_TIMEOUT) return 0;

	ResizeConsoleWindow(m_consoleParams->dwColumns, m_consoleParams->dwRows, 0);

	std::shared_ptr<void> parentProcessWatchdog(::OpenMutex(SYNCHRONIZE, FALSE, (LPCTSTR)((SharedMemNames::formatWatchdog % m_consoleParams->dwParentProcessId).str().c_str())), ::CloseHandle);
	TRACE(L"Watchdog handle: 0x%08X\n", parentProcessWatchdog.get());

	NamedPipeMessage           npmsg;
	size_t                     npmsglen = 0;
	std::unique_ptr<wchar_t[]> text;
	m_consoleMsgPipe.BeginReadAsync(&npmsg, sizeof(NamedPipeMessage));

	HANDLE arrWaitHandles[] =
	{
		m_hMonitorThreadExit.get(),
		m_consoleCopyInfo.GetReqEvent(),
		m_newScrollPos.GetReqEvent(),
		m_consoleMouseEvent.GetReqEvent(),
		m_newConsoleSize.GetReqEvent(),
		m_multipleInfo.GetReqEvent(),
		m_consoleMsgPipe.Get(),
	};

	DWORD dwWaitRes = 0;
	auto timePoint1 = std::chrono::system_clock::now();

	while ((dwWaitRes = ::WaitForMultipleObjects(
							ARRAYSIZE(arrWaitHandles),
							arrWaitHandles,
							FALSE,
							/*m_consoleParams->dwRefreshInterval*/
							m_dwWaitingTime)) != WAIT_OBJECT_0)
	{
		if ((parentProcessWatchdog.get() != NULL) && (::WaitForSingleObject(parentProcessWatchdog.get(), 0) == WAIT_ABANDONED))
		{
			TRACE(L"Watchdog 0x%08X died. Time to exit", parentProcessWatchdog.get());
			::SendMessage(m_consoleParams->hwndConsoleWindow, WM_CLOSE, 0, 0);
			break;
		}

		switch (dwWaitRes)
		{
			// copy request
			case WAIT_OBJECT_0 + 1 :
			{
				SharedMemoryLock memLock(m_consoleCopyInfo);

				if( m_consoleCopyInfo->bClipboard )
				{
					CopyConsoleText();
				}
				else
				{
					GetSelectionPart();
				}
				m_consoleCopyInfo.SetRespEvent();
				break;
			}

			// console scroll request
			case WAIT_OBJECT_0 + 2 :
			{
				SharedMemoryLock memLock(m_newScrollPos);

				ScrollConsole(m_newScrollPos->cx, m_newScrollPos->cy);
				ReadConsoleBuffer();
				break;
			}

			// mouse event request
			case WAIT_OBJECT_0 + 3 :
			{
				SharedMemoryLock memLock(m_consoleMouseEvent);

				SendMouseEvent(hStdIn);
				m_consoleMouseEvent.SetRespEvent();
				break;
			}

			// console resize request
			case WAIT_OBJECT_0 + 4 :
			{
				SharedMemoryLock memLock(m_newConsoleSize);

				ResizeConsoleWindow(m_newConsoleSize->dwColumns, m_newConsoleSize->dwRows, m_newConsoleSize->dwResizeWindowEdge);
				ReadConsoleBuffer();
				break;
			}

			// multiple info
			case WAIT_OBJECT_0 + 5 :
			{
				//  current directory
				if( m_multipleInfo->fMask & MULTIPLEINFO_CURRENT_DIRECTORY )
				{
					m_multipleInfo->szCurrentDirectory[0] = 0;
					DWORD len = ::GetCurrentDirectory(_MAX_PATH, m_multipleInfo->szCurrentDirectory);
					if( len >= _MAX_PATH )
						// truncated
						m_multipleInfo->szCurrentDirectory[0] = 0;
				}

				//  process list
				if( m_multipleInfo->fMask & MULTIPLEINFO_PROCESS_LIST )
				{
					m_multipleInfo->dwProcessCount = ::GetConsoleProcessList(m_multipleInfo->lpdwProcessList, 256);
				}

				//  word selection
				if( m_multipleInfo->fMask & MULTIPLEINFO_SELECT_WORD )
				{
					SelectWord();
				}
				//  search
				else if( m_multipleInfo->fMask & MULTIPLEINFO_SEARCH_TEXT )
				{
					SearchText();
				}
				//  click link
				if( m_multipleInfo->fMask & MULTIPLEINFO_CLICK_LINK )
				{
					ClickLink();
				}
				//  font info
				if( m_multipleInfo->fMask & MULTIPLEINFO_FONT )
				{
					GetFontInfo();
				}
				//  code page
				if( m_multipleInfo->fMask & MULTIPLEINFO_CODE_PAGE )
				{
					m_multipleInfo->uiInputCP  = ::GetConsoleCP();
					m_multipleInfo->uiOutputCP = ::GetConsoleOutputCP();
				}

				::SetEvent(m_multipleInfo.GetRespEvent());
				break;
			}

			// pipe
			case WAIT_OBJECT_0 + 6 :
			{
				try
				{
					npmsglen += m_consoleMsgPipe.EndAsync();

					if( text.get() )
					{
						if( npmsglen == (npmsg.data.text.dwTextLen * sizeof(wchar_t)) )
						{
							text.get()[npmsg.data.text.dwTextLen] = 0;

							SendConsoleText(hStdIn, text.get(), npmsg.data.text.dwTextLen);
							ReadConsoleBuffer();

							text.reset();
							npmsglen = 0;
						}
					}
					else
					{
						if( npmsglen == sizeof(NamedPipeMessage) )
						{
							switch( npmsg.type )
							{
							case NamedPipeMessage::POSTMESSAGE:
								TRACE(
									L"NamedPipeMessage::POSTMESSAGE Msg = 0x%08lx WPARAM = %p LPARAM = %p\n",
									npmsg.data.winmsg.msg,
									npmsg.data.winmsg.wparam,
									npmsg.data.winmsg.lparam);

								if( !::PostMessage(
									m_consoleParams->hwndConsoleWindow,
									npmsg.data.winmsg.msg,
									npmsg.data.winmsg.wparam,
									npmsg.data.winmsg.lparam) )
								{
#ifdef _DEBUG
									Win32Exception err("PostMessage", ::GetLastError());
									TRACE(
										L"PostMessage Msg = 0x%08lx WPARAM = %p LPARAM = %p fails (reason: %S)\n",
										npmsg.data.winmsg.msg,
										npmsg.data.winmsg.wparam,
										npmsg.data.winmsg.lparam,
										err.what());
#endif
								}
								break;

							case NamedPipeMessage::SENDMESSAGE:
								TRACE(
									L"NamedPipeMessage::SENDMESSAGE Msg = 0x%08lx WPARAM = %p LPARAM = %p\n",
									npmsg.data.winmsg.msg,
									npmsg.data.winmsg.wparam,
									npmsg.data.winmsg.lparam);

#ifdef _DEBUG
								{
									LRESULT res = ::SendMessage(
										m_consoleParams->hwndConsoleWindow,
										npmsg.data.winmsg.msg,
										npmsg.data.winmsg.wparam,
										npmsg.data.winmsg.lparam);
									TRACE(
										L"SendMessage Msg = 0x%08lx WPARAM = %p LPARAM = %p returns %p (last error 0x%08lx)\n",
										npmsg.data.winmsg.msg,
										npmsg.data.winmsg.wparam,
										npmsg.data.winmsg.lparam,
										res,
										GetLastError());
								}
#else
								::SendMessage(
									m_consoleParams->hwndConsoleWindow,
									npmsg.data.winmsg.msg,
									npmsg.data.winmsg.wparam,
									npmsg.data.winmsg.lparam);
#endif
								break;

							case NamedPipeMessage::SHOWWINDOW:
								TRACE(
									L"NamedPipeMessage::SHOWWINDOW nCmdShow = %ld\n",
									npmsg.data.show.nCmdShow);

								::ShowWindow(
									m_consoleParams->hwndConsoleWindow,
									npmsg.data.show.nCmdShow);
								break;

							case NamedPipeMessage::SETWINDOWPOS:
								TRACE(
									L"NamedPipeMessage::SETWINDOWPOS X = %d Y = %d cx = %d cy = %d uFlags = 0x%08lx\n",
										npmsg.data.windowpos.X,
										npmsg.data.windowpos.Y,
										npmsg.data.windowpos.cx,
										npmsg.data.windowpos.cy,
										npmsg.data.windowpos.uFlags);

								::SetWindowPos(
									m_consoleParams->hwndConsoleWindow,
									NULL,
									npmsg.data.windowpos.X,
									npmsg.data.windowpos.Y,
									npmsg.data.windowpos.cx,
									npmsg.data.windowpos.cy,
									npmsg.data.windowpos.uFlags);
								break;

							case NamedPipeMessage::SENDTEXT:
								TRACE(
									L"NamedPipeMessage::SENDTEXT dwTextLen = %lu\n",
									npmsg.data.text.dwTextLen);

								text.reset(new wchar_t[npmsg.data.text.dwTextLen + 1]);
								break;

							case NamedPipeMessage::WRITECONSOLEINPUT:
								{
									INPUT_RECORD record;
									record.EventType = KEY_EVENT;
									record.Event.KeyEvent = npmsg.data.keyEvent;

									TRACE(
										L"NamedPipeMessage::WRITECONSOLEINPUT\n"
										L"  bKeyDown          = %s\n"
										L"  dwControlKeyState = 0x%08lx\n"
										L"  UnicodeChar       = 0x%04hx\n"
										L"  wRepeatCount      = %hu\n"
										L"  wVirtualKeyCode   = 0x%04hx\n"
										L"  wVirtualScanCode  = 0x%04hx\n",
										record.Event.KeyEvent.bKeyDown?L"TRUE":L"FALSE",
										record.Event.KeyEvent.dwControlKeyState,
										record.Event.KeyEvent.uChar.UnicodeChar,
										record.Event.KeyEvent.wRepeatCount,
										record.Event.KeyEvent.wVirtualKeyCode,
										record.Event.KeyEvent.wVirtualScanCode);

									DWORD dwTextWritten = 0;
									::WriteConsoleInput(hStdIn, &record, 1, &dwTextWritten);

									ReadConsoleBuffer();
								}
								break;

							case NamedPipeMessage::DETACH:
								TRACE(L"NamedPipeMessage::DETACH\n");
								return 0;

							case NamedPipeMessage::CLEAR:
								this->Clear();
								ReadConsoleBuffer();
								break;

							case NamedPipeMessage::CTRL_C:
								::GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
								ReadConsoleBuffer();
								break;

							case NamedPipeMessage::READCONSOLEBUFFER:
								ReadConsoleBuffer();
								break;
							}

							npmsglen = 0;
						}
					}

					if( text.get() )
						m_consoleMsgPipe.BeginReadAsync(reinterpret_cast<LPBYTE>(text.get()) + npmsglen, static_cast<size_t>(npmsg.data.text.dwTextLen) * sizeof(wchar_t) - npmsglen);
					else
						m_consoleMsgPipe.BeginReadAsync(reinterpret_cast<LPBYTE>(&npmsg) + npmsglen, sizeof(NamedPipeMessage) - npmsglen);
				}
				catch(std::exception&)
				{
					// receives ERROR_BROKEN_PIPE when the tab is closed
				}
				break;
			}

			case WAIT_TIMEOUT:
			{
				ForceReadConsoleBuffer();
				break;
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

