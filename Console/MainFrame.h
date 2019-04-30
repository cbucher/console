#pragma once


//////////////////////////////////////////////////////////////////////////////

#define ID_NEW_TAB_FIRST		1000

// Timer that will force a call to ResizeWindow (called from WM_EXITSIZEMOVE handler
// when the ConsoleZ window is resized using a mouse)
// External utilities that might resize ConsoleZ window usually don't send WM_EXITSIZEMOVE
// message after resizing a window.
#define	TIMER_SIZING			42
#define	TIMER_SIZING_INTERVAL	100

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class TabView;
class ConsoleView;
struct ConsoleViewCreate;
typedef std::map< HWND, std::shared_ptr<TabView> > TabViewMap;

enum ShowHideWindowAction
{
	SHWA_SWITCH = 0,
	SHWA_SHOW_ONLY = 1,
	SHWA_HIDE_ONLY = 2,
	SHWA_DONOTHING = 3,
};

//////////////////////////////////////////////////////////////////////////////

struct CommandLineOptions
{
	CommandLineOptions()
		: strWindowTitle()
		, startupWorkspaces()
		, startupTabs()
		, startupTabTitles()
		, startupDirs()
		, startupShellArgs()
		, basePriorities()
		, nMultiStartSleep(0)
		, visibility(ShowHideWindowAction::SHWA_DONOTHING)
		, strWorkingDir()
		, strEnvironment()
		, bAttachConsoles(false)
	{
	}

	std::wstring strWindowTitle;
	std::vector<std::wstring> startupWorkspaces;
	std::vector<std::wstring> startupTabs;
	std::vector<std::wstring> startupTabTitles;
	std::vector<std::wstring> startupDirs;
	std::vector<std::wstring> startupShellArgs;
	std::vector<DWORD> basePriorities;
	int nMultiStartSleep;
	ShowHideWindowAction visibility;
	std::wstring strWorkingDir;
	std::wstring strEnvironment;
	bool bAttachConsoles;
};

//////////////////////////////////////////////////////////////////////////////

class MainFrame 
	: public CTabbedFrameImpl<MainFrame>
	, public CUpdateUI<MainFrame>
	, public CMessageFilter
	, public CIdleHandler
{
	public:
		DECLARE_FRAME_WND_CLASS(L"Console_2_Main", IDR_MAINFRAME)

#ifdef _USE_AERO
		aero::CCommandBarCtrl m_CmdBar;

		void OnComposition()
    {
      this->SetTransparency();
    }
#else
		CCommandBarCtrl m_CmdBar;
#endif

		MainFrame
		(
			LPCTSTR lpstrCmdLine
		);

		static void ParseCommandLine
		(
			LPCTSTR lptstrCmdLine,
			CommandLineOptions& commandLineOptions
		);

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnIdle();

		BEGIN_UPDATE_UI_MAP(MainFrame)
			UPDATE_ELEMENT(ID_FILE_CLOSE_TAB, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_FILE_CLOSE_OTHER_TABS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_FILE_CLOSE_TABS_TO_THE_LEFT, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_FILE_CLOSE_TABS_TO_THE_RIGHT, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_CLOSE_VIEW, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_DETACH_VIEW, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_EDIT_SELECT_ALL, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_CLEAR_SELECTION, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_PASTE_SELECTION, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_MENU, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_SEARCH_BAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TABS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_CONSOLE, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_FORWARD_MOUSE_EVENTS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_FULLSCREEN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_SEARCH_MATCH_CASE, UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_SEARCH_MATCH_WHOLE_WORD, UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_SWITCH_TRANSPARENCY, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_ALWAYS_ON_TOP, UPDUI_MENUPOPUP)

			UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(3, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(4, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(5, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(6, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(7, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(8, UPDUI_STATUSBAR)

		END_UPDATE_UI_MAP()

		BEGIN_MSG_MAP(MainFrame)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_NCACTIVATE, OnActivateApp)
			MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeydown)
			MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
			MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_SIZING, OnSizing)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
			MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)

#ifndef _USING_V110_SDK71_

			MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)

#endif

			MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_MBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_XBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
			MESSAGE_HANDLER(UM_CONSOLE_RESIZED, OnConsoleResized)
			MESSAGE_HANDLER(UM_CONSOLE_CLOSED, OnConsoleClosed)
			MESSAGE_HANDLER(UM_UPDATE_TITLES, OnUpdateTitles)
			MESSAGE_HANDLER(UM_UPDATE_STATUS_BAR, OnUpdateStatusBar)
			MESSAGE_HANDLER(UM_SHOW_POPUP_MENU, OnShowPopupMenu)
			MESSAGE_HANDLER(UM_START_MOUSE_DRAG, OnStartMouseDrag)
			MESSAGE_HANDLER(m_uTaskbarRestart, OnTaskbarCreated)
			MESSAGE_HANDLER(m_uReloadDesktopImages, OnReloadDesktopImages)
#ifdef _USE_AERO
			MESSAGE_HANDLER(m_uTaskbarButtonCreated, OnTaskbarButtonCreated)
#endif
			MESSAGE_HANDLER(UM_TRAY_NOTIFY, OnTrayNotify)
			MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)

			NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnTabChanged)
			NOTIFY_CODE_HANDLER(CTCN_ACCEPTITEMDRAG, OnTabOrderChanged)
			NOTIFY_CODE_HANDLER(CTCN_DELETEITEM, OnTabOrderChanged)
			NOTIFY_CODE_HANDLER(CTCN_CLOSE, OnTabClose)
			NOTIFY_CODE_HANDLER(CTCN_NEWTAB, OnNewTab)
			NOTIFY_CODE_HANDLER(CTCN_MCLICK, OnTabMiddleClick)
			NOTIFY_CODE_HANDLER(NM_RCLICK, OnTabRightClick)
#ifdef _USE_AERO
			NOTIFY_CODE_HANDLER(NM_CLICK, OnStartMouseDragExtendedFrameToClientArea)
			NOTIFY_CODE_HANDLER(NM_LDOWN, OnStartMouseDragExtendedFrameToClientArea)
			NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDBLClickExtendedFrameToClientArea)
#endif //_USE_AERO

			NOTIFY_CODE_HANDLER(RBN_HEIGHTCHANGE, OnRebarHeightChanged)
			NOTIFY_HANDLER(ATL_IDW_TOOLBAR, TBN_DROPDOWN, OnToolbarDropDown)

			CHAIN_MSG_MAP(CUpdateUI<MainFrame>)

			COMMAND_RANGE_HANDLER(ID_NEW_TAB_1, ID_NEW_TAB_1 + 99, OnFileNewTab)
			COMMAND_ID_HANDLER(ID_FILE_NEW_TAB, OnFileNewTab)
			COMMAND_RANGE_HANDLER(ID_SWITCH_TAB_1, ID_SWITCH_TAB_1 + 99, OnSwitchTab)
			COMMAND_ID_HANDLER(ID_NEXT_TAB, OnNextTab)
			COMMAND_ID_HANDLER(ID_PREV_TAB, OnPrevTab)
			COMMAND_ID_HANDLER(ID_MOVE_TAB_LEFT,  OnMoveTab)
			COMMAND_ID_HANDLER(ID_MOVE_TAB_RIGHT, OnMoveTab)

			COMMAND_ID_HANDLER(ID_NEXT_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_PREV_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_LEFT_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_RIGHT_VIEW  , OnSwitchView)
			COMMAND_ID_HANDLER(ID_TOP_VIEW    , OnSwitchView)
			COMMAND_ID_HANDLER(ID_BOTTOM_VIEW , OnSwitchView)

			COMMAND_ID_HANDLER(ID_DEC_HORIZ_SIZE , OnResizeView)
			COMMAND_ID_HANDLER(ID_INC_HORIZ_SIZE , OnResizeView)
			COMMAND_ID_HANDLER(ID_DEC_VERT_SIZE  , OnResizeView)
			COMMAND_ID_HANDLER(ID_INC_VERT_SIZE  , OnResizeView)

			COMMAND_ID_HANDLER(ID_CLOSE_VIEW  , OnCloseView)
			COMMAND_ID_HANDLER(ID_DETACH_VIEW , OnCloseView)
			COMMAND_ID_HANDLER(ID_SPLIT_HORIZ , OnSplit)
			COMMAND_ID_HANDLER(ID_SPLIT_VERT  , OnSplit)
			COMMAND_ID_HANDLER(ID_GROUP_ALL   , OnGroupAll)
			COMMAND_ID_HANDLER(ID_UNGROUP_ALL , OnUngroupAll)
			COMMAND_ID_HANDLER(ID_GROUP_TAB   , OnGroupTab)
			COMMAND_ID_HANDLER(ID_UNGROUP_TAB , OnUngroupTab)
			COMMAND_ID_HANDLER(ID_CLONE_IN_NEW_TAB             , OnCloneInNewTab)
			COMMAND_ID_HANDLER(ID_MOVE_IN_NEW_TAB              , OnMoveInNewTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TAB               , OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_OTHER_TABS        , OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TABS_TO_THE_LEFT  , OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TABS_TO_THE_RIGHT , OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_ATTACH_CONSOLES              , OnAttachConsoles)
			COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
			COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnEditClear)
			COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
			COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, OnEditSelectAll)
			COMMAND_ID_HANDLER(ID_EDIT_CLEAR_SELECTION, OnEditClearSelection)
			COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
			COMMAND_ID_HANDLER(ID_PASTE_SELECTION, OnPasteSelection)

			COMMAND_RANGE_HANDLER(ID_TEXT_SELECTION_LEFT_KEY, ID_COLUMN_SELECTION_PAGEDOWN_KEY, OnSelectionKeyPressed)

			COMMAND_ID_HANDLER(ID_EDIT_STOP_SCROLLING, OnEditStopScrolling)
			COMMAND_ID_HANDLER(ID_EDIT_RESUME_SCROLLING, OnEditResumeScrolling)
			COMMAND_ID_HANDLER(ID_EDIT_RENAME_TAB, OnEditRenameTab)
			COMMAND_ID_HANDLER(ID_EDIT_SETTINGS, OnEditSettings)
			COMMAND_ID_HANDLER(ID_VIEW_MENU, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_MENU2, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
			COMMAND_ID_HANDLER(ID_VIEW_SEARCH_BAR, OnViewSearchBar)
			COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
			COMMAND_ID_HANDLER(ID_VIEW_TABS, OnViewTabs)
			COMMAND_ID_HANDLER(ID_VIEW_CONSOLE, OnViewConsole)
			COMMAND_ID_HANDLER(ID_FORWARD_MOUSE_EVENTS, OnForwardMouseEvents)
			COMMAND_ID_HANDLER(ID_HELP, OnHelp)
			COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
			COMMAND_ID_HANDLER(IDC_DUMP_BUFFER, OnDumpBuffer)
			COMMAND_ID_HANDLER(ID_VIEW_FULLSCREEN, OnFullScreen)
			COMMAND_ID_HANDLER(ID_VIEW_ALWAYS_ON_TOP, OnAlwaysOnTop)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_100, OnZoom)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_INC, OnZoom)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_DEC, OnZoom)
			COMMAND_ID_HANDLER(ID_SEARCH_PREV,             OnSearchText)
			COMMAND_ID_HANDLER(ID_SEARCH_NEXT,             OnSearchText)
			COMMAND_ID_HANDLER(ID_SEARCH_MATCH_CASE,       OnSearchSettings)
			COMMAND_ID_HANDLER(ID_SEARCH_MATCH_WHOLE_WORD, OnSearchSettings)
			COMMAND_ID_HANDLER(ID_FIND,                    OnFind)
			COMMAND_ID_HANDLER(ID_SWITCH_TRANSPARENCY,     OnSwitchTransparency)
			COMMAND_ID_HANDLER(ID_SHOW_CONTEXT_MENU_1,        OnShowContextMenu)
			COMMAND_ID_HANDLER(ID_SHOW_CONTEXT_MENU_2,        OnShowContextMenu)
			COMMAND_ID_HANDLER(ID_SHOW_CONTEXT_MENU_3,        OnShowContextMenu)
			COMMAND_ID_HANDLER(ID_SHOW_CONTEXT_MENU_SNIPPETS, OnShowContextMenu)
			COMMAND_ID_HANDLER(ID_SEND_CTRL_C,             OnSendCtrlEvent)
			COMMAND_ID_HANDLER(ID_FONT_INFO,               OnFontInfo)
			COMMAND_ID_HANDLER(ID_DIAGNOSE,                OnDiagnose)
			COMMAND_ID_HANDLER(ID_SPLIT_SWAP,              OnSwap)
			COMMAND_ID_HANDLER(ID_LOAD_WORKSPACE,          OnLoadWorkspace)
			COMMAND_ID_HANDLER(ID_SAVE_WORKSPACE,          OnSaveWorkspace)
			COMMAND_ID_HANDLER(ID_MERGE_HORIZONTALLY,      OnMergeTabs)
			COMMAND_ID_HANDLER(ID_MERGE_VERTICALLY,        OnMergeTabs)
			COMMAND_ID_HANDLER(ID_MAXIMIZE_VIEW,           OnMaximizeView)
			COMMAND_ID_HANDLER(ID_RESTORE_VIEW,            OnMaximizeView)
			COMMAND_ID_HANDLER(ID_CLONE_TAB,               OnCloneTab)

			COMMAND_RANGE_HANDLER(ID_EXTERNAL_COMMAND_1, (ID_EXTERNAL_COMMAND_1 + EXTERNAL_COMMANDS_COUNT - 1), OnExternalCommand)
			COMMAND_RANGE_HANDLER(ID_SNIPPET_ID_FIRST, ID_SNIPPET_ID_LAST, OnSnippet)

			CHAIN_MSG_MAP(CTabbedFrameImpl<MainFrame>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

//		Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

		LRESULT OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnSysKeydown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSizing(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
		LRESULT OnInitMenuPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDpiChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMouseButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnReloadDesktopImages(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnConsoleResized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */);
		LRESULT OnConsoleClosed(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnUpdateStatusBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnStartMouseDrag(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnTrayNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTaskbarButtonCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnTabOrderChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnNewTab(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnTabMiddleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnTabRightClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
#ifdef _USE_AERO
		LRESULT OnStartMouseDragExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnDBLClickExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
#endif //_USE_AERO
		LRESULT OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAttachConsoles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMoveTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnSwitchView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnResizeView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloseView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSplit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMergeTabs(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMaximizeView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloneInNewTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMoveInNewTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnGroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnGroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloneTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnEditClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPasteSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSelectionKeyPressed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditStopScrolling(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditResumeScrolling(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnViewMenu(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewSearchBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnForwardMouseEvents(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAlwaysOnTop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFullScreen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnZoom(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSearchText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSearchSettings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwitchTransparency(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnShowContextMenu(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSendCtrlEvent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDumpBuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFontInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDiagnose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnExternalCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSnippet(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnLoadWorkspace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSaveWorkspace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		void AdjustWindowRect(CRect& rect);
		void AdjustWindowSize(ADJUSTSIZE as);
		void CloseTab(HWND hwndTabView);
		void SetActiveConsole(HWND hwndTabView, HWND hwndConsoleView);
		void PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam);
		void WriteConsoleInputToConsoles(KEY_EVENT_RECORD* pkeyEvent);
		void PasteToConsoles();
		void SendTextToConsoles(const wchar_t* pszText);
		bool GetAppActiveStatus(void) const { return this->m_bAppActive; }
		void SetProgress(unsigned long long ullProgressCompleted, unsigned long long ullProgressTotal);

		std::wstring FormatTitle(std::wstring strFormat, TabView * tabView, std::shared_ptr<ConsoleView> consoleView);

	private:

		void ActivateApp(void);
		bool CreateNewConsole(DWORD dwTabIndex, const ConsoleOptions& consoleOptions = ConsoleOptions());
		bool CreateNewTab(ConsoleViewCreate* consoleViewCreate, std::shared_ptr<TabData> tabData, int *nTab = nullptr);
		bool CreateSafeConsole();
		void CloseTab(CTabViewTabItem* pTabItem);

		void UpdateTabIcons();
		void UpdateTabTitle(std::shared_ptr<TabView> tabView);
		void UpdateTabsMenu();
		void UpdateOpenedTabsMenu(CMenu& tabsMenu, bool bContextual);
		void UpdateSnippetsMenu();
		void UpdateMenuHotKeys(void);
		void UpdateStatusBar();
		void SetWindowStyles(void);
		void DockWindow(DockPosition dockPosition);
		void SetZOrder(ZOrder zOrder);
		HWND GetDesktopWindow();

		void SetWindowIcons();
		void UpdateUI();

		void ShowMenu      (bool bShow);
		void ShowToolbar   (bool bShow);
		void ShowSearchBar (bool bShow);
		void ShowTabs      (bool bShow);
		void ShowStatusbar (bool bShow);
		void ShowFullScreen(bool bShow);

		void ResizeWindow();
		void SetMargins();
		void SetTransparency();
		void CreateAcceleratorTable();
		void RegisterGlobalHotkeys();
		void UnregisterGlobalHotkeys();
		void CreateStatusBar();
		BOOL SetTrayIcon(DWORD dwMessage);
		void ShowHideWindow(ShowHideWindowAction action = ShowHideWindowAction::SHWA_SWITCH);

		void LoadSearchMRU();
		void SaveSearchMRU();
		void AddSearchMRU(CString& item);

		static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM lpData);
		static BOOL CALLBACK MonitorEnumProcDiag(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData);
		static BOOL CALLBACK ConsoleEnumWindowsProc(HWND hwnd, LPARAM lParam);

		bool LoadWorkspace(const wstring& filename);
		bool SaveWorkspace(const wstring& filename);

	public:
		LRESULT CreateInitialTabs
		(
			const CommandLineOptions& commandLineOptions
		);
		LRESULT OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		bool					m_bOnCreateDone;

	private:
		CommandLineOptions m_commandLineOptions;

		std::shared_ptr<TabView>	m_activeTabView;

		bool m_bMenuVisible;
		bool m_bMenuChecked;
		bool m_bToolbarVisible;
		bool m_bSearchBarVisible;
		bool m_bStatusBarVisible;
		bool m_bTabsVisible;
		bool m_bFullScreen;

		DockPosition	m_dockPosition;
		ZOrder			m_zOrder;

		TabViewMap	m_tabs;
		Mutex			m_tabsMutex;

		CMenu			m_tabsMenu;
		CMenu			m_tabsRPopupMenu;
		CMenu			m_tabsR2PopupMenu;
		CMenu			m_openedTabsMenu;
		CMenu			m_snippetsMenu;

		CIcon			m_icon;
		CIcon			m_smallIcon;

		std::wstring m_strWindowTitle;

		DWORD			m_dwWindowWidth;
		DWORD			m_dwWindowHeight;
		DWORD			m_dwResizeWindowEdge;
		DWORD			m_dwScreenDpi;

		bool                m_bUseCursorPosition;
		bool			m_bAppActive;
		bool			m_bShowingHidingWindow;
		bool			m_bRestoringWindow;
		CRect			m_rectWndNotFS;

		CToolBarCtrl        m_toolbar;
		CComboBoxEx         m_cb;
#ifdef _USE_AERO
		aero::CReBarCtrl    m_rebar;
		aero::CToolBarCtrl  m_searchbar;
		aero::CEdit         m_searchedit;
#else
		CToolBarCtrl        m_searchbar;
		CEdit               m_searchedit;
#endif

		CAccelerator	m_acceleratorTable;
		UINT			m_uTaskbarRestart;
		UINT			m_uReloadDesktopImages;
#ifdef _USE_AERO
		UINT			m_uTaskbarButtonCreated;
#endif
		CMultiPaneStatusBarCtrl m_statusBar;
		CMenuHandle m_contextMenu;

		MARGINS m_Margins;

		int     m_nTabRightClickSel;

		int     m_nFullSreen1Bitmap;
		int     m_nFullSreen2Bitmap;
		HWND    m_hwndPreviousForeground;

		CComPtr<ITaskbarList3> m_pTaskbarList;

		SnippetCollection m_snippetCollection;
};

//////////////////////////////////////////////////////////////////////////////
