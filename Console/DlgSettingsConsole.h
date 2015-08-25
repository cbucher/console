
#pragma once

#include "DlgSettingsBase.h"
#include "CFileNameEdit.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsConsole 
	: public DlgSettingsBase
{
	public:

		DlgSettingsConsole(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsConsole)
			DDX_TEXT(IDC_SHELL, m_strShell)
			DDX_TEXT(IDC_INIT_DIR, m_strInitialDir)
			DDX_CHECK(IDC_CHECK_START_HIDDEN, m_consoleSettings.bStartHidden)
			DDX_CHECK(IDC_CHECK_AUTO_SAVE_SESSION, m_consoleSettings.bAutoSaveSession)
			DDX_CHECK(IDC_CHECK_AUTO_LOAD_SESSION, m_consoleSettings.bAutoLoadSession)
			DDX_UINT(IDC_CHANGE_REFRESH, m_consoleSettings.dwChangeRefreshInterval)
			DDX_UINT(IDC_REFRESH, m_consoleSettings.dwRefreshInterval)
			DDX_UINT_RANGE(IDC_ROWS, m_consoleSettings.dwRows, static_cast<DWORD>(MIN_WINDOW_ROWS), static_cast<DWORD>(MAX_WINDOW_ROWS))
			DDX_UINT_RANGE(IDC_COLUMNS, m_consoleSettings.dwColumns, static_cast<DWORD>(MIN_WINDOW_COLUMNS), static_cast<DWORD>(MAX_WINDOW_COLUMNS))
			DDX_CHECK(IDC_CHECK_SAVE_SIZE, m_consoleSettings.bSaveSize)
			DDX_UINT_RANGE(IDC_BUFFER_ROWS, m_consoleSettings.dwBufferRows, static_cast<DWORD>(MIN_BUFFER_ROWS), static_cast<DWORD>(MAX_BUFFER_ROWS))
			DDX_UINT_RANGE(IDC_BUFFER_COLUMNS, m_consoleSettings.dwBufferColumns, static_cast<DWORD>(MIN_BUFFER_COLUMNS), static_cast<DWORD>(MAX_BUFFER_COLUMNS))
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsConsole)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_BTN_BROWSE_SHELL, BN_CLICKED, OnClickedBtnBrowseShell)
			COMMAND_HANDLER(IDC_BTN_BROWSE_DIR, BN_CLICKED, OnClickedBtnBrowseDir)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		virtual void OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data);

		ConsoleSettings m_consoleSettings;

	private:

		CString         m_strShell;
		CString         m_strInitialDir;

		CFileNameEdit   m_ShellEdit;
		CFileNameEdit   m_InitialDirEdit;
};

//////////////////////////////////////////////////////////////////////////////
