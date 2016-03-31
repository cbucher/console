
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsAppearance 
	: public DlgSettingsBase
{
	public:

		DlgSettingsAppearance(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsAppearance)
			DDX_TEXT(IDC_WINDOW_TITLE, m_strWindowTitle)
			DDX_TEXT(IDC_MAIN_TITLE_FORMAT, m_strMainTitleFormat)
			DDX_TEXT(IDC_TAB_TITLE_FORMAT, m_strTabTitleFormat)
			DDX_CHECK(IDC_CHECK_USE_TAB_TITLE, m_windowSettings.bUseTabTitles)
			DDX_CHECK(IDC_CHECK_TRIM_TAB_TITLES, m_bTrimTabTitles)
			DDX_UINT(IDC_TRIM_TAB_TITLES, m_windowSettings.dwTrimTabTitles)
			DDX_UINT(IDC_TRIM_TAB_TITLES_RIGHT, m_windowSettings.dwTrimTabTitlesRight)
			DDX_TEXT(IDC_WINDOW_ICON, m_strWindowIcon)
			DDX_CHECK(IDC_CHECK_USE_TAB_ICON, m_windowSettings.bUseTabIcon)
			DDX_CHECK(IDC_CHECK_MAXIMIZE, m_bShowMaximized)
			DDX_CHECK(IDC_CHECK_POSITION, m_bUsePosition)
			DDX_INT(IDC_POS_X, m_nX)
			DDX_INT(IDC_POS_Y, m_nY)
			DDX_CHECK(IDC_CHECK_SAVE_POSITION, m_positionSettings.bSavePosition)
			DDX_CHECK(IDC_CHECK_SIZE, m_bUseSize)
			DDX_INT(IDC_POS_W, m_nW)
			DDX_INT(IDC_POS_H, m_nH)
			DDX_CHECK(IDC_CHECK_SAVE_SIZE, m_positionSettings.bSaveSize)
			DDX_CHECK(IDC_CHECK_SNAP, m_bSnapToEdges)
			DDX_INT(IDC_SNAP, m_positionSettings.nSnapDistance)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsAppearance)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_TITLE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_ICON, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_BTN_BROWSE_ICON, BN_CLICKED, OnClickedBtnBrowseIcon)
			COMMAND_HANDLER(IDC_CHECK_TRIM_TAB_TITLES, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_MAXIMIZE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_POSITION, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_SIZE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_SNAP, BN_CLICKED, OnClickedCheckbox)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void EnableControls();

	private:

		WindowSettings				m_windowSettings;
		PositionSettings			m_positionSettings;

		CString						m_strWindowTitle;
		CString						m_strMainTitleFormat;
		CString						m_strTabTitleFormat;
		bool						m_bTrimTabTitles;
		CString						m_strWindowIcon;

		bool                        		m_bShowMaximized;
		bool						m_bUsePosition;
		bool						m_bUseSize;
		int							m_nX;
		int							m_nY;
		int							m_nW;
		int							m_nH;
		bool						m_bSnapToEdges;

		CComboBox					m_comboDocking;
		CComboBox					m_comboZOrder;
};

//////////////////////////////////////////////////////////////////////////////
