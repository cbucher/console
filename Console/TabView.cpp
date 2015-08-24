#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "TabView.h"
#include "DlgCredentials.h"
#include "MainFrame.h"

int CMultiSplitPane::splitBarWidth  = 0;
int CMultiSplitPane::splitBarHeight = 0;


//////////////////////////////////////////////////////////////////////////////

TabView::TabView(MainFrame& mainFrame, std::shared_ptr<TabData> tabData, const wstring& strCmdLineInitialDir, const wstring& strCmdLineInitialCmd, DWORD dwBasePriority)
:m_mainFrame(mainFrame)
,m_viewsMutex(NULL, FALSE, NULL)
,m_tabData(tabData)
,m_strTitle(tabData->strTitle.c_str())
,m_bigIcon()
,m_smallIcon()
,m_boolIsGrouped(false)
,m_strCmdLineInitialDir(strCmdLineInitialDir)
,m_strCmdLineInitialCmd(strCmdLineInitialCmd)
,m_dwBasePriority(dwBasePriority)
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

		// private API TranslateMessageEx
		// called with the TM_POSTCHARBREAKS flag
		// return FALSE if no char is posted
		if( !TranslateMessageEx(pMsg, TM_POSTCHARBREAKS) )
		{
			TRACE_KEY(L"TabView::PreTranslateMessage Msg not translated: 0x%04X, wParam: 0x%08X, lParam: 0x%08X\n", pMsg->message, pMsg->wParam, pMsg->lParam);
			::DispatchMessage(pMsg);
		}
		else
		{
			wLastVirtualKey = static_cast<WORD>(pMsg->wParam);
			TRACE_KEY(L"TabView::PreTranslateMessage Msg translated: 0x%04X, wParam: 0x%08X, lParam: 0x%08X\n", pMsg->message, pMsg->wParam, pMsg->lParam);
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
  m_bigIcon.Attach(m_tabData->GetBigIcon());
  m_smallIcon.Attach(m_tabData->GetSmallIcon());

  LRESULT result = -1;

	CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
	ConsoleViewCreate* consoleViewCreate = reinterpret_cast<ConsoleViewCreate*>(createStruct->lpCreateParams);

  ATLTRACE(_T("TabView::OnCreate\n"));
  MutexLock viewMapLock(m_viewsMutex);
  HWND hwndConsoleView = CreateNewConsole(consoleViewCreate, m_strCmdLineInitialDir, m_strCmdLineInitialCmd, m_dwBasePriority);
  if( hwndConsoleView )
  {
    result = multisplitClass::OnCreate(uMsg, wParam, lParam, bHandled);
    TRACE(L"multisplitClass::OnCreate returns %p\n", result);
    if( result == 0 )
    {
      multisplitClass::tree.window = hwndConsoleView;
      CRect rect;
      m_views.begin()->second->GetRect(rect);
      multisplitClass::RectSet(rect, true);
    }
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
    multisplitClass::RectSet(); // to ClientRect
  }

  bHandled = FALSE;
  return 1;
}

HWND TabView::CreateNewConsole(ConsoleViewCreate* consoleViewCreate, const wstring& strCmdLineInitialDir /*= wstring(L"")*/, const wstring& strCmdLineInitialCmd /*= wstring(L"")*/, DWORD dwBasePriority /*= ULONG_MAX*/)
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
	std::shared_ptr<ConsoleView> consoleView(new ConsoleView(m_mainFrame, m_hWnd, m_tabData, dwRows, dwColumns, strCmdLineInitialDir, strCmdLineInitialCmd, dwBasePriority));
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
      TRACE(L"defaultFocusPane->window = %p not found !!!\n", defaultFocusPane->window);
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
  clientRect = this->visibleRect;
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

void TabView::SetTitle(const CString& strTitle)
{
  m_strTitle = strTitle;
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

void TabView::Split(CMultiSplitPane::SPLITTYPE splitType, wstring strCurrentDirectory)
{
	std::shared_ptr<ConsoleView> activeConsoleView = GetActiveConsole(_T(__FUNCTION__));

	if( strCurrentDirectory == L"" && g_settingsHandler->GetBehaviorSettings2().cloneSettings.bUseCurrentDirectory )
	{
		strCurrentDirectory = activeConsoleView->GetConsoleHandler().GetCurrentDirectory();
	}

	if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
	{
		ConsoleViewCreate consoleViewCreate;
		consoleViewCreate.type = ConsoleViewCreate::CREATE;
		consoleViewCreate.u.userCredentials = nullptr;

		HWND hwndConsoleView = CreateNewConsole(&consoleViewCreate, strCurrentDirectory, wstring(L""), activeConsoleView->GetBasePriority());
		if( hwndConsoleView )
		{
			multisplitClass::SetDefaultFocusPane(multisplitClass::defaultFocusPane->split(
				hwndConsoleView,
				splitType));

			CRect clientRect(0, 0, 0, 0);
			AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

bool TabView::CloseView(HWND hwnd, bool boolDetach, bool& boolTabClosed)
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

			iter->second->DestroyWindow();
			m_views.erase(iter);

#ifdef _DEBUG
			ATLTRACE(L"%p-TabView::CloseView tree\n",
				::GetCurrentThreadId());
			multisplitClass::tree.dump(0, 0);
			ATLTRACE(L"%p-TabView::CloseView defaultFocusPane\n",
				::GetCurrentThreadId());
			if( multisplitClass::defaultFocusPane )
				multisplitClass::defaultFocusPane->dump(0, multisplitClass::defaultFocusPane->parent);
#endif

			if( multisplitClass::defaultFocusPane )
				multisplitClass::SetDefaultFocusPane(multisplitClass::defaultFocusPane->remove());

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
      switch( wID )
      {
      case ID_NEXT_VIEW:
        {
          ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
          ++iter;
          if( iter == m_views.end() )
            iter = m_views.begin();
          multisplitClass::SetDefaultFocusPane(multisplitClass::tree.get(iter->first));
        }
        break;
      case ID_PREV_VIEW:
        {
          ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
          if( iter == m_views.begin() )
            iter = m_views.end();
          --iter;
          multisplitClass::SetDefaultFocusPane(multisplitClass::tree.get(iter->first));
        }
        break;
      case ID_LEFT_VIEW:
        {
          CMultiSplitPane* pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::LEFT);
          if( pane && !pane->isSplitBar() )
            multisplitClass::SetDefaultFocusPane(pane);
        }
        break;
      case ID_RIGHT_VIEW:
        {
          CMultiSplitPane* pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::RIGHT);
          if( pane && !pane->isSplitBar() )
            multisplitClass::SetDefaultFocusPane(pane);
        }
        break;
      case ID_TOP_VIEW:
        {
          CMultiSplitPane* pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::TOP);
          if( pane && !pane->isSplitBar() )
            multisplitClass::SetDefaultFocusPane(pane);
        }
        break;
      case ID_BOTTOM_VIEW:
        {
          CMultiSplitPane* pane = multisplitClass::defaultFocusPane->get(CMultiSplitPane::BOTTOM);
          if( pane && !pane->isSplitBar() )
            multisplitClass::SetDefaultFocusPane(pane);
        }
        break;
      }
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
    multisplitClass::SetDefaultFocusPane(multisplitClass::tree.get(hwnd), m_mainFrame.GetAppActiveStatus());
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

		Helpers::WriteLine(hFile, console->second->GetConsoleHandler().GetFontInfo());
	}
}


//////////////////////////////////////////////////////////////////////////////

void TabView::SaveSession(pt::wptree& prop, wstring prefix, CMultiSplitPane* pane)
{
    prop.put(prefix + L".title", (LPCTSTR) m_strTitle);

    if( pane == nullptr )
        pane = &(multisplitClass::tree);

    if( pane->isSplitBar() )
    {
        prop.put(prefix + L".isSplit", true);
        prop.put(prefix + L".splitType", static_cast<int>(pane->splitType));
        prop.put(prefix + L".splitRatio", pane->splitRatio);
        prop.put(prefix + L".currentDirectory", m_views[pane->pane0->window]->GetConsoleHandler().GetCurrentDirectory());
        SaveSession(prop, prefix + L".pane0", pane->pane0);
        SaveSession(prop, prefix + L".pane1", pane->pane1);
    }
    else
    {
        prop.put(prefix + L".isSplit", false);
        prop.put(prefix + L".currentDirectory", m_views[pane->window]->GetConsoleHandler().GetCurrentDirectory());
    }
}

void TabView::LoadSession(pt::wptree& prop, wstring prefix, CMultiSplitPane* parent)
{
    if( parent == nullptr )
        parent = &(multisplitClass::tree);

    if( prop.get<bool>(prefix + L".isSplit") )
    {
        int splitType = prop.get(prefix + L".splitType", static_cast<int>(CMultiSplitPane::NONE));
        wstring currentDirectory = prop.get(prefix + L".pane1.currentDirectory", L"");
        if( splitType == static_cast<int>(CMultiSplitPane::VERTICAL) || splitType == static_cast<int>(CMultiSplitPane::HORIZONTAL) )
            Split(static_cast<CMultiSplitPane::SPLITTYPE>(splitType), currentDirectory);
    }
}

//////////////////////////////////////////////////////////////////////////////
