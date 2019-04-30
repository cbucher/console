#include "stdafx.h"
#include "resource.h"

#include <fstream>

#include "Console.h"
#include "ConsoleException.h"
#include "ConsoleView.h"
#include "MainFrame.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//CDC		ConsoleView::m_dcOffscreen(::CreateCompatibleDC(NULL));
//CDC		ConsoleView::m_dcText(::CreateCompatibleDC(NULL));

//CBitmap	ConsoleView::m_bmpOffscreen;
//CBitmap	ConsoleView::m_bmpText;

CFont ConsoleView::m_fontText[4];
DWORD ConsoleView::m_dwFontSize(0);
DWORD ConsoleView::m_dwFontZoom(100); // 100 %
DWORD ConsoleView::m_dwScreenDpi(96);

int ConsoleView::m_nCharHeight(0);
int ConsoleView::m_nCharWidth(0);
int ConsoleView::m_nVScrollWidth(0);
int ConsoleView::m_nHScrollWidth(0);
int ConsoleView::m_nVInsideBorder(0);
int ConsoleView::m_nHInsideBorder(0);

bool _boolMenuSysKeyCancelled = false;

//////////////////////////////////////////////////////////////////////////////

ConsoleView::ConsoleView(MainFrame& mainFrame, HWND hwndTabView, std::shared_ptr<TabData> tabDataTab, std::shared_ptr<TabData> tabDataShell, DWORD dwRows, DWORD dwColumns, const ConsoleOptions& consoleOptions)
: m_mainFrame(mainFrame)
, m_hwndTabView(hwndTabView)
, m_consoleOptions(consoleOptions)
, m_bInitializing(true)
, m_bResizing(false)
, m_bAppActive(true)
, m_bActive(true)
, m_bMouseTracking(false)
, m_bNeedFullRepaint(true) // first OnPaint will do a full repaint
, m_bBackgroundChanged(false)
, m_bConsoleWindowVisible(false)
, m_dwStartupRows(dwRows)
, m_dwStartupColumns(dwColumns)
, m_dwVScrollMax(0)
, m_nVWheelDelta(0)
, m_bShowVScroll(false)
, m_bShowHScroll(false)
, m_strUser()
, m_boolNetOnly(false)
, m_consoleHandler()
, m_screenBuffer()
, m_dwScreenRows(0)
, m_dwScreenColumns(0)
, m_consoleSettings(g_settingsHandler->GetConsoleSettings())
, m_appearanceSettings(g_settingsHandler->GetAppearanceSettings())
, m_hotkeys(g_settingsHandler->GetHotKeys())
, m_tabDataShell(tabDataShell.get() ? tabDataShell : tabDataTab)
, m_tabDataTab(m_appearanceSettings.stylesSettings.bKeepViewTheme ? m_tabDataShell : tabDataTab)
, m_background()
, m_backgroundBrush(NULL)
, m_cursor()
, m_cursorDBCS()
, m_selectionHandler()
, m_mouseCommand(MouseSettings::cmdNone)
, m_bFlashTimerRunning(false)
, m_dwFlashes(0)
, m_dcOffscreen(::CreateCompatibleDC(NULL))
, m_dcText(::CreateCompatibleDC(NULL))
, m_boolIsGrouped(false)
, m_boolImmComposition(false)
, m_bForwardMouseEvents(false)
#ifdef CONSOLEZ_CHRONOS
, m_timePoint1(std::chrono::high_resolution_clock::now())
#endif // CONSOLEZ_CHRONOS
, m_startTime(std::chrono::system_clock::now())
{
	m_coordSearchText.X = -1;
	m_coordSearchText.Y = -1;
}

ConsoleView::~ConsoleView()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	DragAcceptFiles(TRUE);

	// set console delegates
	m_consoleHandler.SetupDelegates(
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleChange),
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleClose));

	SetBackground();

	CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
	ConsoleViewCreate* consoleViewCreate = reinterpret_cast<ConsoleViewCreate*>(createStruct->lpCreateParams);

	if( consoleViewCreate->type == ConsoleViewCreate::CREATE )
	{
		ConsoleOptions consoleOptions;

		// startup directory choice (by descending order of priority):
		// 1 - ConsoleZ command line startup tab dir (-d)
		// 2 - Tab setting
		// 3 - Settings global initial dir
		// 4 - ConsoleZ command line working dir (-cwd)
		if (m_consoleOptions.strInitialDir.length() > 0)
		{
			consoleOptions.strInitialDir = m_consoleOptions.strInitialDir;
		}
		else if (m_tabDataShell->strInitialDir.length() > 0)
		{
			consoleOptions.strInitialDir = m_tabDataShell->strInitialDir;
		}
		else if (m_consoleSettings.strInitialDir.length() > 0)
		{
			consoleOptions.strInitialDir = m_consoleSettings.strInitialDir;
		}
		else
		{
			consoleOptions.strInitialDir = m_consoleOptions.strWorkingDir;
		}

		wstring	strShell(m_consoleSettings.strShell);

		if (m_tabDataShell->strShell.length() > 0)
		{
			strShell	= m_tabDataShell->strShell;
		}

		UserCredentials* userCredentials = consoleViewCreate->u.userCredentials;

		consoleOptions.strTitle = m_tabDataShell->strTitle;
		consoleOptions.strShellArguments = m_consoleOptions.strShellArguments;
		consoleOptions.strEnvironment = m_consoleOptions.strEnvironment;
		consoleOptions.dwBasePriority = m_consoleOptions.dwBasePriority == ULONG_MAX ? m_tabDataShell->dwBasePriority : m_consoleOptions.dwBasePriority;

		try
		{
			m_consoleHandler.StartShellProcess(
				consoleOptions,
				strShell,
				*userCredentials,
				m_tabDataShell->environmentVariables,
				m_dwStartupRows,
				m_dwStartupColumns);

			m_strUser = userCredentials->user.c_str();
			m_boolNetOnly = userCredentials->netOnly;
		}
		catch (const ConsoleException& ex)
		{
			m_exceptionMessage = ex.GetMessage();
			return -1;
		}
	}
	else
	{
		try
		{
			m_consoleHandler.AttachToShellProcess(
				consoleViewCreate->u.dwProcessId,
				m_dwStartupRows,
				m_dwStartupColumns);
		}
		catch (const ConsoleException& ex)
		{
			m_exceptionMessage = ex.GetMessage();
			return -1;
		}
	}

	m_bInitializing = false;

	// set current language in the console window
	m_consoleHandler.PostMessage(
		WM_INPUTLANGCHANGEREQUEST,
		0,
		reinterpret_cast<LPARAM>(::GetKeyboardLayout(0)));

	// scrollbar stuff
	InitializeScrollbars();

/*
	// create font
	RecreateFont(g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize, false);
*/

	// create offscreen buffers
	CreateOffscreenBuffers();

	// TODO: put this in console size change handler
	m_dwScreenRows    = m_consoleHandler.GetConsoleParams()->dwRows;
	m_dwScreenColumns = m_consoleHandler.GetConsoleParams()->dwColumns;
	m_screenBuffer.reset(new CharInfo[m_dwScreenRows * m_dwScreenColumns]);
	m_dxWidths.reset(new INT[m_dwScreenColumns]);
	m_dxLigatureWidths.reset(new INT[m_dwScreenColumns]);
	m_orders.reset(new UINT[m_dwScreenColumns]);
	m_glyphs.reset(new wchar_t[m_dwScreenColumns]);

	m_consoleHandler.StartMonitorThread();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_bFlashTimerRunning) KillTimer(FLASH_TAB_TIMER);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// /!\ The WM_PAINT message is sent to your window's WinProc() whenever the window's client area needs repainting.
	// We must call BeginPaint and EndPaint even though the view is inactive
	// otherwise we create a loop!
	CPaintDC	dc(m_hWnd);
	if (!m_bActive) return 0;

	if (m_bNeedFullRepaint)
	{
		// we need to update offscreen buffers here for first paint and relative backgrounds
		RepaintText(m_dcText);
		UpdateOffscreen(dc.m_ps.rcPaint);
		m_bNeedFullRepaint = false;
	}

	// this is the best way I know how to detect if the window is being 
	// repainted while sizing
	// the flag is set in MainFrame::OnSizing and MainFrame::OnExitSizeMove
	if (m_bResizing)
	{
		dc.FillRect(&dc.m_ps.rcPaint, m_backgroundBrush);
	}

	dc.BitBlt(
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
		dc.m_ps.rcPaint.right, 
		dc.m_ps.rcPaint.bottom,
		m_dcOffscreen, 
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
		SRCCOPY);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnPrintClient(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDCHandle dc(reinterpret_cast<HDC>(wParam));
	CRect rect;
	this->GetClientRect(&rect);

	dc.BitBlt(
		rect.left,
		rect.top,
		rect.right,
		rect.bottom,
		m_dcOffscreen,
		0,
		0,
		SRCCOPY);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
long l1 = 0;
LRESULT ConsoleView::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);

  if (!(pWinPos->flags & SWP_NOSIZE))
  {
    TRACE(L"!!! ConsoleView::OnSize (%d) (%i,%i) [%i, %i] !!!\n", ::InterlockedIncrement(&l1), pWinPos->x, pWinPos->y, pWinPos->cx, pWinPos->cy);
  }

	// showing the view, repaint
	if (pWinPos->flags & SWP_SHOWWINDOW) Repaint(false);

	// force full repaint for relative backgrounds
	if (m_tabDataTab->imageData.bRelative && !(pWinPos->flags & SWP_NOMOVE)) m_bNeedFullRepaint = true;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

/* MSDN mentions that you should use the last virtual key code received
  * when putting a virtual key identity to a WM_CHAR message since multiple
  * or translated keys may be involved. */
WORD wLastVirtualKey = 0;

#define CTRL_BUT_NOT_ALT(n) \
        (((n) & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && \
        !((n) & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))

LRESULT ConsoleView::OnConsoleFwdMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (((uMsg == WM_KEYDOWN) || (uMsg == WM_KEYUP)) && (wParam == VK_PACKET)) return 0;

	if (!TranslateKeyDown(uMsg, wParam, lParam))
	{
		TRACE_KEY(L"ConsoleView::OnConsoleFwdMsg Msg: 0x%04X, wParam: 0x%08X, lParam: 0x%08X\n", uMsg, wParam, lParam);

		bool boolPostMessage = false;

		if( uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST )
		{
			KEY_EVENT_RECORD keyEvent;

			keyEvent.bKeyDown          = (lParam & (1<<31)) == 0;
			keyEvent.wRepeatCount      = static_cast<WORD>(lParam & 0xffff);
			keyEvent.wVirtualScanCode  = static_cast<WORD>((lParam >> 16) & 0xff);

			BYTE lpKeyState[256] = { 0 };
			GetKeyboardState(lpKeyState);

			if( uMsg == WM_CHAR || uMsg == WM_SYSCHAR )
			{
				if( m_boolImmComposition && !keyEvent.bKeyDown )
					return 0;
				keyEvent.wVirtualKeyCode = wLastVirtualKey;
				keyEvent.uChar.UnicodeChar = static_cast<WCHAR>(wParam);
			}
			else
			{
				if( m_boolImmComposition || wParam == VK_PROCESSKEY )
					return 0;
				if( uMsg == WM_DEADCHAR || uMsg == WM_SYSDEADCHAR )
					keyEvent.wVirtualKeyCode = wLastVirtualKey;
				else
					keyEvent.wVirtualKeyCode = static_cast<WORD>(wParam);
				keyEvent.uChar.UnicodeChar = 0x0000;
			}

			keyEvent.dwControlKeyState = 0;

			if (lpKeyState[VK_CAPITAL] & 1)
				keyEvent.dwControlKeyState |= CAPSLOCK_ON;

			if (lpKeyState[VK_NUMLOCK] & 1)
				keyEvent.dwControlKeyState |= NUMLOCK_ON;

			if (lpKeyState[VK_SCROLL] & 1)
				keyEvent.dwControlKeyState |= SCROLLLOCK_ON;

			if (lpKeyState[VK_SHIFT] & 0x80)
				keyEvent.dwControlKeyState |= SHIFT_PRESSED;

			if (lpKeyState[VK_LCONTROL] & 0x80)
				keyEvent.dwControlKeyState |= LEFT_CTRL_PRESSED;
			if (lpKeyState[VK_RCONTROL] & 0x80)
				keyEvent.dwControlKeyState |= RIGHT_CTRL_PRESSED;

			if (lpKeyState[VK_LMENU] & 0x80)
				keyEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
			if (lpKeyState[VK_RMENU] & 0x80)
				keyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;

			if( (lParam >> 24) & 0x1 )
				keyEvent.dwControlKeyState |= ENHANCED_KEY;

			if( CTRL_BUT_NOT_ALT(keyEvent.dwControlKeyState) && keyEvent.bKeyDown )
			{
				// in line input mode
				// console handles these keys without generating key events

				// ctrl-C
				if( keyEvent.wVirtualKeyCode == 'C' )
				{
					uMsg = WM_KEYDOWN;
					wParam = 'C';
					lParam = 0x002E0001;
					boolPostMessage = true;
				}
				// ctrl-break
				if( keyEvent.wVirtualKeyCode == VK_CANCEL )
				{
					uMsg = WM_KEYDOWN;
					wParam = VK_CANCEL;
					lParam = 0x01460001;
					boolPostMessage = true;
				}
			}

			if( !boolPostMessage )
			{
				TRACE_KEY(
					L"-> WriteConsoleInput\n"
					L"  bKeyDown          = %s\n"
					L"  dwControlKeyState = 0x%08lx\n"
					L"  UnicodeChar       = 0x%04hx\n"
					L"  wRepeatCount      = %hu\n"
					L"  wVirtualKeyCode   = 0x%04hx\n"
					L"  wVirtualScanCode  = 0x%04hx\n",
					keyEvent.bKeyDown? L"TRUE" : L"FALSE",
					keyEvent.dwControlKeyState,
					keyEvent.uChar.UnicodeChar,
					keyEvent.wRepeatCount,
					keyEvent.wVirtualKeyCode,
					keyEvent.wVirtualScanCode);

				if( this->IsGrouped() )
					m_mainFrame.WriteConsoleInputToConsoles(&keyEvent);
				else
					m_consoleHandler.WriteConsoleInput(&keyEvent);
			}
		}
		else
		{
			boolPostMessage = true;
		}

		if( boolPostMessage )
		{
			if( this->IsGrouped() )
				m_mainFrame.PostMessageToConsoles(uMsg, wParam, lParam);
			else
				m_consoleHandler.PostMessage(uMsg, wParam, lParam);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnSelectionKeyPressed(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	auto state = m_selectionHandler->GetState();

	MutexLock bufferLock(m_consoleHandler.m_bufferMutex);

	COORD coordMaxBuffer =
	{
		static_cast<SHORT>(m_consoleHandler.GetConsoleParams()->dwMaxColumns - 1),
		static_cast<SHORT>(m_consoleHandler.GetConsoleParams()->dwMaxRows - 1)
	};

	if( state == SelectionHandler::selstateNoSelection )
	{
		// start selection from the cursor position
		COORD coordCursorPosition = m_consoleHandler.GetConsoleInfo()->csbi.dwCursorPosition;
		SelectionType seltype = (wID < ID_COLUMN_SELECTION_LEFT_KEY) ? seltypeText : seltypeColumn;

		m_selectionHandler->StartSelection(coordCursorPosition, m_screenBuffer.get(), seltype, false);
	}

	// calculate the shifting
	COORD coordCurrentPosition = m_selectionHandler->GetCurrentPosition();

	// regardless the selection type
	switch( (wID < ID_COLUMN_SELECTION_LEFT_KEY) ? wID : (wID - ID_COLUMN_SELECTION_LEFT_KEY + ID_TEXT_SELECTION_LEFT_KEY) )
	{
	case ID_TEXT_SELECTION_LEFT_KEY:
		if( coordCurrentPosition.X == 0 )
		{
			if( coordCurrentPosition.Y > 0 )
			{
				coordCurrentPosition.X = coordMaxBuffer.X;
				coordCurrentPosition.Y--;
			}
		}
		else
		{
			coordCurrentPosition.X--;
		}
		break;

	case ID_TEXT_SELECTION_RIGHT_KEY:
		if( coordCurrentPosition.X == coordMaxBuffer.X )
		{
			if( coordCurrentPosition.Y < coordMaxBuffer.Y )
			{
				coordCurrentPosition.X = 0;
				coordCurrentPosition.Y++;
			}
		}
		else
		{
			coordCurrentPosition.X++;
		}
		break;

	case ID_TEXT_SELECTION_TOP_KEY:
		if( coordCurrentPosition.Y > 0 )
		{
			coordCurrentPosition.Y--;
		}
		break;

	case ID_TEXT_SELECTION_BOTTOM_KEY:
		if( coordCurrentPosition.Y < coordMaxBuffer.Y )
		{
			coordCurrentPosition.Y++;
		}
		break;

	case ID_TEXT_SELECTION_HOME_KEY:
		coordCurrentPosition.X = 0;
		break;

	case ID_TEXT_SELECTION_END_KEY:
		coordCurrentPosition.X = coordMaxBuffer.X;
		break;

	case ID_TEXT_SELECTION_PAGEUP_KEY:
		coordCurrentPosition.Y -= static_cast<SHORT>(m_dwScreenRows);
		if( coordCurrentPosition.Y < 0 ) coordCurrentPosition.Y = 0;
		break;

	case ID_TEXT_SELECTION_PAGEDOWN_KEY:
		coordCurrentPosition.Y += static_cast<SHORT>(m_dwScreenRows);
		if( coordCurrentPosition.Y > coordMaxBuffer.Y ) coordCurrentPosition.Y = coordMaxBuffer.Y;
		break;
	}

	// update selection
	m_selectionHandler->UpdateSelection(coordCurrentPosition, m_screenBuffer.get());

	// end selection / stop mouse capture
	m_selectionHandler->EndSelection();

	BitBltOffscreen();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  UINT uKeys        = GET_KEYSTATE_WPARAM(wParam);
  int  nWheelDelta  = GET_WHEEL_DELTA_WPARAM(wParam);
  int  nScrollDelta = m_nVWheelDelta + nWheelDelta;

  m_nVWheelDelta    = nScrollDelta % WHEEL_DELTA;
  nScrollDelta      = nScrollDelta / WHEEL_DELTA;

  if (nScrollDelta != 0)
  {
    if (uKeys & MK_CONTROL)
    {
      // recreate font with new size
      if (RecreateFont(m_dwFontSize + nScrollDelta, true, m_dwScreenDpi))
      {
        // only if the new size is different (to avoid flickering at extremes)
        m_mainFrame.AdjustWindowSize(ADJUSTSIZE_FONT);
      }
    }
    else
    {
			if( m_bForwardMouseEvents )
			{
				COORD coord = { 0, 0 };

				DWORD dwMouseButtonState = 0;
				DWORD dwControlKeyState = 0;
				DWORD dwEventFlags = 0;

				// get mouse button states
				if( uKeys & MK_LBUTTON ) dwMouseButtonState |= FROM_LEFT_1ST_BUTTON_PRESSED;
				if( uKeys & MK_MBUTTON ) dwMouseButtonState |= FROM_LEFT_2ND_BUTTON_PRESSED;
				if( uKeys & MK_RBUTTON ) dwMouseButtonState |= RIGHTMOST_BUTTON_PRESSED;
				if( uKeys & MK_XBUTTON1 ) dwMouseButtonState |= FROM_LEFT_3RD_BUTTON_PRESSED;
				if( uKeys & MK_XBUTTON2 ) dwMouseButtonState |= FROM_LEFT_4TH_BUTTON_PRESSED;

				// get control key states
				if( GetKeyState(VK_RMENU) < 0 ) dwControlKeyState |= RIGHT_ALT_PRESSED;
				if( GetKeyState(VK_LMENU) < 0 ) dwControlKeyState |= LEFT_ALT_PRESSED;
				if( GetKeyState(VK_RCONTROL) < 0 ) dwControlKeyState |= RIGHT_CTRL_PRESSED;
				if( GetKeyState(VK_LCONTROL) < 0 ) dwControlKeyState |= LEFT_CTRL_PRESSED;
				if( GetKeyState(VK_CAPITAL) < 0 ) dwControlKeyState |= CAPSLOCK_ON;
				if( GetKeyState(VK_NUMLOCK) < 0 ) dwControlKeyState |= NUMLOCK_ON;
				if( GetKeyState(VK_SCROLL) < 0 ) dwControlKeyState |= SCROLLLOCK_ON;
				if( GetKeyState(VK_SHIFT) < 0 ) dwControlKeyState |= SHIFT_PRESSED;

				dwEventFlags = MOUSE_WHEELED;

				m_consoleHandler.SendMouseEvent(coord, MAKELONG(dwMouseButtonState, GET_WHEEL_DELTA_WPARAM(wParam)), dwControlKeyState, dwEventFlags);
			}
			else
			{
				if( uKeys & MK_SHIFT )
				{
					// scroll pages
					ScrollSettings& scrollSettings = g_settingsHandler->GetBehaviorSettings().scrollSettings;
					if( scrollSettings.dwPageScrollRows > 0 )
					{
						// modified behavior: pagescroll = x lines
						nScrollDelta *= static_cast<int>(scrollSettings.dwPageScrollRows);
					}
					else
					{
						nScrollDelta *= static_cast<int>(m_consoleHandler.GetConsoleParams()->dwRows);
					}
				}
				else
				{
					// scroll lines
					UINT uScrollAmount = 3;
					if( !SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScrollAmount, 0) )
						uScrollAmount = 3;
					nScrollDelta *= static_cast<int>(uScrollAmount);
				}
				DoScroll(SB_VERT, SB_WHEEL, nScrollDelta);
			}
    }
  }

  return 0;
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoScroll(SB_VERT, LOWORD(wParam), HIWORD(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoScroll(SB_HORZ, LOWORD(wParam), HIWORD(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	_boolMenuSysKeyCancelled = true;

	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if( m_bForwardMouseEvents )
	{
		ForwardMouseClick(uMsg, wParam, point);

		return 0;
	}

	UINT						uKeys			= GET_KEYSTATE_WPARAM(wParam); 
	UINT						uXButton		= GET_XBUTTON_WPARAM(wParam);

	MouseSettings&				mouseSettings	= g_settingsHandler->GetMouseSettings();
	MouseSettings::Action		mouseAction;

	// get modifiers
	if (uKeys & MK_CONTROL)			mouseAction.modifiers |= MouseSettings::mkCtrl;
	if (uKeys & MK_SHIFT)			mouseAction.modifiers |= MouseSettings::mkShift;
	if (GetKeyState(VK_MENU) < 0)	mouseAction.modifiers |= MouseSettings::mkAlt;

	// get mouse button
	switch (uMsg)
	{
		case WM_LBUTTONDOWN :
		case WM_LBUTTONUP :
		case WM_LBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnLeft;
			break;

		case WM_RBUTTONDOWN :
		case WM_RBUTTONUP :
		case WM_RBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnRight;
			break;

		case WM_MBUTTONDOWN :
		case WM_MBUTTONUP :
		case WM_MBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnMiddle;
			break;

		case WM_XBUTTONDOWN :
		case WM_XBUTTONUP :
		case WM_XBUTTONDBLCLK :
			if (uXButton == XBUTTON1)
			{
				mouseAction.button = MouseSettings::btn4th;
			}
			else
			{
				mouseAction.button = MouseSettings::btn5th;
			}
			break;
	}

	// get click type
	switch (uMsg)
	{
		case WM_LBUTTONDOWN :
		case WM_RBUTTONDOWN :
		case WM_MBUTTONDOWN :
		case WM_XBUTTONDOWN :
			mouseAction.clickType = MouseSettings::clickSingle;
			break;

		case WM_LBUTTONDBLCLK :
		case WM_RBUTTONDBLCLK :
		case WM_MBUTTONDBLCLK :
		case WM_XBUTTONDBLCLK :
			mouseAction.clickType = MouseSettings::clickDouble;
			break;
	}

	if (m_mouseCommand == MouseSettings::cmdNone)
	{
		// mouse button down

		// no current mouse action
		auto it = mouseSettings.commands.get<MouseSettings::commandID>().end();

		// copy command
		if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected)
		{
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdCopy);

			if ((*it)->action == mouseAction)
			{
				m_mouseCommand = MouseSettings::cmdCopy;
				return 0;
			}
		}

		// select command
		it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdSelect);
		if ((*it)->action == mouseAction)
		{
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));

			MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
			m_selectionHandler->StartSelection(GetConsoleCoord(point, true), m_screenBuffer.get(), seltypeText, true);

			m_mouseCommand = MouseSettings::cmdSelect;
			return 0;
		}

		// select word
		if (MouseSettings::clickDouble == mouseAction.clickType)
		{
			MouseSettings::Action mouseActionCopy = mouseAction;
			mouseActionCopy.clickType = MouseSettings::clickSingle;
			if ((*it)->action == mouseActionCopy)
			{
				MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
				m_selectionHandler->SelectWord(GetConsoleCoord(point));

				m_mouseCommand = MouseSettings::cmdSelect;
				return 0;
			}
		}

		// select column command
		it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdColumnSelect);
		if ((*it)->action == mouseAction)
		{
			::SetCursor(::LoadCursor(NULL, IDC_CROSS));

			MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
			m_selectionHandler->StartSelection(GetConsoleCoord(point, true), m_screenBuffer.get(), seltypeColumn, true);

			m_mouseCommand = MouseSettings::cmdColumnSelect;
			return 0;
		}

		// paste command
		it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdPaste);
		if ((*it)->action == mouseAction)
		{
			m_mouseCommand = MouseSettings::cmdPaste;
			return 0;
		}

		// drag command
		it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdDrag);
		if ((*it)->action == mouseAction)
		{
			CPoint clientPoint(point);

			ClientToScreen(&clientPoint);
			m_mainFrame.PostMessage(UM_START_MOUSE_DRAG, MAKEWPARAM(uKeys, uXButton), MAKELPARAM(clientPoint.x, clientPoint.y));

			// we don't set active command here, main frame handles mouse drag
			return 0;
		}

		// click link
		it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdLink);
		if ((*it)->action == mouseAction)
		{
			MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
			m_consoleHandler.ClickLink(GetConsoleCoord(point));

			return 0;
		}

		// menu commands
		std::array<MouseSettings::Command, 4> menuCommands = {
			MouseSettings::cmdMenu1,
			MouseSettings::cmdMenu2,
			MouseSettings::cmdMenu3,
			MouseSettings::cmdSnippets};

		for(auto i = menuCommands.begin(); i != menuCommands.end(); ++i)
		{
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(*i);
			if ((*it)->action == mouseAction)
			{
				m_mouseCommand = *i;
				return 0;
			}
		}
	}
	else
	{
		// mouse button up

		// we have an active command, handle it...
		switch (m_mouseCommand)
		{
			case MouseSettings::cmdCopy :
			{
				Copy(&point);
				break;
			}

			case MouseSettings::cmdSelect :
			case MouseSettings::cmdColumnSelect :
			{
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));

				if (m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting)
				{
					m_selectionHandler->EndSelection();
					m_selectionHandler->ClearSelection();
				}
				else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelecting ||
						m_selectionHandler->GetState() == SelectionHandler::selstateSelectWord)
				{
					m_selectionHandler->EndSelection();

					// copy on select
					if (g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bCopyOnSelect)
					{
						Copy(NULL);
					}
				}

				break;
			}

			case MouseSettings::cmdPaste :
			{
				m_mainFrame.PasteToConsoles();
				break;
			}

			case MouseSettings::cmdMenu1 :
			case MouseSettings::cmdMenu2 :
			case MouseSettings::cmdMenu3 :
			case MouseSettings::cmdSnippets :
			{
				CPoint	screenPoint(point);
				ClientToScreen(&screenPoint);
				m_mainFrame.SendMessage(UM_SHOW_POPUP_MENU, static_cast<WPARAM>(m_mouseCommand), MAKELPARAM(screenPoint.x, screenPoint.y));
				break;
			}
		}

		m_mouseCommand = MouseSettings::cmdNone;
		return 0;
	}

	ForwardMouseClick(uMsg, wParam, point);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT	uFlags = static_cast<UINT>(wParam);
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if (m_mouseCommand == MouseSettings::cmdSelect
	    ||
	    m_mouseCommand == MouseSettings::cmdColumnSelect)
	{
		CRect	rectClient;
		GetClientRect(&rectClient);

		if (point.x < rectClient.left + m_nVInsideBorder)
		{
			DoScroll(SB_HORZ, SB_LINELEFT, 0);
		}
		else if (point.x > rectClient.right - m_nVInsideBorder)
		{
			DoScroll(SB_HORZ, SB_LINERIGHT, 0);
		}
		
		if (point.y < rectClient.top + m_nHInsideBorder)
		{
			DoScroll(SB_VERT, SB_LINEUP, 0);
		}
		else if (point.y > rectClient.bottom - m_nHInsideBorder)
		{
			DoScroll(SB_VERT, SB_LINEDOWN, 0);
		}

		{
			MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
			m_selectionHandler->UpdateSelection(GetConsoleCoord(point), m_screenBuffer.get());
		}

		BitBltOffscreen();

		m_mainFrame.PostMessage(UM_UPDATE_STATUS_BAR, 0, 0);
	}
	else if ((m_mouseCommand == MouseSettings::cmdNone) && 
			 ((uFlags & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) != 0))
	{
		ForwardMouseClick(uMsg, wParam, point);
	}
	else
	{
		if( !m_bMouseTracking )
		{
			TRACKMOUSEEVENT params;
			params.cbSize = sizeof(TRACKMOUSEEVENT);
			params.dwFlags = TME_LEAVE;
			params.hwndTrack = m_hWnd;
			params.dwHoverTime = HOVER_DEFAULT;
			if( ::TrackMouseEvent(&params) )
			{
				TRACE(L"onhover %p\n", m_hWnd);
				if( g_settingsHandler->GetBehaviorSettings2().focusSettings.bFollowMouse )
					m_mainFrame.SetActiveConsole(m_hwndTabView, m_hWnd);
				m_bMouseTracking = true;
			}
		}

		bHandled = FALSE;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	TRACE(L"onleave %p\n", m_hWnd);
	m_bMouseTracking = false;

	return 0;
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// discards mouse message when activating the window using mouse
	LRESULT ret = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);

	HWND hwndTopLevel	= ::GetForegroundWindow();
	HWND hwndParent		= NULL;

	do
	{
		hwndParent = ::GetParent(hwndTopLevel);
		if (hwndParent != NULL) hwndTopLevel = hwndParent;
	}
	while (hwndParent != NULL);

	// if we're not active, discard the mouse message
	if (hwndTopLevel != m_mainFrame.m_hWnd)
	{
		if (ret == MA_ACTIVATE)
		{
			ret = MA_ACTIVATEANDEAT;
		}
		else if (ret == MA_NOACTIVATE)
		{
			ret = MA_NOACTIVATEANDEAT;
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == FLASH_TAB_TIMER)
	{
		// if we got activated, kill timer
		if (m_bActive)
		{
			KillTimer(FLASH_TAB_TIMER);
			m_bFlashTimerRunning = false;
			return 0;
		}

		m_mainFrame.HighlightTab(m_hwndTabView, (m_dwFlashes % 2) == 0);
		if (++m_dwFlashes == g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.dwFlashes * 2)
		{
			if (g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.bStayHighlighted)
			{
				m_mainFrame.HighlightTab(m_hwndTabView, true);
			}

			KillTimer(FLASH_TAB_TIMER);
			m_bFlashTimerRunning = false;
		}

		return 0;
	}

	if (!m_bActive) return 0;

	if (wParam == CURSOR_TIMER)
	{
		if (m_cursor.get())
		{
			m_cursor->PrepareNext();
			m_cursor->Draw(m_bAppActive, m_consoleHandler.GetCursorInfo()->dwSize);
		}

		if (m_cursorDBCS.get())
		{
			m_cursorDBCS->PrepareNext();
			m_cursorDBCS->Draw(m_bAppActive, m_consoleHandler.GetCursorInfo()->dwSize);
		}

		BitBltOffscreen(true);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnInputLangChangeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_consoleHandler.PostMessage(uMsg, wParam, lParam);
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnInputLangChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_consoleHandler.PostMessage(WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_SYSCHARSET, lParam);
	m_consoleHandler.PostMessage(uMsg, wParam, lParam);
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDROP	hDrop = reinterpret_cast<HDROP>(wParam);
	UINT	uFilesCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	CString	strFilenames;

	// concatenate all filenames
	for (UINT i = 0; i < uFilesCount; ++i)
	{
		CString	strFilename;
		::DragQueryFile(hDrop, i, strFilename.GetBuffer(MAX_PATH), MAX_PATH);
		strFilename.ReleaseBuffer();

		// put quotes around the filename
		strFilename = CString(L"\"") + strFilename + CString("\"");

		if (i > 0) strFilenames += L" ";
		strFilenames += strFilename;

	}
	::DragFinish(hDrop);

	if( this->IsGrouped() )
		m_mainFrame.SendTextToConsoles(strFilenames);
	else
		m_consoleHandler.SendTextToConsole(strFilenames);

	m_mainFrame.SetActiveConsole(m_hwndTabView, m_hWnd);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnUpdateConsoleView(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_bInitializing) return 0;

#ifdef CONSOLEZ_CHRONOS
	auto now1 = std::chrono::high_resolution_clock::now();
#endif // CONSOLEZ_CHRONOS

	bool bResize      = (wParam & UPDATE_CONSOLE_RESIZE       ) ? true : false;
	bool textChanged  = (wParam & UPDATE_CONSOLE_TEXT_CHANGED ) ? true : false;
	bool csbiChanged  = (wParam & UPDATE_CONSOLE_CSBI_CHANGED ) ? true : false;

	if(wParam & UPDATE_CONSOLE_TITLE_CHANGED || wParam & UPDATE_CONSOLE_PROGRESS_CHANGED)
		UpdateTitle();

	if(!bResize && !textChanged && !csbiChanged)
		return 0;

	// console size changed, resize offscreen buffers
	if (bResize)
	{
/*
		TRACE(L"================================================================\n");
		TRACE(L"Resizing console wnd: 0x%08X\n", m_hWnd);
*/
		InitializeScrollbars();

		// notify parent about resize
		m_mainFrame.SendMessage(UM_CONSOLE_RESIZED, 0, 0);
	}

	SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();

	m_dwVScrollMax = max(m_dwVScrollMax, static_cast<DWORD>(consoleInfo->csbi.srWindow.Bottom));
	TRACE(L"OnUpdateConsoleView m_dwVScrollMax=%lu\n", m_dwVScrollMax);

	if (m_bShowVScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask  = SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
		si.nPos   = consoleInfo->csbi.srWindow.Top;
		si.nMax   = m_dwVScrollMax;
		si.nMin   = 0;
		SetScrollInfo(SB_VERT, &si, TRUE);

		//TRACE(L"----------------------------------------------------------------\n");
		//TRACE(L"VScroll pos: %i\n", consoleInfo->csbi.srWindow.Top);
	}

	if (m_bShowHScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS | SIF_DISABLENOSCROLL;
		si.nPos   = consoleInfo->csbi.srWindow.Left; 
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}

	// if the view is not visible, don't repaint
	if (!m_bActive)
	{
		if
		(
			textChanged &&
			!bResize &&
			(g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.dwFlashes > 0) &&
			(!m_bFlashTimerRunning)
		)
		{
			// ignore tab flashing if console view age is less than 3 seconds
			auto age = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_startTime).count();
			if( age > 3LL )
			{
				m_dwFlashes = 0;
				m_bFlashTimerRunning = true;
				SetTimer(FLASH_TAB_TIMER, 500);
			}
		}

		return 0;
	}

	if ((m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting) ||
		(m_selectionHandler->GetState() == SelectionHandler::selstateSelecting))
	{
		CPoint	point;
		::GetCursorPos(&point);
		ScreenToClient(&point);

		MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
		m_selectionHandler->UpdateSelection(GetConsoleCoord(point), m_screenBuffer.get());
	}
	else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected)
	{
		m_selectionHandler->UpdateSelection();
	}

	Repaint(false);

#ifdef CONSOLEZ_CHRONOS
	auto now2 = std::chrono::high_resolution_clock::now();

	TRACE_PERF(
		L"thd %lu cpu3=%lld ns\n",
		::GetCurrentThreadId(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count());
#endif // CONSOLEZ_CHRONOS

	return 0;
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetRect(CRect& clientRect)
{
  RECT rect;
  GetWindowRect(&rect);

  int width  = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  clientRect.left   = 0;
  clientRect.top    = 0;
  clientRect.right  = m_consoleHandler.GetConsoleParams()->dwColumns * m_nCharWidth  + 2 * m_nVInsideBorder;
  clientRect.bottom = m_consoleHandler.GetConsoleParams()->dwRows    * m_nCharHeight + 2 * m_nHInsideBorder;

  if (m_bShowVScroll) clientRect.right  += m_nVScrollWidth;
  if (m_bShowHScroll) clientRect.bottom += m_nHScrollWidth;

  if(  width > clientRect.right  ) clientRect.right  = width;
  if( height > clientRect.bottom ) clientRect.bottom = height;

  //TRACE(L"========ConsoleView::GetRect=====================================\n");
  //TRACE(L"wind: %ix%i - %ix%i\n", rect.left, rect.top, rect.right, rect.bottom);
  //TRACE(L"rect: %ix%i - %ix%i\n", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}

void ConsoleView::GetRectMax(CRect& clientMaxRect)
{
  clientMaxRect.left   = 0;
  clientMaxRect.top    = 0;
  clientMaxRect.right  = (m_consoleHandler.GetConsoleParams()->dwColumns + 1) * m_nCharWidth  + 2 * m_nVInsideBorder;
  clientMaxRect.bottom = (m_consoleHandler.GetConsoleParams()->dwRows    + 1) * m_nCharHeight + 2 * m_nHInsideBorder;

  if (m_bShowVScroll) clientMaxRect.right  += m_nVScrollWidth;
  if (m_bShowHScroll) clientMaxRect.bottom += m_nHScrollWidth;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//long l2 = 0;
void ConsoleView::AdjustRectAndResize(ADJUSTSIZE as, CRect& clientRect, DWORD dwResizeWindowEdge)
{
  GetWindowRect(&clientRect);

  //TRACE(L"========AdjustRectAndResize (%d)=================================\n", ::InterlockedIncrement(&l2));
  //TRACE(L"rect: %ix%i - %ix%i\n", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

  LONG width  = clientRect.right  - clientRect.left;
  LONG height = clientRect.bottom - clientRect.top;

  // exclude scrollbars from row/col calculation
  if (m_bShowVScroll) width  -= m_nVScrollWidth;
  if (m_bShowHScroll) height -= m_nHScrollWidth;

  // exclude inside borders from row/col calculation
  width  -= (m_nVInsideBorder * 2);
  height -= (m_nHInsideBorder * 2);

  //TRACE(L"exclude scrollbars and inside borders from row/col calculation\n");
  //TRACE(L"width: %i height: %i\n", width, height);

  DWORD dwColumns = width  / m_nCharWidth;
  DWORD dwRows    = height / m_nCharHeight;

  //TRACE(L"m_nCharWidth: %i m_nCharHeight: %i\n", m_nCharWidth, m_nCharHeight);

  DWORD dwMaxColumns = this->m_consoleHandler.GetConsoleParams()->dwMaxColumns;
  DWORD dwMaxRows    = this->m_consoleHandler.GetConsoleParams()->dwMaxRows;

  //TRACE(L"dwMaxColumns: %i dwMaxRows: %i\n", dwMaxColumns, dwMaxRows);

  if( dwColumns > dwMaxColumns )
    dwColumns = dwMaxColumns;
  if( dwRows > dwMaxRows )
    dwRows = dwMaxRows;

  //TRACE(L"dwColumns: %i dwRows: %i\n", dwColumns, dwRows);

  clientRect.right  = clientRect.left + dwColumns * m_nCharWidth  + m_nVInsideBorder * 2;
  clientRect.bottom = clientRect.top +  dwRows    * m_nCharHeight + m_nHInsideBorder * 2;

  // adjust for scrollbars
  if (m_bShowVScroll) clientRect.right  += m_nVScrollWidth;
  if (m_bShowHScroll) clientRect.bottom += m_nHScrollWidth;

  SharedMemory<ConsoleSize>& newConsoleSize = m_consoleHandler.GetNewConsoleSize();
  SharedMemoryLock memLock(newConsoleSize);

  newConsoleSize->dwColumns          = dwColumns;
  newConsoleSize->dwRows             = dwRows;
  newConsoleSize->dwResizeWindowEdge = dwResizeWindowEdge;

  //TRACE(L"console view: 0x%08X, adjusted: %ix%i\n", m_hWnd, dwRows, dwColumns);
  //TRACE(L"================================================================\n");

  RecreateOffscreenBuffers(as);
  Repaint(true);

  m_consoleHandler.GetNewConsoleSize().SetReqEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetConsoleWindowVisible(bool bVisible)
{
	m_bConsoleWindowVisible = bVisible;

	if (bVisible)
	{
		CPoint point;
		::GetCursorPos(&point);
		m_consoleHandler.SetWindowPos(point.x, point.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	}

	m_consoleHandler.ShowWindow(bVisible ? SW_SHOW : SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetAppActiveStatus(bool bAppActive)
{
	m_bAppActive = bAppActive;
	if (m_cursor.get()) m_cursor->Draw(m_bAppActive, m_consoleHandler.GetCursorInfo()->dwSize);
	if (m_cursorDBCS.get()) m_cursorDBCS->Draw(m_bAppActive, m_consoleHandler.GetCursorInfo()->dwSize);
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::RecreateFont(DWORD dwNewFontSize, bool boolZooming, DWORD dwScreenDpi)
{
	// calculate new font size in the [5, 36] interval
	DWORD size = dwNewFontSize;
	if( !boolZooming )
		size = ::MulDiv(dwNewFontSize, m_dwFontZoom, 100);
	size = min(36, size);
	size = max(5, size);
	DWORD zoom = ( m_dwFontSize == 0 )? 100 : ::MulDiv(size, 100, g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize);

	TRACE(L"size %lu->%lu / zoom %lu->%lu\n", m_dwFontSize, size, m_dwFontZoom, zoom);

	if (boolZooming && m_dwFontSize == size && m_dwScreenDpi == dwScreenDpi) return false;

	// adjust the font size
	m_dwFontSize = size;
	m_dwFontZoom = zoom;
	m_dwScreenDpi = dwScreenDpi;

	for( CFont& font : m_fontText )
		if( !font.IsNull() ) font.DeleteObject();

	if (!CreateFont(g_settingsHandler->GetAppearanceSettings().fontSettings.strName))
	{
		CreateFont(wstring(L"Courier New"));
	}

	return true;
}

void ConsoleView::RecreateOffscreenBuffers(ADJUSTSIZE as)
{
  if (!m_backgroundBrush.IsNull())m_backgroundBrush.DeleteObject();
  if( as == ADJUSTSIZE_WINDOW )
  {
    if (!m_bmpOffscreen.IsNull())	m_bmpOffscreen.DeleteObject();
    if (!m_bmpText.IsNull())		m_bmpText.DeleteObject();
  }
  CreateOffscreenBuffers();
  m_bNeedFullRepaint = true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Repaint(bool bFullRepaint)
{
#ifdef CONSOLEZ_CHRONOS
	long long i64cpu4 = 0LL;
	long long i64cpu5 = 0LL;
	long long i64cpu6 = 0LL;

	auto now1 = std::chrono::high_resolution_clock::now();
	auto now2 = now1;
#endif // CONSOLEZ_CHRONOS


	// OnPaint will do the work for a full repaint
	if (!m_bNeedFullRepaint)
	{
		// not a forced full text repaint, check text difference
		if (!bFullRepaint) bFullRepaint = (GetBufferDifference() > 15);

#ifdef CONSOLEZ_CHRONOS
		now2 = std::chrono::high_resolution_clock::now();
		i64cpu4 = std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count();
#endif // CONSOLEZ_CHRONOS

		// repaint text layer
 		if (bFullRepaint)
 		{
			RepaintText(m_dcText);
		}
		else
		{
			RepaintTextChanges(m_dcText);
		}

#ifdef CONSOLEZ_CHRONOS
		now1 = std::chrono::high_resolution_clock::now();
		i64cpu5 = std::chrono::duration_cast<std::chrono::nanoseconds>(now1 - now2).count();
#endif // CONSOLEZ_CHRONOS
	}

	BitBltOffscreen();

#ifdef CONSOLEZ_CHRONOS
	now2 = std::chrono::high_resolution_clock::now();
	i64cpu6 = std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count();

	TRACE_PERF(
		L"thd %lu cpu4=%lld ns cpu5=%lld (%s) cpu6=%lld\n",
		::GetCurrentThreadId(),
		i64cpu4,
		i64cpu5,
		bFullRepaint ? L"full" : L"partial",
		i64cpu6);
#endif // CONSOLEZ_CHRONOS
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::MainframeMoving()
{
	// next OnPaint will do a full repaint
	if (m_tabDataTab->imageData.bRelative)
	{
		m_bNeedFullRepaint = true;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetParentTab(HWND hwndTabView, std::shared_ptr<TabData> tabDataTab)
{
	SetParent(hwndTabView);
	m_hwndTabView = hwndTabView;

	m_tabDataTab =
		m_appearanceSettings.stylesSettings.bKeepViewTheme ?
			m_tabDataShell :
			tabDataTab;

	SetBackground();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetResizing(bool bResizing)
{
	m_bResizing = bResizing;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!m_bActive) return;

	Repaint(true);
	UpdateTitle();
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

static CString _strDefaultTitle(DEFAULT_CONSOLE_COMMAND);

CString ConsoleView::GetConsoleCommand()
{
	CWindow consoleWnd(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow);
	CString strConsoleTitle(L"");

	consoleWnd.GetWindowText(strConsoleTitle);

	int len = 0;

	/*if( this->GetConsoleHandler().IsElevated() )*/
	{
		if( m_strUACPrefix.IsEmpty() )
		{
			m_strUACPrefix = Helpers::GetUACPrefix().c_str();
		}

		if( strConsoleTitle.GetLength() >= m_strUACPrefix.GetLength()
		    &&
		    wcsncmp(strConsoleTitle.GetString(), m_strUACPrefix.GetString(), m_strUACPrefix.GetLength()) == 0 )
		{
			len = m_strUACPrefix.GetLength();
		}
	}

	if( (strConsoleTitle.GetLength() - len) >= _strDefaultTitle.GetLength()
	    &&
	    wcsncmp(strConsoleTitle.GetString() + len, _strDefaultTitle.GetString(), _strDefaultTitle.GetLength()) == 0 )
	{
		len += _strDefaultTitle.GetLength();

		if( wcsncmp(strConsoleTitle.GetString() + len, L" - ", 3) == 0 )
			len += 3;
	}

	if( len > 0 )
		strConsoleTitle = strConsoleTitle.Mid(len);

	return strConsoleTitle.Trim();
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetProgress(unsigned long long & ullProgressCompleted, unsigned long long & ullProgressTotal)
{
	auto consoleInfo = m_consoleHandler.GetConsoleInfo();
	SharedMemoryLock consoleInfoLock(consoleInfo);

	ullProgressCompleted = consoleInfo->ullProgressCompleted;
	ullProgressTotal     = consoleInfo->ullProgressTotal;
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Clear()
{
	//clear screen
	m_consoleHandler.Clear();

	// reset vertical scroll to zero
	m_dwVScrollMax = 0;
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Copy(const CPoint* pPoint /* = NULL */)
{
	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected))
	{
		return;
	}

	bool bCopied = false;

	if (!g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bSensitiveCopy)
	{
		pPoint = 0;
	}

	if (pPoint != NULL)
	{
		bCopied = m_selectionHandler->CopySelection(GetConsoleCoord(*pPoint));
	}
	else
	{
		// called by mainframe
		m_selectionHandler->CopySelection();
		bCopied = true;
	}

	if (!bCopied || g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bClearOnCopy) m_selectionHandler->ClearSelection();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SelectAll()
{
	m_selectionHandler->SelectAll();

	// copy on select
	if (g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bCopyOnSelect)
	{
		Copy(NULL);
	}

	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::ClearSelection()
{
	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected))
	{
		return;
	}

	m_selectionHandler->ClearSelection();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::PasteSelection()
{
	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected))
	{
		return;
	}

	std::wstring sel = m_selectionHandler->GetSelection();
	m_selectionHandler->ClearSelection();
	BitBltOffscreen();

	if( this->IsGrouped() )
		m_mainFrame.SendTextToConsoles(sel.c_str());
	else
		m_consoleHandler.SendTextToConsole(sel.c_str());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::DumpBuffer()
{
	wofstream of;
	of.open(Helpers::ExpandEnvironmentStrings(_T("%temp%\\console.dump")).c_str());
	DWORD       dwOffset = 0;
	MutexLock	bufferLock(m_consoleHandler.m_bufferMutex);

	for (DWORD i = 0; i < m_dwScreenRows; ++i)
	{
		for (DWORD j = 0; j < m_dwScreenColumns; ++j)
		{
			of << m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
			++dwOffset;
		}

		of << endl;
	}

	of.close();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleChange(bool bResize)
{
#ifdef CONSOLEZ_CHRONOS
	auto now1 = std::chrono::high_resolution_clock::now();
#endif // CONSOLEZ_CHRONOS

	WPARAM wParam = 0;

	{
		SharedMemory<ConsoleParams>& consoleParams = m_consoleHandler.GetConsoleParams();
		SharedMemory<ConsoleInfo>&   consoleInfo = m_consoleHandler.GetConsoleInfo();
		SharedMemory<CHAR_INFO>&     consoleBuffer = m_consoleHandler.GetConsoleBuffer();

		SharedMemoryLock             consoleInfoLock(consoleInfo);
		SharedMemoryLock             sharedBufferLock(consoleBuffer);
		MutexLock                    localBufferLock(m_consoleHandler.m_bufferMutex);

		// console size changed, resize local buffer
		if(bResize)
		{
			m_dwScreenRows = consoleParams->dwRows;
			m_dwScreenColumns = consoleParams->dwColumns;
			m_screenBuffer.reset(new CharInfo[m_dwScreenRows * m_dwScreenColumns]);
			m_dxWidths.reset(new INT[m_dwScreenColumns]);
			m_dxLigatureWidths.reset(new INT[m_dwScreenColumns]);
			m_orders.reset(new UINT[m_dwScreenColumns]);
			m_glyphs.reset(new wchar_t[m_dwScreenColumns]);
		}
		DWORD dwBufferSize = m_dwScreenRows * m_dwScreenColumns;

		// copy changed data
		for(DWORD dwOffset = 0; dwOffset < dwBufferSize; ++dwOffset)
		{
			m_screenBuffer[dwOffset].copy(consoleBuffer.Get() + dwOffset);
		}

		if(bResize) wParam |= UPDATE_CONSOLE_RESIZE;

		if(consoleInfo->textChanged)
		{
			wParam |= UPDATE_CONSOLE_TEXT_CHANGED;
			consoleInfo->textChanged = false;
		}

		if(consoleInfo->titleChanged)
		{
			wParam |= UPDATE_CONSOLE_TITLE_CHANGED;
			consoleInfo->titleChanged = false;
		}

		if(consoleInfo->csbiChanged)
		{
			wParam |= UPDATE_CONSOLE_CSBI_CHANGED;
			consoleInfo->csbiChanged = false;
		}

		if(consoleInfo->progressChanged)
		{
			wParam |= UPDATE_CONSOLE_PROGRESS_CHANGED;
			consoleInfo->progressChanged = false;
		}
	}

#ifdef CONSOLEZ_CHRONOS
	auto now2 = std::chrono::high_resolution_clock::now();
#endif // CONSOLEZ_CHRONOS

	SendMessage(UM_UPDATE_CONSOLE_VIEW, wParam);

#ifdef CONSOLEZ_CHRONOS
	auto now3 = std::chrono::high_resolution_clock::now();

	TRACE_PERF(
		L"thd %lu delta=%lld ns cpu1=%lld ns cpu2=%lld ns\n",
		::GetCurrentThreadId(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now1 - m_timePoint1).count(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now3 - now2).count());

	m_timePoint1 = now3;
#endif // CONSOLEZ_CHRONOS
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleClose()
{
	if (::IsWindow(m_hWnd)) m_mainFrame.PostMessage(UM_CONSOLE_CLOSED, reinterpret_cast<WPARAM>(m_hWnd), 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetBackground()
{
	// load background image
	switch( m_tabDataTab->backgroundImageType )
	{
	case bktypeImage:
		m_background = g_imageHandler->GetImage(m_tabDataTab->imageData);
		break;

	case bktypeDesktop:
		m_background = g_imageHandler->GetDesktopImage(m_tabDataTab->imageData);
		break;

	case bktypeBing:
		m_background = g_imageHandler->GetBingImage(m_tabDataTab->imageData);
		break;
	}

	if (!m_background) m_tabDataTab->backgroundImageType = bktypeNone;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBuffers()
{
	CWindowDC	dcWindow(m_hWnd);
	CRect		rectWindowMax;

	// get window rect based on font and console size
	GetRect(rectWindowMax);

	// create offscreen bitmaps if needed
	if (m_bmpOffscreen.IsNull()) CreateOffscreenBitmap(m_dcOffscreen, rectWindowMax, m_bmpOffscreen);
	if (m_bmpText.IsNull()) CreateOffscreenBitmap(m_dcText, rectWindowMax, m_bmpText);
	m_dcText.SelectFont(m_fontText[FontTextNormal]);

	// create background brush
	m_backgroundBrush.CreateSolidBrush(m_tabDataTab->crBackgroundColor);

	// initial offscreen paint
	m_dcOffscreen.FillRect(&rectWindowMax, m_backgroundBrush);

	// set text DC stuff
	m_dcText.SetBkMode(OPAQUE);
	m_dcText.FillRect(&rectWindowMax, m_backgroundBrush);

	// create selection handler
	m_selectionHandler.reset(new SelectionHandler(
									m_hWnd, 
#ifndef _USE_AERO
									dcWindow, 
									rectWindowMax, 
#endif //_USE_AERO
									m_consoleHandler,
									m_consoleHandler.GetConsoleParams(), 
									m_consoleHandler.GetConsoleInfo(), 
									m_consoleHandler.GetCopyInfo(),
									m_nCharWidth,
									m_nCharHeight,
									m_nVInsideBorder,
									m_nHInsideBorder,
									m_tabDataTab));

	// create and initialize cursor
	CRect		rectCursor(0, 0, m_nCharWidth, m_nCharHeight);

	m_cursor.reset();
	m_cursorDBCS.reset();

	m_cursor = CursorFactory::CreateCursor(
								m_hWnd,
								m_bAppActive,
								m_tabDataTab.get() ? static_cast<CursorStyle>(m_tabDataTab->dwCursorStyle) : cstyleXTerm,
								dcWindow,
								rectCursor,
								m_tabDataTab.get() ? m_tabDataTab->crCursorColor : RGB(255, 255, 255),
								this,
								true);

	rectCursor.right += m_nCharWidth;
	m_cursorDBCS = CursorFactory::CreateCursor(
								m_hWnd,
								m_bAppActive,
								m_tabDataTab.get() ? static_cast<CursorStyle>(m_tabDataTab->dwCursorStyle) : cstyleXTerm,
								dcWindow,
								rectCursor,
								m_tabDataTab.get() ? m_tabDataTab->crCursorColor : RGB(255, 255, 255),
								this,
								false);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBitmap(CDC& cdc, const CRect& rect, CBitmap& bitmap)
{
	if (!bitmap.IsNull()) return;// bitmap.DeleteObject();

	Helpers::CreateBitmap(cdc, rect.Width(), rect.Height(), bitmap);
	cdc.SelectBitmap(bitmap);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::CreateFont(const wstring& strFontName)
{
	for( CFont& font : m_fontText )
		if( !font.IsNull() ) return true;

  CDC dcText(::CreateCompatibleDC(NULL));

	BYTE	byFontQuality;

	FontSettings& fontSettings = g_settingsHandler->GetAppearanceSettings().fontSettings;

	switch (fontSettings.fontSmoothing)
	{
		case fontSmoothDefault:          byFontQuality = DEFAULT_QUALITY;           break;
		case fontSmoothNone:             byFontQuality = NONANTIALIASED_QUALITY;    break;
		case fontSmoothCleartype:        byFontQuality = CLEARTYPE_QUALITY;         break;
		case fontSmoothCleartypeNatural: byFontQuality = CLEARTYPE_NATURAL_QUALITY; break;
		case fontSmoothAntialiased:      byFontQuality = ANTIALIASED_QUALITY;       break;
		default :                        byFontQuality = DEFAULT_QUALITY;           break;
	}

	bool bBold   = fontSettings.bBold;
	bool bItalic = fontSettings.bItalic;

	m_fontText[FontTextNormal].CreateFont(
		-::MulDiv(m_dwFontSize, m_dwScreenDpi, 72),
		0,
		0,
		0,
		bBold ? FW_BOLD : 0,
		bItalic,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		byFontQuality,
		DEFAULT_PITCH,
		strFontName.c_str());

	m_fontText[FontTextUnderline].CreateFont(
		-::MulDiv(m_dwFontSize, m_dwScreenDpi, 72),
		0,
		0,
		0,
		bBold ? FW_BOLD : 0,
		bItalic,
		TRUE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		byFontQuality,
		DEFAULT_PITCH,
		strFontName.c_str());

	if( fontSettings.bBoldIntensified )
		bBold = !bBold;
	if( fontSettings.bItalicIntensified )
		bItalic = !bItalic;

	m_fontText[FontTextBright].CreateFont(
		-::MulDiv(m_dwFontSize, m_dwScreenDpi, 72),
		0,
		0,
		0,
		bBold ? FW_BOLD : 0,
		bItalic,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		byFontQuality,
		DEFAULT_PITCH,
		strFontName.c_str());

	m_fontText[FontTextBrightUnderline].CreateFont(
		-::MulDiv(m_dwFontSize, m_dwScreenDpi, 72),
		0,
		0,
		0,
		bBold ? FW_BOLD : 0,
		bItalic,
		TRUE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		byFontQuality,
		DEFAULT_PITCH,
		strFontName.c_str());

	TEXTMETRIC	textMetric;

	dcText.SelectFont(m_fontText[FontTextNormal]);
	if( !dcText.GetTextMetrics(&textMetric) ||
		  (textMetric.tmPitchAndFamily & TMPF_FIXED_PITCH) ) // fixed pitch font (TMPF_FIXED_PITCH is cleared!!!)
	{
		TRACE(L"/!\\ can't use %s font\n", strFontName.c_str());
		for( CFont& font : m_fontText )
			if( !font.IsNull() ) font.DeleteObject();
		return false;
	}

	DWORD dwExtraWidth = ::MulDiv(fontSettings.dwExtraWidth, m_dwFontZoom, 100);

	m_nCharWidth  = textMetric.tmAveCharWidth + dwExtraWidth;
	m_nCharHeight = textMetric.tmHeight;

	m_nVScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	m_nHScrollWidth = ::GetSystemMetrics(SM_CXHSCROLL);

	m_nVInsideBorder = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwInsideBorder;
	m_nHInsideBorder = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwInsideBorder;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::InitializeScrollbars()
{
	SharedMemory<ConsoleParams>& consoleParams = m_consoleHandler.GetConsoleParams();

	m_bShowVScroll = m_appearanceSettings.controlsSettings.ShowScrollbars() && (consoleParams->dwBufferRows > consoleParams->dwRows);
	m_bShowHScroll = m_appearanceSettings.controlsSettings.ShowScrollbars() && (consoleParams->dwBufferColumns > consoleParams->dwColumns);

	ShowScrollBar(SB_VERT, m_bShowVScroll);
	ShowScrollBar(SB_HORZ, m_bShowHScroll);

/*
	TRACE(L"InitializeScrollbars, console wnd: 0x%08X\n", m_hWnd);
	TRACE(L"Sizes: %i, %i    %i, %i\n", consoleParams->dwRows, consoleParams->dwBufferRows - 1, consoleParams->dwColumns, consoleParams->dwBufferColumns - 1);
	TRACE(L"----------------------------------------------------------------\n");
*/

	if (m_appearanceSettings.controlsSettings.ShowScrollbars() && (consoleParams->dwBufferRows > consoleParams->dwRows))
	{
		m_dwVScrollMax = max(m_dwVScrollMax, consoleParams->dwRows - 1);

		// set vertical scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO);
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
		si.nPage	= consoleParams->dwRows;
		si.nMax		= m_dwVScrollMax; /*consoleParams->dwBufferRows - 1*/
		si.nMin		= 0 ;
		si.nPos		= m_consoleHandler.GetConsoleInfo()->csbi.srWindow.Top;

		SetScrollInfo(SB_VERT, &si, TRUE);
	}

	if (m_appearanceSettings.controlsSettings.ShowScrollbars() && (consoleParams->dwBufferColumns > consoleParams->dwColumns))
	{
		// set horizontal scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO) ;
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
		si.nPage	= consoleParams->dwColumns;
		si.nMax		= consoleParams->dwBufferColumns - 1;
		si.nMin		= 0 ;
		si.nPos		= m_consoleHandler.GetConsoleInfo()->csbi.srWindow.Left;

		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::DoScroll(int nType, int nScrollCode, int nThumbPos)
{
	int nDelta = 0;
	int nCurrentPos = (nType == SB_VERT) ?
		m_consoleHandler.GetConsoleInfo()->csbi.srWindow.Top :
		m_consoleHandler.GetConsoleInfo()->csbi.srWindow.Left;

	ScrollSettings& scrollSettings = g_settingsHandler->GetBehaviorSettings().scrollSettings;

	switch(nScrollCode)
	{
		case SB_PAGEUP: /* SB_PAGELEFT */

			if (scrollSettings.dwPageScrollRows > 0)
			{
				nDelta = -static_cast<int>(scrollSettings.dwPageScrollRows);
			}
			else
			{
				nDelta = (nType == SB_VERT) ? -static_cast<int>(m_consoleHandler.GetConsoleParams()->dwRows) : -static_cast<int>(m_consoleHandler.GetConsoleParams()->dwColumns);
			}
			break;

		case SB_PAGEDOWN: /* SB_PAGERIGHT */
			if (scrollSettings.dwPageScrollRows > 0)
			{
				nDelta = static_cast<int>(scrollSettings.dwPageScrollRows);
			}
			else
			{
				nDelta = (nType == SB_VERT) ? static_cast<int>(m_consoleHandler.GetConsoleParams()->dwRows) : static_cast<int>(m_consoleHandler.GetConsoleParams()->dwColumns);
			}
			break;

		case SB_LINEUP: /* SB_LINELEFT */
			nDelta = -1;
			break;

		case SB_LINEDOWN: /* SB_LINERIGHT */
			nDelta = 1;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			nDelta = nThumbPos - nCurrentPos;
			break;

		case SB_WHEEL:
			nDelta = -nThumbPos;
			break;

		case SB_ENDSCROLL:
			return;

		default:
			return;
	}

	if( nType == SB_VERT )
	{
		int nVScrollMaxTop = static_cast<int>(m_dwVScrollMax - m_consoleHandler.GetConsoleParams()->dwRows + 1);
		if( (nCurrentPos + nDelta) > nVScrollMaxTop )
			nDelta = nVScrollMaxTop - nCurrentPos;
	}

	if (nDelta != 0)
	{
		SharedMemory<SIZE>& newScrollPos = m_consoleHandler.GetNewScrollPos();

		if (nType == SB_VERT)
		{
			newScrollPos->cx = 0;
			newScrollPos->cy = nDelta;
		}
		else
		{
			newScrollPos->cx = nDelta;
			newScrollPos->cy = 0;
		}

		newScrollPos.SetReqEvent();
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::SearchText(CString& text, bool bNext)
{
	SMALL_RECT& window = m_consoleHandler.GetConsoleInfo()->csbi.srWindow;
	COORD&      buffer = m_consoleHandler.GetConsoleInfo()->csbi.dwSize;

	if( m_coordSearchText.X == -1 && m_coordSearchText.Y == -1 )
	{
		if( bNext )
		{
			// we search from the top/left current window
			m_coordSearchText.X = window.Left - 1;
			m_coordSearchText.Y = window.Top;
		}
		else
		{
			// we search from the bottom/right current window
			m_coordSearchText.X = window.Right + 1;
			m_coordSearchText.Y = window.Bottom;
		}
	}

	COORD left, right;
	m_consoleHandler.SearchText(text, bNext, m_coordSearchText, left, right);

	TRACE(L"searching from %hdx%hd returns %hdx%hd - %hdx%hd\n",
		m_coordSearchText.X, m_coordSearchText.Y,
		left.X, left.Y,
		right.X, right.Y);

	if( left.X != -1 && left.Y != -1 && right.X != -1 && right.Y != -1 )
	{
		LONG cx = 0;
		LONG cy = 0;

		if( left.X < window.Left || right.X > window.Right )
		{
			cx = left.X - window.Left;
			if( (window.Right + cx) > buffer.X )
				cx -= (buffer.X - window.Right);
		}

		if( left.Y < window.Top || right.Y > window.Bottom )
		{
			cy = left.Y - window.Top;
			if( (window.Bottom + cy) > buffer.Y )
				cy -= (buffer.Y - window.Bottom);
		}

		if( cx || cy )
		{
			SharedMemory<SIZE>& newScrollPos = m_consoleHandler.GetNewScrollPos();
			newScrollPos->cx = cx;
			newScrollPos->cy = cy;
			newScrollPos.SetReqEvent();
		}
	}

	m_selectionHandler->SetHighlightCoordinates(left, right);
	BitBltOffscreen();

	m_coordSearchText = left;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

DWORD ConsoleView::GetBufferDifference()
{
	MutexLock	bufferLock(m_consoleHandler.m_bufferMutex);
	DWORD		dwCount				= m_dwScreenRows * m_dwScreenColumns;
	DWORD		dwChangedPositions	= 0;

	for (DWORD i = 0; i < dwCount; ++i)
	{
		if (m_screenBuffer[i].changed) ++dwChangedPositions;
	}

	return dwChangedPositions*100/dwCount;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::UpdateTitle()
{
	m_mainFrame.PostMessage(
					UM_UPDATE_TITLES,
					reinterpret_cast<WPARAM>(m_hwndTabView),
					0);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintText(CDC& dc)
{
#ifdef CONSOLEZ_CHRONOS
	long long i64cpu7 = 0LL;
	long long i64cpu8 = 0LL;
	long long i64cpu9 = 0LL;

	auto now1 = std::chrono::high_resolution_clock::now();
	auto now2 = now1;
#endif // CONSOLEZ_CHRONOS


	SIZE	bitmapSize;
	CRect	bitmapRect;

	dc.GetCurrentBitmap().GetSize(bitmapSize);
	bitmapRect.left = 0;
	bitmapRect.top = 0;
	bitmapRect.right = bitmapSize.cx;
	bitmapRect.bottom = bitmapSize.cy;

#ifdef _USE_AERO
	TransparencySettings2& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings.Settings();
#endif //_USE_AERO

	if(m_tabDataTab->backgroundImageType == bktypeNone)
	{
#ifdef _USE_AERO
		// set transparency
		if(transparencySettings.transType == transGlass)
		{
			Gdiplus::Graphics gr(dc);

			COLORREF backgroundColor = m_tabDataTab->crBackgroundColor;

			gr.Clear(
				Gdiplus::Color(
				this->m_mainFrame.GetAppActiveStatus() ? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha,
				GetRValue(backgroundColor),
				GetGValue(backgroundColor),
				GetBValue(backgroundColor)));
		}
		else
#endif // _USE_AERO
		{
			dc.FillRect(&bitmapRect, m_backgroundBrush);
		}
	}
	else
	{
		CRect rectView;
		GetClientRect(&rectView);
		CRect rectTab;
		::GetClientRect(this->m_hwndTabView, &rectTab);
		CPoint pointView(0, 0);
		ClientToScreen(&pointView);
		CPoint pointTab(0, 0);
		::ClientToScreen(this->m_hwndTabView, &pointTab);

		//TRACE(L"========UpdateImageBitmap=====================================\n"
		//      L"rect: %ix%i - %ix%i\n", rectTab.left, rectTab.top, rectTab.right, rectTab.bottom);

		g_imageHandler->UpdateImageBitmap(dc, rectTab, m_background, m_bBackgroundChanged);
		m_bBackgroundChanged = false;

		int xSrc = rectView.left + pointView.x;
		int ySrc = rectView.top  + pointView.y;

		if( m_tabDataTab->imageData.bRelative )
		{
			xSrc -= ::GetSystemMetrics(SM_XVIRTUALSCREEN);
			ySrc -= ::GetSystemMetrics(SM_YVIRTUALSCREEN);
		}
		else
		{
			xSrc -= pointTab.x;
			ySrc -= pointTab.y;
		}

#ifdef _USE_AERO
		if( transparencySettings.transType == transGlass )
		{
			Gdiplus::Graphics gr(dc);

			gr.Clear(Gdiplus::Color(this->m_mainFrame.GetAppActiveStatus() ? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha, 0, 0, 0));

			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = this->m_mainFrame.GetAppActiveStatus() ? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha;
			bf.AlphaFormat = AC_SRC_ALPHA;

			dc.AlphaBlend(
				rectView.left,
				rectView.top,
				rectView.Width(),
				rectView.Height(),
				m_background->dcImage,
				xSrc,
				ySrc,
				rectView.Width(),
				rectView.Height(),
				bf);
		}
		else
#endif //_USE_AERO
		{
			dc.BitBlt(
				rectView.left,
				rectView.top,
				rectView.Width(),
				rectView.Height(),
				m_background->dcImage,
				xSrc,
				ySrc,
				SRCCOPY);
		}
	}

#ifdef CONSOLEZ_CHRONOS
	now2 = std::chrono::high_resolution_clock::now();
	i64cpu7 = std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count();
#endif // CONSOLEZ_CHRONOS

	MutexLock bufferLock(m_consoleHandler.m_bufferMutex);

#ifdef CONSOLEZ_CHRONOS
	now1 = std::chrono::high_resolution_clock::now();
	i64cpu8 = std::chrono::duration_cast<std::chrono::nanoseconds>(now1 - now2).count();
#endif // CONSOLEZ_CHRONOS

	for (DWORD i = 0; i < m_dwScreenRows; ++i)
	{
		this->RowTextOut(dc, i);
	}

#ifdef CONSOLEZ_CHRONOS
	now2 = std::chrono::high_resolution_clock::now();
	i64cpu9 = std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count();

	TRACE_PERF(
		L"thd %lu cpu7=%lld ns cpu8=%lld cpu9=%lld\n",
		::GetCurrentThreadId(),
		i64cpu7,
		i64cpu8,
		i64cpu9);
#endif // CONSOLEZ_CHRONOS

#if 0
	DWORD dwX			= m_nVInsideBorder;
	DWORD dwY			= m_nHInsideBorder;
	DWORD dwOffset		= 0;
	
	WORD attrBG;
	bool	lastFontHigh = false;
	dc.SelectFont(m_fontText);

	// stuff used for caching
	int			nBkMode		= TRANSPARENT;
	COLORREF	crBkColor	= RGB(0, 0, 0);
	COLORREF	crTxtColor	= RGB(0, 0, 0);
	
	int			nCharWidths	= 0;
	bool		bTextOut	= false;

	wstring		strText(L"");

	for (DWORD i = 0; i < m_dwScreenRows; ++i)
	{
		dwX = m_nVInsideBorder;
		dwY = i*m_nCharHeight + m_nHInsideBorder;

		nBkMode			= TRANSPARENT;
		crBkColor		= RGB(0, 0, 0);
		crTxtColor		= RGB(0, 0, 0);
		
		nCharWidths		= 0;
		bTextOut		= false;
		
		attrBG = (m_screenBuffer[dwOffset].charInfo.Attributes & 0xFF) >> 4;
		
		// here we decide how to paint text over the background
		if (/*consoleColors[attrBG] == RGB(0, 0, 0)*/attrBG == 0)
		{
			nBkMode   = TRANSPARENT;
		}
		else
		{
			nBkMode		= OPAQUE;
			crBkColor	= m_tabData->consoleColors[attrBG];
		}

		dc.SetBkMode(nBkMode);
		dc.SetBkColor(crBkColor);

		crTxtColor		= m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_tabData->consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF];
		dc.SetTextColor(crTxtColor);
		if ((m_screenBuffer[dwOffset].charInfo.Attributes & 0x8) && m_consoleSettings.bBoldIntensified)
		{
			if (!lastFontHigh)
			{
				dc.SelectFont(m_fontTextHigh);
				lastFontHigh = true;
			}
		}
		else
		{
			if (lastFontHigh)
			{
				dc.SelectFont(m_fontText);
				lastFontHigh = false;
			}
		}

		strText		= m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
		m_screenBuffer[dwOffset].changed = false;

		nCharWidths	= 1;
		++dwOffset;

		for (DWORD j = 1; j < m_dwScreenColumns; ++j, ++dwOffset)
		{
			if (m_screenBuffer[dwOffset].charInfo.Attributes & COMMON_LVB_TRAILING_BYTE)
			{
				m_screenBuffer[dwOffset].changed = false;
				++nCharWidths;
				continue;
			}
			
			attrBG = (m_screenBuffer[dwOffset].charInfo.Attributes & 0xFF) >> 4;

			if (/*consoleColors[attrBG] == RGB(0, 0, 0)*/attrBG == 0)
			{
				if (nBkMode != TRANSPARENT)
				{
					nBkMode = TRANSPARENT;
					bTextOut = true;
				}
			}
			else
			{
				if (nBkMode != OPAQUE)
				{
					nBkMode = OPAQUE;
					bTextOut = true;
				}
				if (crBkColor != m_tabData->consoleColors[attrBG])
				{
					crBkColor = m_tabData->consoleColors[attrBG];
					bTextOut = true;
				}
			}

			if (crTxtColor != (m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_tabData->consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF]))
			{
				crTxtColor = m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_tabData->consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF];
				bTextOut = true;
			}

			if (m_screenBuffer[dwOffset].charInfo.Attributes & 0x8)
			{
				bTextOut = true;
			}

			if (bTextOut)
			{
				CRect textOutRect(dwX, dwY, dwX+m_nCharWidth*nCharWidths, dwY+m_nCharHeight);

				dc.ExtTextOut(dwX, dwY, ETO_CLIPPED, &textOutRect, strText.c_str(), static_cast<int>(strText.length()), NULL);
				dwX += static_cast<int>(nCharWidths * m_nCharWidth);

				dc.SetBkMode(nBkMode);
				dc.SetBkColor(crBkColor);
				dc.SetTextColor(crTxtColor);
				if ((m_screenBuffer[dwOffset].charInfo.Attributes & 0x8) && m_consoleSettings.bBoldIntensified)
				{
					if (!lastFontHigh)
					{
						dc.SelectFont(m_fontTextHigh);
						lastFontHigh = true;
					}
				}
				else
				{
					if (lastFontHigh)
					{
						dc.SelectFont(m_fontText);
						lastFontHigh = false;
					}
				}

				strText		= m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
				m_screenBuffer[dwOffset].changed = false;
				nCharWidths	= 1;
				bTextOut	= false;
			}
			else
			{
				strText += m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
				m_screenBuffer[dwOffset].changed = false;
				++nCharWidths;
			}
		}

		if (strText.length() > 0)
		{
			CRect textOutRect(dwX, dwY, dwX+m_nCharWidth*nCharWidths, dwY+m_nCharHeight);
			dc.ExtTextOut(dwX, dwY, ETO_CLIPPED, &textOutRect, strText.c_str(), static_cast<int>(strText.length()), NULL);
		}
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintTextChanges(CDC& dc)
{
  //TRACE(L"ConsoleView::RepaintTextChanges\n");
  DWORD dwY      = m_nHInsideBorder;
  DWORD dwOffset = 0;

  MutexLock bufferLock(m_consoleHandler.m_bufferMutex);

  CRect rectView;
  GetClientRect(&rectView);
  CRect rectTab;
  ::GetClientRect(this->m_hwndTabView, &rectTab);
  CPoint pointView(0,0);
  ClientToScreen(&pointView);
  CPoint pointTab(0,0);
  ::ClientToScreen(this->m_hwndTabView, &pointTab);

#ifdef _USE_AERO
	TransparencySettings2& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings.Settings();
#endif //_USE_AERO

  if (m_tabDataTab->backgroundImageType != bktypeNone)
  {
    //TRACE(L"========UpdateImageBitmap=====================================\n"
    //      L"rect: %ix%i - %ix%i\n", rectTab.left, rectTab.top, rectTab.right, rectTab.bottom);

    g_imageHandler->UpdateImageBitmap(dc, rectTab, m_background);
  }

  for (DWORD i = 0; i < m_dwScreenRows; ++i, dwY += m_nCharHeight)
  {
    DWORD dwX = m_nVInsideBorder;

    bool rowHasChanged = false;

    for (DWORD j = 0; j < m_dwScreenColumns; ++j, ++dwOffset, dwX += m_nCharWidth)
    {
      if (m_screenBuffer[dwOffset].changed)
      {
        rowHasChanged = true;
      }
    }

    if( rowHasChanged )
    {
      CRect rect;
      rect.top    = dwY;
      rect.left   = m_nVInsideBorder;
      rect.bottom = dwY + m_nCharHeight;
      rect.right  = dwX;

      if (m_tabDataTab->backgroundImageType == bktypeNone)
      {
#if _USE_AERO
        // set transparency
        if (transparencySettings.transType == transGlass)
        {
          Gdiplus::Graphics gr(dc);

          gr.SetClip(
            Gdiplus::Rect(
              rect.left, rect.top,
              rect.Width(), rect.Height()),
            Gdiplus::CombineModeReplace);

          COLORREF backgroundColor = m_tabDataTab->crBackgroundColor;

          gr.Clear(
            Gdiplus::Color(
              this->m_mainFrame.GetAppActiveStatus()? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha,
              GetRValue(backgroundColor),
              GetGValue(backgroundColor),
              GetBValue(backgroundColor)));
        }
				else
#endif
				{
					dc.FillRect(&rect, m_backgroundBrush);
				}
      }
      else
      {
				int xSrc = rect.left + pointView.x;
				int ySrc = rect.top  + pointView.y;

				if( m_tabDataTab->imageData.bRelative )
				{
					xSrc -= ::GetSystemMetrics(SM_XVIRTUALSCREEN);
					ySrc -= ::GetSystemMetrics(SM_YVIRTUALSCREEN);
				}
				else
				{
					xSrc -= pointTab.x;
					ySrc -= pointTab.y;
				}

#ifdef _USE_AERO
				if( transparencySettings.transType == transGlass )
				{
					Gdiplus::Graphics gr(dc);

					gr.SetClip(
						Gdiplus::Rect(
							rect.left, rect.top,
							rect.Width(), rect.Height()),
						Gdiplus::CombineModeReplace);

					gr.Clear(Gdiplus::Color(this->m_mainFrame.GetAppActiveStatus() ? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha, 0, 0, 0));

					BLENDFUNCTION bf;
					bf.BlendOp = AC_SRC_OVER;
					bf.BlendFlags = 0;
					bf.SourceConstantAlpha = this->m_mainFrame.GetAppActiveStatus() ? transparencySettings.byActiveAlpha : transparencySettings.byInactiveAlpha;
					bf.AlphaFormat = AC_SRC_ALPHA;

					dc.AlphaBlend(
						rect.left,
						rect.top,
						rect.Width(),
						rect.Height(),
						m_background->dcImage,
						xSrc,
						ySrc,
						rect.Width(),
						rect.Height(),
						bf);
				}
				else
#endif //_USE_AERO
				{
					dc.BitBlt(
						rect.left,
						rect.top,
						rect.Width(),
						rect.Height(),
						m_background->dcImage,
						xSrc,
						ySrc,
						SRCCOPY);
				}
      }

      this->RowTextOut(dc, i);
    }
  }
}


void ConsoleView::RowTextOut(CDC& dc, DWORD dwRow)
{
	auto now1 = std::chrono::high_resolution_clock::now();

  //TRACE(L"ConsoleView::RepaintRow %lu\n", dwRow);
  DWORD dwX      = m_nVInsideBorder;
  DWORD dwY      = m_nHInsideBorder + m_nCharHeight * dwRow;
  DWORD dwOffset = m_dwScreenColumns * dwRow;

  COLORREF * consoleColors = m_tabDataTab->consoleColors;

#ifdef _USE_AERO
	BYTE opacity = m_tabDataTab->backgroundTextOpacity;
  Gdiplus::Graphics gr(dc);
#endif //_USE_AERO

	auto now2 = std::chrono::high_resolution_clock::now();

  // first pass : text background color
  WORD    attrBG    = 0;
  DWORD   dwBGWidth = 0;

  for (DWORD j = 0; j < m_dwScreenColumns; ++j, ++dwOffset)
  {
    // reset change state
    m_screenBuffer[dwOffset].changed = false;

    // compare background color
		WORD attr = m_screenBuffer[dwOffset].charInfo.Attributes;
		WORD attrBG2 = (attr & COMMON_LVB_REVERSE_VIDEO) ? (attr & 0x0F) : ((attr & 0xF0) >> 4);
    if( dwBGWidth == 0 )
    {
      attrBG    = attrBG2;
      dwBGWidth = m_nCharWidth;
    }
    else
    {
      if( attrBG == attrBG2 )
      {
        dwBGWidth += m_nCharWidth;
      }
      else
      {
        // draw background and reset

        if (attrBG != 0)
        {
#ifdef _USE_AERO
          COLORREF backgroundColor = consoleColors[attrBG];
          Gdiplus::SolidBrush backgroundBrush(
            Gdiplus::Color(
              opacity,
              GetRValue(backgroundColor),
              GetGValue(backgroundColor),
              GetBValue(backgroundColor)));

          gr.FillRectangle(
            &backgroundBrush,
            dwX, dwY,
            dwBGWidth, m_nCharHeight);
#else //_USE_AERO
          CBrush backgroundBrush;
          backgroundBrush.CreateSolidBrush(consoleColors[attrBG]);

          CRect rect;
          rect.top    = dwY;
          rect.left   = dwX;
          rect.bottom = dwY + m_nCharHeight;
          rect.right  = dwX + dwBGWidth;

          dc.FillRect(&rect, (HBRUSH)backgroundBrush);
#endif //_USE_AERO
        }

        attrBG    = attrBG2;
        dwX       += dwBGWidth;
        dwBGWidth = m_nCharWidth;
      }
    }
  }

  if( dwBGWidth > 0 )
  {
    if (attrBG != 0)
    {
#ifdef _USE_AERO
      COLORREF backgroundColor = consoleColors[attrBG];
      Gdiplus::SolidBrush backgroundBrush(
        Gdiplus::Color(
          opacity,
          GetRValue(backgroundColor),
          GetGValue(backgroundColor),
          GetBValue(backgroundColor)));

      gr.FillRectangle(
        &backgroundBrush,
        dwX, dwY,
        dwBGWidth, m_nCharHeight);
#else //_USE_AERO
      CBrush backgroundBrush;
      backgroundBrush.CreateSolidBrush(consoleColors[attrBG]);

      CRect rect;
      rect.top    = dwY;
      rect.left   = dwX;
      rect.bottom = dwY + m_nCharHeight;
      rect.right  = dwX + dwBGWidth;
      dc.FillRect(&rect, backgroundBrush);
#endif //_USE_AERO

      dwX        += dwBGWidth;
    }
  }

	auto now3 = std::chrono::high_resolution_clock::now();

  // second pass : text
  dwX      = m_nVInsideBorder;
  dwOffset = m_dwScreenColumns * dwRow;

  wstring      strText(L"");
  COLORREF     colorFG      = 0;
  DWORD        dwFGWidth    = 0;
  DWORD        dwCharIdx    = 0;
  FontTextType fontTextType = FontTextNormal;

  bool     boolIntensified = m_appearanceSettings.fontSettings.bBoldIntensified ||
                             m_appearanceSettings.fontSettings.bItalicIntensified;

  for (DWORD j = 0; j < m_dwScreenColumns; ++j, ++dwOffset)
  {
    CHAR_INFO & charInfo = m_screenBuffer[dwOffset].charInfo;
    if (charInfo.Attributes & COMMON_LVB_TRAILING_BYTE) continue;

		WORD attr = charInfo.Attributes;
		int nCharWidth = (attr & COMMON_LVB_LEADING_BYTE)? m_nCharWidth * 2 : m_nCharWidth;

    // compare foreground color
		WORD         attrFG2       = (attr & COMMON_LVB_REVERSE_VIDEO) ? ((attr & 0xF0) >> 4) : (attr & 0x0F);
		COLORREF     colorFG2      = m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : consoleColors[attrFG2];
    FontTextType fontTextType2 = static_cast<FontTextType>(
			(boolIntensified && (attr & FOREGROUND_INTENSITY) ? 1 : 0) |
			(attr &  COMMON_LVB_UNDERSCORE ? 2 : 0));

    if( dwFGWidth == 0 )
    {
      colorFG      = colorFG2;
      dwFGWidth    = nCharWidth;
      fontTextType = fontTextType2;

      dc.SelectFont(m_fontText[fontTextType]);
    }
    else
    {
      if( colorFG == colorFG2 && fontTextType == fontTextType2 )
      {
        dwFGWidth += nCharWidth;
      }
      else
      {
        // draw text and reset

        CRect rect;
        rect.top    = dwY;
        rect.left   = dwX;
        rect.bottom = dwY + m_nCharHeight;
        // we add the space of the next char
        // in italic a part of the previous char is drawn in the following char space
        rect.right  = dwX + dwFGWidth + nCharWidth;

        ExtTextOut(dc, rect, strText, colorFG);

        strText.clear();
        colorFG   = colorFG2;
        dwX       += dwFGWidth;
        dwFGWidth = nCharWidth;
        dwCharIdx = 0;

        // change font
        if( fontTextType != fontTextType2 )
        {
          fontTextType = fontTextType2;
          dc.SelectFont(m_fontText[fontTextType]);
        }
      }
    }

    strText += charInfo.Char.UnicodeChar;
    m_dxWidths[dwCharIdx ++] = nCharWidth;
  }

  if( dwFGWidth > 0 )
  {
    CRect rect;
    rect.top    = dwY;
    rect.left   = dwX;
    rect.bottom = dwY + m_nCharHeight;
    rect.right  = dwX + dwFGWidth;

    ExtTextOut(dc, rect, strText, colorFG);
  }

	auto now4 = std::chrono::high_resolution_clock::now();

	TRACE_PERF(
		L"thd %lu cpu10=%lld ns cpu11=%lld cpu12=%lld\n",
		::GetCurrentThreadId(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now2 - now1).count(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now3 - now2).count(),
		std::chrono::duration_cast<std::chrono::nanoseconds>(now4 - now3).count());
}

inline void ConsoleView::ExtTextOut(CDC& dc, CRect & rect, std::wstring & strText, COLORREF colorFG)
{
	dc.SetBkMode(TRANSPARENT);

	if( m_appearanceSettings.fontSettings.bLigature )
	{
#if 0
		dc.SetTextColor(RGB(240, 0, 0));
		dc.ExtTextOut(rect.left, rect.top, ETO_CLIPPED, &rect, strText.c_str(), static_cast<UINT>(strText.length()), m_dxWidths.get());
#endif

		dc.SetTextColor(colorFG);

		GCP_RESULTS gcpResults = {
			sizeof(GCP_RESULTS),                // DWORD lStructSize
			nullptr,                            // LPWSTR lpOutString
			m_orders.get(),                     // UINT FAR *lpOrder
			nullptr,                            // int FAR *lpDx
			nullptr,                            // int FAR *lpCaretPos
			nullptr,                            // LPSTR lpClass
			m_glyphs.get(),                     // LPWSTR lpGlyphs
			m_dwScreenColumns,                  // UINT nGlyphs
			static_cast<int>(m_dwScreenColumns) // int nMaxFit
		};

		DWORD dwRes = ::GetCharacterPlacement(
			dc,
			strText.c_str(),
			static_cast<int>(strText.length()),
			0,
			&gcpResults,
			GCP_LIGATE);

		if( dwRes )
		{
			memset(m_dxLigatureWidths.get(), 0, m_dwScreenColumns * sizeof(INT));

			for( int i = 0; i < static_cast<int>(strText.length()); ++i )
			{
				m_dxLigatureWidths[m_orders[i]] += m_dxWidths[i];
			}

			dc.ExtTextOut(rect.left, rect.top, ETO_CLIPPED | ETO_GLYPH_INDEX, &rect, m_glyphs.get(), gcpResults.nGlyphs, m_dxLigatureWidths.get());
		}
		else
		{
			TRACE(L"GetCharacterPlacement fails\n");
		}
	}
	else
	{
		dc.SetTextColor(colorFG);

		dc.ExtTextOut(rect.left, rect.top, ETO_CLIPPED, &rect, strText.c_str(), static_cast<UINT>(strText.length()), m_dxWidths.get());
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::BitBltOffscreen(bool bOnlyCursor /*= false*/)
{
	CRect			rectBlit;

	if (bOnlyCursor)
	{
		// blit only cursor
		if (!(m_cursorDBCS) || !m_consoleHandler.GetCursorInfo()->bVisible) return;

		SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();
		SharedMemoryLock consoleInfoLock(consoleInfo);

		rectBlit = m_cursorDBCS->GetCursorRect();
		rectBlit.MoveToXY(
			(consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth  + m_nVInsideBorder,
			(consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top)  * m_nCharHeight + m_nHInsideBorder);
	}
	else
	{
		// blit rect is entire view
		GetClientRect(&rectBlit);
	}

	// we can skip this for relative background images when a full repaint 
	// is needed (UpdateOffscreen will be called in OnPaint)
	if (!m_tabDataTab->imageData.bRelative || (m_tabDataTab->imageData.bRelative && !m_bNeedFullRepaint))
	{
		// we don't do this for relative backgrounds here
		UpdateOffscreen(rectBlit);
	}

	InvalidateRect(&rectBlit, FALSE);
	// The InvalidateRect or InvalidateRgn function can indirectly generate WM_PAINT messages for your windows.
	// If you do not want the application to wait until the application's message queue has no other messages,
	// use the UpdateWindow function to force the WM_PAINT message to be sent immediately.
	if( !bOnlyCursor ) UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::UpdateOffscreen(const CRect& rectBlit)
{
	m_dcOffscreen.BitBlt(
					rectBlit.left, 
					rectBlit.top, 
					rectBlit.right, 
					rectBlit.bottom, 
					m_dcText, 
					rectBlit.left, 
					rectBlit.top, 
					SRCCOPY);

	// blit cursor
	if (m_consoleHandler.GetCursorInfo()->bVisible)
	{
		SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();
		SharedMemoryLock consoleInfoLock(consoleInfo);

		// don't blit if cursor is outside visible window
		if ((consoleInfo->csbi.dwCursorPosition.X >= consoleInfo->csbi.srWindow.Left) &&
			(consoleInfo->csbi.dwCursorPosition.X <= consoleInfo->csbi.srWindow.Right) &&
			(consoleInfo->csbi.dwCursorPosition.Y >= consoleInfo->csbi.srWindow.Top) &&
			(consoleInfo->csbi.dwCursorPosition.Y <= consoleInfo->csbi.srWindow.Bottom))
		{
			bool bDBCS    = false;
			bool bVisible = true;
			{
				MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
				DWORD dwOffset =
					(consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_dwScreenColumns +
					(consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left);

				if( dwOffset >= (m_dwScreenRows * m_dwScreenColumns) )
				{
					// ConsoleZ screen buffer and console info mismatch
					bVisible = false;
				}
				else
				{
					bDBCS = (m_screenBuffer[dwOffset].charInfo.Attributes & COMMON_LVB_LEADING_BYTE) == COMMON_LVB_LEADING_BYTE;
				}
			}

			if( bVisible )
			{
				if( bDBCS )
				{
					if( m_cursorDBCS.get() )
						m_cursorDBCS->BitBlt(
						m_dcOffscreen,
						(consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth  + m_nVInsideBorder,
						(consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top)  * m_nCharHeight + m_nHInsideBorder);
				}
				else
				{
					if( m_cursor.get() )
						m_cursor->BitBlt(
						m_dcOffscreen,
						(consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth  + m_nVInsideBorder,
						(consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top)  * m_nCharHeight + m_nHInsideBorder);
				}
			}
		}
	}

	// blit selection
#ifdef _USE_AERO
	m_selectionHandler->Draw(m_dcOffscreen);
#else //_USE_AERO
	m_selectionHandler->BitBlt(m_dcOffscreen);
#endif //_USE_AERO
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

bool ConsoleView::TranslateKeyDown(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (m_hotkeys.bUseScrollLock && ((::GetKeyState(VK_SCROLL) & 0x01) == 0x01))
		{
			switch(wParam)
			{
				case VK_UP:
					DoScroll(SB_VERT, SB_LINEUP, 0);
					return true;

				case VK_PRIOR:
					DoScroll(SB_VERT, SB_PAGEUP, 0);
					return true;

				case VK_DOWN:
					DoScroll(SB_VERT, SB_LINEDOWN, 0);
					return true;

				case VK_NEXT:
					DoScroll(SB_VERT, SB_PAGEDOWN, 0);
					return true;

				case VK_LEFT:
					DoScroll(SB_HORZ, SB_LINELEFT, 0);
					return true;

				case VK_RIGHT:
					DoScroll(SB_HORZ, SB_LINERIGHT, 0);
					return true;
			}
		}
	}
	
	if ((uMsg == WM_SYSKEYDOWN) || (uMsg == WM_SYSKEYUP))
	{
		// eat ALT+ENTER
		if ((wParam == VK_RETURN) && ((::GetKeyState(VK_MENU) & 0x80) == 0x80))
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::ForwardMouseClick(UINT uMsg, WPARAM wParam, const CPoint& point)
{
	DWORD dwMouseButtonState= 0;
	DWORD dwControlKeyState	= 0;
	DWORD dwEventFlags		= 0;

	if (uMsg == WM_MOUSEMOVE)
	{
		dwEventFlags |= MOUSE_MOVED;

		UINT	uFlags = static_cast<UINT>(wParam);

		if ((uFlags & MK_LBUTTON) != 0)
		{
			dwMouseButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_RBUTTON) != 0)
		{
			dwMouseButtonState = RIGHTMOST_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_MBUTTON) != 0)
		{
			dwMouseButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_XBUTTON1) != 0)
		{
			dwMouseButtonState = FROM_LEFT_3RD_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_XBUTTON2) != 0)
		{
			dwMouseButtonState = FROM_LEFT_4TH_BUTTON_PRESSED;
		}
	}
	else
	{
		// one of mouse click messages
//		UINT	uKeys			= GET_KEYSTATE_WPARAM(wParam); 
		UINT	uXButton		= GET_XBUTTON_WPARAM(wParam);

		switch (uMsg)
		{
			case WM_LBUTTONDOWN : 
			{
				dwMouseButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
				break;
			}

			case WM_LBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = FROM_LEFT_1ST_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_RBUTTONDOWN : 
			{
				dwMouseButtonState = RIGHTMOST_BUTTON_PRESSED;
				break;
			}

			case WM_RBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = RIGHTMOST_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_MBUTTONDOWN : 
			{
				dwMouseButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
				break;
			}

			case WM_MBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = FROM_LEFT_2ND_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_XBUTTONDOWN : 
			{
				if (uXButton == XBUTTON1)
				{
					dwMouseButtonState = FROM_LEFT_3RD_BUTTON_PRESSED;
				}
				else
				{
					dwMouseButtonState = FROM_LEFT_4TH_BUTTON_PRESSED;
				}
				break;
			}

			case WM_XBUTTONDBLCLK : 
			{
				if (uXButton == XBUTTON1)
				{
					dwMouseButtonState	= FROM_LEFT_3RD_BUTTON_PRESSED;
					dwEventFlags		|= DOUBLE_CLICK;
				}
				else
				{
					dwMouseButtonState	= FROM_LEFT_4TH_BUTTON_PRESSED;
					dwEventFlags		|= DOUBLE_CLICK;
				}
				break;
			}
		}
	}

	// get control key states
	if (GetKeyState(VK_RMENU) < 0)		dwControlKeyState |= RIGHT_ALT_PRESSED;
	if (GetKeyState(VK_LMENU) < 0)		dwControlKeyState |= LEFT_ALT_PRESSED;
	if (GetKeyState(VK_RCONTROL) < 0)	dwControlKeyState |= RIGHT_CTRL_PRESSED;
	if (GetKeyState(VK_LCONTROL) < 0)	dwControlKeyState |= LEFT_CTRL_PRESSED;
	if (GetKeyState(VK_CAPITAL) < 0)	dwControlKeyState |= CAPSLOCK_ON;
	if (GetKeyState(VK_NUMLOCK) < 0)	dwControlKeyState |= NUMLOCK_ON;
	if (GetKeyState(VK_SCROLL) < 0)		dwControlKeyState |= SCROLLLOCK_ON;
	if (GetKeyState(VK_SHIFT) < 0)		dwControlKeyState |= SHIFT_PRESSED;


	m_consoleHandler.SendMouseEvent(GetConsoleCoord(point), dwMouseButtonState, dwControlKeyState, dwEventFlags);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

COORD ConsoleView::GetConsoleCoord(const CPoint& clientPoint, bool bStartSelection)
{
	DWORD			dwColumns		= m_consoleHandler.GetConsoleParams()->dwColumns;
	DWORD			dwBufferColumns	= m_consoleHandler.GetConsoleParams()->dwBufferColumns;
	SMALL_RECT&		srWindow		= m_consoleHandler.GetConsoleInfo()->csbi.srWindow;

	CPoint			point(clientPoint);
	COORD			consolePoint;
	SHORT			maxX = (dwBufferColumns > 0) ? static_cast<SHORT>(dwBufferColumns - 1) : static_cast<SHORT>(dwColumns - 1);

	consolePoint.X = static_cast<SHORT>((point.x - m_nVInsideBorder) / m_nCharWidth + srWindow.Left);
	consolePoint.Y = static_cast<SHORT>((point.y - m_nHInsideBorder) / m_nCharHeight + srWindow.Top);

	if (consolePoint.X < 0)
	{
		if (bStartSelection)
		{
			consolePoint.X = 0;
		}
		else
		{
			consolePoint.X = maxX;
			--consolePoint.Y;
		}
	}

	if (consolePoint.X > srWindow.Right) consolePoint.X = srWindow.Right;

	if (consolePoint.Y < 0) consolePoint.Y = 0;

	if (consolePoint.Y > srWindow.Bottom) consolePoint.Y = srWindow.Bottom;

	return consolePoint;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RedrawCharOnCursor(CDC& dc)
{
  SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();
  SharedMemoryLock           consoleInfoLock(consoleInfo);
  COLORREF *                 consoleColors = m_tabDataTab->consoleColors;

  MutexLock bufferLock(m_consoleHandler.m_bufferMutex);
  DWORD dwOffset =
    (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_dwScreenColumns +
    (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left);

  if( dwOffset >= (m_dwScreenRows * m_dwScreenColumns) )
  {
	  // ConsoleZ screen buffer and console info mismatch
	  return;
  }

  CRect                      rectCursor;
  CHAR_INFO &                charInfo = m_screenBuffer[dwOffset].charInfo;
  int                      nCharWidth = (charInfo.Attributes & COMMON_LVB_LEADING_BYTE)? m_nCharWidth * 2 : m_nCharWidth;

  rectCursor.left   = (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth + m_nVInsideBorder;
  rectCursor.top    = (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_nCharHeight + m_nHInsideBorder;
  rectCursor.right  = rectCursor.left + nCharWidth;
  rectCursor.bottom = rectCursor.top + m_nCharHeight;

  CBrush brush(::CreateSolidBrush(m_tabDataTab->crCursorColor));
  dc.FillRect(rectCursor, brush);

  dc.SetBkMode(TRANSPARENT);
  dc.SelectFont(m_fontText[FontTextNormal]);

  COLORREF colorBG;

  if( g_settingsHandler->GetAppearanceSettings().fontSettings.bItalic && 
      (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) > 0 )
  {
    CHAR_INFO & charInfo2 = m_screenBuffer[dwOffset - 1].charInfo;
    int       nCharWidth2 = (charInfo2.Attributes & COMMON_LVB_TRAILING_BYTE)? m_nCharWidth * 2 : m_nCharWidth;

    colorBG = consoleColors[(charInfo2.Attributes & 0xF0) >> 4];

    dc.SetTextColor(colorBG);
    dc.ExtTextOut(
      rectCursor.left - nCharWidth2, rectCursor.top,
      ETO_CLIPPED,
      &rectCursor,
      &charInfo2.Char.UnicodeChar, 1,
      nullptr);
  }

  colorBG = consoleColors[(charInfo.Attributes & 0xF0) >> 4];

  dc.SetTextColor(colorBG);
  dc.ExtTextOut(
    rectCursor.left, rectCursor.top,
    ETO_CLIPPED,
    &rectCursor,
    &charInfo.Char.UnicodeChar, 1,
    nullptr);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnIMEComposition(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// call ::DefWindowProc()
	bHandled = FALSE;

	if( !g_settingsHandler->GetAppearanceSettings().stylesSettings.bIntegratedIME )
		return 0;

	HIMC hImc = ::ImmGetContext(m_hWnd);

#if 0
	WCHAR buf[32] = {0};
	LONG len = ::ImmGetCompositionString(
		hImc,
		GCS_COMPSTR,
		buf, sizeof(buf));

	TRACE(L"ConsoleView::OnIMEComposition (%d)\n", len);
	for(LONG i = 0; i < (len / sizeof(WCHAR)); i++)
		TRACE(L"  %04hx\n", buf[i]);
#endif

	TEXTMETRIC  textMetric;

	m_dcText.SelectFont(m_fontText[FontTextNormal]);
	m_dcText.GetTextMetrics(&textMetric);

	SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();
	SharedMemoryLock consoleInfoLock(consoleInfo);
	CRect rectCursor = m_cursorDBCS->GetCursorRect();
	rectCursor.MoveToXY(
		(consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth  + m_nVInsideBorder,
		(consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top)  * m_nCharHeight + m_nHInsideBorder);

	COMPOSITIONFORM cf;
	cf.dwStyle = CFS_POINT | CFS_FORCE_POSITION;
	cf.ptCurrentPos.x = rectCursor.left;
	cf.ptCurrentPos.y = rectCursor.top;
	cf.rcArea  = rectCursor;

	::ImmSetCompositionWindow(hImc, &cf);

	LOGFONT lf;
	m_fontText[FontTextNormal].GetLogFont(lf);
	::ImmSetCompositionFont(hImc, &lf);

	::ImmReleaseContext(m_hWnd, hImc);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnIMEStartComposition(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// call ::DefWindowProc()
	bHandled = FALSE;

	m_boolImmComposition = true;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnIMEEndComposition(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// call ::DefWindowProc()
	bHandled = FALSE;

	m_boolImmComposition = false;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring ConsoleView::GetFontInfo(void) const
{
	std::wstring ret;

	DWORD dwFontLanguageInfo = ::GetFontLanguageInfo(m_dcText);

	if( dwFontLanguageInfo == GCP_ERROR )
		ret = L"GetFontLanguageInfo returns an error!\n";
	else
	{
		ret = std::wstring( L"GetFontLanguageInfo returns ") + std::to_wstring(dwFontLanguageInfo) + std::wstring(L".\r\n");
		if( dwFontLanguageInfo & GCP_DBCS       ) ret += L"The character set is DBCS.\r\n";
		if( dwFontLanguageInfo & GCP_DIACRITIC  ) ret += L"The font/language contains diacritic glyphs.\r\n";
		if( dwFontLanguageInfo & FLI_GLYPHS     ) ret += L"The font contains extra glyphs not normally accessible using the code page.\r\n";
		if( dwFontLanguageInfo & GCP_GLYPHSHAPE ) ret += L"The font/language contains multiple glyphs per code point or per code point combination (supports shaping and/or ligation), and the font contains advanced glyph tables to provide extra glyphs for the extra shapes.\r\n";
		if( dwFontLanguageInfo & GCP_KASHIDA    ) ret += L"The font/ language permits Kashidas.\r\n";
		if( dwFontLanguageInfo & GCP_LIGATE     ) ret += L"The font/language contains ligation glyphs which can be substituted for specific character combinations.\r\n";
		if( dwFontLanguageInfo & GCP_USEKERNING ) ret += L"The font contains a kerning table which can be used to provide better spacing between the characters and glyphs.\r\n";
		if( dwFontLanguageInfo & GCP_REORDER    ) ret += L"The language requires reordering for displayfor example, Hebrew or Arabic.\r\n";
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::SaveWorkspace(CComPtr<IXMLDOMElement>& pViewElement)
{
	XmlHelper::SetAttribute(pViewElement, CComBSTR(L"Title"), m_tabDataShell->strTitle);
	XmlHelper::SetAttribute(pViewElement, CComBSTR(L"CurrentDirectory"), this->GetConsoleHandler().GetCurrentDirectory());
	XmlHelper::SetAttribute(pViewElement, CComBSTR(L"ShellArguments"), this->GetShellArguments());
	DWORD dwBasePriority = this->GetBasePriority();
	if( dwBasePriority != ULONG_MAX )
		XmlHelper::SetAttribute(pViewElement, CComBSTR(L"BasePriority"), std::wstring(TabData::PriorityToString(dwBasePriority)));

	return true;
}
