#pragma once

#include "ConsoleView.h"

typedef std::map< HWND, std::shared_ptr<ConsoleView> > ConsoleViewMap;

class TabView
  : public CWindowImpl<TabView>
  , public CMultiSplitImpl<TabView>
{
public:
  DECLARE_WND_CLASS_EX(L"Console_2_TabView", CS_DBLCLKS, COLOR_WINDOW)

  TabView(MainFrame& mainFrame, std::shared_ptr<TabData> tabData, const std::wstring& strTitle);
  ~TabView();

  BOOL PreTranslateMessage(MSG* pMsg);

  typedef CMultiSplitImpl< TabView > multisplitClass;
  BEGIN_MSG_MAP(TabView)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_HANDLER (WM_SIZE, OnSize)

    COMMAND_RANGE_HANDLER(ID_SCROLL_UP, ID_SCROLL_ALL_RIGHT, OnScrollCommand)

    CHAIN_MSG_MAP (multisplitClass)
/*
    {
      std::shared_ptr<ConsoleView> consoleView = this->GetActiveConsole(_T(__FUNCTION__));
      if( consoleView )
      {
        if( consoleView->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult) )
          return true;
      }
    }
*/
  END_MSG_MAP()

  //		Handler prototypes (uncomment arguments if needed):
  //		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  //		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  //		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
  LRESULT OnEraseBackground (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
  LRESULT OnSize (UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled);
  LRESULT OnScrollCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);

  virtual void OnPaneChanged(void);

  virtual void OnSplitBarMove(HWND hwndPane0, HWND hwndPane1, bool /*boolEnd*/);

  std::shared_ptr<ConsoleView> GetActiveConsole(const TCHAR*);
  std::shared_ptr<TabData>     GetTabData() { return m_tabData; }

  void SetTitle(const std::wstring& strTitle);
  const std::wstring& GetTitle() const { return m_strTitle; }
  CIcon& GetIcon(bool bBigIcon = true) { return bBigIcon ? m_bigIcon : m_smallIcon; }
  void SetActive(bool bActive);
  void SetAppActiveStatus(bool bAppActive);
  void SetResizing(bool bResizing);
  bool MainframeMoving();
  void Repaint(bool bFullRepaint);
#ifdef _USE_AERO
	void RepaintBackground(UINT_PTR nIDTimerEvent);
#endif
  void InitializeScrollbars();
  void AdjustRectAndResize(ADJUSTSIZE as, CRect& clientRect, DWORD dwResizeWindowEdge);
  void GetRect(CRect& clientRect);
	void UpdateIcons();

  void Split(CMultiSplitPane::SPLITTYPE);
	void MaximizeView(WORD wID);
  void Merge(std::shared_ptr<TabView>, CMultiSplitPane::SPLITTYPE);
  bool CloseView(HWND hwnd, bool boolDetach, bool boolDestroyWindow, bool& boolTabClosed);
  void SwitchView(WORD wID);
  void ResizeView(WORD wID);
  void SetActiveConsole(HWND hwnd);
	void UpdateTheme();

  void PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam);
  void WriteConsoleInputToConsoles(KEY_EVENT_RECORD* pkeyEvent);
  void SendTextToConsoles(const wchar_t* pszText);
  void SendCtrlCToConsoles();

  inline bool IsGrouped() const { return m_boolIsGrouped; }
  void Group(bool b);

  inline size_t GetViewsCount(void) const { return m_views.size(); }

	void Diagnose(HANDLE hFile);

	bool SaveWorkspace(CComPtr<IXMLDOMElement>& pTabElement);

private:
	HWND CreateNewConsole(ConsoleViewCreate* consoleViewCreate);

	bool LoadWorkspace(CComPtr<IXMLDOMElement>& pElement, CMultiSplitPane* pane);
	bool SaveWorkspace(CComPtr<IXMLDOMElement>& pElement, CMultiSplitPane* pane, const CComBSTR& ident);

private:
  MainFrame&          m_mainFrame;
  ConsoleViewMap      m_views;
  Mutex               m_viewsMutex;
  std::shared_ptr<TabData> m_tabData;
  CIcon               m_bigIcon;
  CIcon               m_smallIcon;
  bool                m_boolIsGrouped;
	std::wstring        m_strTitle;

  // static members
private:

};

//////////////////////////////////////////////////////////////////////////////
