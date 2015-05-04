#ifndef __CUSTOMTABCTRL_H__
#define __CUSTOMTABCTRL_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CCustomTabCtrl - A base class to help implement
//   Tab Controls with different appearances
//
// Original work by Bjarke Viksoe (bjarke@viksoe.dk)
// Revised version by Daniel Bowen (dbowen@es.com).
// Copyright (c) 2001-2002 Bjarke Viksoe.
// Copyright (c) 2002-2004 Daniel Bowen.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
// History (Date/Author/Description):
// ----------------------------------
//
// 2005/07/13: Daniel Bowen
// - Namespace qualify the use of more ATL and WTL classes.
//
// 2005/03/14: Daniel Bowen
// - Fix warnings when compiling for 64-bit.
//
// 2004/06/28: Daniel Bowen
// - More ATLASSERTs
// - Clean up warnings on level 4
// - Fix "FindItem" so you can search on more than 1 criteria at a time
//   (like it was intended to work).  Use #define for flags now
//   instead of enumeration (CTFI_TABVIEW instead of eCustomTabItem_TabView).
//
// 2004/06/21: Peter Carlson
// - "CanClose" for items.
// - HighlightItem - "SetHighlighted" on item, then InvalidateRect for the item
//
// 2004/05/10: Peter Carlson
// - Middle mouse button notifications
//
// 2004/04/29: Daniel Bowen
// - Suport for a new CTCS_DRAGREARRANGE style.  If you set this style,
//   a tab item can be dragged to another position within the
//   same tab control.  Along with this are the new notifications
//     CTCN_BEGINITEMDRAG
//     CTCN_ACCEPTITEMDRAG
//     CTCN_CANCELITEMDRAG
//   If CTCS_SCROLL is also set, then the tabs will scroll when
//   you get near the left or the right edge of the tab control.
//   The drag rearrange methods are all overrideable, so a more
//   derived class could do a different UI for the drag and drop.
//   The current implementation roughly mimics dragging
//   MDI tabs in Visual Studio (with the exceptions of
//   supporting scrolling, and not moving the tab until the
//   cursor is past the half-way point of an adjacent tab).
// - Remove m_idDlgCtrl, and just use GetDlgCtrlID where needed.
//   This way, you can change the ID after the window is created.
// - Move shutdown code that was in OnDestroy to "Uninitialize".
//   Call Uninitialize from both OnDestroy and UnsubclassWindow
//   (UnsubclassWindow wasn't previously being overriden like it should).
// - With "Mouse Down" state flags, specify Left or Right to be
//   more specific.  Currently, the right mouse button state flags
//   are not being set.
// - On a left and right button down, don't take the focus when
//   clicking on a tab item.
// - You can now change the definition of the scroll speed specified by
//     CTCSR_NONE
//     CTCSR_SLOW
//     CTCSR_NORMAL
//     CTCSR_FAST
//   As long as you #define these before including this header file.
// - Support for highlighting items.
//   Add CCustomTabItem::IsHighlighted/SetHighlighted.
//   Uses the custom draw state CDIS_MARKED.
//
// 2003/06/27: Daniel Bowen
// - Update comment referencing DECLARE_WND_CLASS to be DECLARE_WND_CLASS_EX instead
// - If the creation of the window fails, don't try to initialize.
// 
// 2003/06/03: Daniel Bowen
// - Fix compile errors for VC 7.1
//
// 2003/01/07: Daniel Bowen
// - Destroy or detach tooltip control when handling WM_DESTROY
//
// 2002/12/05: Daniel Bowen
// - Handle WM_SYSCOLORCHANGE in case its broadcast to us
//   from a top-level window. Call OnSettingChange.
//
// 2002/11/13: Daniel Bowen
// - New CTCS_FLATEDGE style.  Tab controls derived from
//   CCustomTabCtrl can use this style to determine whether
//   to draw the outline of the control with a flat look.
// - New CalcSize_NonClient.  UpdateLayout will now call
//   this overrideable method before calling any other
//   CalcSize_* methods, so that you can adjust the client
//   RECT to account for non-client areas.
//
// 2002/10/21: Daniel Bowen
// - NMCTCITEM and NMCTC2ITEMS actually have "pt" in client
//   coordinates, not screen coordinates.  Change the comment.
// - Notifications using NMCTCITEM and NMCTC2ITEMS
//   (NM_CLICK, et. al) were incorrectly initialing "pt".
// - Remove some some unnecessary ATLASSERT(::IsWindow(m_hWnd))
//   (if the method doesn't depend on a valid m_hWnd)
// - Add some additional casting when dealing with current selection
// - DeleteItem - after sending CTCN_DELETEITEM, re-get
//   the count of items in case its changed
// - Change a couple of ASSERTs and parameter checks that deal
//   with size_t variables to not needlessly check for < 0
// - Change SetCurSel to take an int instead of a size_t.
//   Passing an int < 0 will clear the current selection
// - CCustomTabCtrl::SetImageList - Should be
//    CImageList imageListOld = m_imageList;
//     instead of
//    CImageList& imageListOld = m_imageList;
//
// 2002/07/16: Daniel Bowen
// - Ensure that any place doing anything with m_tooltip
//   first checks if(m_tooltip.IsWindow()).
// - Update DeleteAllItems to turn off redrawing while
//   the delete happens.
// - DeleteAllItems now takes an optional boolean to
//   specify whether or not to redraw after the deletion
// - Handle WM_SETREDRAW, but still allow the default handling.
//   We'll track WM_SETREDRAW in our own state variable
//   so that we can avoid doing UpdateLayout if
//   someone has called WM_SETREDRAW with FALSE.
//   When they call WM_SETREDRAW with TRUE to turn
//   it back on, we'll UpdateLayout to be ready for the caller
//   to do an InvalidateRect or RedrawWindow.
//
// 2002/06/20: Daniel Bowen
// - SetCurSel -
//   * Added a new optional parameter "bNotify" that allows
//     you to specify whether or not you want the parent
//     to receive the notifications CTCN_SELCHANGING and
//     CTCN_SELCHANGE.
//   * Even if the newly requested selection index is
//     the same index as the previous selection index,
//     go through the whole SetCurSel process.
//     Even though the index is the same, the item
//     might be different (as in the case of
//     InsertItem inserting a new item where the old
//     selection used to be).  EnsureVisible is also called,
//     which in the CTCS_SCROLL case, you really want
//     every time even if the item is the same.
//
//     This is a change from previous versions,
//     where if the index was the same as the current selection,
//     the method returned immediately.
//   
// - DeleteItem -
//   * When bUpdateSelection is true:
//     Now, when you delete the selected item, instead of
//     selecting the 0-index item, it tries to leave the index
//     of the selected item the same.  If the selected item
//     was the last item, the new last item is selected.
//     If the selected item was the only remaining item,
//     the selection is cleared.
//     
//
// 2002/06/13: Daniel Bowen
// - Fix small bug with scroll-repeat when scrolling right.
// 
// 2002/06/12: Daniel Bowen
// - Publish codeproject article.  For history prior
//   to the release of the article, please see the article
//   and the section "Note to previous users"

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error CustomTabCtrl.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
  #error CustomTabCtrl.h requires atlctrls.h to be included first
#endif

#ifndef __ATLGDIX_H__
  #error CustomTabCtrl.h requires atlgdix.h to be included first
#endif

// There are slightly different dependencies under VC 7 (ATL 7) vs. VC 6 (ATL 3)
#if (_ATL_VER >= 0x0700)
	#if !defined(__ATLCOLL_H__)
		#error CustomTabCtrl.h requires atlcoll.h (under VC 7).
	#endif
	#if !defined(__ATLSTR_H__)
		#error CustomTabCtrl.h requires CString. In VC 7, include atlstr.h
	#endif
	#if defined(__ATLMISC_H__) && !defined(_WTL_NO_CSTRING)
		#error In VC 7, please define _WTL_NO_CSTRING if you include atlmisc.h
	#endif
#else
	#if !defined(_WTL_USE_CSTRING)
		#error CustomTabCtrl.h requires CString. In VC 6, be sure to include atlmisc.h.
	#endif
#endif

#if (_WIN32_IE < 0x0400)
  #error CustomTabCtrl.h requires _WIN32_IE >= 0x0400
#endif

// Window styles:
// NOTE: "CTCS" stands for "Custom tab control style"
#define CTCS_SCROLL              0x0001   // TCS_SCROLLOPPOSITE
#define CTCS_BOTTOM              0x0002   // TCS_BOTTOM
//#define CTCS_RIGHT               0x0002   // TCS_RIGHT
//#define CTCS_MULTISELECT         0x0004   // TCS_MULTISELECT
#define CTCS_CLOSEBUTTON         0x0008   // TCS_FLATBUTTONS
//#define CTCS_FORCEICONLEFT       0x0010   // TCS_FORCEICONLEFT
//#define CTCS_FORCELABELLEFT      0x0020   // TCS_FORCELABELLEFT
#define CTCS_HOTTRACK            0x0040   // TCS_HOTTRACK
//#define CTCS_VERTICAL            0x0080   // TCS_VERTICAL
//#define CTCS_TABS                0x0000   // TCS_TABS
#define CTCS_FLATEDGE            0x0100   // TCS_BUTTONS
//#define CTCS_SINGLELINE          0x0000   // TCS_SINGLELINE
//#define CTCS_MULTILINE           0x0200   // TCS_MULTILINE
//#define CTCS_RIGHTJUSTIFY        0x0000   // TCS_RIGHTJUSTIFY
#define CTCS_DRAGREARRANGE       0x0400   // TCS_FIXEDWIDTH
//#define CTCS_OLEDRAGDROP         0x0800   // TCS_RAGGEDRIGHT
//#define CTCS_FOCUSONBUTTONDOWN   0x1000   // TCS_FOCUSONBUTTONDOWN
#define CTCS_BOLDSELECTEDTAB     0x2000   // TCS_OWNERDRAWFIXED
#define CTCS_TOOLTIPS            0x4000   // TCS_TOOLTIPS
//#define CTCS_FOCUSNEVER          0x8000   // TCS_FOCUSNEVER
#define CTCS_CLOSELASTTAB        0x10000

// Notifications:

#define CTCN_FIRST              (0U-550U)           // TCN_FIRST
#define CTCN_LAST               (0U-580U)           // TCN_LAST

#define CTCN_SELCHANGE          (TCN_FIRST - 1)     // TCN_SELCHANGE
#define CTCN_SELCHANGING        (TCN_FIRST - 2)     // TCN_SELCHANGING
//#define CTCN_GETOBJECT          (TCN_FIRST - 3)     // TCN_GETOBJECT
//#define CTCN_FOCUSCHANGE        (TCN_FIRST - 4)     // TCN_FOCUSCHANGE
//#define CTCN_INITIALIZE         (TCN_FIRST - 10)    // obsolete for now
#define CTCN_INSERTITEM         (TCN_FIRST - 11)
#define CTCN_DELETEITEM         (TCN_FIRST - 12)
#define CTCN_MOVEITEM           (TCN_FIRST - 13)
#define CTCN_SWAPITEMPOSITIONS  (TCN_FIRST - 14)
#define CTCN_CLOSE              (TCN_FIRST - 15)
#define CTCN_BEGINITEMDRAG      (TCN_FIRST - 21)
#define CTCN_ACCEPTITEMDRAG     (TCN_FIRST - 22)
#define CTCN_CANCELITEMDRAG     (TCN_FIRST - 23)
#define CTCN_MCLICK             (TCN_FIRST - 24)
#define CTCN_MDBLCLK            (TCN_FIRST - 25)

// Hit Test codes
#define CTCHT_NOWHERE            0x0001             // TCHT_NOWHERE
#define CTCHT_ONITEMICON         0x0002             // TCHT_ONITEMICON
#define CTCHT_ONITEMLABEL        0x0004             // TCHT_ONITEMLABEL
#define CTCHT_ONITEM             (CTCHT_ONITEMICON | CTCHT_ONITEMLABEL)
#define CTCHT_ONCLOSEBTN         0x0010
#define CTCHT_ONSCROLLRIGHTBTN   0x0020
#define CTCHT_ONSCROLLLEFTBTN    0x0040

// Find Item flags
#define CTFI_NONE                0x0000
#define CTFI_RECT                0x0001
#define CTFI_IMAGE               0x0002
#define CTFI_TEXT                0x0004
#define CTFI_TOOLTIP             0x0008
#define CTFI_TABVIEW             0x0010
#define CTFI_HIGHLIGHTED         0x0020
#define CTFI_CANCLOSE            0x0040
#define CTFI_LAST                CTFI_CANCLOSE
#define CTFI_ALL                 0xFFFF

// Number of milliseconds for scroll repeat
#ifndef CTCSR_NONE
	#define CTCSR_NONE           0
#endif
#ifndef CTCSR_SLOW
	#define CTCSR_SLOW           100
#endif
#ifndef CTCSR_NORMAL
	#define CTCSR_NORMAL         25
#endif
#ifndef CTCSR_FAST
	#define CTCSR_FAST           10
#endif

// Drag and drop related constant
#ifndef CTCD_SCROLLZONEWIDTH
	#define CTCD_SCROLLZONEWIDTH 20
#endif

// Structures
typedef struct tagNMCTCITEM
{
	NMHDR   hdr;
	int     iItem;  // Item Index
	POINT   pt;     // Client Coordinates
	UINT	flags;	// Hit Test Flags (hit item or scroll buttons?)
} NMCTCITEM, *LPNMCTCITEM;

typedef struct tagNMCTC2ITEMS
{
	NMHDR   hdr;
	int     iItem1;  // First Item Index
	int     iItem2;  // Second Item Index
	POINT   pt;      // Client Coordinates
} NMCTC2ITEMS, *LPNMCTC2ITEMS;

typedef struct tagCTCHITTESTINFO
{
	POINT pt;        // Client Coordinates of point to test
	UINT flags;
} CTCHITTESTINFO, *LPCTCHITTESTINFO;

typedef struct tagNMCTCCUSTOMDRAW
{
	NMCUSTOMDRAW nmcd;
	HFONT hFontInactive;
	HFONT hFontSelected;
	HBRUSH hBrushBackground;
	COLORREF clrTextInactive;
	COLORREF clrTextSelected;
	COLORREF clrSelectedTab;
	COLORREF clrBtnFace;
	COLORREF clrBtnShadow;
	COLORREF clrBtnHighlight;
	COLORREF clrBtnText;
	COLORREF clrHighlight;
	COLORREF clrHighlightHotTrack;
	COLORREF clrHighlightText;
} NMCTCCUSTOMDRAW, FAR * LPNMCTCCUSTOMDRAW;

typedef struct tagCTCSETTINGS
{
	signed char iPadding;
	signed char iMargin;
	signed char iSelMargin;
	signed char iIndent;
} CTCSETTINGS;

// Tab Item classes

class CCustomTabItem
{
// Member variables
protected:
	RECT m_rcItem;
	int m_nImage;
	_CSTRING_NS::CString m_sText;
	_CSTRING_NS::CString m_sToolTip;
	bool m_bHighlighted;
	bool m_bCanClose;

public:
	// NOTE: These are here for backwards compatibility.
	//  Use the new CTFI_NONE, CTFI_RECT, etc.
	typedef enum FieldFlags
	{
		eCustomTabItem_None    = CTFI_NONE,
		eCustomTabItem_Rect    = CTFI_RECT,
		eCustomTabItem_Image   = CTFI_IMAGE,
		eCustomTabItem_Text    = CTFI_TEXT,
		eCustomTabItem_ToolTip = CTFI_TOOLTIP,
		eCustomTabItem_All     = CTFI_ALL,
	};

#if (_MSC_VER >= 1300)
	#pragma deprecated(eCustomTabItem_None)
	#pragma deprecated(eCustomTabItem_Rect)
	#pragma deprecated(eCustomTabItem_Image)
	#pragma deprecated(eCustomTabItem_Text)
	#pragma deprecated(eCustomTabItem_ToolTip)
	#pragma deprecated(eCustomTabItem_All)
#endif

// Constructors/Destructors
public:
	CCustomTabItem() :
		m_nImage(-1),
		m_bHighlighted(false),
		m_bCanClose(true)
	{
		::SetRectEmpty(&m_rcItem);
	}

	CCustomTabItem(const CCustomTabItem& rhs)
	{
		*this = rhs;
	}

	virtual ~CCustomTabItem()
	{
	}

	const CCustomTabItem& operator=(const CCustomTabItem& rhs)
	{
		if(&rhs != this)
		{
			m_rcItem        = rhs.m_rcItem;
			m_nImage        = rhs.m_nImage;
			m_sText         = rhs.m_sText;
			m_sToolTip      = rhs.m_sToolTip;
			m_bHighlighted  = rhs.m_bHighlighted;
			m_bCanClose     = rhs.m_bCanClose;
		}
		return *this;
	}

// Accessors
public:

	RECT GetRect() const
	{
		return m_rcItem;
	}
	LPCRECT GetRectRef() const
	{
		return &m_rcItem;
	}
	bool SetRect(RECT rcItem)
	{
		m_rcItem = rcItem;
		return true;
	}

	int GetImageIndex() const
	{
		return m_nImage;
	}
	bool SetImageIndex(int nImage = -1)
	{
		m_nImage = nImage;
		return true;
	}

	_CSTRING_NS::CString GetText() const
	{
		return m_sText;
	}
	LPCTSTR GetTextRef() const
	{
		return (LPCTSTR)m_sText;
	}
	bool SetText(LPCTSTR sNewText)
	{
		m_sText = sNewText;
		return true;
	}
	virtual LONG GetTextSize(WTL::CClientDC& dc) const
	{
//#ifdef _USE_AERO
#if 0
    RECT rcText = { 0 };
    Gdiplus::Graphics g(dc.m_hDC);
    Gdiplus::Font font(dc.m_hDC);
    Gdiplus::RectF rectF(0,0,1000,1000);
    Gdiplus::RectF boundingBox;
    Gdiplus::StringFormat stringFormat;
      stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
      stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
      stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap|Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
    g.MeasureString(
      m_sText, m_sText.GetLength(),
      &font,
      rectF,
      &stringFormat,
      &boundingBox);

    return (long)(boundingBox.Width);
#else
		RECT rcText = { 0 };
		dc.DrawText(m_sText, m_sText.GetLength(), &rcText, DT_SINGLELINE | DT_CALCRECT);
		return rcText.right-rcText.left;
#endif
	}

	_CSTRING_NS::CString GetToolTip() const
	{
		return m_sToolTip;
	}
	LPCTSTR GetToolTipRef() const
	{
		return (LPCTSTR)m_sToolTip;
	}
	bool SetToolTip(LPCTSTR sNewText)
	{
		m_sToolTip = sNewText;
		return true;
	}

	bool IsHighlighted() const
	{
		return m_bHighlighted;
	}
	bool SetHighlighted(bool bHighlighted)
	{
		m_bHighlighted = bHighlighted;
		return true;
	}

	bool CanClose() const
	{
		return m_bCanClose;
	}
	bool SetCanClose(bool bCanClose)
	{
		m_bCanClose = bCanClose;
		return true;
	}
	

// Methods:
public:
	bool UsingImage() const
	{
		return (m_nImage >= 0);
	}
	bool UsingText() const
	{
		return (m_sText.GetLength() > 0);
	}
	bool UsingToolTip() const
	{
		return (m_sToolTip.GetLength() > 0);
	}

	BOOL InflateRect(int dx, int dy)
	{
		return ::InflateRect(&m_rcItem, dx, dy);
	}

	bool MatchItem(CCustomTabItem* pItem, DWORD eFlags) const
	{
		bool bMatch = true;
		if(bMatch && (eFlags & CTFI_RECT) == CTFI_RECT)
		{
			bMatch = (TRUE == ::EqualRect(&m_rcItem, &pItem->m_rcItem));
		}
		if(bMatch && (eFlags & CTFI_IMAGE) == CTFI_IMAGE)
		{
			bMatch = (m_nImage == pItem->m_nImage);
		}
		if(bMatch && (eFlags & CTFI_TEXT) == CTFI_TEXT)
		{
			bMatch = (m_sText == pItem->m_sText);
		}
		if(bMatch && (eFlags & CTFI_TOOLTIP) == CTFI_TOOLTIP)
		{
			bMatch = (m_sToolTip == pItem->m_sToolTip);
		}
		if(bMatch && (eFlags & CTFI_HIGHLIGHTED) == CTFI_HIGHLIGHTED)
		{
			bMatch = (m_bHighlighted == pItem->m_bHighlighted);
		}
		if(bMatch && (eFlags & CTFI_CANCLOSE) == CTFI_CANCLOSE)
		{
			bMatch = (m_bCanClose == pItem->m_bCanClose);
		}

		if(bMatch)
		{
			*pItem = *this;
		}

		return bMatch;
	}
};

// Derived Tab Item class that supports an HWND identifying a "tab view"
class CTabViewTabItem : public CCustomTabItem
{
protected:
	typedef CCustomTabItem baseClass;

// Member variables (in addition to CCustomTabItem ones)
protected:
	HWND m_hWndTabView;

public:
	// NOTE: This is here for backwards compatibility.
	//  Use the new CTFI_TABVIEW instead
	typedef enum FieldFlags
	{
		eCustomTabItem_TabView = CTFI_TABVIEW,
	};

// Use CTFI_TABVIEW instead
#if (_MSC_VER >= 1300)
	#pragma deprecated(eCustomTabItem_TabView)
#endif

// Constructors/Destructors
public:
	CTabViewTabItem() :
		m_hWndTabView(NULL)
	{
	}

	CTabViewTabItem(const CTabViewTabItem& rhs)
	{
		*this = rhs;
	}

	virtual ~CTabViewTabItem()
	{
	}

	const CTabViewTabItem& operator=(const CTabViewTabItem& rhs)
	{
		if(&rhs != this)
		{
			m_rcItem        = rhs.m_rcItem;
			m_nImage        = rhs.m_nImage;
			m_sText         = rhs.m_sText;
			m_sToolTip      = rhs.m_sToolTip;
			m_bHighlighted  = rhs.m_bHighlighted;
			m_bCanClose     = rhs.m_bCanClose;
			m_hWndTabView   = rhs.m_hWndTabView;
		}
		return *this;
	}

// Accessors
public:

	HWND GetTabView() const
	{
		return m_hWndTabView;
	}
	bool SetTabView(HWND hWnd = NULL)
	{
		m_hWndTabView = hWnd;
		return true;
	}

// Methods:
public:
	bool UsingTabView() const
	{
		return (m_hWndTabView != NULL);
	}

	bool MatchItem(CTabViewTabItem* pItem, DWORD eFlags) const
	{
		bool bMatch = true;
		if(eFlags == CTFI_TABVIEW)
		{
			// Make the common case a little faster
			// (searching only for a match to the "tab view" HWND)
			bMatch = (m_hWndTabView == pItem->m_hWndTabView);
		}
		else
		{
			// Do an extensive comparison
			bMatch = baseClass::MatchItem(pItem, eFlags);
			if(bMatch && (eFlags & CTFI_TABVIEW) == CTFI_TABVIEW)
			{
				bMatch = (m_hWndTabView == pItem->m_hWndTabView);
			}
		}

		if(bMatch)
		{
			*pItem = *this;
		}

		return bMatch;
	}
};

#if (_ATL_VER < 0x0700)
// With ATL 7, CAtlArray was introduced which is better than
//  CSimpleArray. Among other things, it supports inserting
//  items in any place.  If this code is compiled under ATL 7,
//  we'll use the real CAtlArray.  If this is compiled under ATL 3,
//  we'll use a "fake" CAtlArray where we implement the
//  functionality we're using that the real CAtlArray provides.
//
// Important! This isn't the real ATL 7 CAtlArray.
//  We inherit from CSimpleArray as "protected", so that you
//  can't call its versions of functions (so you have
//  to use the CAtlArray style of functions)

namespace ATL {

template<typename E>
class CAtlArray : protected ATL::CSimpleArray<E>
{
protected:
	typedef CAtlArray thisClass;
	typedef ATL::CSimpleArray<E> baseClass;

public:

	//Real CAtlArray:  size_t GetCount() const throw();
	size_t GetCount() const
	{
		return m_nSize;
	}

	//Real CAtlArray:  void InsertAt( size_t iElement, INARGTYPE element, size_t nCount = 1 );
	void InsertAt( size_t nIndex, E& element )
	{
		if(m_nSize == m_nAllocSize)
		{
			E* aT;
			int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
			aT = (E*)realloc(m_aT, nNewAllocSize * sizeof(E));
			if(aT == NULL)
				return; // FALSE;
			m_nAllocSize = nNewAllocSize;
			m_aT = aT;
		}
		memmove((void*)&m_aT[nIndex+1], (void*)&m_aT[nIndex], (m_nSize - nIndex ) * sizeof(E));
		m_nSize++;
		SetAtIndex(nIndex, element);
		//return TRUE;
	}

	//Real CAtlArray:  void RemoveAt( size_t iElement, size_t nCount = 1 );
	void RemoveAt( size_t nIndex )
	{
		// This is an improvement over CSimpleArray::RemoveAt suggested
		//  by Jim Springfield on the ATL discussion list
		m_aT[nIndex].~E();
		if((int)nIndex != (m_nSize - 1))
		{
			memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(E));
		}
		m_nSize--;
		//return TRUE;
	}

	//Real CAtlArray:  const E& operator[]( size_t iElement ) const throw();
	const E& operator[]( size_t iElement ) const
	{
		ATLASSERT( iElement < (size_t)m_nSize );
		return( m_aT[iElement] );
	}

	//Real CAtlArray:  E& operator[]( size_t iElement ) throw();
	E& operator[]( size_t iElement )
	{
		ATLASSERT( iElement < (size_t)m_nSize );
		return( m_aT[iElement] );
	}

};

}; // namespace ATL

#endif // (_ATL_VER < 0x0700)



typedef ATL::CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CTCS_TOOLTIPS, 0> CCustomTabCtrlWinTraits;

template <class T, class TItem = CCustomTabItem, class TBase = ATL::CWindow, class TWinTraits = CCustomTabCtrlWinTraits>
class ATL_NO_VTABLE CCustomTabCtrl : 
	public ATL::CWindowImpl< T, TBase, TWinTraits >,
	public COffscreenDrawRect< T >
{
public:
	// Expose the item type (that's a template parameter to this base class)
	typedef typename TItem TItem;

protected:
	typedef ATL::CWindowImpl< T, TBase, TWinTraits > baseClass;
	typedef COffscreenDrawRect< T > offscreenDrawClass;

// Member variables
protected:
	int m_iCurSel;
	int m_iHotItem;
	CTCSETTINGS m_settings;
	ATL::CAtlArray< TItem* > m_Items;
	WTL::CFont m_font;
	WTL::CFont m_fontSel;
	WTL::CImageList m_imageList;
	WTL::CToolTipCtrl m_tooltip;

	RECT m_rcTabItemArea;
	RECT m_rcScrollLeft;
	RECT m_rcScrollRight;
	RECT m_rcCloseButton;

	int m_iDragItem;
	int m_iDragItemOriginal;
	POINT m_ptDragOrigin;
	HCURSOR m_hCursorMove;
	HCURSOR m_hCursorNoDrop;

	int m_iScrollOffset;

	// Flags, internal state, etc.
	//
	//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
	//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
	//  +-------+-------+-------+-------+---+-----------+---------------+
	//  |  FUT  |  MO   |  MD   |  HT   |SR |    SD     |     FLAGS     |
	//  +-------+-------+-------+-------+---+-----------+---------------+
	//
	//  FLAGS - boolean flags
	//  SD - Scroll delta.  The number of pixels to move in a single scroll.
	//       Valid values are 0-63 (the value is bit shifted to/from position).
	//  SR - Scroll repeat speed.  Valid values are no-repeat,
	//       slow repeat, normal repeat and fast repeat
	//  HT - Current hot tracked item (if its a tab, then m_iHotItem is the hot tab item) 
	//  MD - Item under mouse when mouse button down message was sent
	//       but before mouse button up message is sent
	//  MO - Item current under mouse cursor
	//  FUT - Not used at this time, but reserved for the future.
	DWORD m_dwState;

	enum StateBits
	{
		// Flags
		// bits                   = 0x000000ff
		ectcMouseInWindow         = 0x00000001,
		ectcOverflowLeft          = 0x00000002,
		ectcOverflowRight         = 0x00000004,
		//ectcOverflowBottom        = 0x00000002,  // alias for vertical mode
		//ectcOverflowTop           = 0x00000004,  // alias for vertical mode
		ectcEnableRedraw          = 0x00000008,
		ectcDraggingItem          = 0x00000010,
		//ectcFlag20                = 0x00000020,
		//ectcFlag40                = 0x00000040,
		//ectcFlag80                = 0x00000080,

		// Scroll
		// bits                   = 0x0000ff00
		ectcScrollDeltaMask       = 0x00003f00,    //0011 1111
		ectcScrollDeltaShift      = 8,

		// We have to publicly expose these:
		ectcScrollRepeat          = 0x0000c000,    //1100 0000
		//ectcScrollRepeat_None     = 0x00000000,
		//ectcScrollRepeat_Slow     = 0x00004000,    //0100 0000
		//ectcScrollRepeat_Normal   = 0x00008000,    //1000 0000
		//ectcScrollRepeat_Fast     = 0x0000c000,    //1100 0000

		// Hot Tracking
		// bits                   = 0x000f0000
		ectcHotTrack              = 0x000f0000,
		ectcHotTrack_CloseButton  = 0x00010000,
		ectcHotTrack_ScrollRight  = 0x00020000,
		ectcHotTrack_ScrollLeft   = 0x00030000,
		ectcHotTrack_TabItem      = 0x00040000,

		// Mouse Down
		// bits                    = 0x00f00000
		ectcMouseDown              = 0x00f00000,
		ectcMouseDownL_CloseButton = 0x00100000,
		ectcMouseDownL_ScrollRight = 0x00200000,
		ectcMouseDownL_ScrollLeft  = 0x00300000,
		ectcMouseDownL_TabItem     = 0x00400000,

		ectcMouseDownR_CloseButton = 0x00900000,
		ectcMouseDownR_ScrollRight = 0x00a00000,
		ectcMouseDownR_ScrollLeft  = 0x00b00000,
		ectcMouseDownR_TabItem     = 0x00c00000,

		// Mouse Over
		// bits                   = 0x0f000000
		ectcMouseOver             = 0x0f000000,
		ectcMouseOver_CloseButton = 0x01000000,
		ectcMouseOver_ScrollRight = 0x02000000,
		ectcMouseOver_ScrollLeft  = 0x03000000,
		ectcMouseOver_TabItem     = 0x04000000,
	};

	enum ButtonToolTipIDs
	{
		ectcToolTip_Close         = 0xFFFFFFF0,
		ectcToolTip_ScrollRight   = 0xFFFFFFF1,
		ectcToolTip_ScrollLeft    = 0xFFFFFFF2,
	};

	enum TimerIDs
	{
		ectcTimer_ScrollLeft      = 0x00000010,
		ectcTimer_ScrollRight     = 0x00000020,
	};

// Public enumerations
public:
	enum ScrollRepeat
	{
		ectcScrollRepeat_None     = 0x00000000,
		ectcScrollRepeat_Slow     = 0x00004000,
		ectcScrollRepeat_Normal   = 0x00008000,
		ectcScrollRepeat_Fast     = 0x0000c000,
	};

// Constructors
public:
	CCustomTabCtrl() :
		m_iCurSel(-1),
		m_iHotItem(-1),
		m_dwState(0),
		m_iDragItem(-1),
		m_iDragItemOriginal(-1),
		m_hCursorMove(NULL),
		m_hCursorNoDrop(NULL),
		m_iScrollOffset(0)
	{
		::ZeroMemory(&m_settings, sizeof(CTCSETTINGS));
		::ZeroMemory(&m_ptDragOrigin, sizeof(POINT));

		::SetRectEmpty(&m_rcTabItemArea);
		::SetRectEmpty(&m_rcCloseButton);
		::SetRectEmpty(&m_rcScrollLeft);
		::SetRectEmpty(&m_rcScrollRight);

		m_dwState |=  ((40 << ectcScrollDeltaShift) & ectcScrollDeltaMask);
		m_dwState |=  ectcScrollRepeat_Normal;

		m_dwState |= ectcEnableRedraw;
	}

// Implementation
protected:

	void InitializeTooltips(void)
	{
		ATLASSERT(!m_tooltip.IsWindow());
		if(!m_tooltip.IsWindow())
		{
			// Be sure InitCommonControlsEx is called before this,
			//  with one of the flags that includes the tooltip control
			m_tooltip.Create(m_hWnd, NULL, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP /* | TTS_BALLOON */, WS_EX_TOOLWINDOW);
			if(m_tooltip.IsWindow())
			{
				m_tooltip.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				m_tooltip.SetDelayTime(TTDT_INITIAL, ::GetDoubleClickTime());
				m_tooltip.SetDelayTime(TTDT_AUTOPOP, ::GetDoubleClickTime() * 20);
				m_tooltip.SetDelayTime(TTDT_RESHOW, ::GetDoubleClickTime() / 5);
			}
		}
	}

	void ActivateTooltips(BOOL bActivate = TRUE)
	{
		ATLASSERT(m_tooltip.IsWindow());
		if(m_tooltip.IsWindow())
		{
			m_tooltip.Activate(bActivate);
		}
	}

	void ClearCurrentHotTracking(bool bRedrawEffectedArea = true)
	{
		switch(m_dwState & ectcHotTrack)
		{
		case ectcHotTrack_CloseButton:
			m_dwState &= ~ectcHotTrack;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcCloseButton);
			}
			break;
		case ectcHotTrack_ScrollRight:
			m_dwState &= ~ectcHotTrack;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollRight);
			}
			break;
		case ectcHotTrack_ScrollLeft:
			m_dwState &= ~ectcHotTrack;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollLeft);
			}
			break;
		case ectcHotTrack_TabItem:
			m_dwState &= ~ectcHotTrack;
			m_iHotItem = -1;

			if(bRedrawEffectedArea)
			{
				// In case the derived class actually changes the width
				//  of a hot tracked tab, invalidate the whole tab window
				this->Invalidate();
			}
			break;
		default:
			m_dwState &= ~ectcHotTrack;
			m_iHotItem = -1;
			if(bRedrawEffectedArea)
			{
				this->Invalidate();
			}
			break;
		}
	}

	void ClearCurrentMouseDownTracking(bool bRedrawEffectedArea = true)
	{
		switch(m_dwState & ectcMouseDown)
		{
		case ectcMouseDownL_CloseButton:
		case ectcMouseDownR_CloseButton:
			m_dwState &= ~ectcMouseDown;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcCloseButton);
			}
			break;
		case ectcMouseDownL_ScrollRight:
		case ectcMouseDownR_ScrollRight:
			m_dwState &= ~ectcMouseDown;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollRight);
			}
			break;
		case ectcMouseDownL_ScrollLeft:
		case ectcMouseDownR_ScrollLeft:
			m_dwState &= ~ectcMouseDown;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollLeft);
			}
			break;
		case ectcMouseDownL_TabItem:
		case ectcMouseDownR_TabItem:
			m_dwState &= ~ectcMouseDown;

			if(bRedrawEffectedArea)
			{
				// In case the derived class actually changes the width
				//  of a hot tracked tab, invalidate the whole tab window
				this->Invalidate();

				//if(m_iActionItem >= 0 && m_iActionItem < m_Items.GetCount())
				//{
				//	RECT rcItemDP;
				//	this->GetItemRect(m_iActionItem, &rcItemDP);
				//	this->InvalidateRect(rcItemDP);
				//}
			}
			break;
		default:
			m_dwState &= ~ectcMouseDown;
			if(bRedrawEffectedArea)
			{
				this->Invalidate();
			}
			break;
		}
	}

	void ClearCurrentMouseOverTracking(bool bRedrawEffectedArea = true)
	{
		switch(m_dwState & ectcMouseOver)
		{
		case ectcMouseOver_CloseButton:
			m_dwState &= ~ectcMouseOver;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcCloseButton);
			}
			break;
		case ectcMouseOver_ScrollRight:
			m_dwState &= ~ectcMouseOver;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollRight);
			}
			break;
		case ectcMouseOver_ScrollLeft:
			m_dwState &= ~ectcMouseOver;
			if(bRedrawEffectedArea)
			{
				this->InvalidateRect(&m_rcScrollLeft);
			}
			break;
		case ectcMouseOver_TabItem:
			m_dwState &= ~ectcMouseOver;

			if(bRedrawEffectedArea)
			{
				// In case the derived class actually changes the width
				//  of a hot tracked tab, invalidate the whole tab window
				this->Invalidate();

				//if(m_iActionItem >= 0 && m_iActionItem < m_Items.GetCount())
				//{
				//	RECT rcItemDP = {0};
				//	this->GetItemRect(m_iActionItem, &rcItemDP);
				//	this->InvalidateRect(rcItemDP);
				//}
			}
			break;
		default:
			m_dwState &= ~ectcMouseOver;
			if(bRedrawEffectedArea)
			{
				this->Invalidate();
			}
			break;
		}
	}

// Drag and drop methods (overrideable)
// In this base class implementation, we mimic the drag rearrange
// of MDI tabs in VS.NET (except, we add scroll support)
public:

	void AcceptItemDrag(bool bRedrawEffectedArea = true)
	{
		T* pT = static_cast<T*>(this);

		NMCTC2ITEMS nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_ACCEPTITEMDRAG }, m_iDragItemOriginal, m_iDragItem, {-1,-1}};
		::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);

		// In this implementation, the tab is moved as they drag.
		pT->StopItemDrag(bRedrawEffectedArea);
	}

	void CancelItemDrag(bool bRedrawEffectedArea = true)
	{
		T* pT = static_cast<T*>(this);

		// In this implementation, the tab is moved as they drag.
		// To cancel the drag, we move the item back to its original place.
		if(	m_iDragItemOriginal >= 0 &&
			m_iDragItem >= 0 &&
			m_iDragItemOriginal != m_iDragItem)
		{
			pT->MoveItem(m_iDragItem, m_iDragItemOriginal, true, false);
			pT->EnsureVisible(m_iDragItemOriginal);
		}

		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_CANCELITEMDRAG }, m_iDragItemOriginal, {-1,-1}, 0};
		::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);

		pT->StopItemDrag(bRedrawEffectedArea);
	}

	void StopItemDrag(bool bRedrawEffectedArea = true)
	{
		if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
		{
			m_dwState &= ~ectcDraggingItem;

			// Restore the default cursor
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
			::SetCursor((HCURSOR)::GetClassLongPtr(m_hWnd, GCLP_HCURSOR));
#else
			::SetCursor((HCURSOR)LongToHandle(::GetClassLongPtr(m_hWnd, GCLP_HCURSOR)));
#endif

			if(m_hCursorMove != NULL)
			{
				::DestroyCursor(m_hCursorMove);
				m_hCursorMove = NULL;
			}
			if(m_hCursorNoDrop != NULL)
			{
				::DestroyCursor(m_hCursorNoDrop);
				m_hCursorNoDrop = NULL;
			}

			m_iDragItem = -1;
			m_iDragItemOriginal = -1;
			::ZeroMemory(&m_ptDragOrigin, sizeof(POINT));

			if(bRedrawEffectedArea)
			{
				this->Invalidate();
			}
		}
	}

	void BeginItemDrag(int index, POINT ptDragOrigin)
	{
		T* pT = static_cast<T*>(this);
		if(index >= 0)
		{
			DWORD dwStyle = this->GetStyle();

			if(CTCS_DRAGREARRANGE == (dwStyle & CTCS_DRAGREARRANGE))
			{
				NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_BEGINITEMDRAG }, index, {ptDragOrigin.x, ptDragOrigin.y}, 0};
				if(FALSE != ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
				{
					// Returning non-zero prevents our default handling.
					// We've possibly already set a couple things that we
					// need to cleanup, so call StopItemDrag
					pT->StopItemDrag(false);
				}
				else
				{
					// returning FALSE let's us do our default handling

					// Mark the beginning of a drag operation.
					// We should have already done SetCapture, but just
					// in case, we'll set it again.
					this->SetCapture();

					// Set focus so that we get an ESC key press
					pT->SetFocus();

					// This call to DoDragDrop is just to ensure a dependency on OLE32.dll.
					// In the future, if we support true OLE drag and drop,
					// we'll really use DoDragDrop.
					::DoDragDrop(NULL, NULL, 0, 0);

					// To save on resources, we'll load the drag cursor
					// only when we need it, and destroy it when the drag is done
					HMODULE hOle32 = ::GetModuleHandle(_T("OLE32.dll"));
					if(hOle32 != NULL)
					{
						// Cursor ID identified using resource editor in Visual Studio
						int dragCursor = 2;
						int noDropCursor = 1;
						m_hCursorMove = ::LoadCursor(hOle32, MAKEINTRESOURCE(dragCursor));
						m_hCursorNoDrop = ::LoadCursor(hOle32, MAKEINTRESOURCE(noDropCursor));
					}

					m_dwState |= ectcDraggingItem;

					m_iDragItem = index;
					m_iDragItemOriginal = index;
					m_ptDragOrigin = ptDragOrigin;
				}
			}
		}
	}

	// Update the drag operation.
	//  ptCursor should be in client coordinates
	void ContinueItemDrag(POINT ptCursor)
	{
		// We're dragging an item
		T* pT = static_cast<T*>(this);

		RECT rcClient = {0};
		this->GetClientRect(&rcClient);
		if(::PtInRect(&rcClient, ptCursor))
		{
			::SetCursor(m_hCursorMove);
		}
		else
		{
			::SetCursor(m_hCursorNoDrop);
		}

		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int index = pT->HitTest(&tchti);
		if((index != m_iDragItem) && (index >= 0))
		{
			RECT rcItem = {0};
			this->GetItemRect(index, &rcItem);

			int itemMiddlePointX = rcItem.left + ((rcItem.right - rcItem.left) / 2);
			
			if((m_iDragItem < index) && (ptCursor.x > itemMiddlePointX))
			{
				// They're dragging an item from left to right,
				// and have dragged it over the half way mark to the right
				pT->MoveItem(m_iDragItem, index, true, false);
				m_iDragItem = index;
			}
			else if((m_iDragItem > index) && (ptCursor.x < itemMiddlePointX))
			{
				// They're dragging an item from right to left,
				// and have dragged it over the half way mark to the left
				pT->MoveItem(m_iDragItem, index, true, false);
				m_iDragItem = index;
			}
		}

		// Now scroll if necessary
		DWORD dwStyle = this->GetStyle();
		if(CTCS_SCROLL == (dwStyle & CTCS_SCROLL))
		{
			RECT rcLeftScrollZone = {rcClient.left, rcClient.top, (m_rcTabItemArea.left + CTCD_SCROLLZONEWIDTH), rcClient.bottom};
			RECT rcRightScrollZone = {(m_rcTabItemArea.right - CTCD_SCROLLZONEWIDTH), rcClient.top, rcClient.right, rcClient.bottom};

			if(	::PtInRect(&rcLeftScrollZone, ptCursor) &&
				(ectcOverflowLeft == (m_dwState & ectcOverflowLeft)))
			{
				pT->ScrollLeft(true);
				this->SetTimer(ectcTimer_ScrollLeft, CTCSR_SLOW);
			}
			else if(::PtInRect(&rcRightScrollZone, ptCursor) &&
				(ectcOverflowRight == (m_dwState & ectcOverflowRight)))
			{
				pT->ScrollRight(true);
				this->SetTimer(ectcTimer_ScrollRight, CTCSR_SLOW);
			}
		}
	}

// Message Handling
public:
	// Your derived class should use DECLARE_WND_CLASS or DECLARE_WND_SUPERCLASS, etc.
	//DECLARE_WND_CLASS_EX(_T("WTL_CustomTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

	BEGIN_MSG_MAP(CCustomTabCtrl)
		CHAIN_MSG_MAP(offscreenDrawClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDBLCLK, OnRButtonDoubleClick)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
		MESSAGE_HANDLER(WM_MBUTTONDBLCLK, OnMButtonDoubleClick)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_GETFONT, OnGetFont)
		MESSAGE_HANDLER(WM_SETFONT, OnSetFont)
		MESSAGE_HANDLER(WM_STYLECHANGED, OnStyleChanged)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_SETREDRAW, OnSetRedraw)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, OnGetToolTipInfo)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc();
		if(lRes == -1)
		{
			return -1;
		}

		T* pT = static_cast<T*>(this);
		pT->Initialize();
		return lRes;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->Uninitialize();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		this->Invalidate();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		this->Invalidate();
		return 0;
	}

	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTARROWS;
	}

	LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		if(m_tooltip.IsWindow())
		{
			MSG msg = { m_hWnd, uMsg, wParam, lParam };
			m_tooltip.RelayEvent(&msg);
		}
		return 1;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;

		DWORD dwStyle = this->GetStyle();
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		DWORD dwBefore = m_dwState & ectcHotTrack;

		T* pT = static_cast<T*>(this);
		if(ectcMouseInWindow != (m_dwState & ectcMouseInWindow))
		{
			TRACKMOUSEEVENT tme = { 0 };
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			if( _TrackMouseEvent(&tme) )
			{
				m_dwState |= ectcMouseInWindow;

				//pT->UpdateLayout();
				//this->Invalidate();

				// "OnMouseEnter"
				//...
			}
		}

		if(	(m_iDragItem >= 0) &&
			(ectcMouseInWindow == (m_dwState & ectcMouseInWindow)) &&
			(ectcDraggingItem != (m_dwState & ectcDraggingItem)) &&
			(ectcMouseDownL_TabItem == (m_dwState & ectcMouseDown)) &&
			(m_ptDragOrigin.x != ptCursor.x))
		{
			pT->BeginItemDrag(m_iDragItem, m_ptDragOrigin);
		}
		else if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
		{
			pT->ContinueItemDrag(ptCursor);
		}
		else if(ectcMouseInWindow == (m_dwState & ectcMouseInWindow))
		{
			// hit test
			bool boolHitNotFound = true;
			if(boolHitNotFound && ::PtInRect(&m_rcCloseButton, ptCursor))
			{
				boolHitNotFound = false;
				if( ectcMouseOver_CloseButton != (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
					m_dwState |= ectcMouseOver_CloseButton;

					if(ectcMouseDownL_CloseButton == (m_dwState & ectcMouseDown))
					{
						this->InvalidateRect(&m_rcCloseButton);
					}
				}
				else if( 0 == (m_dwState & ectcMouseDown) &&
					ectcHotTrack_CloseButton != (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
					m_dwState |= ectcHotTrack_CloseButton;
					this->InvalidateRect(&m_rcCloseButton);
				}
			}
			else
			{
				if(ectcMouseOver_CloseButton == (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
				}
				if(ectcHotTrack_CloseButton == (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
				}
			}

			if(boolHitNotFound && ::PtInRect(&m_rcScrollRight, ptCursor))
			{
				boolHitNotFound = false;
				if( ectcMouseOver_ScrollRight != (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
					m_dwState |= ectcMouseOver_ScrollRight;

					if(ectcMouseDownL_ScrollRight == (m_dwState & ectcMouseDown))
					{
						this->InvalidateRect(&m_rcScrollRight);
					}
				}
				else if(0 == (m_dwState & ectcMouseDown) &&
					ectcHotTrack_ScrollRight != (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
					m_dwState |= ectcHotTrack_ScrollRight;
					this->InvalidateRect(&m_rcScrollRight);
				}
			}
			else
			{
				if(ectcMouseOver_ScrollRight == (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
				}
				if(ectcHotTrack_ScrollRight == (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
				}
			}

			if(boolHitNotFound && ::PtInRect(&m_rcScrollLeft, ptCursor))
			{
				boolHitNotFound = false;
				if( ectcMouseOver_ScrollLeft != (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
					m_dwState |= ectcMouseOver_ScrollLeft;

					if(ectcMouseDownL_ScrollLeft == (m_dwState & ectcMouseDown))
					{
						this->InvalidateRect(&m_rcScrollLeft);
					}
				}
				else if(0 == (m_dwState & ectcMouseDown) &&
					ectcHotTrack_ScrollLeft != (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
					m_dwState |= ectcHotTrack_ScrollLeft;
					this->InvalidateRect(&m_rcScrollLeft);
				}
			}
			else
			{
				if(ectcMouseOver_ScrollLeft == (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
				}
				if(ectcHotTrack_ScrollLeft == (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
				}
			}

			if(boolHitNotFound && ::PtInRect(&m_rcTabItemArea, ptCursor))
			{
				boolHitNotFound = false;
				if( ectcMouseOver_TabItem != (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
					m_dwState |= ectcMouseOver_TabItem;

					// Not needed for simple hot tracking:
					//if(ectcMouseDownL_TabItem == (m_dwState & ectcMouseDown))
					//{
					//	this->InvalidateRect(&m_rcTabItemArea);
					//}
				}
				else if( CTCS_HOTTRACK == (dwStyle & CTCS_HOTTRACK) )
					// && ectcHotTrack_TabItem != (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);

					CTCHITTESTINFO tchti = { 0 };
					tchti.pt = ptCursor;
					int nIndex = pT->HitTest(&tchti);
					if(nIndex >= 0)
					{
						m_iHotItem = nIndex;

						m_dwState |= ectcHotTrack_TabItem;
						RECT rcItem = {0};
						this->GetItemRect(nIndex, &rcItem);
						this->InvalidateRect(&rcItem);
					}
				}
			}
			else
			{
				if(ectcMouseOver_TabItem == (m_dwState & ectcMouseOver))
				{
					this->ClearCurrentMouseOverTracking(true);
				}
				if( CTCS_HOTTRACK == (dwStyle & CTCS_HOTTRACK) &&
					ectcHotTrack_TabItem == (m_dwState & ectcHotTrack))
				{
					this->ClearCurrentHotTracking(true);
				}
			}
		}

		DWORD dwAfter = m_dwState & ectcHotTrack;

		if( dwAfter != dwBefore )
			if(m_tooltip.IsWindow())
				m_tooltip.Pop();

		return 1;
	}

	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);

		bHandled = FALSE;

		m_dwState &= ~ectcMouseInWindow;
		this->ClearCurrentHotTracking(false);
		this->ClearCurrentMouseOverTracking(true);

		//pT->UpdateLayout();
		//this->Invalidate();

		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);

		bHandled = FALSE;

		this->ClearCurrentMouseDownTracking(false);
		if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
		{
			pT->CancelItemDrag(false);
		}

		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
		if(::PtInRect(&m_rcCloseButton, ptCursor))
		{
			m_dwState |= (ectcMouseDownL_CloseButton | ectcMouseOver_CloseButton);
			this->InvalidateRect(&m_rcCloseButton);
			this->SetCapture();
		}
		else if(::PtInRect(&m_rcScrollRight, ptCursor))
		{
			m_dwState |= (ectcMouseDownL_ScrollRight | ectcMouseOver_ScrollRight);
			if(ectcOverflowRight == (m_dwState & ectcOverflowRight))
			{
				int nScrollSpeed = 0;
				switch(m_dwState & ectcScrollRepeat)
				{
				case ectcScrollRepeat_Slow:
					nScrollSpeed = CTCSR_SLOW;
					break;
				case ectcScrollRepeat_Normal:
					nScrollSpeed = CTCSR_NORMAL;
					break;
				case ectcScrollRepeat_Fast:
					nScrollSpeed = CTCSR_FAST;
					break;
				case ectcScrollRepeat_None:
				default:
					nScrollSpeed = CTCSR_NONE;
					break;
				}

				pT->ScrollRight(true);
				this->SetTimer(ectcTimer_ScrollRight, nScrollSpeed);
			}
			this->SetCapture();
		}
		else if(::PtInRect(&m_rcScrollLeft, ptCursor))
		{
			m_dwState |= (ectcMouseDownL_ScrollLeft | ectcMouseOver_ScrollLeft);
			if(ectcOverflowLeft == (m_dwState & ectcOverflowLeft))
			{
				int nScrollSpeed = 0;
				switch(m_dwState & ectcScrollRepeat)
				{
				case ectcScrollRepeat_Slow:
					nScrollSpeed = CTCSR_SLOW;
					break;
				case ectcScrollRepeat_Normal:
					nScrollSpeed = CTCSR_NORMAL;
					break;
				case ectcScrollRepeat_Fast:
					nScrollSpeed = CTCSR_FAST;
					break;
				case ectcScrollRepeat_None:
				default:
					nScrollSpeed = CTCSR_NONE;
					break;
				}

				pT->ScrollLeft(true);
				this->SetTimer(ectcTimer_ScrollLeft, nScrollSpeed);
			}
			this->SetCapture();
		}
		else
		{
			// Search for a tab
			CTCHITTESTINFO tchti = { 0 };
			tchti.pt = ptCursor;
			int nIndex = pT->HitTest(&tchti);

			NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), NM_CLICK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
			if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
			{
				// returning FALSE let's us do our default handling
				if(nIndex >= 0)
				{
					// NOTE: If they click on a tab, only grab the focus
					//  if a drag operation is started.
					//pT->SetFocus();
					pT->SetCurSel(nIndex);

					m_iDragItem = nIndex;
					m_ptDragOrigin = ptCursor;

					m_dwState |= ectcMouseDownL_TabItem;

					// This could be a drag operation.  We'll start the actual drag
					// operation in OnMouseMove if the mouse moves while the left mouse
					// button is still pressed.  OnLButtonUp will ReleaseCapture.
					this->SetCapture();
				}
			}
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if(m_hWnd == ::GetCapture())
		{
			T* pT = static_cast<T*>(this);

			POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

			if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
			{
				pT->AcceptItemDrag();
			}

			// Before we release the capture, remember what the state was
			// (in WM_CAPTURECHANGED we ClearCurrentMouseDownTracking)
			DWORD dwState = m_dwState;

			::ReleaseCapture();

			if(ectcMouseDownL_CloseButton == (dwState & ectcMouseDown) &&
				ectcMouseOver_CloseButton == (dwState & ectcMouseOver))
			{
				// Close Button
				NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_CLOSE }, m_iCurSel, {ptCursor.x, ptCursor.y}, 0};
				::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
			}

			pT->UpdateLayout();
			this->Invalidate();
		}
		return 0;
	}

	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		// Search for a tab
		T* pT = static_cast<T*>(this);
		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int nIndex = pT->HitTest(&tchti);

		// returning TRUE tells us not to do our default handling
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), NM_DBLCLK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
		if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
		{
			// returning FALSE let's us do our default handling
			if(nIndex >= 0)
			{
				//pT->SetFocus();
				//pT->SetCurSel(nIndex);
			}
		}

		return 0;
	}

	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		// Search for a tab
		T* pT = static_cast<T*>(this);
		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int nIndex = pT->HitTest(&tchti);

		// returning TRUE tells us not to do our default handling
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), NM_RCLICK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
		if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
		{
			// returning FALSE let's us do our default handling
			if(nIndex >= 0)
			{
				// NOTE: If they click on a tab, only grab the focus
				//  if a drag operation is started.
				//pT->SetFocus();
				pT->SetCurSel(nIndex);
			}
		}

		return 0;
	}

	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnRButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		// Search for a tab
		T* pT = static_cast<T*>(this);
		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int nIndex = pT->HitTest(&tchti);

		// returning TRUE tells us not to do our default handling
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), NM_RDBLCLK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
		if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
		{
			// returning FALSE let's us do our default handling
			if(nIndex >= 0)
			{
				//pT->SetFocus();
				//pT->SetCurSel(nIndex);
			}
		}

		return 0;
	}

	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		// Search for a tab
		T* pT = static_cast<T*>(this);
		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int nIndex = pT->HitTest(&tchti);

		// returning TRUE tells us not to do our default handling
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_MCLICK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
		if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
		{
			// returning FALSE let's us do our default handling
			if( nIndex!=-1 )
			{
				//pT->SetFocus();
				//pT->SetCurSel(nIndex);
			}
		}

		return 0;
	}

	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnMButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT ptCursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		// Search for a tab
		T* pT = static_cast<T*>(this);
		CTCHITTESTINFO tchti = { 0 };
		tchti.pt = ptCursor;
		int nIndex = pT->HitTest(&tchti);

		// returning TRUE tells us not to do our default handling
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_MDBLCLK }, nIndex, {ptCursor.x, ptCursor.y}, tchti.flags};
		if(FALSE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
		{
			// returning FALSE let's us do our default handling
			if( nIndex!=-1 )
			{
				//pT->SetFocus();
				//pT->SetCurSel(nIndex);
			}
		}

		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		switch( wParam )
		{
		case VK_LEFT:
			if( m_iCurSel > 0 )
			{
				pT->SetCurSel(m_iCurSel-1);
			}
			return 0;
		case VK_RIGHT:
			if( m_iCurSel < (int)m_Items.GetCount()-1 )
			{
				pT->SetCurSel(m_iCurSel+1);
			}
			return 0;
		case VK_ESCAPE:
			if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
			{
				pT->CancelItemDrag(true);
				return 0;
			}
			break;
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetFont(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// DDB: 2002/04/22
		// The code was doing GetFont and SetFont, but wasn't actually
		//  properly dealing with implementing it if the window
		//  was not a subclassed static control.
		return (LRESULT)(HFONT)m_font;
	}

	LRESULT OnSetFont(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// DDB: 2002/04/22
		// The code was doing GetFont and SetFont, but wasn't actually
		//  properly dealing with implementing it if the window
		//  was not a subclassed static control.
		//
		// Also, we're managing the lifetime of our font
		//  (i.e., we're going to DeleteObject it in our destructor),
		//  so if someone calls SetFont, keep a copy of the
		//  font instead of just "pointing" to it
		LOGFONT lfCopy = {0};
		::GetObject((HFONT)wParam, sizeof(LOGFONT), &lfCopy);

		if(!m_font.IsNull()) m_font.DeleteObject();

		m_font.CreateFontIndirect(&lfCopy);

		if(LOWORD(lParam))
		{
			this->Invalidate();
		}

		return 0;
	}

	LRESULT OnStyleChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(wParam == GWL_STYLE)
		{
			LPSTYLESTRUCT pStyles = (LPSTYLESTRUCT)lParam;
			if(pStyles)
			{
				T* pT = static_cast<T*>(this);

				// Tooltips
				if((((pStyles->styleOld) & CTCS_TOOLTIPS) != CTCS_TOOLTIPS) &&
					(((pStyles->styleNew) & CTCS_TOOLTIPS) == CTCS_TOOLTIPS))
				{
					this->ActivateTooltips(TRUE);
				}
				else if((((pStyles->styleOld) & CTCS_TOOLTIPS) == CTCS_TOOLTIPS) &&
					(((pStyles->styleNew) & CTCS_TOOLTIPS) != CTCS_TOOLTIPS))
				{
					this->ActivateTooltips(FALSE);
				}

				// Scroll to fit
				if((((pStyles->styleOld) & CTCS_SCROLL) != CTCS_SCROLL) &&
					(((pStyles->styleNew) & CTCS_SCROLL) == CTCS_SCROLL))
				{
					if(m_tooltip.IsWindow())
					{
						m_tooltip.AddTool(m_hWnd, _T("Scroll Right"), &rcDefault, (UINT)ectcToolTip_ScrollRight);
						m_tooltip.AddTool(m_hWnd, _T("Scroll Left"), &rcDefault, (UINT)ectcToolTip_ScrollLeft);
					}

					//pT->UpdateLayout();
					//this->Invalidate();
				}
				else if((((pStyles->styleOld) & CTCS_SCROLL) == CTCS_SCROLL) &&
					(((pStyles->styleNew) & CTCS_SCROLL) != CTCS_SCROLL))
				{
					if(m_tooltip.IsWindow())
					{
						m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_ScrollRight);
						m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_ScrollLeft);
					}

					m_iScrollOffset = 0;
					//pT->UpdateLayout();
					//this->Invalidate();
				}

				// Close Button
				if((((pStyles->styleOld) & CTCS_CLOSEBUTTON) != CTCS_CLOSEBUTTON) &&
					(((pStyles->styleNew) & CTCS_CLOSEBUTTON) == CTCS_CLOSEBUTTON))
				{
					if(m_tooltip.IsWindow())
					{
						m_tooltip.AddTool(m_hWnd, _T("Close"), &rcDefault, (UINT)ectcToolTip_Close);
					}

					//pT->UpdateLayout();
					//this->Invalidate();
				}
				else if((((pStyles->styleOld) & CTCS_CLOSEBUTTON) == CTCS_CLOSEBUTTON) &&
					(((pStyles->styleNew) & CTCS_CLOSEBUTTON) != CTCS_CLOSEBUTTON))
				{
					if(m_tooltip.IsWindow())
					{
						m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_Close);
					}

					//pT->UpdateLayout();
					//this->Invalidate();
				}

				pT->UpdateLayout();
				this->Invalidate();
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		switch(wParam)
		{
		case ectcTimer_ScrollRight:
			if(ectcOverflowRight != (m_dwState & ectcOverflowRight))
			{
				this->KillTimer(ectcTimer_ScrollRight);
			}
			else // ectcOverflowRight == (m_dwState & ectcOverflowRight)
			{
				if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
				{
					// We're scrolling because they're dragging a tab near the edge.
					// First kill the timer, then update the drag
					// (which might set the timer again)
					this->KillTimer(ectcTimer_ScrollRight);

					POINT ptCursor = {0};
					::GetCursorPos(&ptCursor);
					this->ScreenToClient(&ptCursor);
					pT->ContinueItemDrag(ptCursor);
				}
				else if(ectcMouseDownL_ScrollRight == (m_dwState & ectcMouseDown) &&
					ectcMouseOver_ScrollRight == (m_dwState & ectcMouseOver))
				{
					// We're scrolling because they're holding down the scroll button
					pT->ScrollRight(true);

					if(ectcScrollRepeat_None == (m_dwState & ectcScrollRepeat))
					{
						this->KillTimer(ectcTimer_ScrollRight);
					}
				}
			}
			break;
		case ectcTimer_ScrollLeft:
			if(ectcOverflowLeft != (m_dwState & ectcOverflowLeft))
			{
				this->KillTimer(ectcTimer_ScrollLeft);
			}
			else // ectcOverflowLeft == (m_dwState & ectcOverflowLeft)
			{
				if(ectcDraggingItem == (m_dwState & ectcDraggingItem))
				{
					// We're scrolling because they're dragging a tab near the edge.
					// First kill the timer, then update the drag
					// (which might set the timer again)
					this->KillTimer(ectcTimer_ScrollLeft);

					POINT ptCursor = {0};
					::GetCursorPos(&ptCursor);
					this->ScreenToClient(&ptCursor);
					pT->ContinueItemDrag(ptCursor);
				}
				else if(ectcMouseDownL_ScrollLeft == (m_dwState & ectcMouseDown) &&
					ectcMouseOver_ScrollLeft == (m_dwState & ectcMouseOver))
				{
					// We're scrolling because they're holding down the scroll button
					pT->ScrollLeft(true);

					if(ectcScrollRepeat_None == (m_dwState & ectcScrollRepeat))
					{
						this->KillTimer(ectcTimer_ScrollLeft);
					}
				}
			}
			break;
		default:
			bHandled = FALSE;
			break;
		}
		return 0;
	}

	LRESULT OnSetRedraw(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// If someone sends us WM_SETREDRAW with FALSE, we can avoid
		// doing an update layout until they set it back to TRUE.
		if(wParam)
		{
			if(ectcEnableRedraw != (m_dwState & ectcEnableRedraw))
			{
				// Redrawing was turned off, but now its being
				// turned back on again
				m_dwState |= ectcEnableRedraw;

				T* pT = static_cast<T*>(this);
				pT->UpdateLayout();

				// The caller will typically call InvalidateRect
				// or RedrawWindow after sending WM_SETREDRAW with TRUE,
				// so we won't do that here (but we will UpdateLayout,
				// so that we'll be ready to redraw)
			}
		}
		else
		{
			if(ectcEnableRedraw == (m_dwState & ectcEnableRedraw))
			{
				// Redrawing was turned on, but now its being turned off
				m_dwState &= ~ectcEnableRedraw;
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetToolTipInfo(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
	{
		LPNMTTDISPINFO pToolTipInfo = (LPNMTTDISPINFO)pnmh;
		if(pToolTipInfo)
		{
			// The way we implement tooltips for tab items
			// is to have as many "tools" as there are tabs.
			// The relationship of tool ID => tab index is:
			// tool ID = tab index + 1	(to avoid 0 as an ID)
			//
			// We supply the RECT elsewhere and the text here
			UINT_PTR id = pToolTipInfo->hdr.idFrom;
			if(id > 0 && id <= m_Items.GetCount())
			{
				if( (m_dwState & ectcHotTrack) == ectcHotTrack_CloseButton )
				{
					pToolTipInfo->lpszText = _T("Close");
				}
				else
				{
				TItem* pItem = m_Items[id-1];
				ATLASSERT(pItem != NULL);
				if(pItem)
				{
					if(pItem->UsingToolTip())
					{
						pToolTipInfo->lpszText = const_cast<LPTSTR>(pItem->GetToolTipRef());
					}
					else if(pItem->UsingText())
					{
						pToolTipInfo->lpszText = const_cast<LPTSTR>(pItem->GetTextRef());
					}
				}
			}
		}
		}

		return 0;
	}

// Overridables
public:

	void Initialize(void)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		this->SendMessage(WM_SETTINGCHANGE, 0, 0);

		this->InitializeTooltips();

		// NOTE: you can change the style at any time
		//  for a number of the cool tab control styles
		//  (tool tips, close button, scroll buttons, etc.)
		DWORD dwStyle = this->GetStyle();

		this->ActivateTooltips(CTCS_TOOLTIPS == (dwStyle & CTCS_TOOLTIPS));

		if(CTCS_SCROLL == (dwStyle & CTCS_SCROLL))
		{
			if(m_tooltip.IsWindow())
			{
				m_tooltip.AddTool(m_hWnd, _T("Scroll Right"), &rcDefault, (UINT)ectcToolTip_ScrollRight);
				m_tooltip.AddTool(m_hWnd, _T("Scroll Left"), &rcDefault, (UINT)ectcToolTip_ScrollLeft);
			}
		}

		if(CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON))
		{
			if(m_tooltip.IsWindow())
			{
				m_tooltip.AddTool(m_hWnd, _T("Close"), &rcDefault, (UINT)ectcToolTip_Close);
			}
		}
	}

	void Uninitialize(void)
	{
		T* pT = static_cast<T*>(this);

		DWORD dwStyle = this->GetStyle();

		if(m_tooltip.IsWindow())
		{
			if(CTCS_SCROLL == (dwStyle & CTCS_SCROLL))
			{
				m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_ScrollRight);
				m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_ScrollLeft);
			}

			if(CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON))
			{
				m_tooltip.DelTool(m_hWnd, (UINT)ectcToolTip_Close);
			}
		}

		pT->DeleteAllItems();

		if(m_tooltip.IsWindow())
		{
			// Also sets the contained m_hWnd to NULL
			m_tooltip.DestroyWindow();
		}
		else
		{
			m_tooltip = NULL;
		}
	}

	TItem* CreateNewItem(void* pInitData = NULL)
	{
		pInitData; // avoid level 4 warning

#if defined (_CPPUNWIND) & (defined(_ATL_EXCEPTIONS) | defined(_AFX))
		TItem* pNewItem = NULL;
		try { pNewItem = new TItem; }
		catch (...) { ATLTRACE(_T("!! Exception thrown in CCustomTabCtrl::CreateNewItem\r\n")); }
#else
		TItem* pNewItem = new TItem;
#endif
		return pNewItem;
	}

	void DeleteItem(TItem* pOldItem)
	{
#if defined (_CPPUNWIND) & (defined(_ATL_EXCEPTIONS) | defined(_AFX))
		try { delete pOldItem; }
		catch (...) { ATLTRACE(_T("!! Exception thrown in CCustomTabCtrl::DeleteItem\r\n")); }
#else
		delete pOldItem;
#endif
	}

	void UpdateLayout(void)
	{
		if(	!m_hWnd ||
			!::IsWindow(m_hWnd) ||
			(ectcEnableRedraw != (m_dwState & ectcEnableRedraw)))
		{
			return;
		}

		this->GetClientRect(&m_rcTabItemArea);

		T* pT = static_cast<T*>(this);

		DWORD dwStyle = this->GetStyle();

		pT->CalcSize_NonClient(&m_rcTabItemArea);

		if(CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON))
		{
			if( (m_iCurSel >= 0) && ((size_t)m_iCurSel < m_Items.GetCount()) )
			{
				TItem* pItem = m_Items[m_iCurSel];
				ATLASSERT(pItem != NULL);
				if((pItem != NULL) && pItem->CanClose())
				{
					pT->CalcSize_CloseButton(&m_rcTabItemArea);
				}
			}
		}

		if(CTCS_SCROLL == (dwStyle & CTCS_SCROLL))
		{
			pT->CalcSize_ScrollButtons(&m_rcTabItemArea);
			pT->UpdateLayout_ScrollToFit(m_rcTabItemArea);
			pT->UpdateScrollOverflowStatus();
		}
		else
		{
			pT->UpdateLayout_Default(m_rcTabItemArea);
		}

		pT->UpdateTabItemTooltipRects();
	}

	void CalcSize_NonClient(LPRECT prcTabItemArea)
	{
	}

	void CalcSize_CloseButton(LPRECT prcTabItemArea)
	{
	}

	void CalcSize_ScrollButtons(LPRECT prcTabItemArea)
	{
	}

	void UpdateLayout_Default(RECT rcTabItemArea)
	{
		UpdateLayout_ScrollToFit(rcTabItemArea);
	}

	void UpdateLayout_ScrollToFit(RECT rcTabItemArea)
	{
		WTL::CClientDC dc(m_hWnd);
		HFONT hOldFont = dc.SelectFont(this->GetFont());    

		int height = rcTabItemArea.bottom-rcTabItemArea.top;

		// Reposition tabs
		size_t nCount = m_Items.GetCount();
		int xpos = m_settings.iIndent;
		for( size_t i=0; i<nCount; ++i )
		{
			TItem* pItem = m_Items[i];
			ATLASSERT(pItem != NULL);
			if(pItem)
			{
				RECT rc = {xpos, 0, xpos, height};
				if( pItem->UsingText() )
				{
					rc.right += pItem->GetTextSize(dc) + (m_settings.iPadding*2);
				}
				pItem->SetRect(rc);
				xpos += (rc.right-rc.left) + m_settings.iMargin;
			}
		}
		if( (m_iCurSel >= 0) && ((size_t)m_iCurSel < nCount) )
		{
			TItem* pItem = m_Items[m_iCurSel];
			ATLASSERT(pItem != NULL);
			if(pItem)
			{
				pItem->InflateRect(m_settings.iSelMargin, 0);
			}
		}

		dc.SelectFont(hOldFont);
	}

	void UpdateTabItemTooltipRects(void)
	{
		// The way we implement tooltips for tab items
		// is to have as many "tools" as there are tabs.
		// The relationship of tool ID => tab index is:
		// tool ID = tab index + 1	(to avoid 0 as an ID)
		//
		// We supply the RECT here and text elsewhere.
		if(m_tooltip.IsWindow())
		{
			size_t i = 0;
			size_t nCount = m_Items.GetCount();
			RECT rcIntersect = {0};
			for(i=0; i<nCount; ++i )
			{
				RECT rcItemDP = {0};
				this->GetItemRect(i, &rcItemDP);

				::IntersectRect(&rcIntersect, &rcItemDP, &m_rcTabItemArea);

				// NOTE: Even if IntersectRect determines the rectangles
				// don't intersect at all, we still need
				// to update the tool rect, or we'll get the wrong
				// tooltip in some cases.
				m_tooltip.SetToolRect(m_hWnd, (UINT)i+1, &rcIntersect);
			}
		}
	}

	void ScrollLeft(bool bForceRedraw = true)
	{
		m_iScrollOffset += ((m_dwState & ectcScrollDeltaMask) >> ectcScrollDeltaShift);

		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		pT->UpdateScrollOverflowStatus();
		pT->UpdateTabItemTooltipRects();
		if(bForceRedraw)
		{
			this->Invalidate();
			// If something a little more forceful is needed:
			//::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	void ScrollRight(bool bForceRedraw = true)
	{
		m_iScrollOffset -= ((m_dwState & ectcScrollDeltaMask) >> ectcScrollDeltaShift);

		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		pT->UpdateScrollOverflowStatus();
		pT->UpdateTabItemTooltipRects();
		if(bForceRedraw)
		{
			this->Invalidate();
			// If something a little more forceful is needed:
			//::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	void UpdateScrollOverflowStatus(void)
	{
		// Check for overflow left
		if(m_iScrollOffset >= 0)
		{
			m_iScrollOffset = 0;
			m_dwState &= ~ectcOverflowLeft;
		}
		else
		{
			m_dwState |= ectcOverflowLeft;
		}

		// Check for overflow right
		m_dwState &= ~ectcOverflowRight;

		size_t nCount = m_Items.GetCount();
		if(nCount > 0)
		{
			// Check last item
			RECT rcItemDP = {0};
			this->GetItemRect(nCount-1, &rcItemDP);
			if(rcItemDP.right > m_rcTabItemArea.right)
			{
				m_dwState |= ectcOverflowRight;
			}
		}
	}

	void DoPaint(CDCHandle dc, RECT &rcClip)
	{
		// Save current DC selections
		int nSave = dc.SaveDC();
		ATLASSERT(nSave != 0);

		DWORD dwStyle = this->GetStyle();

		// Make sure we don't paint outside client area (possible with paint dc)
		RECT rcClient = {0};
		GetClientRect(&rcClient);
		dc.IntersectClipRect(&rcClient);


		// Prepare DC
		dc.SelectFont(this->GetFont());

		T* pT = static_cast<T*>(this);
		LRESULT lResCustom;
		NMCTCCUSTOMDRAW nmc = { 0 };
		LPNMCUSTOMDRAW pnmcd= &(nmc.nmcd);
		pnmcd->hdr.hwndFrom = m_hWnd;
		pnmcd->hdr.idFrom = this->GetDlgCtrlID();
		pnmcd->hdr.code = NM_CUSTOMDRAW;
		pnmcd->hdc = dc;
		pnmcd->uItemState = 0;

#if 0
    CBufferedPaint bufferedPaint;
    HDC hDCPaint = NULL;
    BP_PAINTPARAMS paintParams = { sizeof(BP_PAINTPARAMS), BPPF_ERASE, NULL, NULL };
    bufferedPaint.Begin(dc, &rcClient, BPBF_TOPDOWNDIB, &paintParams, &hDCPaint);
    pnmcd->hdc = hDCPaint;
#endif

		pT->InitializeDrawStruct(&nmc);

		pnmcd->dwDrawStage = CDDS_PREPAINT;
		lResCustom = ::SendMessage(GetParent(), WM_NOTIFY, pnmcd->hdr.idFrom, (LPARAM)&nmc);

		if( CDRF_SKIPDEFAULT != (lResCustom & CDRF_SKIPDEFAULT) )
		{
			pT->DoPrePaint(rcClient, &nmc);
		}

		if( lResCustom==CDRF_DODEFAULT || CDRF_NOTIFYITEMDRAW==(lResCustom & CDRF_NOTIFYITEMDRAW) )
		{
			POINT ptPreviousViewport = {0};
			HRGN hRgnClip = NULL;

			bool bScrollStyle = (CTCS_SCROLL == (dwStyle & CTCS_SCROLL));
			bool bHotTrackStyle = (CTCS_HOTTRACK == (dwStyle & CTCS_HOTTRACK));

			if(bScrollStyle)
			{
				// Remember clip region before we modify it
				//  to only include the tab item area
				// NOTE: GetClipRgn expects an already created
				//  region, so we create one with an empty rectangle.
				hRgnClip = ::CreateRectRgn(0,0,0,0);
				::GetClipRgn(dc, hRgnClip);

				dc.IntersectClipRect(&m_rcTabItemArea);

				dc.SetViewportOrg(m_iScrollOffset, 0, &ptPreviousViewport);
			}

			size_t nCount = m_Items.GetCount();
			// Draw the list items, except the selected one. It is drawn last
			// so it can cover the tabs below it.
			RECT rcIntersect = {0};
			for( size_t i=0; i<nCount; ++i )
			{
				if( (int)i != m_iCurSel )
				{
					TItem* pItem = m_Items[i];
					ATLASSERT(pItem != NULL);
					if(pItem)
					{
						RECT rcItemLP = {0}, rcItemDP = {0};
						rcItemLP = pItem->GetRect();

						::CopyRect(&rcItemDP, &rcItemLP);
						::OffsetRect(&rcItemDP, m_iScrollOffset, 0);

						if( ::IntersectRect(&rcIntersect, &rcItemDP, &rcClip) )
						{
							pnmcd->dwItemSpec = i;
							pnmcd->uItemState = 0;
							if(bHotTrackStyle && ((DWORD)m_iHotItem == i))
							{
								pnmcd->uItemState |= CDIS_HOT;
							}
							if(pItem->IsHighlighted())
							{
								pnmcd->uItemState |= CDIS_MARKED;
							}
							pnmcd->rc = rcItemLP;
							pT->ProcessItem(lResCustom, &nmc);
						}
					}
				}
			}
			if( m_iCurSel >=0 && (size_t)m_iCurSel < nCount )
			{
				TItem* pItem = m_Items[m_iCurSel];
				ATLASSERT(pItem != NULL);
				if(pItem)
				{
					RECT rcItemLP = {0}, rcItemDP = {0};
					rcItemLP = pItem->GetRect();

					::CopyRect(&rcItemDP, &rcItemLP);
					::OffsetRect(&rcItemDP, m_iScrollOffset, 0);

					if( ::IntersectRect(&rcIntersect, &rcItemDP, &rcClip) )
					{
						pnmcd->dwItemSpec = m_iCurSel;
						pnmcd->uItemState = CDIS_SELECTED;
						if(bHotTrackStyle && (m_iHotItem == m_iCurSel))
						{
							pnmcd->uItemState |= CDIS_HOT;
						}
						if(pItem->IsHighlighted())
						{
							pnmcd->uItemState |= CDIS_MARKED;
						}
						pnmcd->rc = rcItemLP;
						pT->ProcessItem(lResCustom, &nmc);
					}
				}
			}
			pnmcd->uItemState = 0;
			pnmcd->dwItemSpec = 0;

			if(bScrollStyle)
			{
				dc.SetViewportOrg(ptPreviousViewport);

				dc.SelectClipRgn(hRgnClip);
				::DeleteObject(hRgnClip);
			}
		}

		if( CDRF_SKIPDEFAULT != (lResCustom & CDRF_SKIPDEFAULT) )
		{
			pT->DoPostPaint(rcClient, &nmc);
		}

#if 0
    bufferedPaint.End();
    pnmcd->hdc = dc;
#endif

		if( CDRF_NOTIFYPOSTPAINT == (lResCustom & CDRF_NOTIFYPOSTPAINT) )
		{
			pnmcd->dwDrawStage = CDDS_POSTPAINT;
			pnmcd->dwItemSpec = 0;
			pnmcd->uItemState = 0;
			pnmcd->rc = rcClient;
			::SendMessage(GetParent(), WM_NOTIFY, pnmcd->hdr.idFrom, (LPARAM)&nmc);
		}

		dc.RestoreDC(nSave);
	}

	void ProcessItem(LRESULT lResCustom, LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
		LRESULT lResItem = CDRF_DODEFAULT;
		if( CDRF_NOTIFYITEMDRAW == (lResCustom & CDRF_NOTIFYITEMDRAW) )
		{
			lpNMCustomDraw->nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
			lResItem = ::SendMessage(GetParent(), WM_NOTIFY, lpNMCustomDraw->nmcd.hdr.idFrom, (LPARAM)lpNMCustomDraw);
		}
		if( CDRF_SKIPDEFAULT != (lResCustom & CDRF_SKIPDEFAULT) )
		{
			// Do default item-drawing
			T* pT = static_cast<T*>(this);
			pT->DoItemPaint(lpNMCustomDraw);
		}
		if( CDRF_NOTIFYITEMDRAW == (lResCustom & CDRF_NOTIFYITEMDRAW) &&
			CDRF_NOTIFYPOSTPAINT == (lResItem & CDRF_NOTIFYPOSTPAINT))
		{
			lpNMCustomDraw->nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
			::SendMessage(GetParent(), WM_NOTIFY, lpNMCustomDraw->nmcd.hdr.idFrom, (LPARAM)lpNMCustomDraw);
		}
	}

	void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
		DWORD dwStyle = this->GetStyle();

		lpNMCustomDraw->hFontInactive = m_font;
		lpNMCustomDraw->hFontSelected = m_fontSel;
		lpNMCustomDraw->hBrushBackground = ::GetSysColorBrush(COLOR_BTNFACE);
		lpNMCustomDraw->clrTextSelected = ::GetSysColor(COLOR_BTNTEXT);
		lpNMCustomDraw->clrTextInactive = ::GetSysColor(COLOR_BTNTEXT);
		lpNMCustomDraw->clrSelectedTab = ::GetSysColor(COLOR_BTNFACE);
		lpNMCustomDraw->clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
		lpNMCustomDraw->clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
		lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_BTNHIGHLIGHT);
		lpNMCustomDraw->clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
		lpNMCustomDraw->clrHighlight = ::GetSysColor(COLOR_HIGHLIGHT);
#if WINVER >= 0x0500 || _WIN32_WINNT >= 0x0500
		lpNMCustomDraw->clrHighlightHotTrack = ::GetSysColor(COLOR_HOTLIGHT);
#else
		lpNMCustomDraw->clrHighlightHotTrack = ::GetSysColor(COLOR_HIGHLIGHT);
#endif
		lpNMCustomDraw->clrHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	}

	void DoPrePaint(RECT rcClient, LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
		// "Erase Background"
		// NOTE: Your derived class might be able to do a
		//  better job of erasing only the necessary area
		//  (using the clip box, etc.)
		WTL::CDCHandle dc(lpNMCustomDraw->nmcd.hdc);

		HBRUSH hOldBrush = dc.SelectBrush(lpNMCustomDraw->hBrushBackground);
		dc.PatBlt(rcClient.left, rcClient.top, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, PATCOPY);
		dc.SelectBrush(hOldBrush);

		dc.SetTextColor(lpNMCustomDraw->clrTextInactive);
		dc.SetBkMode(TRANSPARENT);
	}

	void DoItemPaint(LPNMCTCCUSTOMDRAW /*lpNMCustomDraw*/)
	{
	}

	void DoPostPaint(RECT /*rcClient*/, LPNMCTCCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		// In your derived verion, paint the scroll buttons if CTCS_SCROLL
		// is set and the close button if CTCS_CLOSEBUTTON is set
		// (if you want to support these)
	}

// Operations
public:

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = baseClass::SubclassWindow(hWnd);
		if( bRet )
		{
			T* pT = static_cast<T*>(this);
			pT->Initialize();
		}
		return bRet;
	}

	HWND UnsubclassWindow(BOOL bForce = FALSE)
	{
		T* pT = static_cast<T*>(this);
		pT->Uninitialize();

		return baseClass::UnsubclassWindow(bForce);
	}

	WTL::CImageList SetImageList(HIMAGELIST hImageList)
	{
		CImageList imageListOld = m_imageList;
		m_imageList = hImageList;
		return imageListOld;
	}
	WTL::CImageList& GetImageList() const
	{
		return m_imageList;
	}

	WTL::CToolTipCtrl GetTooltips() const
	{
		return m_tooltip;
	}
	//void SetTooltips(HWND hWndToolTip)
	//{
	//	ATLASSERT(::IsWindow(m_hWnd));
	//	::SendMessage(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)hWndToolTip, 0L);
	//}

	bool SetScrollDelta(UINT nDelta)
	{
		if(nDelta > (ectcScrollDeltaMask >> ectcScrollDeltaShift))
		{
			return false;
		}
		m_dwState |=  ((nDelta << ectcScrollDeltaShift) & ectcScrollDeltaMask);

		return true;
	}

	UINT GetScrollDelta(void)
	{
		return ((m_dwState & ectcScrollDeltaMask) >> ectcScrollDeltaShift);
	}

	bool SetScrollRepeat(ScrollRepeat ectcNewScrollRepeat = ectcScrollRepeat_Normal)
	{
		m_dwState &= ~ectcScrollRepeat;
		m_dwState |= (ectcNewScrollRepeat & ectcScrollRepeat);

		return true;
	}

	ScrollRepeat GetScrollRepeat(void)
	{
		return (m_dwState & ectcScrollRepeat);
	}

	int InsertItem(int nItem, LPCTSTR sText = NULL, int nImage = -1, LPCTSTR sToolTip = NULL, bool bSelectItem = false)
	{
		T* pT = static_cast<T*>(this);
		TItem* pItem = pT->CreateNewItem();
		if(pItem)
		{
			pItem->SetText(sText);
			pItem->SetImageIndex(nImage);
			pItem->SetToolTip(sToolTip);

			return InsertItem(nItem, pItem, bSelectItem);
		}
		return -1;
	}

	int InsertItem(int nItem, TItem* pItem, bool bSelectItem = false)
	{
		T* pT = static_cast<T*>(this);

		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pItem);
		ATLASSERT(nItem >= 0 && nItem <= (int)m_Items.GetCount());
		if(!::IsWindow(m_hWnd) || pItem == NULL)
		{
			return -1;
		}

		size_t nOldCount = m_Items.GetCount();

		if( nItem < 0 || nItem > (int)nOldCount )
		{
			nItem = (int)nOldCount;
		}

		m_Items.InsertAt((size_t)nItem, pItem);

		size_t nNewCount = m_Items.GetCount();

		// Send notification
		NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_INSERTITEM }, nItem, {-1,-1}, 0};
		::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
		// Select if first tab
		if( nNewCount==1 )
		{
			pT->SetCurSel(0);
		}
		else if(bSelectItem)
		{
			pT->SetCurSel(nItem);
		}

		// The way we implement tooltips is to have as many
		// "tools" as there are tabs.  The relationship of
		// tool ID => tab index is:
		// tool ID = tab index + 1	(to avoid 0 as an ID)
		//
		// We supply the RECT and text elsewhere.
		if(m_tooltip.IsWindow())
		{
			m_tooltip.AddTool(m_hWnd, LPSTR_TEXTCALLBACK, &rcDefault, (UINT)nNewCount);
		}

		pT->UpdateLayout();
		this->Invalidate();
		return nItem;
	}

	BOOL MoveItem(size_t nFromIndex, size_t nToIndex, bool bUpdateSelection = true, bool bNotify = true)
	{
		T* pT = static_cast<T*>(this);

		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nFromIndex < m_Items.GetCount());
		ATLASSERT(nToIndex < m_Items.GetCount());

		if(!::IsWindow(m_hWnd) || nFromIndex >= m_Items.GetCount() || nToIndex >= m_Items.GetCount())
		{
			return FALSE;
		}

		TItem* pFromItem = m_Items[nFromIndex];
		ATLASSERT(pFromItem != NULL);
		m_Items.RemoveAt(nFromIndex);
		m_Items.InsertAt(nToIndex, pFromItem);

		// The number of items is staying the same, so m_iCurSel
		// won't be invalid whether it gets updated or not
		if(bUpdateSelection)
		{
			if(nFromIndex == (size_t)m_iCurSel)
			{
				pT->SetCurSel((int)nToIndex);
			}
		}

		if(bNotify)
		{
			NMCTC2ITEMS nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_MOVEITEM }, (int)nFromIndex, (int)nToIndex, {-1,-1}};
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
		}

		return TRUE;
	}

	BOOL SwapItemPositions(size_t nFromIndex, size_t nToIndex, bool bUpdateSelection = true, bool bNotify = true)
	{
		T* pT = static_cast<T*>(this);

		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nFromIndex < m_Items.GetCount());
		ATLASSERT(nToIndex < m_Items.GetCount());

		if(!::IsWindow(m_hWnd) || nFromIndex >= m_Items.GetCount() || nToIndex >= m_Items.GetCount())
		{
			return FALSE;
		}

		TItem* pFromItem = m_Items[nFromIndex];
		TItem* pToItem = m_Items[nToIndex];
		ATLASSERT(pFromItem != NULL);
		ATLASSERT(pToItem != NULL);
		m_Items[nFromIndex] = pToItem;
		m_Items[nToIndex] = pFromItem;

		// The number of items is staying the same, so m_iCurSel
		// won't be invalid whether it gets updated or not
		if(bUpdateSelection)
		{
			if(nFromIndex == (size_t)m_iCurSel)
			{
				pT->SetCurSel((int)nToIndex);
			}
			else if(nToIndex == (size_t)m_iCurSel)
			{
				pT->SetCurSel((int)nFromIndex);
			}
		}

		if(bNotify)
		{
			NMCTC2ITEMS nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_SWAPITEMPOSITIONS }, (int)nFromIndex, (int)nToIndex, {-1,-1}};
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
		}

		return TRUE;
	}

	BOOL DeleteItem(size_t nItem, bool bUpdateSelection = true, bool bNotify = true)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(m_hWnd));
		size_t nOldCount = m_Items.GetCount();
		if( nItem >= nOldCount )
		{
			return FALSE;
		}

		// Send notification
		if(bNotify)
		{
			// Returning TRUE tells us not to delete the item
			NMCTCITEM nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_DELETEITEM }, (int)nItem, {-1,-1}, 0};
			if( TRUE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh) )
			{
				// Cancel the attempt
				return FALSE;
			}
			else
			{
				// Just in case handler of the notification
				// changed the count somehow, get it again
				nOldCount = m_Items.GetCount();
				if( nItem >= nOldCount )
				{
					return FALSE;
				}
			}
		}

		int nPostDeleteSelection = -1;

		// The number of items is changing, so m_iCurSel
		// might be invalid if we don't change it.
		if(nOldCount <= 1)
		{
			// There's only one item left,
			// and its being deleted.
			m_iCurSel = -1;
		}
		else if(bUpdateSelection)
		{
			if( (int)nItem < m_iCurSel )
			{
				// The item being removed is before the current selection.
				// We still want the same item to be selected, but
				// the index needs to be adjusted to account for the missing item
				m_iCurSel--;
				pT->EnsureVisible(m_iCurSel);
				// NOTE: We don't call SetCurSel because we don't want
				//  any of the notifications to go to the parent - because
				//  the selection didn't change, the index just had to be adjusted.
			}
			else if( (int)nItem == m_iCurSel )
			{
				// If the item to be deleted is currently selected,
				//  select something else.
				// NOTE: We've already handled the "we're deleting
				//  the only remaining item" case.
				if(nItem >= (nOldCount-1))
				{
					// The selected item was the last item,
					// and there will still be at least one
					// item after this deletion occurs.
					// Select the item that will be the
					// new last item.
					// We need to do this before the actual
					// deletion so that m_iCurSel
					// will still be valid during the SetCurSel
					// call and the notification handlers of
					// CTCN_SELCHANGING and CTCN_SELCHANGE
					pT->SetCurSel(m_iCurSel-1);
				}
				else
				{
					// The selected item was NOT the last item,
					// and there will still be at least one
					// item after this deletion occurs.
					// Force a selection of the item that will
					// have the same index as the selected item being
					// deleted, but do it after the actual deletion.
					//pT->SetCurSel(m_iCurSel);
					nPostDeleteSelection = m_iCurSel;
				}
			}
		}
		else
		{
			if(((int)nItem == m_iCurSel) && (nItem >= (nOldCount-1)))
			{
				// If bUpdateSelection is false,
				// the item being deleted is selected,
				// and the item being deleted is the last
				// item, we need to clear the current selection
				// (setting m_iCurSel to -1)
				// so that our call to UpdateLayout
				// and our paint message handling don't
				// crash and burn with an invalid selected
				// item index. Its likely that the
				// caller is going to SetCurSel right
				// after this call to DeleteItem.
				m_iCurSel = -1;
			}
		}

		// Remove tooltip and item

		// The way we implement tooltips is to have as many
		// "tools" as there are tabs.  The relationship of
		// tool ID => tab index is:
		// tool ID = tab index + 1	(to avoid 0 as an ID)
		//
		// We supply the RECT and text elsewhere.
		if(m_tooltip.IsWindow())
		{
			m_tooltip.DelTool(m_hWnd, (UINT)m_Items.GetCount());
		}

		TItem* pItem = m_Items[nItem];
		ATLASSERT(pItem != NULL);
		m_Items.RemoveAt(nItem);

		pT->DeleteItem(pItem);

		if(nPostDeleteSelection >= 0)
		{
			pT->SetCurSel(nPostDeleteSelection);
		}

		// Repaint
		pT->UpdateLayout();
		this->Invalidate();
		return TRUE;
	}

	BOOL DeleteAllItems(bool bForceRedraw = false)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_iCurSel = -1;

		this->SendMessage(WM_SETREDRAW, FALSE, 0);

		while( GetItemCount()>0 ) DeleteItem(0U, false, true);

		this->SendMessage(WM_SETREDRAW, TRUE, 0);

		// UpdateLayout will already have been called
		// when sending WM_SETREDRAW with TRUE after
		// having sent it with FALSE
		// (or in DeleteItem if we weren't doing WM_SETREDRAW stuff)
		if(bForceRedraw)
		{
			this->Invalidate();
			// If something a little more forceful is needed:
			//::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}

		return TRUE;
	}

	// NOTE: Now, instead of calling SetItem, call
	//   GetItem to get the pointer to the TItem, update
	//   what you want, then call UpdateLayout and Invalidate
	//   if appropriate.
	/*
	BOOL SetItem(int nItem, TItem* pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(!::IsBadReadPtr(pItem,sizeof(TItem)));
		CHECK_ITEM(nItem);

		// Copy caller's data to the internal structure
		TItem* pItemT = m_Items[nItem];
		UINT mask = pItem->mask;
		if( mask & TCIF_TEXT )
		{
			pItemT->SetText(pItem->pszText);
		}
		// PBI: Added support for ImageList.
		if( mask & TCIF_IMAGE )
		{
			pItemT->iImage = pItem->iImage;
			pItemT->mask |= TCIF_IMAGE;
		}
		if( mask & TCIF_PARAM )
		{
			pItemT->lParam = pItem->lParam;
			pItemT->mask |= TCIF_PARAM;
		}

		// Repaint control
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		this->Invalidate();
		return TRUE;
	}
	*/

	TItem* GetItem(size_t nItem) const
	{
		ATLASSERT(nItem < m_Items.GetCount());
		if( nItem >= m_Items.GetCount() )
		{
			return NULL;
		}

		return m_Items[nItem];
	}

	int SetCurSel(int nItem, bool bNotify = true)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(m_hWnd));

		// NEW (DDB):
		// Even if the newly requested selection index is
		// the same index as the previous selection index,
		// the item might be different (as in the case of
		// InsertItem inserting a new item where the old
		// selection used to be).  We also call EnsureVisible
		// and UpdateLayout in this method, which we will want
		// called even if it is the same item
		//
		// OLD:
		// // Selecting same tab?  If so, we won't go through all the notifications
		// if( (int)nItem==m_iCurSel ) return m_iCurSel;

		if( nItem >= (int)m_Items.GetCount() )
		{
			nItem = m_iCurSel;
		}

		int iOldSel = m_iCurSel;
		// Send notification
		NMCTC2ITEMS nmh = {{ m_hWnd, this->GetDlgCtrlID(), CTCN_SELCHANGING }, iOldSel, nItem, {-1,-1}};
		if(bNotify)
		{
			if( TRUE == ::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh) )
			{
				// Cancel the attempt
				return -1;
			}
		}

		// Change tab
		m_iCurSel = nItem;

		if(m_iCurSel >= 0)
		{
			pT->EnsureVisible(m_iCurSel);
		}

		// Recalc new layout and redraw
		pT->UpdateLayout();
		this->Invalidate();

		if(bNotify)
		{
			// Send notification
			nmh.hdr.code = CTCN_SELCHANGE;
			::SendMessage(GetParent(), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
		}
		return iOldSel;
	}

	int GetCurSel() const
	{
		return m_iCurSel;
	}

	int GetItemCount() const
	{
		return (int)m_Items.GetCount();
	}

	int HitTest(LPCTCHITTESTINFO pHitTestInfo) const
	{
		ATLASSERT(!::IsBadWritePtr(pHitTestInfo,sizeof(CTCHITTESTINFO)));
		pHitTestInfo->flags = CTCHT_NOWHERE;
		if(::PtInRect(&m_rcTabItemArea, pHitTestInfo->pt))
		{
			// TODO: Do a smarter search.  Currently,
			//  the tabs are always going to be left to right.
			//  Use this knowledge to do a better spacial search.

			RECT rcItemDP = {0};
			size_t nCount = m_Items.GetCount();
			for( size_t i=0; i<nCount; ++i )
			{
				// NOTE: GetItemRect accounts for any scroll
				this->GetItemRect(i, &rcItemDP);

				if( ::PtInRect(&rcItemDP, pHitTestInfo->pt) )
				{
					// TODO: check for ONITEMLABEL, ONITEMICON
					pHitTestInfo->flags = CTCHT_ONITEM;
					return (int)i;
				}
			}
		}
		else
		{
			if(::PtInRect(&m_rcCloseButton, pHitTestInfo->pt))
			{
				pHitTestInfo->flags = CTCHT_ONCLOSEBTN;
			}
			else if(::PtInRect(&m_rcScrollRight, pHitTestInfo->pt))
			{
				pHitTestInfo->flags = CTCHT_ONSCROLLRIGHTBTN;
			}
			else if(::PtInRect(&m_rcScrollLeft, pHitTestInfo->pt))
			{
				pHitTestInfo->flags = CTCHT_ONSCROLLLEFTBTN;
			}
		}
		return -1;
	}

	bool EnsureVisible(int nItem, bool bPartialOK = false, bool bRecalcAndRedraw = false)
	{
		bool bAdjusted = false;

		// Adjust scroll offset so that item is visible
		if(0 != (m_dwState & (ectcOverflowLeft|ectcOverflowRight)))
		{
			if(nItem < 0 || nItem >= (int)m_Items.GetCount())
			{
				return false;
			}

			// TODO: Depend on some system metric for this value
			int nScrollToViewPadding = 20;

			RECT rcItemDP = {0};
			this->GetItemRect(nItem, &rcItemDP);
			if(rcItemDP.left < m_rcTabItemArea.left)
			{
				if(!bPartialOK || (rcItemDP.right < m_rcTabItemArea.left))
				{
					m_iScrollOffset += (m_rcTabItemArea.left-rcItemDP.left) + nScrollToViewPadding;
					bAdjusted = true;
				}
			}
			else if(rcItemDP.right > m_rcTabItemArea.right)
			{
				if(!bPartialOK || (rcItemDP.left > m_rcTabItemArea.right))
				{
					m_iScrollOffset -= (rcItemDP.right-m_rcTabItemArea.right) + nScrollToViewPadding;
					bAdjusted = true;
				}
			}

			// Note: UpdateLayout should call UpdateScrollOverflowStatus which
			//  will catch m_iScrollOffset being scrolled too far either direction
		}

		if(bAdjusted && bRecalcAndRedraw)
		{
			T* pT = static_cast<T*>(this);
			if(bRecalcAndRedraw)
			{
				pT->UpdateLayout();
				this->Invalidate();
			}
			else
			{
				pT->UpdateScrollOverflowStatus();
				pT->UpdateTabItemTooltipRects();
			}
		}

		return true;
	}

	int GetRowCount() const
	{
		return 1;
	}

	DWORD SetItemSize(size_t nItem, int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nItem < m_Items.GetCount());
		if( nItem >= m_Items.GetCount() )
		{
			return 0;
		}

		// TODO: Review this method.  It seems that all the tabs
		//  after the one being set would have the wrong RECT.
		//  (unless the caller is iterating through all of the items)

		TItem* pItem = m_Items[nItem];
		ATLASSERT(pItem != NULL);

		RECT rcOld = pItem->GetRect();
		RECT rcNew = { rcOld.left, rcOld.top, rcOld.left + cx, rcOld.top cy };
		pItem->SetRect(rcNew);
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		this->Invalidate();
		return MAKELONG(rcOld.right-rcOld.left, rcOld.bottom-rcOld.top);
	}

	BOOL GetItemRect(size_t nItem, RECT *prcItem) const
	{
		ATLASSERT(prcItem);
		if( prcItem == NULL ) return FALSE;
		if( nItem >= m_Items.GetCount() )
		{
			::SetRectEmpty(prcItem);
			return FALSE;
		}
		TItem* pItem = m_Items[nItem];
		ATLASSERT(pItem != NULL);
		if(pItem)
		{
			*prcItem = pItem->GetRect();
		}

		// Adjust for any scroll, so that the caller
		// gets the RECT in device coordinates
		// instead of logical coordinates
		::OffsetRect(prcItem, m_iScrollOffset, 0);

		return TRUE;
	}

	BOOL HighlightItem(size_t nItem, bool bHighlight = true)
	{
		ATLASSERT(nItem < m_Items.GetCount());
		if(nItem >= m_Items.GetCount())
		{
			return FALSE;
		}

		TItem* pItem = m_Items[nItem];
		ATLASSERT(pItem != NULL);
		if(pItem)
		{
			bool bCurrentHighlight = pItem->IsHighlighted();
			if(bCurrentHighlight != bHighlight)
			{
				pItem->SetHighlighted(bHighlight);

				RECT rcItem = {0};
				this->GetItemRect(nItem, &rcItem);
				this->InvalidateRect(&rcItem);
			}
		}
		return TRUE;
	}

	void SetPadding(int iPadding) 
	{ 
		m_iPadding = iPadding; 
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		this->Invalidate();
	};

	// FindItem:    Find the next tab item matching the search criteria
	//              The functions is meant to mimic how
	//              CListViewCtrl::FindItem and LVM_FINDITEM work,
	//              since there are no comparable messages or functions
	//              for a tab control
	//
	//  eFlags should specify a mask of things to check for,
	//   and have the corresponding fields set in pFindItem.
	//   For example, set the flags to eCustomTabItem_TabView and set the
	//   tab view on pFindItem to search for a tab with a particular HWND.
	//  If nStart is -1, the search begins from the beginning.
	//   If nStart is not -1, the search begins with the item
	//   just after nStart (like with LVM_FINDITEM).
	//  If a matching item is found, its index is returned.
	//   Otherwise -1 is returned.
	int FindItem(TItem* pFindItem, DWORD eFlags, int nStart = -1) const
	{
		if(nStart < 0)
		{
			nStart = -1;
		}

		// Find the next item matching the criteria specified
		size_t nCount = m_Items.GetCount();
		for( size_t i=(nStart+1); i < nCount; ++i )
		{
			if(m_Items[i]->MatchItem(pFindItem, eFlags))
			{
				return (int)i;
			}
		}

		return -1;
	}

	int FindItem(HWND hwndFindItem, int nStart = -1) const
	{
		if(nStart < 0)
		{
			nStart = -1;
		}

		// Find the next item matching the criteria specified
		size_t nCount = m_Items.GetCount();
		for( size_t i=(nStart+1); i < nCount; ++i )
		{
			if(m_Items[i]->GetTabView() == hwndFindItem)
			{
				return (int)i;
			}
		}

		return -1;
	}
};

#endif // __CUSTOMTABCTRL_H__

