#pragma once

#include "Cursors.h"
#include "SelectionHandler.h"

//////////////////////////////////////////////////////////////////////////////

#define	FLASH_TAB_TIMER		444

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class MainFrame;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleView
	: public CWindowImpl<ConsoleView, CWindow, CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL, 0> >
	, public CursorCharDrawer
{
	public:
//		DECLARE_WND_CLASS(NULL)
		DECLARE_WND_CLASS_EX(L"Console_2_View", CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, COLOR_WINDOW)
//		DECLARE_WND_CLASS_EX(L"Console_2_View", CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

		ConsoleView(MainFrame& mainFrame, HWND hwndTabView, std::shared_ptr<TabData> tabDataTab, std::shared_ptr<TabData> tabDataShell, DWORD dwRows, DWORD dwColumns, const ConsoleOptions& consoleOptions);
		~ConsoleView();

		BEGIN_MSG_MAP(ConsoleView)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSKEYUP, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_KEYDOWN, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_KEYUP, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_CHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_DEADCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSDEADCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
			MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseButton)
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_MBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_XBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_XBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
			MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_INPUTLANGCHANGEREQUEST, OnInputLangChangeRequest)
			MESSAGE_HANDLER(WM_INPUTLANGCHANGE, OnInputLangChange)
			MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
			MESSAGE_HANDLER(UM_UPDATE_CONSOLE_VIEW, OnUpdateConsoleView)

			MESSAGE_HANDLER(WM_IME_COMPOSITION, OnIMEComposition)
			MESSAGE_HANDLER(WM_IME_STARTCOMPOSITION, OnIMEStartComposition)
			MESSAGE_HANDLER(WM_IME_ENDCOMPOSITION, OnIMEEndComposition)

			MESSAGE_HANDLER(WM_PRINTCLIENT, OnPrintClient)
		END_MSG_MAP()

//		Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnPrintClient(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnConsoleFwdMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnMouseButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnInputLangChangeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnInputLangChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnUpdateConsoleView(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnIMEComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnIMEStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnIMEEndComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		virtual void RedrawCharOnCursor(CDC& dc);

		LRESULT OnSelectionKeyPressed(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		void GetRect(CRect& clientRect);
		void GetRectMax(CRect& clientMaxRect);
		void AdjustRectAndResize(ADJUSTSIZE as, CRect& clientRect, DWORD dwResizeWindowEdge);
		CPoint GetCellSize() { return CPoint(m_nCharWidth, m_nCharHeight); };

		ConsoleHandler& GetConsoleHandler() { return m_consoleHandler; }
		std::shared_ptr<TabData> GetTabData() { return m_tabDataShell; }

		bool GetConsoleWindowVisible() const { return m_bConsoleWindowVisible; }
		void SetConsoleWindowVisible(bool bVisible);

		inline bool GetForwardMouseEvents() const { return m_bForwardMouseEvents; }
		inline void SetForwardMouseEvents(bool value) { m_bForwardMouseEvents = value; }

		inline const std::wstring & GetImageCopyright() const { static const std::wstring empty(L""); return m_background? m_background->imageData.strCopyright : empty; }
		inline const std::wstring & GetImageCopyrightLink() const { static const std::wstring empty(L""); return m_background ? m_background->imageData.strCopyrightLink : empty; }

		void SetAppActiveStatus(bool bAppActive);

		static bool RecreateFont(DWORD dwNewFontSize, bool boolZooming, DWORD dwScreenDpi);
		inline DWORD GetFontZoom(void) const { return m_dwFontZoom; }
		void RecreateOffscreenBuffers(ADJUSTSIZE as);
		void Repaint(bool bFullRepaint);
		bool MainframeMoving();

		void SetParentTab(HWND hwndTabView, std::shared_ptr<TabData> tabDataTab);

		void SetResizing(bool bResizing);
		void SetActive(bool bActive);
		const CString& GetUser() const { return m_strUser; }
		bool  IsRunningAsUserNetOnly() const { return m_strUser.GetLength() > 0 && m_boolNetOnly; }
		bool  IsRunningAsUser() const { return m_strUser.GetLength() > 0 && !m_boolNetOnly; }

		CString GetConsoleCommand();
		void    GetProgress(unsigned long long & ullProgressCompleted, unsigned long long & ullProgressTotal);

		void Clear();
		void Copy(const CPoint* pPoint = NULL);
		void SelectAll();
		void ClearSelection();
		void PasteSelection();
		DWORD GetSelectionSize(void) const { return m_selectionHandler->GetSelectionSize(); }

		bool CanCopy() const { return m_selectionHandler->GetState() == SelectionHandler::selstateSelected; }
		bool CanClearSelection() const { return m_selectionHandler->GetState() > SelectionHandler::selstateNoSelection; }
		bool CanPaste() const { return (m_selectionHandler->GetState() == SelectionHandler::selstateNoSelection) && (::IsClipboardFormatAvailable(CF_UNICODETEXT) || ::IsClipboardFormatAvailable(CF_TEXT) || ::IsClipboardFormatAvailable(CF_OEMTEXT)) ; }

		void DumpBuffer();
		void InitializeScrollbars();

		inline const std::wstring& GetExceptionMessage() const { return m_exceptionMessage; }

		inline bool IsGrouped() const { return m_boolIsGrouped; }
		void Group(bool b) { m_boolIsGrouped = b; }

		inline DWORD GetBasePriority() const { return m_consoleOptions.dwBasePriority; }
		inline const std::wstring& GetShellArguments() const { return m_consoleOptions.strShellArguments; }
		inline const ConsoleOptions& GetOptions() const { return m_consoleOptions; }

		void DoScroll(int nType, int nScrollCode, int nThumbPos);

		void SearchText(CString& text, bool bNext);

		std::wstring GetFontInfo(void) const;

		static inline int GetCharWidth(void) { return m_nCharWidth; }
		static inline int GetCharHeight(void) { return m_nCharHeight; }

		bool SaveWorkspace(CComPtr<IXMLDOMElement>& pViewElement);

#ifdef _USE_AERO
		inline UINT_PTR GetBackGroundIDTimerEvent() const { return m_background ? m_background->originalImage->getIDTimerEvent() : 0; }
#endif
		inline void SetBackgroundChanged() { m_bBackgroundChanged = true; }
	private:

		void OnConsoleChange(bool bResize);
		void OnConsoleClose();

		void SetBackground();
		void CreateOffscreenBuffers();
		void CreateOffscreenBitmap(CDC& cdc, const CRect& rect, CBitmap& bitmap);
		static bool CreateFont(const wstring& strFontName);

		DWORD GetBufferDifference();

		void UpdateTitle();

		void RepaintText(CDC& dc);
		void RepaintTextChanges(CDC& dc);
		void RowTextOut(CDC& dc, DWORD dwRow);
		inline void ExtTextOut(CDC& dc, CRect & rect, std::wstring & strText, COLORREF colorFG);

		void BitBltOffscreen(bool bOnlyCursor = false);
		void UpdateOffscreen(const CRect& rectBlit);

		bool TranslateKeyDown(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/);
		void ForwardMouseClick(UINT uMsg, WPARAM wParam, const CPoint& point);

		COORD GetConsoleCoord(const CPoint& clientPoint, bool bStartSelection = false);

	private:

		MainFrame& m_mainFrame;
		HWND       m_hwndTabView;

		ConsoleOptions m_consoleOptions;

		bool	m_bInitializing;
		bool	m_bResizing;
		bool	m_bAppActive;
		bool	m_bActive;
		bool	m_bMouseTracking;
		bool	m_bNeedFullRepaint;
		bool	m_bBackgroundChanged;
		bool	m_bConsoleWindowVisible;
		bool  m_boolIsGrouped;
		bool  m_boolImmComposition;
		bool  m_bForwardMouseEvents;

		DWORD	m_dwStartupRows;
		DWORD	m_dwStartupColumns;
		DWORD m_dwVScrollMax;
		int   m_nVWheelDelta;

		bool	m_bShowVScroll;
		bool	m_bShowHScroll;

		CString	m_strUser;
		bool	m_boolNetOnly;


		ConsoleHandler	m_consoleHandler;

		std::unique_ptr<CharInfo[]> m_screenBuffer;
		DWORD	                      m_dwScreenRows;
		DWORD	                      m_dwScreenColumns;
		std::unique_ptr<INT[]>      m_dxWidths;
		std::unique_ptr<INT[]>      m_dxLigatureWidths;
		std::unique_ptr<UINT[]>     m_orders;
		std::unique_ptr<wchar_t[]>  m_glyphs;

		ConsoleSettings&				m_consoleSettings;
		AppearanceSettings&				m_appearanceSettings;
		HotKeys&						m_hotkeys;

		std::shared_ptr<TabData>    m_tabDataShell;
		std::shared_ptr<TabData>    m_tabDataTab;
		std::shared_ptr<BackgroundImage>		m_background;

		CBrush							m_backgroundBrush;

		std::shared_ptr<Cursor>				m_cursor;
		std::shared_ptr<Cursor>				m_cursorDBCS;
		std::unique_ptr<SelectionHandler>	m_selectionHandler;

		MouseSettings::Command			m_mouseCommand;

		bool							m_bFlashTimerRunning;
		DWORD							m_dwFlashes;

		// since message handlers are not exception-safe,
		// we'll store error messages thrown during OnCreate
		// handler here...
		std::wstring        m_exceptionMessage;

		CString             m_strUACPrefix;

		COORD               m_coordSearchText;

#ifdef CONSOLEZ_CHRONOS
		std::chrono::high_resolution_clock::time_point m_timePoint1;
#endif // CONSOLEZ_CHRONOS

		std::chrono::system_clock::time_point m_startTime;

// static members
private:

  /*static*/ CDC        m_dcOffscreen;
  /*static*/ CDC        m_dcText;

  /*static*/ CBitmap    m_bmpOffscreen;
  /*static*/ CBitmap    m_bmpText;

	enum FontTextType
	{
		FontTextNormal          = 0,
		FontTextBright          = 1,
		FontTextUnderline       = 2,
		FontTextBrightUnderline = 3,
	};

  static CFont          m_fontText[4];

  static int            m_nCharHeight;
  static int            m_nCharWidth;
  static int            m_nVScrollWidth;
  static int            m_nHScrollWidth;
  static int            m_nVInsideBorder;
  static int            m_nHInsideBorder;
  static DWORD          m_dwFontSize;
  static DWORD          m_dwFontZoom;
  static DWORD          m_dwScreenDpi;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleViewCreate
{
	enum TYPE
	{
		CREATE,
		ATTACH,
		LOAD_WORKSPACE,
		MOVE
	} type;

	union
	{
		UserCredentials* userCredentials;
		DWORD            dwProcessId;
	} u;

	ConsoleOptions consoleOptions;

	std::shared_ptr<TabData> tabDataShell;

	CComPtr<IXMLDOMElement> pTabElement;

	std::shared_ptr<ConsoleView> consoleView;
};

//////////////////////////////////////////////////////////////////////////////
