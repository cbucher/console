#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "TabView.h"
#include "DlgCredentials.h"
#include "MainFrame.h"

int  CMultiSplitPane::splitBarWidth    = 0;
int  CMultiSplitPane::splitBarHeight   = 0;
bool CMultiSplitPane::splitBarAutoSize = true;


//////////////////////////////////////////////////////////////////////////////

TabView::TabView(MainFrame& mainFrame, std::shared_ptr<TabData> tabData, const std::wstring& strTitle)
:m_mainFrame(mainFrame)
,m_viewsMutex(NULL, FALSE, NULL)
,m_tabData(tabData)
,m_bigIcon()
,m_smallIcon()
,m_boolIsGrouped(false)
,m_strTitle(strTitle.empty() ? tabData->strTitle : strTitle)
{
}

TabView::~TabView()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

extern WORD wLastVirtualKey;
extern bool _boolMenuSysKeyCancelled;

BOOL TabView::PreTranslateMessage(MSG* pMsg)
{
	if( (pMsg->message == WM_KEYDOWN)    ||
	    (pMsg->message == WM_KEYUP)      ||
	    (pMsg->message == WM_SYSKEYDOWN) ||
	    (pMsg->message == WM_SYSKEYUP) )
	{
		// Avoid calling ::TranslateMessage for WM_KEYDOWN, WM_KEYUP,
		// WM_SYSKEYDOWN and WM_SYSKEYUP
		// except for wParam == VK_PACKET,
		// which is sent by SendInput when pasting text
		if (pMsg->wParam == VK_PACKET) return FALSE;

		// except for wParam == VK_PROCESSKEY,
		// Input Method Manager
		if (pMsg->wParam == VK_PROCESSKEY) return FALSE;

		/*
		lParam
		Bits Meaning
		29   The context code. The value is 1 if the ALT key is down while the key is pressed;
		it is 0 if the WM_SYSKEYDOWN message is posted to the active window
		because no window has the keyboard focus.
		*/
		if (pMsg->message == WM_SYSKEYDOWN && (pMsg->wParam == VK_SPACE) && (pMsg->lParam & (0x1 << 29)))
		{
			TRACE(L"WM_SYSKEYDOWN + VK_SPACE\n");
			_boolMenuSysKeyCancelled = true;
			m_mainFrame.PostMessage(WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
			::DispatchMessage(pMsg);
			return TRUE;
		}

		if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_MENU)
		{
			TRACE(L"WM_SYSKEYDOWN + VK_MENU\n");
			/*
			lParam
			Bits Meaning
			30   The previous key state.
			The value is 1 if the key is down before the message is sent,
			or it is 0 if the key is up.
			*/
			if ((pMsg->lParam & (0x1 << 30)) == 0)
			{
				_boolMenuSysKeyCancelled = false;
			}
		}

		if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU)
		{
			TRACE(L"WM_SYSKEYUP + VK_MENU\n");
			if (!_boolMenuSysKeyCancelled)
			{
				m_mainFrame.PostMessage(WM_COMMAND, ID_VIEW_MENU2);
			}
		}

		// private API TranslateMessageEx
		// called with the TM_POSTCHARBREAKS flag
		// returns FALSE if no char is posted
		if (TranslateMessageEx(pMsg, TM_POSTCHARBREAKS))
		{
			TRACE_KEY(L"TabView::PreTranslateMessage Msg translated: 0x%04X, wParam: 0x%08X, lParam: 0x%08X\n", pMsg->message, pMsg->wParam, pMsg->lParam);
			wLastVirtualKey = static_cast<WORD>(pMsg->wParam);
		}
		else
		{
			::DispatchMessage(pMsg);
		}

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT TabView::OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	// load icon
	UpdateIcons();

	LRESULT result = -1;

	CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
	ConsoleViewCreate* consoleViewCreate = reinterpret_cast<ConsoleViewCreate*>(createStruct->lpCreateParams);

	ATLTRACE(_T("TabView::OnCreate\n"));
	MutexLock viewMapLock(m_viewsMutex);
	switch( consoleViewCreate->type )
	{
	case ConsoleViewCreate::CREATE:
	case ConsoleViewCreate::ATTACH:
		{
			HWND hwndConsoleView = CreateNewConsole(consoleViewCreate);
			if( hwndConsoleView )
			{
				result = multisplitClass::OnCreate(uMsg, wParam, lParam, bHandled);
				TRACE(L"multisplitClass::OnCreate returns %p\n", result);
				if( result == 0 )
				{
					multisplitClass::rootPane.window = hwndConsoleView;
				}
			}
		}
		break;

	case ConsoleViewCreate::MOVE:
		{
			result = multisplitClass::OnCreate(uMsg, wParam, lParam, bHandled);
			TRACE(L"multisplitClass::OnCreate returns %p\n", result);
			if( result == 0 )
			{
				// modify the parent window
				::SetParent(consoleViewCreate->consoleView->m_hWnd, m_hWnd);
				::ShowWindow(consoleViewCreate->consoleView->m_hWnd, SW_SHOWNOACTIVATE);
				// and insert
				m_views.insert(ConsoleViewMap::value_type(consoleViewCreate->consoleView->m_hWnd, consoleViewCreate->consoleView));
				multisplitClass::rootPane.window = consoleViewCreate->consoleView->m_hWnd;
			}
		}
		break;

	case ConsoleViewCreate::LOAD_WORKSPACE:
		{
			result = multisplitClass::OnCreate(uMsg, wParam, lParam, bHandled);
			TRACE(L"multisplitClass::OnCreate returns %p\n", result);
			if( result == 0 )
			{
				if( !LoadWorkspace(consoleViewCreate->pTabElement, &(multisplitClass::rootPane)) || m_views.empty() )
					result = -1;
			}
		}
		break;
	}

	if( result == 0 )
	{
		CRect rect;
		m_views.begin()->second->GetRect(rect);
		multisplitClass::SetRect(rect, true);
	}

    bHandled = TRUE;
    ATLTRACE(_T("TabView::OnCreate done\n"));
    return result; // windows sets focus to first control
}

LRESULT TabView::OnEraseBackground (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	// handled, no background painting needed
	return 1;
}

LRESULT TabView::OnSize (UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled)
{
  if (wParam != SIZE_MINIMIZED && m_mainFrame.m_bOnCreateDone)
  {
    TRACE(L"TabView::OnSize -> multisplitClass::RectSet\n");
    multisplitClass::SetRect(); // to ClientRect
  }

  bHandled = FALSE;
  return 1;
}

HWND TabView::CreateNewConsole(ConsoleViewCreate* consoleViewCreate)
{
	DWORD dwRows    = g_settingsHandler->GetConsoleSettings().dwRows;
	DWORD dwColumns = g_settingsHandler->GetConsoleSettings().dwColumns;

	MutexLock	viewMapLock(m_viewsMutex);
#if 0
	if (m_views.size() > 0)
	{
		SharedMemory<ConsoleParams>& consoleParams = m_views.begin()->second->GetConsoleHandler().GetConsoleParams();
		dwRows		= consoleParams->dwRows;
		dwColumns	= consoleParams->dwColumns;
	}
	else
	{
		// initialize member variables for the first view
		m_dwRows	= dwRows;
		m_dwColumns	= dwColumns;
	}
#endif
	std::shared_ptr<ConsoleView> consoleView(new ConsoleView(m_mainFrame, m_hWnd, m_tabData, consoleViewCreate->tabDataShell, dwRows, dwColumns, consoleViewCreate->consoleOptions));
	consoleView->Group(this->IsGrouped());
	UserCredentials userCredentials;

	if( consoleViewCreate->type == ConsoleViewCreate::CREATE )
	{
		consoleViewCreate->u.userCredentials = &userCredentials;

		if (m_tabData->bRunAsUser)
		{
			userCredentials.netOnly = m_tabData->bNetOnly;
#ifdef _USE_AERO
			// Display a dialog box to request credentials.
			std::wstring message = Helpers::LoadString(MSG_TABVIEW_RUNAS);
			CREDUI_INFO ui;
			ui.cbSize = sizeof(ui);
			ui.hwndParent = ::IsWindowVisible(m_mainFrame.m_hWnd)? m_mainFrame.m_hWnd : NULL;
			ui.pszMessageText = m_tabData->strShell.c_str();
			ui.pszCaptionText = message.c_str();
			ui.hbmBanner = NULL;

			// we need a target
			WCHAR szModuleFileName[_MAX_PATH] = L"";
			::GetModuleFileName(NULL, szModuleFileName, ARRAYSIZE(szModuleFileName));

			WCHAR szUser    [CREDUI_MAX_USERNAME_LENGTH + 1] = L"";
			WCHAR szPassword[CREDUI_MAX_PASSWORD_LENGTH + 1] = L"";
			wcscpy_s(szUser, ARRAYSIZE(szUser), m_tabData->strUser.c_str());

			try
			{
				if (g_settingsHandler->GetBehaviorSettings2().runAsUserSettings.bUseCredentialProviders)
				{
					ULONG ulAuthPackage = 0;
					std::unique_ptr<BYTE[]> pvInAuthBlob;
					ULONG cbInAuthBlob  = 0;
					std::unique_ptr<void, CoTaskMemFreeHelper> pvOutAuthBlob;
					ULONG cbOutAuthBlob = 0;
					BOOL  fSave         = FALSE;

					if( szUser[0] )
					{
						::CredPackAuthenticationBuffer(
							0,                                //_In_     DWORD dwFlags,
							szUser,                           //_In_     LPTSTR pszUserName,
							szPassword,                       //_In_     LPTSTR pszPassword,
							nullptr,                          //_Out_    PBYTE pPackedCredentials,
							&cbInAuthBlob                     //_Inout_  DWORD *pcbPackedCredentials
							);

						pvInAuthBlob.reset(new BYTE [cbInAuthBlob]);

						if( !::CredPackAuthenticationBuffer(
							0,                                //_In_     DWORD dwFlags,
							szUser,                           //_In_     LPTSTR pszUserName,
							szPassword,                       //_In_     LPTSTR pszPassword,
							pvInAuthBlob.get(),               //_Out_    PBYTE pPackedCredentials,
							&cbInAuthBlob                     //_Inout_  DWORD *pcbPackedCredentials
							) )
							Win32Exception::ThrowFromLastError("CredPackAuthenticationBuffer");
					}

					{
						PVOID pvAuthBlob = nullptr;
						DWORD rc = ::CredUIPromptForWindowsCredentials(
							&ui,                              //_In_opt_     PCREDUI_INFO pUiInfo,
							0,                                //_In_         DWORD dwAuthError,
							&ulAuthPackage,                   //_Inout_      ULONG *pulAuthPackage,
							pvInAuthBlob.get(),               //_In_opt_     LPCVOID pvInAuthBuffer,
							cbInAuthBlob,                     //_In_         ULONG ulInAuthBufferSize,
							&pvAuthBlob,                      //_Out_        LPVOID *ppvOutAuthBuffer,
							&cbOutAuthBlob,                   //_Out_        ULONG *pulOutAuthBufferSize,
							&fSave,                           //_Inout_opt_  BOOL *pfSave,
							pvInAuthBlob.get()                //_In_         DWORD dwFlags
							? CREDUIWIN_IN_CRED_ONLY
							: 0
							);

						if( rc == ERROR_CANCELLED )
							return 0;

						if( rc != NO_ERROR )
							Win32Exception::Throw("CredUIPromptForWindowsCredentials", rc);

						pvOutAuthBlob.reset(pvAuthBlob);
					}

					TCHAR szDomain[CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1] = L"";
					DWORD maxLenName     = CREDUI_MAX_USERNAME_LENGTH      + 1;
					DWORD maxLenPassword = CREDUI_MAX_PASSWORD_LENGTH      + 1;
					DWORD maxLenDomain   = CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1;

					if( !::CredUnPackAuthenticationBuffer(
						0,
						pvOutAuthBlob.get(),
						cbOutAuthBlob,
						szUser,
						&maxLenName,
						szDomain,
						&maxLenDomain,
						szPassword,
						&maxLenPassword
						) )
						Win32Exception::ThrowFromLastError("CredUnPackAuthenticationBuffer");

					userCredentials.SetUser(szUser);
					userCredentials.password = szPassword;

					::SecureZeroMemory(pvOutAuthBlob.get(), cbOutAuthBlob);
				}
				else
				{
					DWORD rc = ::CredUIPromptForCredentials(
						&ui,                                //__in_opt  PCREDUI_INFO pUiInfo,
						szModuleFileName,                   //__in      PCTSTR pszTargetName,
						NULL,                               //__in      PCtxtHandle Reserved,
						0,                                  //__in_opt  DWORD dwAuthError,
						szUser,                             //__inout   PCTSTR pszUserName,
						ARRAYSIZE(szUser),                  //__in      ULONG ulUserNameMaxChars,
						szPassword,                         //__inout   PCTSTR pszPassword,
						ARRAYSIZE(szPassword),              //__in      ULONG ulPasswordMaxChars,
						NULL,                               //__inout   PBOOL pfSave,
						CREDUI_FLAGS_EXCLUDE_CERTIFICATES | //__in      DWORD dwFlags
						CREDUI_FLAGS_ALWAYS_SHOW_UI       |
						CREDUI_FLAGS_GENERIC_CREDENTIALS  |
						CREDUI_FLAGS_DO_NOT_PERSIST
						);

						if( rc == ERROR_CANCELLED )
							return 0;

						if( rc != NO_ERROR )
							Win32Exception::Throw("CredUIPromptForCredentials", rc);

					userCredentials.SetUser(szUser);
					userCredentials.password = szPassword;
				}
			}
			catch(std::exception& err)
			{
				MessageBox(
					boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % m_tabData->strUser % err.what()).c_str(),
					Helpers::LoadString(IDS_CAPTION_ERROR).c_str(),
					MB_OK|MB_ICONERROR);
				return 0;
			}

#else
			DlgCredentials dlg(m_tabData->strUser.c_str());

			if (dlg.DoModal() != IDOK) return 0;

			userCredentials.SetUser(dlg.GetUser());
			userCredentials.password = dlg.GetPassword();
#endif
		}
		else
		{
			userCredentials.runAsAdministrator = m_tabData->bRunAsAdministrator;
		}
	}

	HWND hwndConsoleView = consoleView->Create(
											m_hWnd, 
											rcDefault, 
											NULL, 
											WS_CHILD | WS_VISIBLE,// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
											0,
											0U,
											reinterpret_cast<void*>(consoleViewCreate));

	if (hwndConsoleView == NULL)
	{
		std::wstring strMessage(consoleView->GetExceptionMessage());

		if(strMessage.empty())
		{
			strMessage = boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_TAB_CREATE_FAILED)) % m_tabData->strTitle.c_str() % m_tabData->strShell.c_str());
		}

		// /!\ MessageBox must be a child of NULL
		// otherwise if an exception occured during the creation of mainframe
		// error message is hidden
		::MessageBox(
			0,
			strMessage.c_str(),
			Helpers::LoadString(IDS_CAPTION_ERROR).c_str(),
			MB_OK|MB_ICONERROR);

		return 0;
	}

	m_views.insert(ConsoleViewMap::value_type(hwndConsoleView, consoleView));

	return hwndConsoleView;
}

std::shared_ptr<ConsoleView> TabView::GetActiveConsole(const TCHAR* /*szFrom*/)
{
  std::shared_ptr<ConsoleView> result;
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    MutexLock viewMapLock(m_viewsMutex);
    ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
    if( iter != m_views.end() )
      result = iter->second;
    else
      TRACE(L"defaultFocusPane->window = %p not found !!!\n", multisplitClass::defaultFocusPane->window);
  }
  else
  {
    TRACE(L"TabView::GetActiveConsole multisplitClass::defaultFocusPane = %p\n", multisplitClass::defaultFocusPane);
  }
  //TRACE(L"TabView::GetActiveConsole called by %s returns %p\n", szFrom, result.get());
  return result;
}


void TabView::GetRect(CRect& clientRect)
{
  clientRect = multisplitClass::GetRect();
}

void TabView::UpdateIcons()
{
  m_bigIcon.Attach(m_tabData->GetBigIcon(g_settingsHandler->GetConsoleSettings().strShell));
  m_smallIcon.Attach(m_tabData->GetSmallIcon(g_settingsHandler->GetConsoleSettings().strShell));
}

void TabView::InitializeScrollbars()
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->InitializeScrollbars();
  }
}

void TabView::Repaint(bool bFullRepaint)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->Repaint(bFullRepaint);
  }
}

#ifdef _USE_AERO
void TabView::RepaintBackground(UINT_PTR nIDTimerEvent)
{
	MutexLock	viewMapLock(m_viewsMutex);
	for( ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it )
	{
		if( it->second->GetBackGroundIDTimerEvent() == nIDTimerEvent )
		{
			it->second->SetBackgroundChanged();
			it->second->Repaint(true);
		}
	}
}
#endif

void TabView::SetResizing(bool bResizing)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->SetResizing(bResizing);
  }
}

bool TabView::MainframeMoving()
{
  bool bRelative = false;
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    bRelative |= it->second->MainframeMoving();
  }
  return bRelative;
}

void TabView::SetTitle(const std::wstring& strTitle)
{
	// by default an empty title is replaced by
	// the title of shell tab title from settings
	m_strTitle = strTitle.empty()? m_tabData->strTitle : strTitle;
}

void TabView::SetActive(bool bActive)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->SetActive(bActive);
  }
}

void TabView::SetAppActiveStatus(bool bAppActive)
{
  MutexLock	viewMapLock(m_viewsMutex);
  if( bAppActive )
  {
    if( this->m_boolIsGrouped )
    {
      for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
      {
        it->second->SetAppActiveStatus(true);
      }
    }
    else
    {
      std::shared_ptr<ConsoleView> consoleView = this->GetActiveConsole(_T(__FUNCTION__));
      for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
      {
        it->second->SetAppActiveStatus(it->second == consoleView);
      }
    }
  }
  else
  {
    for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
    {
      it->second->SetAppActiveStatus(false);
    }
  }
}

void TabView::AdjustRectAndResize(ADJUSTSIZE as, CRect& clientRect, DWORD dwResizeWindowEdge)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->AdjustRectAndResize(as, clientRect, dwResizeWindowEdge);
  }
  this->GetRect(clientRect);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::Split(CMultiSplitPane::SPLITTYPE splitType)
{
	std::wstring strCurrentDirectory(L"");

	std::shared_ptr<ConsoleView> activeConsoleView = GetActiveConsole(_T(__FUNCTION__));

	if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
	{
		ConsoleViewCreate consoleViewCreate;
		consoleViewCreate.type = ConsoleViewCreate::CREATE;
		consoleViewCreate.u.userCredentials = nullptr;
		consoleViewCreate.consoleOptions = activeConsoleView->GetOptions();
		consoleViewCreate.tabDataShell   = activeConsoleView->GetTabData();

		if (g_settingsHandler->GetBehaviorSettings2().cloneSettings.bUseCurrentDirectory)
		{
			consoleViewCreate.consoleOptions.strInitialDir = activeConsoleView->GetConsoleHandler().GetCurrentDirectory();
		}

		HWND hwndConsoleView = CreateNewConsole(&consoleViewCreate);
		if( hwndConsoleView )
		{
			multisplitClass::Split(
				hwndConsoleView,
				splitType);

			CRect clientRect(0, 0, 0, 0);
			AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::MaximizeView(WORD wID)
{
	switch( wID )
	{
	case ID_MAXIMIZE_VIEW:
		if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
		{
			if( multisplitClass::Maximize() )
			{
				CRect clientRect(0, 0, 0, 0);
				AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
			}
		}
		break;

	case ID_RESTORE_VIEW:
		if( multisplitClass::Restore() )
		{
			CRect clientRect(0, 0, 0, 0);
			AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::Merge(std::shared_ptr<TabView> other, CMultiSplitPane::SPLITTYPE splitType)
{
	// lock access to views map
	MutexLock	viewMapLock(m_viewsMutex);
	MutexLock	viewMapLock2(other->m_viewsMutex);

	// move views from other tab to current tab
	for( auto it = other->m_views.begin(); it != other->m_views.end(); ++it )
	{
		// modify the tab owner of the view to current tab
		it->second->SetParentTab(m_hWnd, m_tabData);

		// insert into current tab
		m_views.insert(*it);
	}
	other->m_views.clear();

	// merge multisplit
	multisplitClass::Merge(*other, splitType);

	// resize consoles row/column
	CRect clientRect(0, 0, 0, 0);
	AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

bool TabView::CloseView(HWND hwnd, bool boolDetach, bool boolDestroyWindow, bool& boolTabClosed)
{
	boolTabClosed = false;

	if( hwnd == 0 )
	{
		if( multisplitClass::defaultFocusPane )
			hwnd = multisplitClass::defaultFocusPane->window;
	}

	if( hwnd )
	{
		MutexLock viewMapLock(m_viewsMutex);
		ConsoleViewMap::iterator iter = m_views.find(hwnd);
		if( iter != m_views.end() )
		{
			if( boolDetach )
				iter->second->GetConsoleHandler().Detach();

			if( boolDestroyWindow )
				iter->second->DestroyWindow();

			m_views.erase(iter);

			multisplitClass::Remove();

			if( m_views.empty() )
				boolTabClosed = true;
			else
			{
				CRect clientRect(0, 0, 0, 0);
				AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
			}

			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::SwitchView(WORD wID)
{
	if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
	{
		MutexLock viewMapLock(m_viewsMutex);

		if( m_views.size() > 1 )
		{
			CMultiSplitPane * pane = nullptr;

			switch( wID )
			{
			case ID_NEXT_VIEW:
			{
				ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
				for( size_t i = m_views.size(); i > 0; --i )
				{
					++iter;
					if( iter == m_views.end() )
						iter = m_views.begin();
					pane = multisplitClass::maximizedPane->get(iter->first);
					if( pane ) break;
				}
			}
			break;

			case ID_PREV_VIEW:
			{
				ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
				for( size_t i = m_views.size(); i > 0; --i )
				{
					if( iter == m_views.begin() )
						iter = m_views.end();
					--iter;
					pane = multisplitClass::maximizedPane->get(iter->first);
					if( pane ) break;
				}
			}
			break;

			case ID_LEFT_VIEW:
				pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::LEFT);
				break;

			case ID_RIGHT_VIEW:
				pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::RIGHT);
				break;

			case ID_TOP_VIEW:
				pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::TOP);
				break;

			case ID_BOTTOM_VIEW:
				pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::BOTTOM);
				break;
			}

			if( pane && !pane->isSplitBar() )
				multisplitClass::SetDefaultFocusPane(pane);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::ResizeView(WORD wID)
{
	if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
	{
		MutexLock viewMapLock(m_viewsMutex);

		if( m_views.size() > 1 )
		{
			switch( wID )
			{
			case ID_DEC_HORIZ_SIZE:
				multisplitClass::defaultFocusPane->resize(CMultiSplitPane::VERTICAL, -ConsoleView::GetCharWidth());
				break;

			case ID_INC_HORIZ_SIZE:
				multisplitClass::defaultFocusPane->resize(CMultiSplitPane::VERTICAL, ConsoleView::GetCharWidth());
				break;

			case ID_DEC_VERT_SIZE:
				multisplitClass::defaultFocusPane->resize(CMultiSplitPane::HORIZONTAL, -ConsoleView::GetCharHeight());
				break;

			case ID_INC_VERT_SIZE:
				multisplitClass::defaultFocusPane->resize(CMultiSplitPane::HORIZONTAL, ConsoleView::GetCharHeight());
				break;
			}

			CRect clientRect(0, 0, 0, 0);
			AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::SetActiveConsole(HWND hwnd)
{
  MutexLock viewMapLock(m_viewsMutex);
  auto it = m_views.find(hwnd);
  if( it != m_views.end() )
    multisplitClass::SetDefaultFocusPane(multisplitClass::maximizedPane->get(hwnd), m_mainFrame.GetAppActiveStatus());
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::UpdateTheme()
{
	MutexLock viewMapLock(m_viewsMutex);
	for( auto it = m_views.begin(); it != m_views.end(); ++it )
	{
		// modify the tab owner of the view to current tab
		it->second->SetParentTab(m_hWnd, m_tabData);
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::OnSplitBarMove(HWND /*hwndPane0*/, HWND /*hwndPane1*/, bool /*boolEnd*/)
{
  CRect clientRect(0, 0, 0, 0);
  AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	MutexLock	viewMapLock(m_viewsMutex);
	for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
	{
		if( it->second->IsGrouped() )
			it->second->GetConsoleHandler().PostMessage(Msg, wParam, lParam);
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::WriteConsoleInputToConsoles(KEY_EVENT_RECORD* pkeyEvent)
{
	MutexLock	viewMapLock(m_viewsMutex);
	for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
	{
		if( it->second->IsGrouped() )
			it->second->GetConsoleHandler().WriteConsoleInput(pkeyEvent);
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::SendTextToConsoles(const wchar_t* pszText)
{
	MutexLock	viewMapLock(m_viewsMutex);
	for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
	{
		if( it->second->IsGrouped() )
			it->second->GetConsoleHandler().SendTextToConsole(pszText);
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::SendCtrlCToConsoles()
{
	MutexLock	viewMapLock(m_viewsMutex);
	for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
	{
		if( it->second->IsGrouped() )
			it->second->GetConsoleHandler().SendCtrlC();
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::Group(bool b)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->Group(b);
  }
  m_boolIsGrouped = b;
  SetAppActiveStatus(true);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT TabView::OnScrollCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	int	nScrollType	= 0;
	int nScrollCode	= 0;

	switch (wID)
	{
		case ID_SCROLL_UP :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_LINEUP;
			break;
		}

		case ID_SCROLL_LEFT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_LINELEFT;
			break;
		}

		case ID_SCROLL_DOWN :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_LINEDOWN;
			break;
		}

		case ID_SCROLL_RIGHT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_LINERIGHT;
			break;
		}

		case ID_SCROLL_PAGE_UP :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_PAGEUP;
			break;
		}

		case ID_SCROLL_PAGE_LEFT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_PAGELEFT;
			break;
		}

		case ID_SCROLL_PAGE_DOWN :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_PAGEDOWN;
			break;
		}

		case ID_SCROLL_PAGE_RIGHT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_PAGERIGHT;
			break;
		}


		default : bHandled = FALSE; return 0;
	}

  std::shared_ptr<ConsoleView> consoleView = this->GetActiveConsole(_T(__FUNCTION__));
  if( consoleView )
    consoleView->DoScroll(nScrollType, nScrollCode, 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void TabView::OnPaneChanged(void)
{
  SetAppActiveStatus(m_mainFrame.GetAppActiveStatus());
  SetActive(true);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void TabView::Diagnose(HANDLE hFile)
{
	MutexLock viewMapLock(m_viewsMutex);

	std::shared_ptr<ConsoleView> activeConsole = this->GetActiveConsole(_T(__FUNCTION__));

	for(auto console = m_views.begin(); console != m_views.end(); ++console)
	{
		WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;
		wstring strViewTitle = m_mainFrame.FormatTitle(windowSettings.strTabTitleFormat, this, console->second);

		std::wstring dummy =
			(console->second == activeConsole ? std::wstring(L"  View (active): ") : std::wstring(L"  View: "))
			+ strViewTitle;
		Helpers::WriteLine(hFile, dummy);

		dummy = L"  is elevated? ";
		dummy += console->second->GetConsoleHandler().IsElevated() ? L"yes" : L"no";
		Helpers::WriteLine(hFile, dummy);

		UINT input = 0, output = 0;
		console->second->GetConsoleHandler().GetCP(input, output);
		dummy = L"  input code page ";
		dummy += std::to_wstring(input);
		Helpers::WriteLine(hFile, dummy);
		dummy = L"  output code page ";
		dummy += std::to_wstring(output);
		Helpers::WriteLine(hFile, dummy);

		Helpers::WriteLine(hFile, L"  Windows console font");
		Helpers::WriteLine(hFile, console->second->GetConsoleHandler().GetFontInfo());

		Helpers::WriteLine(hFile, L"  ConsoleZ font");
		Helpers::WriteLine(hFile, console->second->GetFontInfo());
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabView::SaveWorkspace(CComPtr<IXMLDOMElement>& pTabElement)
{
	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"Title"), m_tabData->strTitle);
	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"Name"), m_strTitle);

	if( !SaveWorkspace(pTabElement, &(multisplitClass::rootPane), CComBSTR(L"\r\n\t\t")) ) return false;

	XmlHelper::AddTextNode(pTabElement, CComBSTR(L"\r\n\t"));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabView::SaveWorkspace(CComPtr<IXMLDOMElement>& pElement, CMultiSplitPane* pane, const CComBSTR& ident)
{
	if( pane->isSplitBar() )
	{
		// SplitView
		CComPtr<IXMLDOMElement> pSplitViewElement;
		if( FAILED(XmlHelper::CreateDomElement(pElement, CComBSTR(L"SplitView"), pSplitViewElement)) ) return false;

		XmlHelper::SetAttribute(pSplitViewElement, CComBSTR(L"Type"), std::wstring(pane->splitType == CMultiSplitPane::HORIZONTAL ? L"Horizontal" : L"Vertical"));
		XmlHelper::SetAttribute(pSplitViewElement, CComBSTR(L"Ratio"), pane->splitRatio);

		CComBSTR identPaneOut = ident;
		identPaneOut += L"\t";
		CComBSTR identPaneIn = identPaneOut;
		identPaneIn += L"\t";

		// SplitView/Pane0
		CComPtr<IXMLDOMElement> pPane0Element;
		if( FAILED(XmlHelper::CreateDomElement(pSplitViewElement, CComBSTR(L"Pane0"), pPane0Element)) ) return false;

		if( SaveWorkspace(pPane0Element, pane->pane0, identPaneIn) == false ) return false;

		XmlHelper::AddTextNode(pPane0Element, identPaneOut);

		// SplitView/Pane1
		CComPtr<IXMLDOMElement> pPane1Element;
		if( FAILED(XmlHelper::CreateDomElement(pSplitViewElement, CComBSTR(L"Pane1"), pPane1Element)) ) return false;

		if( SaveWorkspace(pPane1Element, pane->pane1, identPaneIn) == false ) return false;

		XmlHelper::AddTextNode(pPane1Element, identPaneOut);

		XmlHelper::AddTextNode(pSplitViewElement, ident);
	}
	else
	{
		// View
		CComPtr<IXMLDOMElement> pViewElement;
		if( FAILED(XmlHelper::CreateDomElement(pElement, CComBSTR(L"View"), pViewElement)) ) return false;

		if( m_views[pane->window]->SaveWorkspace(pViewElement) == false ) return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabView::LoadWorkspace(CComPtr<IXMLDOMElement>& pElement, CMultiSplitPane* pane)
{
	CComPtr<IXMLDOMElement> pSplitViewElement;
	if( SUCCEEDED(XmlHelper::GetDomElement(pElement, CComBSTR(L"SplitView"), pSplitViewElement)) )
	{
		// SplitView

		std::wstring strSplitViewType;
		XmlHelper::GetAttribute(pSplitViewElement, CComBSTR(L"Type"), strSplitViewType, L"");

		CMultiSplitPane::SPLITTYPE splitType;
		if( strSplitViewType == L"Horizontal" ) splitType = CMultiSplitPane::HORIZONTAL;
		else if( strSplitViewType == L"Vertical" ) splitType = CMultiSplitPane::VERTICAL;
		else return false;

		int splitRatio;
		XmlHelper::GetAttribute(pSplitViewElement, CComBSTR(L"Ratio"), splitRatio, 50);

		if( !pane->split(splitType, splitRatio, nullptr, nullptr, false) ) return false;

		// SplitView/Pane0
		CComPtr<IXMLDOMElement> pPane0Element;
		if( FAILED(XmlHelper::GetDomElement(pSplitViewElement, CComBSTR(L"Pane0"), pPane0Element)) ) return false;

		if( LoadWorkspace(pPane0Element, pane->pane0) == false ) return false;

		// SplitView/Pane1
		CComPtr<IXMLDOMElement> pPane1Element;
		if( FAILED(XmlHelper::GetDomElement(pSplitViewElement, CComBSTR(L"Pane1"), pPane1Element)) ) return false;

		if( LoadWorkspace(pPane1Element, pane->pane1) == false ) return false;
	}
	else
	{
		// View
		CComPtr<IXMLDOMElement> pViewElement;
		if( FAILED(XmlHelper::GetDomElement(pElement, CComBSTR(L"View"), pViewElement)) ) return false;

		std::wstring strTitle;
		XmlHelper::GetAttribute(pViewElement, CComBSTR(L"Title"), strTitle, L"");

		// find tab with corresponding name...
		TabSettings&	tabSettings = g_settingsHandler->GetTabSettings();
		for( size_t i = 0; i < tabSettings.tabDataVector.size(); ++i )
		{
			if( tabSettings.tabDataVector[i]->strTitle == strTitle )
			{
				ConsoleViewCreate consoleViewCreate;
				consoleViewCreate.type = ConsoleViewCreate::CREATE;
				consoleViewCreate.u.userCredentials = nullptr;
				XmlHelper::GetAttribute(pViewElement, CComBSTR(L"CurrentDirectory"), consoleViewCreate.consoleOptions.strInitialDir, L"");
				// for backward compatibility (InitialCommand attribute has been replaced by ShellArguments)
				XmlHelper::GetAttribute(pViewElement, CComBSTR(L"InitialCommand"), consoleViewCreate.consoleOptions.strShellArguments, L"");
				if(consoleViewCreate.consoleOptions.strShellArguments.empty())
					XmlHelper::GetAttribute(pViewElement, CComBSTR(L"ShellArguments"), consoleViewCreate.consoleOptions.strShellArguments, L"");
				std::wstring strBasePriority;
				XmlHelper::GetAttribute(pViewElement, CComBSTR(L"BasePriority"), strBasePriority, L"");
				consoleViewCreate.consoleOptions.dwBasePriority = TabData::StringToPriority(strBasePriority.c_str());
				consoleViewCreate.tabDataShell = tabSettings.tabDataVector[i];

				pane->window = CreateNewConsole(&consoleViewCreate);
				return pane->window ? true : false;
			}
		}

		MessageBox(
			boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_UNDEFINED_TAB)) % strTitle).c_str(),
			Helpers::LoadString(IDS_CAPTION_ERROR).c_str(),
			MB_ICONERROR | MB_OK);

		return false;
	}

	return true;
}
