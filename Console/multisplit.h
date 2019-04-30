/****************************************************************************
*                                                                          *
* MultiSplit.h: Scrolling multi-panel splitter for use with WTL            *
*                                                                          *
* Modified by Christophe Bucher (cbucher@users.sourceforge.net)            *
*                                                                          *
* Based on version written by Ted Szoczei (ted.szoczei@nimajin.com),       *
* Nimajin Software Consulting                                              *
* for Microtronix Systems Ltd.                                             *
* Copyright (c) 2003-2004 Ted Szoczei.                                     *
*                                                                          *
* Portions adapted from Windows Template Library - WTL version 7.0,        *
*    atlsplit.h                                                            *
* Copyright (C) 1997-2002 Microsoft Corporation, All rights reserved.      *
*                                                                          *
* Permission to copy, use, sell and distribute this file is granted        *
* provided this copyright notice appears in all copies.                    *
* Permission to modify the code herein and to distribute modified code is  *
* granted provided this copyright notice appears in all copies, and a      *
* notice that the code was modified is included with the copyright notice. *
*                                                                          *
* This software and information is provided "as is" without express or im- *
* plied warranty, and with no claim as to its suitability for any purpose. *
*                                                                          *
****************************************************************************/



#ifndef __ATLMULTISPLIT_H__
#define __ATLMULTISPLIT_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error multisplit.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error multisplit.h requires atlwin.h to be included first
#endif


#include <AtlTypes.h>


/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CMultiSplitImpl<T>
// CMultiSplitWindowImpl<T, TBase, TWinTraits>
// CMultiSplitWindow


namespace WTL
{

	/////////////////////////////////////////////////////////////////////////////
	// CMultiSplitImpl - Provides multi-pane splitter support to any window

	class CMultiSplitPane
	{
	public:
		typedef enum
		{
			NONE,
			HORIZONTAL,
			VERTICAL
		}
		SPLITTYPE;

		typedef enum
		{
			ROOT,
			TOP,
			BOTTOM,
			LEFT,
			RIGHT,
			ZOOM
		}
		WHERE;

		HWND window;
		int  x;
		int  y;
		int  width;
		int  height;
		bool visible;             // visible ?
		SPLITTYPE splitType;      // split type
		int       splitRatio;     // ratio in percent of pane0
		CMultiSplitPane* pane0;   // if splitted left or top pane
		CMultiSplitPane* pane1;   // if splitted right or bottom pane
		CMultiSplitPane* parent;  // pane containing this pane

		CMultiSplitPane(void)
			: window    (0)
			, x         (0)
			, y         (0)
			, width     (0)
			, height    (0)
			, visible   (true)
			, splitType (NONE)
			, splitRatio(100)
			, pane0     (0)
			, pane1     (0)
			, parent    (0)
		{
		}

		CMultiSplitPane(HWND window, int x, int y, int width, int height, CMultiSplitPane* parent)
			: window    (window)
			, x         (x)
			, y         (y)
			, width     (width)
			, height    (height)
			, visible   (true)
			, splitType (NONE)
			, splitRatio(100)
			, pane0     (0)
			, pane1     (0)
			, parent    (parent)
		{
		}

		~CMultiSplitPane(void)
		{
			delete this->pane0;
			delete this->pane1;
		}

		WHERE whereAmI(void)
		{
			if( this->parent == 0 )
				return ROOT;

			bool isPane0 = this->parent->pane0 == this;
			if( this->parent->splitType == VERTICAL )
				return isPane0 ? LEFT : RIGHT;
			else
				return isPane0 ? TOP : BOTTOM;
		}

		bool split(SPLITTYPE splitType, int splitRatio, HWND windowPane0, HWND windowPane1, bool updateSize)
		{
			int pane0Width = 0, pane0Height = 0, pane1Width = 0, pane1Height = 0, pane1X = 0, pane1Y = 0;

			this->splitType  = splitType;
			this->splitRatio = splitRatio;

			if( updateSize )
			{
				if( this->splitType == HORIZONTAL )
				{
					pane1Width = pane0Width = this->width;
					pane0Height = (this->height - CMultiSplitPane::splitBarHeight) / 2;
					pane1Height = this->height - CMultiSplitPane::splitBarHeight - pane0Height;
					pane1X = this->x;
					pane1Y = this->y + pane0Height + CMultiSplitPane::splitBarHeight;
				}
				else
				{
					pane0Width = (this->width - CMultiSplitPane::splitBarWidth) / 2;
					pane1Width = this->width - CMultiSplitPane::splitBarWidth - pane0Width;
					pane1Height = pane0Height = this->height;
					pane1X = this->x + pane0Width + CMultiSplitPane::splitBarWidth;
					pane1Y = this->y;
				}

				if( pane0Width <= 0 || pane1Width <= 0 || pane0Height <= 0 || pane1Height <= 0 )
					// too small
					return false;
			}

			this->pane0 = new CMultiSplitPane(
				windowPane0,
				this->x, this->y,
				pane0Width, pane0Height,
				this);

			this->pane1 = new CMultiSplitPane(
				windowPane1,
				pane1X, pane1Y,
				pane1Width, pane1Height,
				this);

			this->window = 0;

			return true;
		}

		CMultiSplitPane* split(HWND windowPane1, SPLITTYPE splitType)
		{
			if( this->isSplitBar() )
				return 0;

			if( !split(splitType, 50, this->window, windowPane1, true) ) return nullptr;

			// resize two children
			this->pane0->resize(this->pane0->width, this->pane0->height);
			this->pane1->updateLayout();

			return this->pane1;
		}

		void resize(SPLITTYPE splitType, int delta)
		{
			if( this->parent == nullptr )
				return;

			if( this->parent->splitType == splitType )
			{
				this->parent->moveSplitBar(this->parent->pane0 == this? delta : -delta);
				this->parent->pane0->updateLayout();
				this->parent->pane1->updateLayout();
			}
			else
			{
				this->parent->resize(splitType, delta);
			}
		}

		CMultiSplitPane* remove(void)
		{
			if( this->isSplitBar() )
				return 0;

			if( this->parent )
			{
				CMultiSplitPane* survivor =
					this->window == this->parent->pane0->window?
					this->parent->pane1:
				this->parent->pane0;

				::SetWindowPos(
					this->window,
					0,
					0,
					0,
					0,
					0,
					SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);

				// parent = survivor
				CMultiSplitPane* result = this->parent;
				result->window = survivor->window;
				result->splitType = survivor->splitType;
				result->splitRatio = survivor->splitRatio;
				result->pane0 = survivor->pane0;
				if( result->pane0 )
					result->pane0->parent = result;
				result->pane1 = survivor->pane1;
				if( result->pane1 )
					result->pane1->parent = result;

				// resize parent
				result->resize(result->width, result->height);

				// delete the two children of the parent
				survivor->pane0 = 0;
				survivor->pane1 = 0;
				delete survivor;
				delete this;

#ifdef _DEBUG
				ATLTRACE(L"%08lx-remove returns\n",
					::GetCurrentThreadId());
				result->dump(0, result->parent);
#endif

				return result;
			}

			return 0;
		}

		void resize(int width, int height)
		{
			this->width = width;
			this->height = height;
			if( this->pane0 )
			{
				int pane0Width, pane0Height, pane1Width, pane1Height, pane1X, pane1Y;

				if( splitType == HORIZONTAL )
				{
					pane1Width = pane0Width = this->width;
					pane0Height = ::MulDiv(this->height - CMultiSplitPane::splitBarHeight, this->splitRatio, 100);
					pane1Height = this->height - CMultiSplitPane::splitBarHeight - pane0Height;
					pane1X = this->x;
					pane1Y = this->y + pane0Height + CMultiSplitPane::splitBarHeight;
				}
				else
				{
					pane0Width = ::MulDiv(this->width - CMultiSplitPane::splitBarWidth, this->splitRatio, 100);
					pane1Width = this->width - CMultiSplitPane::splitBarWidth - pane0Width;
					pane1Height = pane0Height = this->height;
					pane1X = this->x + pane0Width + CMultiSplitPane::splitBarWidth;
					pane1Y = this->y;
				}

				this->pane0->x = this->x;
				this->pane0->y = this->y;
				this->pane0->resize(pane0Width, pane0Height);
				this->pane1->x = pane1X;
				this->pane1->y = pane1Y;
				this->pane1->resize(pane1Width, pane1Height);
			}
			else
			{
				// resize pane view
				this->updateLayout();
			}
		}

		void updateLayout(void)
		{
			::SetWindowPos(
				this->window,
				0,
				this->x,
				this->y,
				this->width,
				this->height,
				SWP_NOZORDER | SWP_NOACTIVATE);

			ATLTRACE(_T("CMultiSplitPane::updateLayout: %d %d (%d %d)\n"),
				this->x, this->y,
				this->width, this->height);
		}

		int getSplitBarDelta(POINT point)
		{
			if( this->pane0 == nullptr )
				return 0;

			if( splitType == HORIZONTAL )
			{
				return point.y - (this->pane0->y + this->pane0->height);
			}
			else
			{
				return point.x - (this->pane0->x + this->pane0->width);
			}
		}

		void getSplitBarRect(RECT& rect, int delta)
		{
			if( this->pane0 )
			{
				if( this->splitType == HORIZONTAL )
				{
					rect.left   = this->x;
					rect.right  = this->x + this->width;
					rect.top    = this->pane1->y - CMultiSplitPane::splitBarHeight + delta;
					rect.bottom = this->pane1->y + delta;

					if( rect.top < this->y )
					{
						rect.top    = this->y;
						rect.bottom = this->y + CMultiSplitPane::splitBarHeight;
					}

					if( rect.bottom >= (this->y + this->height) )
					{
						rect.top    = this->y + this->height - CMultiSplitPane::splitBarHeight;
						rect.bottom = this->y + this->height;
					}
				}
				else
				{
					rect.left   = this->pane1->x - CMultiSplitPane::splitBarWidth + delta;
					rect.right  = this->pane1->x + delta;
					rect.top    = this->y;
					rect.bottom = this->y + this->height;

					if( rect.left < this->x )
					{
						rect.left  = this->x;
						rect.right = this->x + CMultiSplitPane::splitBarWidth;
					}

					if( rect.right > (this->x + this->width) )
					{
						rect.left  = this->x + this->width - CMultiSplitPane::splitBarWidth;
						rect.right = this->x + this->width;
					}
				}
			}
		}

		void moveSplitBar(int delta)
		{
			if( this->pane0 == nullptr )
				return;

			if( delta == 0 )
				return;

			RECT rect;
			this->getSplitBarRect(rect, delta);

			if( splitType == HORIZONTAL )
			{
				if( this->pane1->y == rect.bottom )
				{
					// move cancelled
					ATLTRACE(_T("move cancelled\n"));
					return;
				}

				this->pane0->resize(this->width, rect.top - this->y);
				this->pane1->y = rect.bottom;
				this->pane1->resize(this->width, this->y + this->height - rect.bottom);

				this->splitRatio = ::MulDiv(this->pane0->height, 100, this->height - CMultiSplitPane::splitBarWidth);
			}
			else
			{
				if( this->pane1->x == rect.right )
				{
					// move cancelled
					ATLTRACE(_T("move cancelled\n"));
					return;
				}

				this->pane0->resize(rect.left - this->x, this->height);
				this->pane1->x = rect.right;
				this->pane1->resize(this->x + this->width - rect.right, this->height);

				this->splitRatio = ::MulDiv(this->pane0->width, 100, this->width - CMultiSplitPane::splitBarWidth);
			}
		}

		bool contains(CMultiSplitPane* pane)
		{
			if( this == pane )
				return true;

			if( this->pane0 )
				return this->pane0->contains(pane) || this->pane1->contains(pane);
			else
				return false;
		}

		void swap(CMultiSplitPane* pane)
		{
			// exchange only panes with a window
			if( this->window == nullptr || pane->window == nullptr ) return;

			HWND _window = this->window;
			this->window = pane->window;
			pane->window = _window;

			this->updateLayout();
			pane->updateLayout();
		}

		void setVisiblity(bool boolVisible, const CMultiSplitPane* paneAllButThis)
		{
			if( this == paneAllButThis ) return;

			this->visible = boolVisible;

			if( this->window ) ::ShowWindow(this->window, boolVisible ? SW_SHOW : SW_HIDE);
			if( this->pane0 ) this->pane0->setVisiblity(boolVisible, paneAllButThis);
			if( this->pane1 ) this->pane1->setVisiblity(boolVisible, paneAllButThis);
		}

		CMultiSplitPane* get(WHERE position)
		{
			POINT point;

			switch( position )
			{
			case ROOT:
				return this->parent ? this->parent->get(ROOT) : this;
				break;

			case ZOOM:
				if( !this->visible ) return nullptr;        // we are not in visible sub tree
				if( this->parent == nullptr ) return this;  // we are at the tree's root
				return this->parent->visible ? this->parent->get(ZOOM) : this;
				break;

			case TOP:
				point.x = this->x;
				point.y = this->y - CMultiSplitPane::splitBarHeight - 1;
				break;

			case BOTTOM:
				point.x = this->x;
				point.y = this->y + this->height + CMultiSplitPane::splitBarHeight;
				break;

			case LEFT:
				point.x = this->x - CMultiSplitPane::splitBarWidth - 1;
				point.y = this->y;
				break;

			case RIGHT:
				point.x = this->x + this->width + CMultiSplitPane::splitBarWidth;
				point.y = this->y;
				break;

			default:
				return nullptr;
			}

			auto zoomPane = this->get(ZOOM);
			return zoomPane ? zoomPane->getPane(point) : nullptr;
		}

		CMultiSplitPane* get(HWND window)
		{
			if( this->window == window )
				return this;
			else if( this->pane0 )
			{
				CMultiSplitPane* result = this->pane0->get(window);
				if( result )
					return result;
				else
					return this->pane1->get(window);
			}
			else
				return nullptr;
		}

		CMultiSplitPane* get(POINT point)
		{
			CMultiSplitPane* result = nullptr;
			if( this->isInRect(point) )
			{
				if( this->pane0 )
				{
					result = this->pane0->get(point);
					if( result == nullptr )
						result = this->pane1->get(point);
				}

				if( result == nullptr )
					result = this;
			}
			return result;
		}

		bool isSplitBar(void)
		{
			return this->pane0? true : false;
		}

		CMultiSplitPane* getSplitBar(POINT point)
		{
			CMultiSplitPane* result = this->get(point);
			if( result && result->pane0 )
				return result;
			else
				return nullptr;
		}

		CMultiSplitPane* getPane(POINT point)
		{
			CMultiSplitPane* result = this->get(point);
			if( result && result->pane0 == 0 )
				return result;
			else
				return nullptr;
		}

		void draw(CDCHandle dc)
		{
			if( this->pane0 )
			{
				this->pane0->draw(dc);
				this->pane1->draw(dc);

				RECT rect;
				this->getSplitBarRect(rect, 0);

#ifdef _USE_AERO
				dc.FillRect (&rect, COLOR_SCROLLBAR);
#else
				dc.FillRect (&rect, COLOR_3DFACE);
				dc.DrawEdge (&rect, EDGE_RAISED, this->splitType == VERTICAL? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
#endif
			}
		}
#ifdef _DEBUG
		void dump(int level, CMultiSplitPane* parent)
		{
			ATLASSERT(this->parent == parent);

			wchar_t szTab [256];
			for(int i = 0; i < level; i ++)
				szTab[i] = L' ';
			szTab[level] = 0;

			ATLTRACE(L"%08lx-%swindow: %p(addr=%p)\n",
				::GetCurrentThreadId(),
				szTab,
				this->window,
				this);

			ATLTRACE(L"%08lx-%sparent: %p\n",
				::GetCurrentThreadId(),
				szTab,
				this->parent);

			ATLTRACE(L"%08lx-%s  size: %dx%d\n",
				::GetCurrentThreadId(),
				szTab,
				this->width,
				this->height);

			ATLTRACE(L"%08lx-%s pane0: %p\n",
				::GetCurrentThreadId(),
				szTab,
				this->pane0);
			if( this->pane0 )
				this->pane0->dump(level + 2, this);

			ATLTRACE(L"%08lx-%s pane1: %p\n",
				::GetCurrentThreadId(),
				szTab,
				this->pane1);
			if( this->pane1 )
				this->pane1->dump(level + 2, this);
		}
#endif

	private:
		bool isInRect(POINT point)
		{
			if( point.x >= this->x && 
				(point.x - this->x) < this->width &&
				point.y >= this->y && 
				(point.y - this->y) < this->height )
				return true;
			else
				return false;
		}

	public:
		static int splitBarWidth, splitBarHeight; // splitter bar width/height (system setting)
		static bool splitBarAutoSize;
	};

	template <class T>
	class CMultiSplitImpl
	{
	private:
		CRect            visibleRect;      // visible area defined by parent
	public:
		CMultiSplitPane  rootPane;
		CMultiSplitPane* maximizedPane;
		CMultiSplitPane* defaultFocusPane; // pane to focus when splitter gets focus

	private:
		CMultiSplitPane* previousFocusPane;// pane to focus when splitter gets focus
		CMultiSplitPane* resizingPane;     // pane with splitbar moving
		int              resizingDelta;    // splitbar delta move
		int              resizingDelta0;   // splitbar delta on click
		bool drawContentWhileResizing;     // resize content while moving splitter bar (system setting)
		int edgeWidth, edgeHeight;         // edge width/height (system setting)
		static HCURSOR vertCursor;         // cursor to display on vertical splitter bar
		static HCURSOR horzCursor;         // cursor to display on horizontal splitter bar

	public:

		// Constructor

		CMultiSplitImpl (void)
			: maximizedPane (&this->rootPane)
			, defaultFocusPane (&this->rootPane)
			, previousFocusPane (nullptr)
			, resizingPane (nullptr)
			, resizingDelta (0)
			, resizingDelta0 (0)
			, drawContentWhileResizing (true)
			, edgeWidth (0)
			, edgeHeight (0)
		{
			visibleRect.SetRectEmpty ();

			if (vertCursor == 0)
			{
				::EnterCriticalSection (&_Module.m_csStaticDataInit);
				if (vertCursor == 0)
					vertCursor = ::LoadCursor (0, IDC_SIZEWE);

				::LeaveCriticalSection (&_Module.m_csStaticDataInit);
			}
			if (horzCursor == 0)
			{
				::EnterCriticalSection (&_Module.m_csStaticDataInit);
				if (horzCursor == 0)
					horzCursor = ::LoadCursor (0, IDC_SIZENS);

				::LeaveCriticalSection (&_Module.m_csStaticDataInit);
			}
		}

		// Attributes

		void SetRect(LPRECT newRect = 0, bool update = true)
		{										// define visible area within parent
			T * pT = static_cast<T *> (this);

			if (newRect == 0)					// use all of parent's client area
				pT->GetClientRect (&visibleRect);// includes edges
			else
				visibleRect = *newRect;

			ATLTRACE(_T("CMultiSplitImpl::RectSet: %d %d %d %d (%d %d) from %p\n"),
				visibleRect.left, visibleRect.top, visibleRect.right, visibleRect.bottom,
				visibleRect.Width (), visibleRect.Height (), newRect);

			if (update)
			{
				UpdateLayout ();
			}
		}

    inline const CRect & GetRect() const
    {
      return visibleRect;
    }

		// draw all splitter bars & empty panes, occupied panes draw themselves

		void Draw (CDCHandle dc)
		{
			ATLASSERT(!(dc.m_hDC == 0));

#if 0
			CPen penYellow;
			penYellow.CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
			dc.SelectPen(penYellow);
			dc.FillSolidRect(&visibleRect, RGB(255, 0, 0));

			int marge = 0;
			dc.MoveTo(visibleRect.left + marge, visibleRect.top + marge);
			dc.LineTo(visibleRect.right - 1 - marge, visibleRect.top + marge);
			dc.LineTo(visibleRect.right - 1 - marge, visibleRect.bottom - 1 - marge);
			dc.LineTo(visibleRect.left + marge, visibleRect.bottom - 1 - marge);
			dc.LineTo(visibleRect.left + marge, visibleRect.top + marge);
			dc.LineTo(visibleRect.right - 1 - marge, visibleRect.bottom - 1 - marge);
			dc.MoveTo(visibleRect.right - 1 - marge, visibleRect.top + marge);
			dc.LineTo(visibleRect.left + marge, visibleRect.bottom - 1 - marge);
#endif

#if 0
			ATLTRACE(_T("CMultiSplitImpl::Draw: %d %d %d %d (%d %d)\n"),
				visibleRect.left, visibleRect.top, visibleRect.right, visibleRect.bottom,
				visibleRect.Width (), visibleRect.Height ());
#endif

			this->maximizedPane->draw(dc);
		}

		// Overrideable by derived classes

		void GhostBarDraw (void)
		{
			CRect BarRect;
			if( this->resizingPane )
			{
				this->resizingPane->getSplitBarRect(BarRect, this->resizingDelta - this->resizingDelta0);

				ATLTRACE(_T("CMultiSplitImpl::GhostBarDraw: %d %d %d %d (%d %d) delta = %d\n"),
					BarRect.left, BarRect.top, BarRect.right, BarRect.bottom,
					BarRect.Width (), BarRect.Height (),
					this->resizingDelta);

				// invert the brush pattern (looks just like frame window sizing)
				T * pT = static_cast<T *> (this);
				CWindowDC dc (pT->m_hWnd);
				BarRect.OffsetRect(this->edgeWidth, this->edgeHeight);

				CBrush GhostBrush = CDCHandle::GetHalftoneBrush ();
				if (!GhostBrush.IsNull ())
				{
					CBrushHandle OldBrush = dc.SelectBrush (GhostBrush);
					dc.PatBlt (BarRect.left, BarRect.top, BarRect.Width (), BarRect.Height (), PATINVERT);
					dc.SelectBrush (OldBrush);
				}
			}
		}

		// Message map and handlers

		typedef CMultiSplitImpl< T>	thisClass;
		BEGIN_MSG_MAP (thisClass)
			MESSAGE_HANDLER (WM_CREATE, OnCreate)
			MESSAGE_HANDLER (WM_PAINT, OnPaint)
			//MESSAGE_HANDLER (WM_PRINTCLIENT, OnPaint)
			MESSAGE_HANDLER (WM_SETCURSOR, OnSetCursor) // cursor in client area
			MESSAGE_HANDLER (WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER (WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER (WM_LBUTTONUP, OnLButtonUp)
#if 0
			MESSAGE_HANDLER (WM_VSCROLL, OnScroll)    // scroll bar messages
			MESSAGE_HANDLER (WM_HSCROLL, OnScroll)
			MESSAGE_HANDLER (WM_MOUSEWHEEL, OnMouseWheel)
			MESSAGE_HANDLER (WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
#endif
			MESSAGE_HANDLER (WM_SETFOCUS, OnSetFocus)
			MESSAGE_HANDLER (WM_MOUSEACTIVATE, OnMouseActivate)
			MESSAGE_HANDLER (WM_SETTINGCHANGE, OnSettingChange)
			MESSAGE_HANDLER (WM_SYSCOLORCHANGE, OnSettingChange)
		END_MSG_MAP ()

		LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & bHandled)
		{
			SystemSettingsGet (false);

			CRect InitRect (0, 0, ((CREATESTRUCT *) lParam)->cx, ((CREATESTRUCT *) lParam)->cy);
			SetRect(&InitRect, false);

			bHandled = FALSE;
			return 0;
		}

		LRESULT OnPaint (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
		{
			T * pT = static_cast<T *> (this);
			CPaintDC dc (pT->m_hWnd);
			pT->Draw (dc.m_hDC);
			return 0;
		}

		LRESULT OnSetCursor (UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
		{
			T * pT = static_cast<T *> (this);
			if (reinterpret_cast<HWND>(wParam) == pT->m_hWnd && LOWORD (lParam) == HTCLIENT)
			{
				DWORD Position = ::GetMessagePos ();
				POINT Point = { GET_X_LPARAM (Position), GET_Y_LPARAM (Position) };
				pT->ScreenToClient (&Point);
				if ( this->maximizedPane->getSplitBar(Point) )
					return 1;
			}
			bHandled = FALSE;
			return 0;
		}

		// Dragging splitbar resizes panes to either side of bar
		// Pane size is checked and changed on every receipt of this message.

		LRESULT OnMouseMove (UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
		{
			T * pT = static_cast<T *> (this);
			POINT Point = { GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam) };
			if ((wParam & MK_LBUTTON) && ::GetCapture () == pT->m_hWnd)
			{
				// resizing
				int delta = resizingPane->getSplitBarDelta(Point);

				if ( delta )
				{
					if (drawContentWhileResizing)
					{
						resizingPane->moveSplitBar(delta - this->resizingDelta0);
						pT->InvalidateRect(NULL);
						pT->UpdateWindow ();
					}
					else
					{
						GhostBarDraw ();
						this->resizingDelta = delta;
						GhostBarDraw ();
					}
				}
			}
			else
			{
				// not dragging, just set cursor
				CMultiSplitPane* splitBar = this->maximizedPane->getSplitBar(Point);
				if ( splitBar )
					::SetCursor (splitBar->splitType == CMultiSplitPane::VERTICAL ? vertCursor : horzCursor);

				bHandled = FALSE;
			}
			return 0;
		}

		// start splitbar drag

		LRESULT OnLButtonDown (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & bHandled)
		{
			POINT Point = { GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam) };
			this->resizingPane = this->maximizedPane->getSplitBar(Point);
			if( resizingPane )
			{
				this->resizingDelta0 = this->resizingDelta = resizingPane->getSplitBarDelta(Point);
				T * pT = static_cast<T *> (this);
				pT->SetCapture ();
				::SetCursor (resizingPane->splitType == CMultiSplitPane::VERTICAL ? vertCursor : horzCursor);
				if (!drawContentWhileResizing)
					GhostBarDraw ();
			}
			bHandled = FALSE;
			return 1;
		}

		// end splitbar drag

		LRESULT OnLButtonUp (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & bHandled)
		{
			if( this->resizingPane )
			{
				int delta = this->resizingDelta - this->resizingDelta0;
				if( !drawContentWhileResizing )
				{
					GhostBarDraw ();
					T * pT = static_cast<T *> (this);
					resizingPane->moveSplitBar(delta);
					pT->InvalidateRect(NULL);
					pT->UpdateWindow ();
				}

				HWND hwndPane0 = nullptr, hwndPane1 = nullptr;
				if( this->resizingPane->pane0 ) hwndPane0 = this->resizingPane->pane0->window;
				if( this->resizingPane->pane1 ) hwndPane1 = this->resizingPane->pane1->window;

				this->resizingPane = nullptr;
				::ReleaseCapture ();

				if( delta )
					this->OnSplitBarMove(hwndPane0, hwndPane1, true);
			}
			bHandled = FALSE;
			return 1;
		}

		virtual void OnSplitBarMove(HWND /*hwndPane0*/, HWND /*hwndPane1*/, bool /*boolEnd*/)
		{
		}

		LRESULT OnSetFocus (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM, BOOL & bHandled)
		{
			// give focus to defaultPane child
			//ATLTRACE(_T("CMultiSplitImpl::OnSetFocus: %p\n"), defaultFocusPane);
			if( defaultFocusPane && defaultFocusPane->window )
				::SetFocus(defaultFocusPane->window);

			bHandled = FALSE;
			return 1;
		}

		LRESULT OnMouseActivate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
		{
			T * pT = static_cast<T *> (this);
			LRESULT Result = pT->DefWindowProc (uMsg, wParam, lParam);
#if 0
			LPCTSTR Text;
			switch (Result)
			{
			case MA_ACTIVATE :
			Text = _T("MA_ACTIVATE");
			break;
			case MA_ACTIVATEANDEAT :
			Text = _T("MA_ACTIVATEANDEAT");
			break;
			case MA_NOACTIVATE :
			Text = _T("MA_NOACTIVATE");
			break;
			case MA_NOACTIVATEANDEAT :
			Text = _T("MA_NOACTIVATEANDEAT");
			break;
			}
			ATLTRACE(_T("CMultiSplitImpl::OnMouseActivate: %s\n"), Text);
#endif
			if (Result == MA_ACTIVATE || Result == MA_ACTIVATEANDEAT)
			{
				// select focus pane from mouse position
				DWORD Position = ::GetMessagePos ();
				POINT Point = { GET_X_LPARAM (Position), GET_Y_LPARAM (Position) };
				pT->ScreenToClient (&Point);

				CMultiSplitPane* pane = this->maximizedPane->getPane(Point);
				if( pane && !pane->isSplitBar() )
				{
					SetDefaultFocusPane(pane);
					//pT->SetFocus (); // focus child window
					ATLTRACE(_T("CMultiSplitImpl::OnMouseActivate: defaultFocusPane = %p\n"), this->defaultFocusPane);
				}
			}
			return Result;
		}

		void SetDefaultFocusPane(CMultiSplitPane* newDefaultPane, bool bAppActive = true)
		{
			bool boolNotify = newDefaultPane != this->defaultFocusPane;

			if( boolNotify )
				this->previousFocusPane = this->defaultFocusPane;

			this->defaultFocusPane = newDefaultPane;
			if( bAppActive && newDefaultPane && ::IsWindow(newDefaultPane->window) )
				::SetFocus(newDefaultPane->window);

			if( boolNotify )
				this->OnPaneChanged();

#if 0
			this->tree.dump(0, nullptr);
			ATLTRACE(L"defaultFocusPane=%p\n", defaultFocusPane);
			ATLTRACE(L"previousFocusPane=%p\n", previousFocusPane);
#endif
		}

		bool SwapWithPreviousFocusPane()
		{
			if( this->defaultFocusPane &&
			    this->previousFocusPane &&
			    this->defaultFocusPane != this->previousFocusPane &&
			    this->maximizedPane->contains(this->defaultFocusPane) &&        // exchange only panes contained in the visible tree
			    this->maximizedPane->contains(this->previousFocusPane) &&
			    this->defaultFocusPane->window &&                               // exchange only panes with a window
			    this->previousFocusPane->window )
			{
				this->defaultFocusPane->swap(this->previousFocusPane);
				SetDefaultFocusPane(this->previousFocusPane);

				return true;
			}

			return false;
		}

		bool Split(HWND windowPane1, CMultiSplitPane::SPLITTYPE splitType)
		{
			if( this->defaultFocusPane == nullptr )
				return false;

			CMultiSplitPane* pane1 = this->defaultFocusPane->split(
				windowPane1,
				splitType);

			if( pane1 == nullptr )
				return false;

			// defaultFocusPane has been split
			// pane0 must be used as new previousFocusPane
			this->defaultFocusPane = this->defaultFocusPane->pane0;
			this->SetDefaultFocusPane(pane1);

			return true;
		}

		bool Merge(CMultiSplitImpl & other, CMultiSplitPane::SPLITTYPE splitType)
		{
			// restore visibility to whole other tree before merging
			other.Restore(false);

#ifdef _DEBUG
			ATLTRACE(L"%08lx-CMultiSplitImpl::Merge into tree\n", ::GetCurrentThreadId());
			this->rootPane.dump(0, 0);
			ATLTRACE(L"%08lx-CMultiSplitImpl::Merge into defaultFocusPane\n", ::GetCurrentThreadId());
			if( this->defaultFocusPane )
				this->defaultFocusPane->dump(0, this->defaultFocusPane->parent);

			ATLTRACE(L"%08lx-CMultiSplitImpl::Merge from tree\n", ::GetCurrentThreadId());
			other.rootPane.dump(0, 0);
			ATLTRACE(L"%08lx-CMultiSplitImpl::Merge from defaultFocusPane\n", ::GetCurrentThreadId());
			if( other.defaultFocusPane )
				other.defaultFocusPane->dump(0, other.defaultFocusPane->parent);
#endif
			// clone maximizedPane (visible tree) into pane0
			CMultiSplitPane * pane0 = new CMultiSplitPane(*this->maximizedPane);
			if( pane0->pane0 ) pane0->pane0->parent = pane0;
			if( pane0->pane1 ) pane0->pane1->parent = pane0;

			// clone other.rootPane into pane1
			CMultiSplitPane * pane1 = new CMultiSplitPane(other.rootPane);
			if( pane1->pane0 ) pane1->pane0->parent = pane1;
			if( pane1->pane1 ) pane1->pane1->parent = pane1;

			// set panes
			this->maximizedPane->window = nullptr;
			this->maximizedPane->splitType = splitType;
			this->maximizedPane->splitRatio = 50;
			this->maximizedPane->pane0 = pane0;
			this->maximizedPane->pane1 = pane1;
			pane0->parent = this->maximizedPane;
			pane1->parent = this->maximizedPane;

			// get current defaultFocusPane
			// if unsplit current defaultFocusPane is pane0
			CMultiSplitPane * defaultFocusPane0 = pane0->isSplitBar() ? this->defaultFocusPane : pane0;

			// get other defaultFocusPane
			// if unsplit other defaultFocusPane is pane1
			CMultiSplitPane * defaultFocusPane1 = pane1->isSplitBar() ? other.defaultFocusPane : pane1;

			// clean other tree
			other.rootPane.window = nullptr;
			other.rootPane.pane0 = nullptr;
			other.rootPane.pane1 = nullptr;
			other.defaultFocusPane = nullptr;
			other.previousFocusPane = nullptr;

			// resize
			UpdateLayout();

#ifdef _DEBUG
			ATLTRACE(L"%08lx-CMultiSplitImpl::Merge merged tree\n", ::GetCurrentThreadId());
			this->rootPane.dump(0, 0);
#endif

			// current defaultFocusPane keeps the same
			// other defaultFocusPane becomes previousFocusPane
			this->defaultFocusPane = defaultFocusPane1;
			this->SetDefaultFocusPane(defaultFocusPane0);

			return true;
		}

		bool Remove()
		{
#ifdef _DEBUG
			ATLTRACE(L"%08lx-CMultiSplitImpl::Remove tree\n", ::GetCurrentThreadId());
			this->rootPane.dump(0, 0);
			ATLTRACE(L"%08lx-CMultiSplitImpl::Remove defaultFocusPane\n", ::GetCurrentThreadId());
			if( this->defaultFocusPane )
				this->defaultFocusPane->dump(0, this->defaultFocusPane->parent);
#endif

			if( this->defaultFocusPane )
				this->SetDefaultFocusPane(this->defaultFocusPane->remove());
			else
				return false;

			return true;
		}

		bool Maximize()
		{
			if( this->defaultFocusPane == nullptr )
				return false;

			this->maximizedPane = this->defaultFocusPane;
			this->maximizedPane->x = 0;
			this->maximizedPane->y = 0;

			this->rootPane.setVisiblity(false, this->maximizedPane);
			UpdateLayout();

			// prevent swap with a hidden pane
			this->previousFocusPane = nullptr;

			return true;
		}

		bool Restore(bool boolUpdateLayout = true)
		{
			if( this->maximizedPane == &this->rootPane )
				return false;

			this->rootPane.setVisiblity(true, this->maximizedPane);
			this->maximizedPane = &this->rootPane;
			if( boolUpdateLayout ) UpdateLayout();

			return true;
		}

		virtual void OnPaneChanged(void)
		{
		}

		LRESULT OnSettingChange (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
		{
			SystemSettingsGet (true);
			return 0;
		}

		// Implementation - internal helpers

		// Invalidate splitter bar areas and empty spaces, set child window pane positions & sizes.
		// Calculate contentSize.
		// Called whenever SystemSettings changed, RectSet, LButtonUp, child windows changed

		void UpdateLayout (void)
		{
			T * pT = static_cast<T *> (this);
			pT->InvalidateRect(NULL);
			this->maximizedPane->resize(this->visibleRect.Width(), this->visibleRect.Height());
		}


		// get the bar & edge widthssplitBarWidth

		void SystemSettingsGet (bool update)
		{
			if( !CMultiSplitPane::splitBarAutoSize )
			{
#if _USE_AERO
				CMultiSplitPane::splitBarWidth  = ::GetSystemMetrics (SM_CXSMICON) / 4;
				CMultiSplitPane::splitBarHeight = ::GetSystemMetrics (SM_CYSMICON) / 4;
#else
				CMultiSplitPane::splitBarWidth  = ::GetSystemMetrics(SM_CXSIZEFRAME);
				CMultiSplitPane::splitBarHeight = ::GetSystemMetrics(SM_CYSIZEFRAME);
#endif
			}

			this->edgeWidth  = ::GetSystemMetrics (SM_CXEDGE);
			this->edgeHeight = ::GetSystemMetrics (SM_CYEDGE);

			::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &drawContentWhileResizing, 0);
			drawContentWhileResizing = false;

			T * pT = static_cast<T *> (this);
			if (update && pT->IsWindow ())
			{
				UpdateLayout ();
				pT->UpdateWindow ();
			}
		}

	};


	template <class T> HCURSOR CMultiSplitImpl<T>::vertCursor = 0;
	template <class T> HCURSOR CMultiSplitImpl<T>::horzCursor = 0;


	/////////////////////////////////////////////////////////////////////////////
	// CMultiSplitWindowImpl - Implements a splitter window

	template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
	class ATL_NO_VTABLE CMultiSplitWindowImpl
		:	public CWindowImpl< T, TBase, TWinTraits >, 
		public CMultiSplitImpl< T >
	{
	public :
		DECLARE_WND_CLASS_EX(0, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

		typedef CMultiSplitWindowImpl< T , TBase, TWinTraits >	thisClass;
		typedef CMultiSplitImpl< T > baseClass;
		BEGIN_MSG_MAP (thisClass)
			MESSAGE_HANDLER (WM_ERASEBKGND, OnEraseBackground)
			MESSAGE_HANDLER (WM_SIZE, OnSize)
			CHAIN_MSG_MAP (baseClass)
			FORWARD_NOTIFICATIONS ()
		END_MSG_MAP ()

		LRESULT OnEraseBackground (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
		{
			return 1;							// handled, no background painting needed
		}

		LRESULT OnSize (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
		{
			CRect Rect (0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			SetRect(&Rect);
			return 0;							// handled
		}
	};


	/////////////////////////////////////////////////////////////////////////////
	// CMultiSplitWindow - Implements a splitter window to be used as is

	class CMultiSplitWindow : public CMultiSplitWindowImpl<CMultiSplitWindow>
	{
	public :
		DECLARE_WND_CLASS_EX (_T ("MultiSplitWindow"), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)
	};


}; //namespace WTL


#endif // __ATLMULTISPLIT_H__
