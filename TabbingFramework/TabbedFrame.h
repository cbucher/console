/////////////////////////////////////////////////////////////////////////////
// TabbedFrame.h - Base template class for supporting a frame
//   window with multiple views that you switch between using
//   a "CustomTabCtrl" (such as CDotNetTabCtrl)
//
// Written by Daniel Bowen (dbowen@es.com)
// Copyright (c) 2002-2005 Daniel Bowen.
//
// Depends on CustomTabCtrl.h originally by Bjarke Viksoe (bjarke@viksoe.dk)
//  with the modifications by Daniel Bowen
//
// CCustomTabOwnerImpl -
//   MI class that helps implement the parent of the actual custom tab control window.
//   The class doesn't have a message map itself, and is meant
//   to be inherited from along-side a CWindowImpl derived class.
//   This class handles creation of the tab window as well as
//   adding, removing, switching and renaming tabs based on an HWND.
// CTabbedFrameImpl -
//   Base template to derive your specialized frame window class from to get
//   a frame window with multiple "view" child windows that you
//   switch between using a custom tab control (such as CDotNetTabCtrl).
// CTabbedPopupFrame -
//   Simple class deriving from CTabbedFrameImpl that is suitable
//   for implementing a tabbed "popup frame" tool window, with one or more views.
// CTabbedChildWindow -
//   Simple class deriving from CTabbedFrameImpl that is suitable
//   for implementing a tabbed child window, with one or more views.
//
//   
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever.
//
// If you find bugs, have suggestions for improvements, etc.,
// please contact the author.
//
// History (Date/Author/Description):
// ----------------------------------
//
// 2005/07/13: Daniel Bowen
// - Namespace qualify the use of more ATL and WTL classes.
//
// 2005/04/12: Daniel Bowen
// - CCustomTabOwnerImpl::CalcTabAreaHeight - 
//   * CDC dc = TabCtrl.GetDC();
//       should be
//     CClientDC dc(TabCtrl);
//
// 2005/04/08: Daniel Bowen
// - Generalize support for having the tab control automatically hidden
//   if the number of tabs is below a certain count.
// - CCustomTabOwnerImpl -
//   * Change OnAddFirstTab and OnRemoveLastTab to be more
//     general purpose. Have OnAddTab and OnRemoveTab instead,
//     and have them called for every AddTab or RemoveTab.
//   * Add KeepTabsHidden (overrideable). Previously only in CMDITabOwnerImpl.
//   * Add ShowTabControl, HideTabControl (overrideable)
//   * Add SetMinTabCountForVisibleTabs (method)
// - CTabbedFrameImpl -
//   * Pass in T instead of CTabbedFrameImpl<T,...> to CCustomTabOwnerImpl inheritance
//     (so you can override CCustomTabOwnerImpl overrideables in
//     CTabbedFrameImpl derived classes)
//   * Add ModifyTabStyles (method)
//   * OnSettingChange - Call CalcTabAreaHeight through pT
//   * SetTabAreaHeight - Support for "KeepTabsHidden"
//
// 2005/03/14: Daniel Bowen
// - Fix warnings when compiling for 64-bit.
//
// 2004/11/29: Daniel Bowen
// - Update all WM_NOTIFY handlers to check that the notification is
//   from the tab control (and not from a sibling like a list view control)
// - Update CTabbedFrameImpl::OnRemoveLastTab to call this->Invalidate()
//
// 2004/06/28: Daniel Bowen
// - CCustomTabOwnerImpl - 
//   * HighlightTab
// - Clean up warnings on level 4
//
// 2004/06/21: Peter Carlson
// - CCustomTabOwnerImpl - 
//   * UpdateTabCanClose
//
// 2004/05/14: Daniel Bowen
// - CTabbedFrameImpl - 
//   * Update OnClick handling so it only sets focus to the tab view
//     if the selected tab is being clicked. Without this update,
//     other code that tries to minimize flickering when switching
//     the active view doesn't get called.
//
// 2004/04/29: Daniel Bowen
// - Use LongToHandle with GetClassLong when getting HICON
// - CTabbedFrameImpl -
//   * Only forward focus to the active view if
//     the tab isn't currently capturing the mouse.
//   * Respond to NM_CLICK, CTCN_ACCEPTITEMDRAG and CTCN_CANCELITEMDRAG
//     from the tab control, and set focus to the tab item's view
//  
// 2004/02/03: Daniel Bowen
// - CTabbedFrameImpl -
//   * Add new Set/GetForwardNotifications in case you want the parent of the tab
//     window to forward notifications on to its parent.  A good example where
//     you might want to use this would be with CTabbedChildWindow.
//
// 2004/01/19: Daniel Bowen
// - CTabbedFrameImpl -
//   * Have new "CHAIN_ACTIVETABVIEW_CHILD_COMMANDS" and "CHAIN_ACTIVETABVIEW_CHILD_COMMANDS2"
//     macro that is used to forward WM_COMMAND messages to the active view of a tab window
//     from outside the implementation of that tab window (such as in the Main Frame).
//     
// 2003/06/27: Daniel Bowen
// - CCustomTabOwnerImpl -
//   * Remove WTL:: scope off of CImageList member.
// - CTabbedFrameImpl -
//   * Have new "CHAIN_ACTIVETABVIEW_COMMANDS" macro that is used to forward
//     WM_COMMAND messages to the active view.  This is done after
//     the CHAIN_MSG_MAP(baseClass), so be careful if the base
//     class also handles WM_COMMAND messages (the default
//     CFrameWindowImpl does not, and neither does CMDIChildWindowImpl
//     or CTabbedMDIChildWindowImpl).
//   * New "GetActiveView" that returns what CTabbedFrameImpl
//     thinks is the active view.
//   * Replace
//      DECLARE_FRAME_WND_CLASS(_T("TabbedFrame"), 0)
//     with
//      DECLARE_FRAME_WND_CLASS_EX(_T("TabbedFrame"), 0, 0, COLOR_APPWORKSPACE)
//     (gets rid of CS_DBLCLKS, CS_HREDRAW and CS_VREDRAW, sets background brush)
//   * Support "empty" tabbed frame (have window class brush, 
//     let default handling of WM_ERASEBKGND happen if no active view,
//     and NULL out m_hWndActive in OnRemoveLastTab).
// - CTabbedPopupFrame -
//   * Replace
//      DECLARE_FRAME_WND_CLASS(_T("TabbedPopupFrame"), 0)
//     with
//      DECLARE_FRAME_WND_CLASS_EX(_T("TabbedPopupFrame"), 0, 0, COLOR_APPWORKSPACE)
//     (gets rid of CS_DBLCLKS, CS_HREDRAW and CS_VREDRAW, sets background brush)
// - CTabbedChildWindow -
//   * Replace
//      DECLARE_WND_CLASS(_T("TabbedChildWindow"))
//     with
//      DECLARE_FRAME_WND_CLASS_EX(_T("TabbedChildWindow"), 0, 0, COLOR_APPWORKSPACE)
//     (gets rid of CS_DBLCLKS, CS_HREDRAW and CS_VREDRAW, sets background brush)
//
// 2003/02/27: Daniel Bowen
// - Use _U_STRINGorID instead of WTL::_U_STRINGorID.
//   For VC7, this means you must #define _WTL_NO_UNION_CLASSES
//   before including the WTL header files, or you will
//   get compile errors (the ATL7 union classes are defined
//   in atlwin.h).
//
// 2002/11/27: Daniel Bowen
// - CTabbedFrameImpl::GetTabStyles needs to return DWORD, not bool 
//
// 2002/09/25: Daniel Bowen
// - CTabbedFrameImpl -
//   * Expose "SetTabStyles" and "GetTabStyles" so that you can change
//     the tab related styles to something different than the default
// - CTabbedPopupFrame -
//   * Expose "SetCloseCommand" and "GetCloseCommand" so that
//     instead of destroying the window when the close button
//     on the popup frame is pushed, a command ID of your choice
//     is sent to the parent (such as a menu ID that corresponds
//     to toggling the visibility of the popup frame)
//
// 2002/06/26: Daniel Bowen
// - New "CTabbedChildWindow" that derives from CTabbedFrameImpl.
//   You can use this class when you want a child window to
//   use a tab control to switch between multiple views
// - Provide "PreTranslateMessage" function in CTabbedPopupFrame
//   (and the new CTabbedChildWindow)
// - CCustomTabOwnerImpl -
//   * Rename "GetTabs" method to "GetTabCtrl"
//   * Rename member "m_tabs" to "m_TabCtrl"
//   * Rename template argument "TTab" to "TTabCtrl"
//   * Rename "ShowTabs" and "HideTabs" overrideables to "OnAddFirstTab" and "OnRemoveLastTab",
//     and change the place that calls these to live up to those new names
//   * Remove GetCurSel (just call GetTabCtrl().GetCurSel() instead)
//   * DisplayTab - 
//     + Add new parameter that says whether to use the window's icon.
//       If TRUE, the icon is requested first by sending the window WM_GETICON
//       looking for the "small" icon, then asking the window class for a small icon.
//       If no small icon is found, the same procedure is used to look for the
//       "big" icon.
//     + Call "SetCurSel" even if the tab to display has the same index
//       as the current selection
//     + Call "OnAddFirstTab" (which was "ShowTabs") only when the count
//       of tabs goes from 0 to 1.
//  
// 2002/06/12: Daniel Bowen
// - Publish codeproject article.  For history prior
//   to the release of the article, please see the article
//   and the section "Note to previous users"

#ifndef __WTL_TABBED_FRAME_H__
#define __WTL_TABBED_FRAME_H__

#pragma once

#ifndef __cplusplus
	#error TabbedFrame.h requires C++ compilation
#endif

#ifndef __ATLAPP_H__
	#error TabbedFrame.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error TabbedFrame.h requires atlwin.h to be included first
#endif

#ifndef __ATLFRAME_H__
	#error TabbedFrame.h requires atlframe.h to be included first
#endif

#ifndef __CUSTOMTABCTRL_H__
#include "CustomTabCtrl.h"
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CCustomTabOwnerImpl
//  an MI template to help implement the owner window that uses CustomTabCtrl
//  to switch between windows / views
//
/////////////////////////////////////////////////////////////////////////////

template <class T, class TTabCtrl>
class CCustomTabOwnerImpl
{
// Member variables
protected:
	TTabCtrl m_TabCtrl;
	WTL::CImageList m_ImageList;
	int m_cxImage, m_cyImage;
	int m_nTabAreaHeight;
	size_t m_nMinTabCountForVisibleTabs;
	bool m_bKeepTabsHidden;

// Constructors
public:
	CCustomTabOwnerImpl() :
		m_cxImage(16),
		m_cyImage(16),
		m_nTabAreaHeight(24),
		m_nMinTabCountForVisibleTabs(1)
	{
		m_bKeepTabsHidden = (m_nMinTabCountForVisibleTabs > 0);
	}

// Overrideables
public:

	void OnAddTab(size_t nNewTabCount)
	{
		T* pT = static_cast<T*>(this);

		// NOTE: Derived classes should call this base class version as well
		if(nNewTabCount == m_nMinTabCountForVisibleTabs)
		{
			pT->ShowTabControl();
		}
	}

	void OnRemoveTab(size_t nNewTabCount)
	{
		T* pT = static_cast<T*>(this);

		// NOTE: Derived classes should call this base class version as well
		if((nNewTabCount+1) == m_nMinTabCountForVisibleTabs)
		{
			pT->HideTabControl();
		}
		else if(nNewTabCount == 0)
		{
			pT->Invalidate();
		}
	}

	void KeepTabsHidden(bool bKeepTabsHidden = true)
	{
		if(m_bKeepTabsHidden != bKeepTabsHidden)
		{
			m_bKeepTabsHidden = bKeepTabsHidden;

			// CalcTabAreaHeight will end up doing UpdateLayout and Invalidate
			T* pT = static_cast<T*>(this);
			pT->CalcTabAreaHeight();
		}
	}

	void ShowTabControl(void)
	{
		T* pT = static_cast<T*>(this);
		pT->KeepTabsHidden(false);
	}

	void HideTabControl(void)
	{
		T* pT = static_cast<T*>(this);
		pT->KeepTabsHidden(true);
	}

	void SetTabAreaHeight(int nNewTabAreaHeight)
	{
		if(m_nTabAreaHeight != nNewTabAreaHeight)
		{
			m_nTabAreaHeight = nForceTabAreaHeight;

			/*
			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
			Invalidate();
			*/
		}
	}

	int CalcTabAreaHeight2(void)
	{
		// Dynamically figure out a reasonable tab area height
		// based on the tab's font metrics

		const int nNominalHeight = 24;
		const int nNominalFontLogicalUnits = 11;	// 8 point Tahoma with 96 DPI

		// Initialize nFontLogicalUnits to the typical case
		// appropriate for CDotNetTabCtrl
		LOGFONT lfIcon = { 0 };
		::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0);
		int nFontLogicalUnits = -lfIcon.lfHeight;

		// Use the actual font of the tab control
		TTabCtrl& TabCtrl = this->GetTabCtrl();
		if(TabCtrl.IsWindow())
		{
			HFONT hFont = TabCtrl.GetFont();
			if(hFont != NULL)
			{
				CClientDC dc(TabCtrl);
				CFontHandle hFontOld = dc.SelectFont(hFont);
				TEXTMETRIC tm = {0};
				dc.GetTextMetrics(&tm);
				nFontLogicalUnits = tm.tmAscent;
				dc.SelectFont(hFontOld);
			}
		}

		return nNominalHeight + ( ::MulDiv(nNominalHeight, nFontLogicalUnits, nNominalFontLogicalUnits) - nNominalHeight ) / 2;
	}

	// A derived class might not need to override this although they can.
	// (but they will probably need to specialize SetTabAreaHeight)
	void CalcTabAreaHeight(void)
	{
		int nNewTabAreaHeight = CalcTabAreaHeight2();

		T* pT = static_cast<T*>(this);
		pT->SetTabAreaHeight(nNewTabAreaHeight);
	}

// Methods
public:
	TTabCtrl& GetTabCtrl(void)
	{
		return m_TabCtrl;
	}

	int GetTabAreaHeight(void) const
	{
		return m_nTabAreaHeight;
	}

	void SetMinTabCountForVisibleTabs(size_t nMinTabCountForVisibleTabs)
	{
		if(m_nMinTabCountForVisibleTabs != nMinTabCountForVisibleTabs)
		{
			T* pT = static_cast<T*>(this);
			m_nMinTabCountForVisibleTabs = nMinTabCountForVisibleTabs;
			size_t nCurrentTabCount = m_TabCtrl.GetItemCount();
			if(nCurrentTabCount < m_nMinTabCountForVisibleTabs)
			{
				pT->HideTabControl();
			}
			else
			{
				pT->ShowTabControl();
			}
		}
	}

	void CreateTabWindow(HWND hWndTabParent, RECT rcTab, DWORD dwOtherStyles = CTCS_TOOLTIPS)
	{
		if(m_TabCtrl.IsWindow())
		{
			m_TabCtrl.DestroyWindow();
		}

		BOOL bCreate = FALSE;
		bCreate = m_ImageList.Create(m_cxImage, m_cyImage, ILC_COLOR32 | ILC_MASK, 4, 4);
		if(bCreate)
		{
			m_TabCtrl.SetImageList(m_ImageList);
		}

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwOtherStyles;

		m_TabCtrl.Create(hWndTabParent, rcTab, NULL, dwStyle);

		T* pT = static_cast<T*>(this);
		pT->CalcTabAreaHeight();
	}

	BOOL DestroyTabWindow()
	{
		return m_ImageList.Destroy();
	}

	// AddBitmap (with a couple of overloaded versions)
	int AddBitmap(HBITMAP hBitmap, HBITMAP hBitmapMask = NULL)
	{
		return m_ImageList.Add(hBitmap, hBitmapMask);
	}

	int AddBitmap(HBITMAP hBitmap, COLORREF crMask)
	{
		return m_ImageList.Add(hBitmap, crMask);
	}

	int AddBitmap(_U_STRINGorID bitmap, COLORREF crMask, HMODULE hModule = _Module.GetResourceInstance())
	{
		HBITMAP hBitmap = (HBITMAP)::LoadImage(
			hModule,
			bitmap.m_lpstr,
			IMAGE_BITMAP, 0, 0, LR_SHARED);
		return hBitmap ? m_ImageList.Add(hBitmap, crMask) : -1;
	}

	int AddBitmap(_U_STRINGorID bitmap, HBITMAP hBitmapMask = NULL, HMODULE hModule = _Module.GetResourceInstance())
	{
		HBITMAP hBitmap = (HBITMAP)::LoadImage(
			hModule,
			bitmap.m_lpstr,
			IMAGE_BITMAP, 0, 0, LR_SHARED);
		return hBitmap ? m_ImageList.Add(hBitmap, hBitmapMask) : -1;
	}

	// AddIcon (with a couple of overloaded versions)
	int AddIcon(HICON hIcon)
	{
		return m_ImageList.AddIcon(hIcon);
	}

	int AddIcon(_U_STRINGorID icon, HMODULE hModule = _Module.GetResourceInstance())
	{
		HICON hIcon = (HICON)::LoadImage(
			hModule,
			icon.m_lpstr,
			IMAGE_ICON, m_cxImage, m_cyImage, LR_SHARED);
		return hIcon ? m_ImageList.AddIcon(hIcon) : -1;
	}

	// AddTabWithBitmap (with a couple of overloaded versions)
	int AddTabWithBitmap(HWND hWnd, LPCTSTR sTabText, HBITMAP hBitmap, HBITMAP hBitmapMask = NULL)
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddBitmap(hBitmap, hBitmapMask);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	int AddTabWithBitmap(HWND hWnd, LPCTSTR sTabText, HBITMAP hBitmap, COLORREF crMask)
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddBitmap(hBitmap, crMask);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	int AddTabWithBitmap(HWND hWnd, LPCTSTR sTabText, _U_STRINGorID bitmap, HBITMAP hBitmapMask = NULL, HMODULE hModule = _Module.GetResourceInstance())
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddBitmap(bitmap, hBitmapMask, hModule);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	int AddTabWithBitmap(HWND hWnd, LPCTSTR sTabText, _U_STRINGorID bitmap, COLORREF crMask, HMODULE hModule = _Module.GetResourceInstance())
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddBitmap(bitmap, crMask, hModule);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	// AddTabWithIcon (with a couple of overloaded versions)
	int AddTabWithIcon(HWND hWnd, LPCTSTR sTabText, HICON hIcon)
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddIcon(hIcon);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	int AddTabWithIcon(HWND hWnd, LPCTSTR sTabText, _U_STRINGorID icon, HMODULE hModule = _Module.GetResourceInstance())
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nImageIndex = this->AddIcon(icon, hModule);

		return this->AddTab(hWnd, sTabText, nImageIndex);
	}

	// AddTab - either referencing an image in the image list, or no image used
	int AddTab(HWND hWnd, LPCTSTR sTabText, int nImageIndex = -1)
	{
		if(hWnd == NULL)
		{
			return -1;
		}

		int nNewTabIndex = -1;

		TTabCtrl::TItem* pItem = m_TabCtrl.CreateNewItem();
		if(pItem)
		{
			pItem->SetText(sTabText);
			pItem->SetImageIndex(nImageIndex);
			// NOTE: You must use a tab item class derived off of CCustomTabCtrl
			//  that tracks a view HWND, such as CTabViewTabItem
			pItem->SetTabView(hWnd);

			size_t nOldCount = m_TabCtrl.GetItemCount();

			// The tab control takes ownership of the new item
			nNewTabIndex = m_TabCtrl.InsertItem(nOldCount, pItem);

			size_t nNewCount = m_TabCtrl.GetItemCount();

			if((nOldCount+1) == nNewCount)
			{
				T* pT = static_cast<T*>(this);
				pT->OnAddTab(nNewCount);
			}
		}

		return nNewTabIndex;
	}

	int DisplayTab(HWND hWnd, BOOL bAddIfNotFound = TRUE, BOOL bUseIcon = FALSE)
	{
		int nTab = -1;
		if(hWnd)
		{
			TTabCtrl::TItem tcItem;
			tcItem.SetTabView(hWnd);

			nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
			if((bAddIfNotFound == TRUE) && (nTab < 0))
			{
				// The corresponding tab doesn't exist yet. Create it.

				LPTSTR sWindowText = NULL;
				int cchWindowText = ::GetWindowTextLength(hWnd);
				if(cchWindowText > 0)
				{
					sWindowText = new TCHAR[cchWindowText + 1];
					if(sWindowText != NULL)
					{
						::GetWindowText(hWnd, sWindowText, cchWindowText+1);

						HICON hIcon = NULL;
						if(bUseIcon)
						{
							if(hIcon == NULL)
							{
								hIcon = (HICON) ::SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
							}
							if(hIcon == NULL)
							{
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
								hIcon = (HICON)::GetClassLongPtr(hWnd, GCLP_HICONSM);
#else
								hIcon = (HICON)LongToHandle(::GetClassLongPtr(hWnd, GCLP_HICONSM));
#endif
							}
							if(hIcon == NULL)
							{
								hIcon = (HICON) ::SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
							}
							if(hIcon == NULL)
							{
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
								hIcon = (HICON)::GetClassLongPtr(hWnd, GCLP_HICON);
#else
								hIcon = (HICON)LongToHandle(::GetClassLongPtr(hWnd, GCLP_HICON));
#endif
							}
						}

						if(hIcon == NULL)
						{
							nTab = AddTab(hWnd, sWindowText);
						}
						else
						{
							nTab = AddTabWithIcon(hWnd, sWindowText, hIcon);
						}

						delete [] sWindowText;
					}
				}

				if(nTab < 0)
				{
					// We had trouble getting the window text
					// TODO: What should we put for the text and/or icon
					//  in this case?
					ATLASSERT(0 && "Adding a tab where no name was provided");
					nTab = AddTab(hWnd, _T("Untitled"));
				}
			}

			if(nTab >= 0)
			{
				m_TabCtrl.SetCurSel(nTab);
			}

		}

		return nTab;
	}

	BOOL RemoveTab(HWND hWnd)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			size_t nOldCount = m_TabCtrl.GetItemCount();

			bSuccess = m_TabCtrl.DeleteItem(nTab);

			size_t nNewCount = m_TabCtrl.GetItemCount();

			T* pT = static_cast<T*>(this);
			if((nOldCount-1) == nNewCount)
			{
				pT->OnRemoveTab(nNewCount);
			}
		}

		return bSuccess;
	}

	BOOL UpdateTabText(HWND hWnd, LPCTSTR sText = NULL)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(nTab);
			_CSTRING_NS::CString sCurrentTabText = pItem->GetText();

			if(sText != NULL)
			{
				if(sCurrentTabText != sText)
				{
					bSuccess = pItem->SetText(sText);
					m_TabCtrl.UpdateLayout();
					m_TabCtrl.Invalidate();
				}
			}
			else
			{
				LPTSTR sWindowText = NULL;
				int cchWindowText = ::GetWindowTextLength(hWnd);
				if(cchWindowText > 0)
				{
					sWindowText = new TCHAR[cchWindowText + 1];
					if(sWindowText != NULL)
					{
						::GetWindowText(hWnd, sWindowText, cchWindowText+1);

						if(sWindowText != NULL &&
							sCurrentTabText != sWindowText)
						{
							bSuccess = pItem->SetText(sWindowText);
							m_TabCtrl.UpdateLayout();
							m_TabCtrl.Invalidate();
						}

						delete [] sWindowText;
					}
				}
			}
		}

		return bSuccess;
	}

	BOOL UpdateTabImage(HWND hWnd, int nImageIndex = -1)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(nTab);
			int nCurrentImageIndex = pItem->GetImageIndex();
			if(nCurrentImageIndex != nImageIndex)
			{
				bSuccess = pItem->SetImageIndex(nImageIndex);
				m_TabCtrl.UpdateLayout();
				m_TabCtrl.Invalidate();
			}
		}

		return bSuccess;
	}

	BOOL UpdateTabToolTip(HWND hWnd, LPCTSTR sToolTip = NULL)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(nTab);
			_CSTRING_NS::CString sCurrentToolTip = pItem->GetToolTip();
			if(sCurrentToolTip != sToolTip)
			{
				bSuccess = pItem->SetToolTip(sToolTip);
			}
		}

		return bSuccess;
	}

	BOOL HighlightTab(HWND hWnd, bool bHighlight = true)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			bSuccess = m_TabCtrl.HighlightItem((size_t)nTab, bHighlight);
		}

		return bSuccess;
	}

	BOOL UpdateTabCanClose(HWND hWnd, bool bCanClose = true)
	{
		BOOL bSuccess = FALSE;

		TTabCtrl::TItem tcItem;
		tcItem.SetTabView(hWnd);

		int nTab = m_TabCtrl.FindItem(&tcItem, CTFI_TABVIEW);
		if(nTab >= 0)
		{
			TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(nTab);
			bool bCurrentCanClose = pItem->CanClose();
			if(bCurrentCanClose != bCanClose)
			{
				bSuccess = pItem->SetCanClose(bCanClose);
				m_TabCtrl.UpdateLayout();
				m_TabCtrl.Invalidate();
			}
		}

		return bSuccess;
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// CTabbedFrameImpl
//
/////////////////////////////////////////////////////////////////////////////

#define CHAIN_ACTIVETABVIEW_COMMANDS() \
	if(uMsg == WM_COMMAND && m_hWndActive != NULL) \
		::SendMessage(m_hWndActive, uMsg, wParam, lParam);

#define CHAIN_ACTIVETABVIEW_CHILD_COMMANDS(tabClass) \
	if(uMsg == WM_COMMAND) \
	{ \
		HWND hWndChild = tabClass.GetActiveView(); \
		if(hWndChild != NULL) \
			::SendMessage(hWndChild, uMsg, wParam, lParam); \
	}

// Use this if forwarding to an ActiveX control.
#define CHAIN_ACTIVETABVIEW_CHILD_COMMANDS2(tabClass) \
	if(uMsg == WM_COMMAND) \
	{ \
		HWND hWndChild = tabClass.GetActiveView(); \
		if(hWndChild != NULL) \
			::SendMessage(hWndChild, uMsg, wParam, 0); \
	}

template <
	class T,
#ifdef _USE_AERO
	class TTabCtrl = CAeroTabCtrl<CTabViewTabItem>,
	class TBase = aero::CFrameWindowImpl<T, ATL::CWindow, ATL::CFrameWinTraits>
#else
	class TTabCtrl = CDotNetTabCtrl<CTabViewTabItem>,
	class TBase = WTL::CFrameWindowImpl<T, ATL::CWindow, ATL::CFrameWinTraits>
#endif
>
class CTabbedFrameImpl :
	public TBase,
	public CCustomTabOwnerImpl<T, TTabCtrl>
{
protected:
	typedef CTabbedFrameImpl<T, TTabCtrl, TBase> thisClass;
	typedef TBase baseClass;
	typedef CCustomTabOwnerImpl<T, TTabCtrl> customTabOwnerClass;

// Member variables
protected:
	bool m_bReflectNotifications, m_bForwardNotifications;
	DWORD m_nTabStyles;
	HWND m_hWndActive;

// Constructors
public:
	CTabbedFrameImpl(bool bReflectNotifications = false, bool bForwardNotifications = false) :
		m_bReflectNotifications(bReflectNotifications),
		m_bForwardNotifications(bForwardNotifications),
		m_nTabStyles(CTCS_BOTTOM | CTCS_TOOLTIPS),
		m_hWndActive(NULL)
	{
		m_nMinTabCountForVisibleTabs = 1;
		m_bKeepTabsHidden = (m_nMinTabCountForVisibleTabs > 0);
	}

// Methods
public:
	void SetReflectNotifications(bool bReflectNotifications = true)
	{
		m_bReflectNotifications = bReflectNotifications;
	}

	bool GetReflectNotifications(void) const
	{
		return m_bReflectNotifications;
	}

	void SetForwardNotifications(bool bForwardNotifications = true)
	{
		m_bForwardNotifications = bForwardNotifications;
	}

	bool GetForwardNotifications(void) const
	{
		return m_bForwardNotifications;
	}

	void SetTabStyles(DWORD nTabStyles)
	{
		m_nTabStyles = nTabStyles;
	}

	DWORD GetTabStyles(void) const
	{
		return m_nTabStyles;
	}

	void ModifyTabStyles(DWORD dwRemove, DWORD dwAdd)
	{
		DWORD dwNewStyle = (m_nTabStyles & ~dwRemove) | dwAdd;
		if(m_nTabStyles != dwNewStyle)
		{
			m_nTabStyles = dwNewStyle;
		}
	}

	HWND GetActiveView(void) const
	{
		return m_hWndActive;
	}

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/)
	{
		// TODO: Have support both for "new"ing an
		//  instance of this class, or having
		//  a member variable of this class.
		//  Currently, we don't support deleting our
		//  instance because someone created us with "new"
		//delete this;
	}

// Message Handling
public:
	// The class that derives from this class should set an appropriate background brush
	DECLARE_FRAME_WND_CLASS_EX(_T("TabbedFrame"), 0, 0, COLOR_APPWORKSPACE)

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)

		NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		NOTIFY_CODE_HANDLER(CTCN_ACCEPTITEMDRAG, OnAcceptItemDrag)
		NOTIFY_CODE_HANDLER(CTCN_CANCELITEMDRAG, OnCancelItemDrag)
		NOTIFY_CODE_HANDLER(CTCN_DELETEITEM, OnDeleteItem)
		NOTIFY_CODE_HANDLER(CTCN_SELCHANGING, OnSelChanging)
		NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnSelChange)

		CHAIN_MSG_MAP(baseClass)

		// If there are key messages that haven't been handled yet,
		// pass those along to the active child window
		if(uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
		{
			if(m_hWndActive != NULL && ::IsWindow(m_hWndActive))
			{
				lResult = ::SendMessage(m_hWndActive, uMsg, wParam, lParam);

				return TRUE;
			}
		}

		CHAIN_ACTIVETABVIEW_COMMANDS()
		if(m_bReflectNotifications)
		{
			REFLECT_NOTIFICATIONS()
		}
		if(m_bForwardNotifications)
		{
			FORWARD_NOTIFICATIONS()
		}
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// "baseClass::OnCreate()"
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		bHandled = TRUE;
		if(lRet == -1)
		{
			return -1;
		}

		// The derived C++ class should set the background brush for
		// the window class (DECLARE_FRAME_WND_CLASS_EX)
		//::SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, COLOR_APPWORKSPACE+1);

		this->CreateTabWindow(m_hWnd, rcDefault, m_nTabStyles);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		DestroyTabWindow();

		// Say that we didn't handle it so that anyone else
		//  interested gets to handle the message
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// Be sure tab gets message before we recalculate the tab area height,
		//  so that it can adjust its font metrics first.
		// NOTE: This causes the tab to get the WM_SETTINGCHANGE message twice,
		//  but that's OK.
		m_TabCtrl.SendMessage(uMsg, wParam, lParam);

		T* pT = static_cast<T*>(this);
		pT->CalcTabAreaHeight();

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_hWndActive)
		{
			// Let the active view and the tabs do all the drawing
			// as flicker-free as possible.
			bHandled = TRUE;
			return 1;
		}
		else
		{
			// There is no active tab view.
			// Let the default erase happen with the window class brush.
			bHandled = FALSE;
			return 0;
		}
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// NOTE: ::IsWindowVisible(m_hWndActive) will be false if
		//  the frame is maximized.  So just use "IsWindow" instead.
		if(m_hWndActive != NULL && ::IsWindow(m_hWndActive))
		{
			// Also - only forward the focus on to the active view
			// if the tab isn't currently capturing the mouse
			if(m_TabCtrl != ::GetCapture())
			{
				::SetFocus(m_hWndActive);
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		//LPMSG pMsg = (LPMSG)lParam;
		//
		//if(PreTranslateMessage(pMsg))
		//	return TRUE;
		//
		//return m_view.PreTranslateMessage(pMsg);

		return ::SendMessage(m_hWndActive, WM_FORWARDMSG, 0, lParam);
	}

	LRESULT OnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
			// If they left click on an item, set focus on the tab view,
			// but only if the view was already the active tab view.
			NMCTCITEM* item = (NMCTCITEM*)pnmh;
			if(item && (item->iItem >= 0) && (item->iItem == m_TabCtrl.GetCurSel()))
			{
				TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(item->iItem);
				if(pItem->UsingTabView())
				{
					::SetFocus(pItem->GetTabView());
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnAcceptItemDrag(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
			// If finished dragging, set focus on the tab view.
			NMCTC2ITEMS* item = (NMCTC2ITEMS*)pnmh;
			if(item && (item->iItem2 >= 0))
			{
				TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(item->iItem2);
				if(pItem->UsingTabView())
				{
					::SetFocus(pItem->GetTabView());
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCancelItemDrag(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
			// If finished dragging, set focus on the tab view.
			NMCTCITEM* item = (NMCTCITEM*)pnmh;
			if(item && (item->iItem >= 0))
			{
				TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(item->iItem);
				if(pItem->UsingTabView())
				{
					::SetFocus(pItem->GetTabView());
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSelChanging(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSelChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Be sure the notification is from the tab control
		// (and not from a sibling like a list view control)
		if(pnmh && (m_TabCtrl == pnmh->hwndFrom))
		{
			int nNewTab = m_TabCtrl.GetCurSel();

			if(nNewTab >= 0)
			{
				TTabCtrl::TItem* pItem = m_TabCtrl.GetItem(nNewTab);
				if(pItem->UsingTabView())
				{
					HWND hWndNew = pItem->GetTabView();
					HWND hWndOld = m_hWndActive;
					if( hWndNew != hWndOld )
					{
						m_hWndActive = hWndNew;

						//UpdateLayout is going to essentially do a
						//  "ShowWindow(hWndNew, SW_SHOW)" for us
						// (Call the most derived class's version of UpdateLayout)
						T* pT = static_cast<T*>(this);
						pT->UpdateLayout();

						if(hWndOld)
						{
							::ShowWindow(hWndOld, SW_HIDE);
						}

						::SetFocus(hWndNew);
					}
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

// Overrides from CCustomTabOwnerImpl
public:

	void OnRemoveTab(size_t nNewTabCount)
	{
		T* pT = static_cast<T*>(this);

		// NOTE: Derived classes should call this base class version as well
		if(nNewTabCount == 0)
		{
			m_hWndActive = NULL;
		}

		customTabOwnerClass::OnRemoveTab(nNewTabCount);
	}

#ifdef _USE_AERO
  void CalcTabAreaHeight(void)
  {
    SIZE size;

    HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_NAVIGATION);

    ::GetThemePartSize(
      hTheme,
      NULL,
      NAV_BACKBUTTON,
      0,
      NULL,
      TS_TRUE,
      &size
      );

    ::CloseThemeData(hTheme);

    int nNewTabAreaHeight2 = customTabOwnerClass::CalcTabAreaHeight2();
    int nNewTabAreaHeight  = max(size.cy + 4, nNewTabAreaHeight2);

    this->GetTabCtrl().SetTopMargin(nNewTabAreaHeight - nNewTabAreaHeight2);

    T* pT = static_cast<T*>(this);
    pT->SetTabAreaHeight(nNewTabAreaHeight);
  }
#endif

	void SetTabAreaHeight(int nNewTabAreaHeight)
	{
		if(m_bKeepTabsHidden)
		{
			m_nTabAreaHeight = 0;

			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
			Invalidate();
		}
		else if(m_nTabAreaHeight != nNewTabAreaHeight)
		{
			m_nTabAreaHeight = nNewTabAreaHeight;

			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
			Invalidate();
		}
	}

// Overrides from TBase
public:

	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
		RECT rect;
		GetClientRect(&rect);

		// position bars and offset their dimensions
		T* pT = static_cast<T*>(this);
		pT->UpdateBarsPosition(rect, bResizeBars);

		/*
		// resize client window
		if(m_hWndClient != NULL)
			::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		*/

		int nWindowPosCount=0;
		if(m_TabCtrl) nWindowPosCount++;
		if(m_hWndActive) nWindowPosCount++;

		if(nWindowPosCount > 0)
		{
			HDWP hdwp = BeginDeferWindowPos(nWindowPosCount);
			DWORD dwStyle = (DWORD)m_TabCtrl.GetWindowLong(GWL_STYLE);
			if(CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
			{
				if(m_TabCtrl)
				{
					::DeferWindowPos(
						hdwp,
						m_TabCtrl,
						NULL,
						rect.left, rect.bottom - m_nTabAreaHeight,
						rect.right - rect.left, m_nTabAreaHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
				if(m_hWndActive)
				{
					::DeferWindowPos(
						hdwp,
						m_hWndActive,
						NULL,
						rect.left, rect.top,
						rect.right - rect.left, (rect.bottom-m_nTabAreaHeight) - rect.top,
						SWP_NOZORDER | SWP_SHOWWINDOW);
				}
			}
			else
			{
				if(m_TabCtrl)
				{
					::DeferWindowPos(
						hdwp,
						m_TabCtrl,
						NULL,
						rect.left, rect.top,
						rect.right-rect.left, m_nTabAreaHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
				if(m_hWndActive)
				{
					::DeferWindowPos(
						hdwp,
						m_hWndActive,
						NULL,
						rect.left, rect.top + m_nTabAreaHeight,
						rect.right - rect.left,
						rect.bottom - (rect.top+m_nTabAreaHeight),
						SWP_NOZORDER | SWP_SHOWWINDOW);
				}
			}
			EndDeferWindowPos(hdwp);
		}

		m_TabCtrl.UpdateLayout();
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// CTabbedPopupFrame
//
/////////////////////////////////////////////////////////////////////////////

typedef ATL::CWinTraits<WS_POPUP | WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_THICKFRAME, WS_EX_TOOLWINDOW> TabbedPopupFrameWinTraits;

template <class TTabCtrl = CDotNetTabCtrl<CTabViewTabItem> >
class CTabbedPopupFrame :
	public CTabbedFrameImpl<CTabbedPopupFrame<TTabCtrl>, TTabCtrl, WTL::CFrameWindowImpl<CTabbedPopupFrame<TTabCtrl>, ATL::CWindow, TabbedPopupFrameWinTraits> >
{
protected:
	typedef CTabbedPopupFrame<TTabCtrl> thisClass;
	typedef CTabbedFrameImpl<CTabbedPopupFrame, TTabCtrl, WTL::CFrameWindowImpl<CTabbedPopupFrame, ATL::CWindow, TabbedPopupFrameWinTraits> > baseClass;

// Members:
protected:
	// NOTE: If the "Close Command" is 0, than we really
	//  just let the default frame handling of "closing"
	//  happen, otherwise, we send the specified command to the parent
	WORD m_nCloseCommand;

// Constructors
public:
	CTabbedPopupFrame(bool bReflectNotifications = false) :
		baseClass(bReflectNotifications),
		m_nCloseCommand(0U)
	{
	}

// Accessors
public:
	WORD GetCloseCommand(void) const
	{
		return m_nCloseCommand;
	}

	void SetCloseCommand(WORD nCloseCommand)
	{
		m_nCloseCommand = nCloseCommand;
	}

// Message Handling
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("TabbedPopupFrame"), 0, 0, COLOR_APPWORKSPACE)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(baseClass::PreTranslateMessage(pMsg))
			return TRUE;

		//return m_view.PreTranslateMessage(pMsg);

		HWND hWndFocus = ::GetFocus();
		if(m_hWndActive != NULL && ::IsWindow(m_hWndActive) &&
			(m_hWndActive == hWndFocus || ::IsChild(m_hWndActive, hWndFocus)))
		{
			//active.PreTranslateMessage(pMsg);
			if(::SendMessage(m_hWndActive, WM_FORWARDMSG, 0, (LPARAM)pMsg))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	BEGIN_MSG_MAP(thisClass)
		if(m_nCloseCommand != 0)
		{
			if(uMsg == WM_SYSCOMMAND && wParam == SC_CLOSE)
			{
				bHandled = TRUE;
				lResult = ::SendMessage(this->GetParent(), WM_COMMAND, MAKEWPARAM(m_nCloseCommand, 0), 0);

				return TRUE;
			}
		}
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//
// CTabbedChildWindow
//
/////////////////////////////////////////////////////////////////////////////

// We need CTabbedChildWindowBase because of how CTabbedFrameImpl is currently implemented -
// It's expecting that the "base" class to usually be derived from CFrameWindowImpl.
// We'll have this special class for CTabbedChildWindow to
// inherit from instead of CWindowImpl, so that we provide
// the couple of extra things that CTabbedFrameImpl would
// like to be able to depend on (currently - a message map that
// at least handles WM_SIZE and overrideable methods
// "UpdateLayout" and "UpdateBarsPosition")

typedef ATL::CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE> TabbedChildWindowWinTraits;

template <class T, class TBase = ATL::CWindow, class TWinTraits = TabbedChildWindowWinTraits>
class ATL_NO_VTABLE CTabbedChildWindowBase : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
	typedef CTabbedChildWindowBase< T, TBase, TWinTraits >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
		}
		bHandled = FALSE;
		return 1;
	}

// Overrideables
public:
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
	}

	void UpdateBarsPosition(RECT& /*rect*/, BOOL bResizeBars = TRUE)
	{
		bResizeBars; //avoid level 4 warning
	}
};

template <class TTabCtrl = CDotNetTabCtrl<CTabViewTabItem> >
class CTabbedChildWindow :
	public CTabbedFrameImpl<CTabbedChildWindow<TTabCtrl>, TTabCtrl, CTabbedChildWindowBase<CTabbedChildWindow<TTabCtrl>, ATL::CWindow, TabbedChildWindowWinTraits> >
{
protected:
	typedef CTabbedChildWindow<TTabCtrl> thisClass;
	typedef CTabbedFrameImpl<CTabbedChildWindow<TTabCtrl>, TTabCtrl, CTabbedChildWindowBase<CTabbedChildWindow, ATL::CWindow, TabbedChildWindowWinTraits> > baseClass;

// Constructors
public:
	CTabbedChildWindow(bool bReflectNotifications = false) :
		baseClass(bReflectNotifications)
	{
	}

// Message Handling
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("TabbedChildWindow"), 0, 0, COLOR_APPWORKSPACE)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		//if(baseClass::PreTranslateMessage(pMsg))
		//	return TRUE;

		//return m_view.PreTranslateMessage(pMsg);

		HWND hWndFocus = ::GetFocus();
		if(m_hWndActive != NULL && ::IsWindow(m_hWndActive) &&
			(m_hWndActive == hWndFocus || ::IsChild(m_hWndActive, hWndFocus)))
		{
			//active.PreTranslateMessage(pMsg);
			if(::SendMessage(m_hWndActive, WM_FORWARDMSG, 0, (LPARAM)pMsg))
			{
				return TRUE;
			}
		}

		return FALSE;
	}


	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
};

#endif // __WTL_TABBED_FRAME_H__
