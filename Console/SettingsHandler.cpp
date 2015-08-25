#include "stdafx.h"
#include "resource.h"

#include "XmlHelper.h"
#include "SettingsHandler.h"

using namespace boost::algorithm;

//////////////////////////////////////////////////////////////////////////////

extern std::shared_ptr<ImageHandler>		g_imageHandler;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleSettings::ConsoleSettings()
: strShell(L"")
, strInitialDir(L"")
, dwRefreshInterval(100)
, dwChangeRefreshInterval(10)
, dwRows(25)
, dwColumns(80)
, dwBufferRows(500)
, dwBufferColumns(0)
, bStartHidden(false)
, bAutoSaveSession(false)
, bAutoLoadSession(false)
, bSaveSize(false)
, backgroundTextOpacity(255)
, dwCursorStyle(0)
, crCursorColor(RGB(255, 255, 255))
, backgroundImageType(bktypeNone)
, crBackgroundColor(RGB(0, 0, 0))
, imageData()
{
	defaultConsoleColors[0]	= 0x000000;
	defaultConsoleColors[1]	= 0x800000;
	defaultConsoleColors[2]	= 0x008000;
	defaultConsoleColors[3]	= 0x808000;
	defaultConsoleColors[4]	= 0x000080;
	defaultConsoleColors[5]	= 0x800080;
	defaultConsoleColors[6]	= 0x008080;
	defaultConsoleColors[7]	= 0xC0C0C0;
	defaultConsoleColors[8]	= 0x808080;
	defaultConsoleColors[9]	= 0xFF0000;
	defaultConsoleColors[10]= 0x00FF00;
	defaultConsoleColors[11]= 0xFFFF00;
	defaultConsoleColors[12]= 0x0000FF;
	defaultConsoleColors[13]= 0xFF00FF;
	defaultConsoleColors[14]= 0x00FFFF;
	defaultConsoleColors[15]= 0xFFFFFF;

	::CopyMemory(consoleColors, defaultConsoleColors, sizeof(COLORREF)*16);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval, 100);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval, 10);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows, 25);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns, 80);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows, 500);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns, 0);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"start_hidden"), bStartHidden, false);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"auto_save_session"), bAutoSaveSession, false);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"auto_load_session"), bAutoLoadSession, false);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"save_size"), bSaveSize, false);

	// old config compatibility
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"background_text_opacity"), backgroundTextOpacity, 255);

	if( !XmlHelper::LoadColors(pConsoleElement, consoleColors, backgroundTextOpacity) )
		::CopyMemory(consoleColors, defaultConsoleColors, sizeof(COLORREF)*16);

	CComPtr<IXMLDOMElement>	pCursorElement;
	if (SUCCEEDED(XmlHelper::AddDomElementIfNotExist(pConsoleElement, CComBSTR(L"cursor"), pCursorElement)))
	{
		XmlHelper::GetAttribute(pCursorElement, CComBSTR(L"style"), dwCursorStyle, 0);
		XmlHelper::GetRGBAttribute(pCursorElement, crCursorColor, RGB(255, 255, 255));
	}

	CComPtr<IXMLDOMElement>	pBackgroundElement;
	if (SUCCEEDED(XmlHelper::AddDomElementIfNotExist(pConsoleElement, CComBSTR(L"background"), pBackgroundElement)))
	{
		DWORD dwBackgroundImageType = 0;

		XmlHelper::GetAttribute(pBackgroundElement, CComBSTR(L"type"), dwBackgroundImageType, 0);
		backgroundImageType = static_cast<BackgroundImageType>(dwBackgroundImageType);

		XmlHelper::GetRGBAttribute(pBackgroundElement, crBackgroundColor, RGB(0, 0, 0));

		// load image settings and let ImageHandler return appropriate bitmap
		CComPtr<IXMLDOMElement>	pImageElement;
		if (SUCCEEDED(XmlHelper::AddDomElementIfNotExist(pBackgroundElement, CComBSTR(L"image"), pImageElement)))
		{
			CComPtr<IXMLDOMElement>	pTintElement;
			if (SUCCEEDED(XmlHelper::AddDomElementIfNotExist(pImageElement, CComBSTR(L"tint"), pTintElement)))
			{
				XmlHelper::GetRGBAttribute(pTintElement, imageData.crTint, RGB(0, 0, 0));
				XmlHelper::GetAttribute(pTintElement, CComBSTR(L"opacity"), imageData.byTintOpacity, 0);
			}

			DWORD dwImagePosition = 0;

			XmlHelper::GetAttribute(pImageElement, CComBSTR(L"file"),     imageData.strFilename, wstring(L""));
			XmlHelper::GetAttribute(pImageElement, CComBSTR(L"relative"), imageData.bRelative,   false);
			XmlHelper::GetAttribute(pImageElement, CComBSTR(L"extend"),   imageData.bExtend,     false);
			XmlHelper::GetAttribute(pImageElement, CComBSTR(L"position"), dwImagePosition,       0);

			imageData.imagePosition = static_cast<ImagePosition>(dwImagePosition);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"start_hidden"), bStartHidden);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"auto_save_session"), bAutoSaveSession);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"auto_load_session"), bAutoLoadSession);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"save_size"), bSaveSize);

	XmlHelper::SaveColors(pConsoleElement, consoleColors, backgroundTextOpacity);

	CComPtr<IXMLDOMElement>	pCursorElement;
	if (FAILED(XmlHelper::GetDomElement(pConsoleElement, CComBSTR(L"cursor"), pCursorElement))) return false;
	XmlHelper::SetAttribute(pCursorElement, CComBSTR(L"style"), dwCursorStyle);
	XmlHelper::SetAttribute(pCursorElement, CComBSTR(L"r"), GetRValue(crCursorColor));
	XmlHelper::SetAttribute(pCursorElement, CComBSTR(L"g"), GetGValue(crCursorColor));
	XmlHelper::SetAttribute(pCursorElement, CComBSTR(L"b"), GetBValue(crCursorColor));

	// add <background> tag
	CComPtr<IXMLDOMElement>	pBackgroundElement;
	if (FAILED(XmlHelper::GetDomElement(pConsoleElement, CComBSTR(L"background"), pBackgroundElement))) return false;

	XmlHelper::SetAttribute(pBackgroundElement, CComBSTR(L"type"), backgroundImageType);
	XmlHelper::SetRGBAttribute(pBackgroundElement, crBackgroundColor);

	// add <image> tag
	CComPtr<IXMLDOMElement>	pImageElement;
	if (FAILED(XmlHelper::GetDomElement(pBackgroundElement, CComBSTR(L"image"), pImageElement))) return false;

	if (backgroundImageType == bktypeImage)
	{
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"file"),     imageData.strFilename);
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"relative"), imageData.bRelative ? true : false);
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"extend"),   imageData.bExtend ? true : false);
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"position"), static_cast<DWORD>(imageData.imagePosition));
	}
	else
	{
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"file"), wstring(L""));
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"relative"), false);
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"extend"), false);
		XmlHelper::SetAttribute(pImageElement, CComBSTR(L"position"), 0);
	}

	// add <tint> tag
	CComPtr<IXMLDOMElement>	pTintElement;
	if (FAILED(XmlHelper::GetDomElement(pImageElement, CComBSTR(L"tint"), pTintElement))) return false;

	XmlHelper::SetAttribute(pTintElement, CComBSTR(L"opacity"), imageData.byTintOpacity);
	XmlHelper::SetRGBAttribute(pTintElement, imageData.crTint);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleSettings& ConsoleSettings::operator=(const ConsoleSettings& other)
{
	strShell				= other.strShell;
	strInitialDir			= other.strInitialDir;

	dwRefreshInterval		= other.dwRefreshInterval;
	dwChangeRefreshInterval	= other.dwChangeRefreshInterval;
	dwRows					= other.dwRows;
	dwColumns				= other.dwColumns;
	dwBufferRows			= other.dwBufferRows;
	dwBufferColumns			= other.dwBufferColumns;
	bStartHidden			= other.bStartHidden;
	bAutoSaveSession		= other.bAutoSaveSession;
	bAutoLoadSession		= other.bAutoLoadSession;
	bSaveSize				= other.bSaveSize;

	::CopyMemory(defaultConsoleColors, other.defaultConsoleColors, sizeof(COLORREF)*16);
	::CopyMemory(consoleColors, other.consoleColors, sizeof(COLORREF)*16);

	backgroundTextOpacity = other.backgroundTextOpacity;
	dwCursorStyle         = other.dwCursorStyle;
	crCursorColor         = other.crCursorColor;

	backgroundImageType   = other.backgroundImageType;
	crBackgroundColor     = other.crBackgroundColor;
	imageData             = other.imageData;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FontSettings::FontSettings()
: strName(L"Courier New")
, dwSize(10)
, dwExtraWidth(0)
, bBold(false)
, bItalic(false)
, fontSmoothing(fontSmoothDefault)
, bUseColor(false)
, crFontColor(0)
, bBoldIntensified(false)
, bItalicIntensified(false)
{
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	int nFontSmoothing;

	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"name"), strName, wstring(L"Courier New"));
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"size"), dwSize, 10);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"extra_width"), dwExtraWidth, 0);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"bold"), bBold, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"italic"), bItalic, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"smoothing"), nFontSmoothing, 0);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"bold_intensified"), bBoldIntensified, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"italic_intensified"), bItalicIntensified, false);

	fontSmoothing = static_cast<FontSmoothing>(nFontSmoothing);

	CComPtr<IXMLDOMElement>	pColorElement;

	if (FAILED(XmlHelper::GetDomElement(pFontElement, CComBSTR(L"color"), pColorElement))) return false;

	XmlHelper::GetAttribute(pColorElement, CComBSTR(L"use"), bUseColor, false);
	XmlHelper::GetRGBAttribute(pColorElement, crFontColor, RGB(0, 0, 0));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"name"), strName);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"size"), dwSize);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"extra_width"), dwExtraWidth);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"bold"), bBold);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"italic"), bItalic);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"smoothing"), static_cast<int>(fontSmoothing));
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"bold_intensified"), bBoldIntensified);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"italic_intensified"), bItalicIntensified);

	CComPtr<IXMLDOMElement>	pColorElement;

	if (FAILED(XmlHelper::GetDomElement(pFontElement, CComBSTR(L"color"), pColorElement))) return false;

	XmlHelper::SetAttribute(pColorElement, CComBSTR(L"use"), bUseColor);
	XmlHelper::SetRGBAttribute(pColorElement, crFontColor);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FontSettings& FontSettings::operator=(const FontSettings& other)
{
	strName			= other.strName;
	dwSize			= other.dwSize;
	dwExtraWidth		= other.dwExtraWidth;
	bBold			= other.bBold;
	bItalic			= other.bItalic;
	fontSmoothing	= other.fontSmoothing;
	bBoldIntensified		= other.bBoldIntensified;
	bItalicIntensified		= other.bItalicIntensified;

	bUseColor		= other.bUseColor;
	crFontColor		= other.crFontColor;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

WindowSettings::WindowSettings()
: strTitle(L"ConsoleZ")
, strIcon(L"")
, strMainTitleFormat(L"?U([%u] )?N({%u} )%m?s( - %s)")
, strTabTitleFormat(L"%n. ?U([%u] )?N({%u} )%t?s( - %s)")
, bUseTabIcon(false)
, bUseTabTitles(false)
, dwTrimTabTitles(0)
, dwTrimTabTitlesRight(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"title"), strTitle, wstring(L"ConsoleZ"));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon, wstring(L""));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"main_title_format"), strMainTitleFormat, wstring(L""));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"tab_title_format"), strTabTitleFormat, wstring(L""));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles"), dwTrimTabTitles, 0);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles_right"), dwTrimTabTitlesRight, 0);

	bool bUseConsoleTitle, bShowCommand, bShowCommandInTabs;
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_console_title"), bUseConsoleTitle, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd"), bShowCommand, true);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd_tabs"), bShowCommandInTabs, true);

	if( strMainTitleFormat.empty() )
	{
		strMainTitleFormat = L"?U([%u] )?N({%u} )%m";
		if( bShowCommand )
			strMainTitleFormat += L"?s( - %s)";
	}

	if( strTabTitleFormat.empty() )
	{
		if( bUseConsoleTitle )
		{
			strTabTitleFormat = L"%n. ?s(%s):(%t)";
		}
		else
		{
			strTabTitleFormat = L"%n. ?U([%u] )?N({%u} )%t";
			if( bShowCommandInTabs )
				strTabTitleFormat += L"?s( - %s)";
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"title"), strTitle);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"main_title_format"), strMainTitleFormat);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"tab_title_format"), strTabTitleFormat);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles"), dwTrimTabTitles);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles_right"), dwTrimTabTitlesRight);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

WindowSettings& WindowSettings::operator=(const WindowSettings& other)
{
	strTitle			= other.strTitle;
	strIcon				= other.strIcon;
	strMainTitleFormat = other.strMainTitleFormat;
	strTabTitleFormat  = other.strTabTitleFormat;
	bUseTabIcon			= other.bUseTabIcon;
	bUseTabTitles		= other.bUseTabTitles;
	dwTrimTabTitles		= other.dwTrimTabTitles;
	dwTrimTabTitlesRight= other.dwTrimTabTitlesRight;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FullScreenSettings::FullScreenSettings()
: bStartInFullScreen(false)
, dwFullScreenMonitor(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FullScreenSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pAppearanceElement;
	CComPtr<IXMLDOMElement>	pFullScreenElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance"), pAppearanceElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pAppearanceElement, CComBSTR(L"fullscreen"), pFullScreenElement))) return false;

	XmlHelper::GetAttribute(pFullScreenElement, CComBSTR(L"start_in_fullscreen"), bStartInFullScreen,  false);
	XmlHelper::GetAttribute(pFullScreenElement, CComBSTR(L"fullscreen_monitor"),  dwFullScreenMonitor, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FullScreenSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/fullscreen"), pWindowElement))) return false;

	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"start_in_fullscreen"), bStartInFullScreen);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"fullscreen_monitor"),  dwFullScreenMonitor);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FullScreenSettings& FullScreenSettings::operator=(const FullScreenSettings& other)
{
	bStartInFullScreen  = other.bStartInFullScreen;
	dwFullScreenMonitor = other.dwFullScreenMonitor;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ControlsSettings::ControlsSettings()
: bShowMenu(true)
, bShowToolbar(true)
, bShowSearchbar(true)
, bShowStatusbar(true)
, bShowTabs(true)
, bHideSingleTab(false)
, bTabsOnBottom(false)
, bHideTabIcons(false)
, bShowScrollbars(true)
, bFlatScrollbars(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_searchbar"), bShowSearchbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"hide_single_tab"), bHideSingleTab, false);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"tabs_on_bottom"), bTabsOnBottom, false);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"hide_tab_icons"), bHideTabIcons, false);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_scrollbars"), bShowScrollbars, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"flat_scrollbars"), bFlatScrollbars, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_searchbar"), bShowSearchbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"hide_single_tab"), bHideSingleTab);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"tabs_on_bottom"), bTabsOnBottom);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"hide_tab_icons"), bHideTabIcons);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_scrollbars"), bShowScrollbars);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"flat_scrollbars"), bFlatScrollbars);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ControlsSettings& ControlsSettings::operator=(const ControlsSettings& other)
{
	bShowMenu		= other.bShowMenu;
	bShowToolbar	= other.bShowToolbar;
	bShowSearchbar	= other.bShowSearchbar;
	bShowStatusbar	= other.bShowStatusbar;
	bShowTabs		= other.bShowTabs;
	bHideSingleTab	= other.bHideSingleTab;
	bTabsOnBottom	= other.bTabsOnBottom;
	bHideTabIcons	= other.bHideTabIcons;
	bShowScrollbars	= other.bShowScrollbars;
	bFlatScrollbars	= other.bFlatScrollbars;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

StylesSettings::StylesSettings()
: bCaption(true)
, bResizable(true)
, bTaskbarButton(true)
, bBorder(true)
, bTrayIcon(false)
, bQuake(false)
, bJumplist(false)
, bIntegratedIME(false)
, bHideWhenInactive(false)
, dwInsideBorder(2)
, dwQuakeAnimationTime(300)
, crSelectionColor(RGB(255, 255, 255))
, crHighlightColor(RGB(191, 191, 191))
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool StylesSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"caption"), bCaption, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"resizable"), bResizable, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"taskbar_button"), bTaskbarButton, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"border"), bBorder, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBorder, 2);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon, false);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"quake_like"), bQuake, false);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"quake_animation_time"), dwQuakeAnimationTime, 300);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"jumplist"), bJumplist, false);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"integrated_ime"), bIntegratedIME, false);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"hide_when_inactive"), bHideWhenInactive, false);

	CComPtr<IXMLDOMElement>	pSelColorElement;
	if (SUCCEEDED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"selection_color"), pSelColorElement)))
		XmlHelper::GetRGBAttribute(pSelColorElement, crSelectionColor, RGB(255, 255, 255));

	CComPtr<IXMLDOMElement>	pHiColorElement;
	if (SUCCEEDED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"highlight_color"), pHiColorElement)))
		XmlHelper::GetRGBAttribute(pHiColorElement, crHighlightColor, RGB(191, 191, 191));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool StylesSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"caption"), bCaption);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"resizable"), bResizable);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"taskbar_button"), bTaskbarButton);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"border"), bBorder);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBorder);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"quake_like"), bQuake);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"quake_animation_time"), dwQuakeAnimationTime);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"jumplist"), bJumplist);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"integrated_ime"), bIntegratedIME);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"hide_when_inactive"), bHideWhenInactive);

	CComPtr<IXMLDOMElement>	pSelColorElement;
	if (SUCCEEDED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"selection_color"), pSelColorElement)))
		XmlHelper::SetRGBAttribute(pSelColorElement, crSelectionColor);

	CComPtr<IXMLDOMElement>	pHiColorElement;
	if (SUCCEEDED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"highlight_color"), pHiColorElement)))
		XmlHelper::SetRGBAttribute(pHiColorElement, crHighlightColor);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

StylesSettings& StylesSettings::operator=(const StylesSettings& other)
{
	bCaption		= other.bCaption;
	bResizable		= other.bResizable;
	bTaskbarButton	= other.bTaskbarButton;
	bBorder			= other.bBorder;
	bTrayIcon		= other.bTrayIcon;
	bQuake		= other.bQuake;
	bJumplist		= other.bJumplist;
	bIntegratedIME		= other.bIntegratedIME;
	bHideWhenInactive = other.bHideWhenInactive;
	dwInsideBorder	= other.dwInsideBorder;
	dwQuakeAnimationTime	= other.dwQuakeAnimationTime;
	crSelectionColor= other.crSelectionColor;
	crHighlightColor= other.crHighlightColor;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PositionSettings::PositionSettings()
: nX(-1)
, nY(-1)
, bSavePosition(false)
, nW(-1)
, nH(-1)
, bSaveSize(false)
, zOrder(zorderNormal)
, dockPosition(dockNone)
, nSnapDistance(-1)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"x"), nX, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"y"), nY, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"save_position"), bSavePosition, false);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"w"), nW, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"h"), nH, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"save_size"), bSaveSize, false);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"z_order"), reinterpret_cast<int&>(zOrder), static_cast<int>(zorderNormal));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"dock"), reinterpret_cast<int&>(dockPosition), static_cast<int>(dockNone));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance, -1);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"x"), nX);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"y"), nY);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"save_position"), bSavePosition);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"w"), nW);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"h"), nH);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"save_size"), bSaveSize);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"z_order"), static_cast<int>(zOrder));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"dock"), static_cast<int>(dockPosition));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PositionSettings& PositionSettings::operator=(const PositionSettings& other)
{
	nX            = other.nX;
	nY            = other.nY;
	bSavePosition = other.bSavePosition;
	nW            = other.nW;
	nH            = other.nH;
	bSaveSize     = other.bSaveSize;
	zOrder        = other.zOrder;
	dockPosition  = other.dockPosition;
	nSnapDistance = other.nSnapDistance;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


BYTE TransparencySettings::minAlpha = 5;

//////////////////////////////////////////////////////////////////////////////

TransparencySettings::TransparencySettings()
: transType(transNone)
, byActiveAlpha(255)
, byInactiveAlpha(255)
, crColorKey(RGB(0, 0, 0))
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TransparencySettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType), static_cast<DWORD>(transNone));
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha, 255);
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha, 255);
	XmlHelper::GetRGBAttribute(pTransElement, crColorKey, RGB(0, 0, 0));

	if (byActiveAlpha < minAlpha) byActiveAlpha = minAlpha;
	if (byInactiveAlpha < minAlpha) byInactiveAlpha = minAlpha;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TransparencySettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType));
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha);
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha);
	XmlHelper::SetRGBAttribute(pTransElement, crColorKey);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TransparencySettings& TransparencySettings::operator=(const TransparencySettings& other)
{
	transType		= other.transType;
	byActiveAlpha	= other.byActiveAlpha;
	byInactiveAlpha	= other.byInactiveAlpha;
	crColorKey		= other.crColorKey;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AppearanceSettings::AppearanceSettings()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	fontSettings.Load(pSettingsRoot);
	windowSettings.Load(pSettingsRoot);
	controlsSettings.Load(pSettingsRoot);
	stylesSettings.Load(pSettingsRoot);
	positionSettings.Load(pSettingsRoot);
	transparencySettings.Load(pSettingsRoot);
	fullScreenSettings.Load(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	fontSettings.Save(pSettingsRoot);
	windowSettings.Save(pSettingsRoot);
	controlsSettings.Save(pSettingsRoot);
	stylesSettings.Save(pSettingsRoot);
	positionSettings.Save(pSettingsRoot);
	transparencySettings.Save(pSettingsRoot);
	fullScreenSettings.Save(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AppearanceSettings& AppearanceSettings::operator=(const AppearanceSettings& other)
{
	fontSettings		= other.fontSettings;
	windowSettings		= other.windowSettings;
	controlsSettings	= other.controlsSettings;
	stylesSettings		= other.stylesSettings;
	positionSettings	= other.positionSettings;
	transparencySettings= other.transparencySettings;
	fullScreenSettings	= other.fullScreenSettings;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CopyPasteSettings::CopyPasteSettings()
: bCopyOnSelect(false)
, bClearOnCopy(true)
, bSensitiveCopy(true)
, bNoWrap(false)
, bTrimSpaces(false)
, bRTF(true)
, copyNewlineChar(newlineCRLF)
, bIncludeLeftDelimiter(false)
, bIncludeRightDelimiter(false)
, strLeftDelimiters(L" ([")
, strRightDelimiters(L" )]")
, dwEOLSpaces(1)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	int nNewlineChar;

	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"clear_on_copy"), bClearOnCopy, true);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"sensitive_copy"), bSensitiveCopy, true);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"eol_spaces"), dwEOLSpaces, 1);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"rtf"), bRTF, true);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_newline_char"), nNewlineChar, 0);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"include_left_delimiter"), bIncludeLeftDelimiter, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"include_right_delimiter"), bIncludeRightDelimiter, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"left_delimiters"), strLeftDelimiters, L" ([");
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"right_delimiters"), strRightDelimiters, L" )]");

	copyNewlineChar = static_cast<CopyNewlineChar>(nNewlineChar);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"clear_on_copy"), bClearOnCopy);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"sensitive_copy"), bSensitiveCopy);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"eol_spaces"), dwEOLSpaces);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"rtf"), bRTF);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_newline_char"), static_cast<int>(copyNewlineChar));

	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"include_left_delimiter"), bIncludeLeftDelimiter);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"include_right_delimiter"), bIncludeRightDelimiter);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"left_delimiters"), strLeftDelimiters);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"right_delimiters"), strRightDelimiters);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CopyPasteSettings& CopyPasteSettings::operator=(const CopyPasteSettings& other)
{
	bCopyOnSelect	= other.bCopyOnSelect;
	bClearOnCopy	= other.bClearOnCopy;
	bSensitiveCopy	= other.bSensitiveCopy;
	bNoWrap			= other.bNoWrap;
	bTrimSpaces		= other.bTrimSpaces;
	bRTF		= other.bRTF;
	copyNewlineChar	= other.copyNewlineChar;
	bIncludeLeftDelimiter	= other.bIncludeLeftDelimiter;
	bIncludeRightDelimiter	= other.bIncludeRightDelimiter;
	strLeftDelimiters	= other.strLeftDelimiters;
	strRightDelimiters	= other.strRightDelimiters;
	dwEOLSpaces = other.dwEOLSpaces;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ScrollSettings::ScrollSettings()
: dwPageScrollRows(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ScrollSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pScrollElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/scroll"), pScrollElement))) return false;

	XmlHelper::GetAttribute(pScrollElement, CComBSTR(L"page_scroll_rows"), dwPageScrollRows, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ScrollSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pScrollElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/scroll"), pScrollElement))) return false;

	XmlHelper::SetAttribute(pScrollElement, CComBSTR(L"page_scroll_rows"), dwPageScrollRows);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ScrollSettings& ScrollSettings::operator=(const ScrollSettings& other)
{
	dwPageScrollRows= other.dwPageScrollRows;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabHighlightSettings::TabHighlightSettings()
: dwFlashes(0)
, bStayHighlighted(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabHighlightSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTabElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/tab_highlight"), pTabElement))) return false;

	XmlHelper::GetAttribute(pTabElement, CComBSTR(L"flashes"), dwFlashes, 0);
	XmlHelper::GetAttribute(pTabElement, CComBSTR(L"stay_highligted"), bStayHighlighted, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabHighlightSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTabElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/tab_highlight"), pTabElement))) return false;

	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"flashes"), dwFlashes);
	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"stay_highligted"), bStayHighlighted);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabHighlightSettings& TabHighlightSettings::operator=(const TabHighlightSettings& other)
{
	dwFlashes		= other.dwFlashes;
	bStayHighlighted= other.bStayHighlighted;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CloseSettings::CloseSettings()
  : bAllowClosingLastView(false)
  , bConfirmClosingMultipleViews(true)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CloseSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pCloseElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"close"), pCloseElement))) return false;

	XmlHelper::GetAttribute(pCloseElement, CComBSTR(L"allow_closing_last_view"),        bAllowClosingLastView,        false);
	XmlHelper::GetAttribute(pCloseElement, CComBSTR(L"confirm_closing_multiple_views"), bConfirmClosingMultipleViews, true);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CloseSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCloseElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/close"), pCloseElement))) return false;

	XmlHelper::SetAttribute(pCloseElement, CComBSTR(L"allow_closing_last_view"),        bAllowClosingLastView       );
	XmlHelper::SetAttribute(pCloseElement, CComBSTR(L"confirm_closing_multiple_views"), bConfirmClosingMultipleViews);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CloseSettings& CloseSettings::operator=(const CloseSettings& other)
{
	bAllowClosingLastView        = other.bAllowClosingLastView;
	bConfirmClosingMultipleViews = other.bConfirmClosingMultipleViews;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FocusSettings::FocusSettings()
	: bFollowMouse(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FocusSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pFocusElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"focus"), pFocusElement))) return false;

	XmlHelper::GetAttribute(pFocusElement, CComBSTR(L"follow_mouse"), bFollowMouse, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FocusSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pFocusElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/focus"), pFocusElement))) return false;

	XmlHelper::SetAttribute(pFocusElement, CComBSTR(L"follow_mouse"), bFollowMouse);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FocusSettings& FocusSettings::operator=(const FocusSettings& other)
{
	bFollowMouse = other.bFollowMouse;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

InstanceSettings::InstanceSettings()
	: bAllowMultipleInstances(true)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool InstanceSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pInstanceElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"instance"), pInstanceElement))) return false;

	XmlHelper::GetAttribute(pInstanceElement, CComBSTR(L"allow_multi"), bAllowMultipleInstances, true);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool InstanceSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pInstanceElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/instance"), pInstanceElement))) return false;

	XmlHelper::SetAttribute(pInstanceElement, CComBSTR(L"allow_multi"), bAllowMultipleInstances);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

InstanceSettings& InstanceSettings::operator=(const InstanceSettings& other)
{
	bAllowMultipleInstances = other.bAllowMultipleInstances;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CloneSettings::CloneSettings()
	: bUseCurrentDirectory(true)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CloneSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pCloneElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"clone"), pCloneElement))) return false;

	XmlHelper::GetAttribute(pCloneElement, CComBSTR(L"use_current_dir"), bUseCurrentDirectory, true);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CloneSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCloneElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/clone"), pCloneElement))) return false;

	XmlHelper::SetAttribute(pCloneElement, CComBSTR(L"use_current_dir"), bUseCurrentDirectory);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CloneSettings& CloneSettings::operator=(const CloneSettings& other)
{
	bUseCurrentDirectory = other.bUseCurrentDirectory;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SearchSettings::SearchSettings()
	: bMatchCase(false)
	, bMatchWholeWord(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SearchSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pSearchElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"search"), pSearchElement))) return false;

	XmlHelper::GetAttribute(pSearchElement, CComBSTR(L"match_case"),       bMatchCase,      false);
	XmlHelper::GetAttribute(pSearchElement, CComBSTR(L"match_whole_word"), bMatchWholeWord, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SearchSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pSearchElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/search"), pSearchElement))) return false;

	XmlHelper::SetAttribute(pSearchElement, CComBSTR(L"match_case"),       bMatchCase);
	XmlHelper::SetAttribute(pSearchElement, CComBSTR(L"match_whole_word"), bMatchWholeWord);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SearchSettings& SearchSettings::operator=(const SearchSettings& other)
{
	bMatchCase      = other.bMatchCase;
	bMatchWholeWord = other.bMatchWholeWord;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

RunAsUserSettings::RunAsUserSettings()
	: bUseCredentialProviders(true)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool RunAsUserSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pBehaviorElement;
	CComPtr<IXMLDOMElement>	pRunAsUserElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior"), pBehaviorElement))) return false;
	if (FAILED(XmlHelper::AddDomElementIfNotExist(pBehaviorElement, CComBSTR(L"runas"), pRunAsUserElement))) return false;

	XmlHelper::GetAttribute(pRunAsUserElement, CComBSTR(L"use_credprov"), bUseCredentialProviders, true);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool RunAsUserSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pRunAsUserElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/runas"), pRunAsUserElement))) return false;

	XmlHelper::SetAttribute(pRunAsUserElement, CComBSTR(L"use_credprov"), bUseCredentialProviders);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

RunAsUserSettings& RunAsUserSettings::operator=(const RunAsUserSettings& other)
{
	bUseCredentialProviders = other.bUseCredentialProviders;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings::BehaviorSettings()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	copyPasteSettings.Load(pSettingsRoot);
	scrollSettings.Load(pSettingsRoot);
	tabHighlightSettings.Load(pSettingsRoot);
	closeSettings.Load(pSettingsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	copyPasteSettings.Save(pSettingsRoot);
	scrollSettings.Save(pSettingsRoot);
	tabHighlightSettings.Save(pSettingsRoot);
	closeSettings.Save(pSettingsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings& BehaviorSettings::operator=(const BehaviorSettings& other)
{
	copyPasteSettings    = other.copyPasteSettings;
	scrollSettings       = other.scrollSettings;
	tabHighlightSettings = other.tabHighlightSettings;
	closeSettings        = other.closeSettings;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings2::BehaviorSettings2()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings2::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	focusSettings.Load(pSettingsRoot);
	instanceSettings.Load(pSettingsRoot);
	cloneSettings.Load(pSettingsRoot);
	searchSettings.Load(pSettingsRoot);
	runAsUserSettings.Load(pSettingsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings2::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	focusSettings.Save(pSettingsRoot);
	instanceSettings.Save(pSettingsRoot);
	cloneSettings.Save(pSettingsRoot);
	searchSettings.Save(pSettingsRoot);
	runAsUserSettings.Save(pSettingsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings2& BehaviorSettings2::operator=(const BehaviorSettings2& other)
{
	focusSettings        = other.focusSettings;
	instanceSettings     = other.instanceSettings;
	cloneSettings        = other.cloneSettings;
	searchSettings       = other.searchSettings;
	runAsUserSettings    = other.runAsUserSettings;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HotKeys::HotKeys()
: bUseScrollLock(false)
{
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"settings",           ID_EDIT_SETTINGS,        IDS_EDIT_SETTINGS       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"help",               ID_HELP,                 IDS_HELP                )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"exit",               ID_APP_EXIT,             IDS_APP_EXIT            )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"savesession",        ID_SAVE_SESSION,         IDS_SAVE_SESSION        )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"loadsession",        ID_LOAD_SESSION,         IDS_LOAD_SESSION        )));

	for(WORD i = 0; i < 12; ++i)
	{
		commands.push_back(std::shared_ptr<CommandData>(new CommandData(std::wstring(L"newtab") + std::to_wstring(i + 1), ID_NEW_TAB_1 + i, IDS_NEW_TAB_1 + i)));
	}

	for(WORD i = 0; i < 12; ++i)
	{
		commands.push_back(std::shared_ptr<CommandData>(new CommandData(std::wstring(L"switchtab") + std::to_wstring(i + 1), ID_SWITCH_TAB_1 + i, IDS_SWITCH_TAB_1 + i)));
	}

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"nexttab",            ID_NEXT_TAB,             IDS_NEXT_TAB            )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"prevtab",            ID_PREV_TAB,             IDS_PREV_TAB            )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"closetab",           ID_FILE_CLOSE_TAB,       IDS_FILE_CLOSE_TAB      )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"renametab",          ID_EDIT_RENAME_TAB,      IDS_EDIT_RENAME_TAB     )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"nextview",           ID_NEXT_VIEW,            IDS_NEXT_VIEW           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"prevview",           ID_PREV_VIEW,            IDS_PREV_VIEW           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"leftview",           ID_LEFT_VIEW,            IDS_LEFT_VIEW           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"rightview",          ID_RIGHT_VIEW,           IDS_RIGHT_VIEW          )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"topview",            ID_TOP_VIEW,             IDS_TOP_VIEW            )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"bottomview",         ID_BOTTOM_VIEW,          IDS_BOTTOM_VIEW         )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"dechsize",           ID_DEC_HORIZ_SIZE,       IDS_DEC_HORIZ_SIZE      )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"inchsize",           ID_INC_HORIZ_SIZE,       IDS_INC_HORIZ_SIZE      )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"decvsize",           ID_DEC_VERT_SIZE,        IDS_DEC_VERT_SIZE       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"incvsize",           ID_INC_VERT_SIZE,        IDS_INC_VERT_SIZE       )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"closeview",          ID_CLOSE_VIEW,           IDS_CLOSE_VIEW          )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"detachview",         ID_DETACH_VIEW,          IDS_DETACH_VIEW         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"attach",             ID_ATTACH_CONSOLES,      IDS_ATTACH_CONSOLES     )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"splithoriz",         ID_SPLIT_HORIZ,          IDS_SPLIT_HORIZ         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"splitvert",          ID_SPLIT_VERT,           IDS_SPLIT_VERT          )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"clone",              ID_CLONE_IN_NEW_TAB,     IDS_CLONE_IN_NEW_TAB    )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"fullscreen",         ID_VIEW_FULLSCREEN,      IDS_VIEW_FULLSCREEN     )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"zoom100",            ID_VIEW_ZOOM_100,        IDS_VIEW_ZOOM_100       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"zoominc",            ID_VIEW_ZOOM_INC,        IDS_VIEW_ZOOM_INC       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"zoomdec",            ID_VIEW_ZOOM_DEC,        IDS_VIEW_ZOOM_DEC       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"groupall",           ID_GROUP_ALL,            IDS_GROUP_ALL           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"ungroupall",         ID_UNGROUP_ALL,          IDS_UNGROUP_ALL         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"grouptab",           ID_GROUP_TAB,            IDS_GROUP_TAB           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"ungrouptab",         ID_UNGROUP_TAB,          IDS_UNGROUP_TAB         )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"clear",              ID_EDIT_CLEAR,           IDS_EDIT_CLEAR          )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"copy",               ID_EDIT_COPY,            IDS_EDIT_COPY           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"selectall",          ID_EDIT_SELECT_ALL,      IDS_EDIT_SELECT_ALL     )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"clear_selection",    ID_EDIT_CLEAR_SELECTION, IDS_EDIT_CLEAR_SELECTION)));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"paste",              ID_EDIT_PASTE,           IDS_EDIT_PASTE          )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"paste_selection",    ID_PASTE_SELECTION,      IDS_PASTE_SELECTION     )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"stopscroll",         ID_EDIT_STOP_SCROLLING,  IDS_EDIT_STOP_SCROLLING )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollrowup",        ID_SCROLL_UP,            IDS_SCROLL_UP           )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollrowdown",      ID_SCROLL_DOWN,          IDS_SCROLL_DOWN         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollpageup",       ID_SCROLL_PAGE_UP,       IDS_SCROLL_PAGE_UP      )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollpagedown",     ID_SCROLL_PAGE_DOWN,     IDS_SCROLL_PAGE_DOWN    )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollcolleft",      ID_SCROLL_LEFT,          IDS_SCROLL_LEFT         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollcolright",     ID_SCROLL_RIGHT,         IDS_SCROLL_RIGHT        )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollpageleft",     ID_SCROLL_PAGE_LEFT,     IDS_SCROLL_PAGE_LEFT    )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"scrollpageright",    ID_SCROLL_PAGE_RIGHT,    IDS_SCROLL_PAGE_RIGHT   )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"find",               ID_FIND,                 IDS_FIND                )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"findnext",           ID_SEARCH_NEXT,          IDS_SEARCH_NEXT         )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"findprev",           ID_SEARCH_PREV,          IDS_SEARCH_PREV         )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"switchtransparency", ID_SWITCH_TRANSPARENCY,  IDS_SWITCH_TRANSPARENCY )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"dumpbuffer",         IDC_DUMP_BUFFER,         IDS_DUMP_BUFFER         )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"cmdMenu1",           ID_SHOW_CONTEXT_MENU_1,  IDS_SHOW_CONTEXT_MENU_1 )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"cmdMenu2",           ID_SHOW_CONTEXT_MENU_2,  IDS_SHOW_CONTEXT_MENU_2 )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"cmdMenu3",           ID_SHOW_CONTEXT_MENU_3,  IDS_SHOW_CONTEXT_MENU_3 )));

	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"ctrlC",              ID_SEND_CTRL_C,          IDS_SEND_CTRL_C         )));

	for(WORD i = 0; i < EXTERNAL_COMMANDS_COUNT; ++i)
	{
		commands.push_back(std::shared_ptr<CommandData>(new CommandData(std::wstring(L"externalcmd") + std::to_wstring(i + 1), ID_EXTERNAL_COMMAND_1 + i, IDS_EXTERNAL_COMMAND_1 + i)));
	}

	// global commands
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(L"activate",           IDC_GLOBAL_ACTIVATE,     IDS_GLOBAL_ACTIVATE, true)));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;

	CComPtr<IXMLDOMElement>		pHotkeysElement;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"hotkeys"), pHotkeysElement))) return false;

	XmlHelper::GetAttribute(pHotkeysElement, CComBSTR(L"use_scroll_lock"), bUseScrollLock, false);

	hr = pHotkeysElement->selectNodes(CComBSTR(L"hotkey"), &pHotKeyNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	hr = pHotKeyNodes->get_length(&lListLength);
	if (FAILED(hr)) return false;

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pHotKeyNode;
		CComPtr<IXMLDOMElement>	pHotKeyElement;

		pHotKeyNodes->get_item(i, &pHotKeyNode);
		if (FAILED(pHotKeyNode.QueryInterface(&pHotKeyElement))) continue;

		wstring	strCommand(L"");
		bool	bShift;
		bool	bCtrl;
		bool	bAlt;
		bool	bExtended;
		DWORD	dwKeyCode;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"command"), strCommand, wstring(L""));

		CommandNameIndex::iterator it = commands.get<command>().find(strCommand);
		if (it == commands.get<command>().end()) continue;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"extended"), bExtended, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		(*it)->accelHotkey.fVirt = FVIRTKEY;
		(*it)->accelHotkey.key   = static_cast<WORD>(dwKeyCode);
		(*it)->accelHotkey.cmd   = (*it)->wCommandID;
		(*it)->bExtended         = bExtended;

		if (bShift) (*it)->accelHotkey.fVirt |= FSHIFT;
		if (bCtrl)  (*it)->accelHotkey.fVirt |= FCONTROL;
		if (bAlt)   (*it)->accelHotkey.fVirt |= FALT;

		if( (*it)->bGlobal )
			XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"win"), (*it)->bWin, false);
	}

	CComPtr<IXMLDOMElement>  pExternalCommandsElement;
	CComPtr<IXMLDOMNodeList> pExternalCommandsNodes;

	if (FAILED(XmlHelper::AddDomElementIfNotExist(pSettingsRoot, CComBSTR(L"external_commands"), pExternalCommandsElement))) return false;

	if (FAILED(pExternalCommandsElement->selectNodes(CComBSTR(L"external_command"), &pExternalCommandsNodes))) return false;

	if (FAILED(pExternalCommandsNodes->get_length(&lListLength))) return false;

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>    pExternalCommandNode;
		CComPtr<IXMLDOMElement> pExternalCommandElement;

		pExternalCommandsNodes->get_item(i, &pExternalCommandNode);
		if (FAILED(pExternalCommandNode.QueryInterface(&pExternalCommandElement))) continue;

		int nId = 0;
		XmlHelper::GetAttribute(pExternalCommandElement, CComBSTR(L"id"), nId, 0);
		if( nId < 1 || nId > EXTERNAL_COMMANDS_COUNT ) continue;

		XmlHelper::GetAttribute(pExternalCommandElement, CComBSTR(L"value"), externalCommands[nId - 1], L"");
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>		pHotkeysElement;
	CComPtr<IXMLDOMNodeList>	pHotKeyChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"hotkeys"), pHotkeysElement))) return false;

	XmlHelper::SetAttribute(pHotkeysElement, CComBSTR(L"use_scroll_lock"), bUseScrollLock);

	if (FAILED(pHotkeysElement->get_childNodes(&pHotKeyChildNodes))) return false;

	long	lListLength;
	pHotKeyChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pHotKeyChildNode;
		CComPtr<IXMLDOMNode>	pRemovedHotKeyNode;
		if (FAILED(pHotKeyChildNodes->get_item(i, &pHotKeyChildNode))) continue;

		pHotkeysElement->removeChild(pHotKeyChildNode, &pRemovedHotKeyNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;
	pHotkeysElement->get_ownerDocument(&pSettingsDoc);

	for (auto itCommand = commands.begin(); itCommand != commands.end(); ++itCommand)
	{
		// this is just for pretty printing
		XmlHelper::AddTextNode(pHotkeysElement, CComBSTR(L"\n\t\t"));

		CComPtr<IXMLDOMElement>	pNewHotkeyElement;
		CComPtr<IXMLDOMNode>	pNewHotkeyOut;
		bool					bAttrVal;

		pSettingsDoc->createElement(CComBSTR(L"hotkey"), &pNewHotkeyElement);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FCONTROL) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"ctrl"), bAttrVal);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FSHIFT) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"shift"), bAttrVal);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FALT) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"alt"), bAttrVal);

		bAttrVal = ((*itCommand)->bExtended) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"extended"), bAttrVal);

		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"code"), (*itCommand)->accelHotkey.key);
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"command"), (*itCommand)->strCommand);

		if( (*itCommand)->bGlobal )
		{
			bAttrVal = ((*itCommand)->bWin) ? true : false;
			XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"win"), bAttrVal);
		}

		pHotkeysElement->appendChild(pNewHotkeyElement, &pNewHotkeyOut);
	}

	// this is just for pretty printing
	XmlHelper::AddTextNode(pHotkeysElement, CComBSTR(L"\n\t"));

	CComPtr<IXMLDOMElement>  pExternalCommandsElement;
	CComPtr<IXMLDOMNodeList> pExternalCommandsNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"external_commands"), pExternalCommandsElement))) return false;

	if (FAILED(pExternalCommandsElement->selectNodes(CComBSTR(L"external_command"), &pExternalCommandsNodes))) return false;

	if (FAILED(pExternalCommandsNodes->get_length(&lListLength))) return false;

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode> pExternalCommandChildNode;
		CComPtr<IXMLDOMNode> pRemovedExternalCommandNode;
		if (FAILED(pExternalCommandsNodes->get_item(i, &pExternalCommandChildNode))) continue;

		pExternalCommandsElement->removeChild(pExternalCommandChildNode, &pRemovedExternalCommandNode);
	}

	for (auto i = externalCommands.begin(); i != externalCommands.end(); ++i)
	{
		if( i->empty() ) continue;

		// this is just for pretty printing
		XmlHelper::AddTextNode(pExternalCommandsElement, CComBSTR(L"\n\t\t"));

		CComPtr<IXMLDOMElement> pNewExternalCommandElement;
		CComPtr<IXMLDOMNode>    pNewExternalCommandOut;

		pSettingsDoc->createElement(CComBSTR(L"external_command"), &pNewExternalCommandElement);

		XmlHelper::SetAttribute(pNewExternalCommandElement, CComBSTR(L"id"), static_cast<int>(i - externalCommands.begin() + 1));
		XmlHelper::SetAttribute(pNewExternalCommandElement, CComBSTR(L"value"), *i);

		pExternalCommandsElement->appendChild(pNewExternalCommandElement, &pNewExternalCommandOut);
	}

	// this is just for pretty printing
	XmlHelper::AddTextNode(pExternalCommandsElement, CComBSTR(L"\n"));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HotKeys& HotKeys::operator=(const HotKeys& other)
{
	bUseScrollLock = other.bUseScrollLock;

	commands.clear();
	commands.insert(commands.begin(), other.commands.begin(), other.commands.end());

	externalCommands = other.externalCommands;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseSettings::MouseSettings()
: commands()
{
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdCopy,         L"copy",          IDS_MOUSE_COPY        )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdSelect,       L"select",        IDS_MOUSE_SELECT      )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdColumnSelect, L"select_column", IDS_MOUSE_COLUMNSELECT)));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdPaste,        L"paste",         IDS_MOUSE_PASTE       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdDrag,         L"drag",          IDS_MOUSE_DRAG        )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdLink,         L"link",          IDS_MOUSE_LINK        )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdMenu1,        L"menu",          IDS_MOUSE_MENU1       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdMenu2,        L"menu2",         IDS_MOUSE_MENU2       )));
	commands.push_back(std::shared_ptr<CommandData>(new CommandData(cmdMenu3,        L"menu3",         IDS_MOUSE_MENU3       )));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;

	CComPtr<IXMLDOMElement>		pActionsElement;
	CComPtr<IXMLDOMNodeList>	pActionNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"mouse/actions"), pActionsElement))) return false;

	hr = pActionsElement->selectNodes(CComBSTR(L"action"), &pActionNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pActionNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pActionNode;
		CComPtr<IXMLDOMElement>	pActionElement;

		pActionNodes->get_item(i, &pActionNode);
		if (FAILED(pActionNode.QueryInterface(&pActionElement))) continue;

		wstring	strName;
		DWORD	dwButton;
		bool	bUseCtrl;
		bool	bUseShift;
		bool	bUseAlt;
		
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"name"), strName, L"");
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"button"), dwButton, 0);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"ctrl"), bUseCtrl, false);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"shift"), bUseShift, false);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"alt"), bUseAlt, false);

		typedef Commands::index<commandName>::type		CommandNameIndex;

		CommandNameIndex::iterator it = commands.get<commandName>().find(strName);
		if (it == commands.get<commandName>().end()) continue;

		(*it)->action.clickType = clickSingle;
		(*it)->action.button	= static_cast<Button>(dwButton);
		if (bUseCtrl)	(*it)->action.modifiers |= mkCtrl;
		if (bUseShift)	(*it)->action.modifiers |= mkShift;
		if (bUseAlt)	(*it)->action.modifiers |= mkAlt;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>		pMouseActionsElement;
	CComPtr<IXMLDOMNodeList>	pMouseActionsChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"mouse/actions"), pMouseActionsElement))) return false;

	if (FAILED(pMouseActionsElement->get_childNodes(&pMouseActionsChildNodes))) return false;

	long	lListLength;
	pMouseActionsChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pMouseActionsChildNode;
		CComPtr<IXMLDOMNode>	pRemovedMouseActionsNode;
		if (FAILED(pMouseActionsChildNodes->get_item(i, &pMouseActionsChildNode))) continue;

		pMouseActionsElement->removeChild(pMouseActionsChildNode, &pRemovedMouseActionsNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;

	CommandsSequence::iterator	itCommand;
	CommandsSequence::iterator	itLastCommand = commands.end();
	--itLastCommand;

	pMouseActionsElement->get_ownerDocument(&pSettingsDoc);

	for (itCommand = commands.begin(); itCommand != commands.end(); ++itCommand)
	{
		CComPtr<IXMLDOMElement>	pNewMouseActionsElement;
		CComPtr<IXMLDOMNode>	pNewMouseActionsOut;
		bool					bVal;

		pSettingsDoc->createElement(CComBSTR(L"action"), &pNewMouseActionsElement);

		bVal = ((*itCommand)->action.modifiers & mkCtrl) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"ctrl"), bVal);

		bVal = ((*itCommand)->action.modifiers & mkShift) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"shift"), bVal);

		bVal = ((*itCommand)->action.modifiers & mkAlt) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"alt"), bVal);

		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"button"), static_cast<int>((*itCommand)->action.button));
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"name"), (*itCommand)->strCommand);

		pMouseActionsElement->appendChild(pNewMouseActionsElement, &pNewMouseActionsOut);

		// this is just for pretty printing
		if (itCommand == itLastCommand)
		{
			XmlHelper::AddTextNode(pMouseActionsElement, CComBSTR(L"\n\t\t"));
		}
		else
		{
			XmlHelper::AddTextNode(pMouseActionsElement, CComBSTR(L"\n\t\t\t"));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseSettings& MouseSettings::operator=(const MouseSettings& other)
{
	commands.clear();
	commands.insert(commands.begin(), other.commands.begin(), other.commands.end());

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabSettings::TabSettings()
: strDefaultShell(L"")
, strDefaultInitialDir(L"")
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	HRESULT hr = pSettingsRoot->selectNodes(CComBSTR(L"tabs/tab"), &pTabNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pTabNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pTabNode;
		CComPtr<IXMLDOMElement>	pTabElement;

		pTabNodes->get_item(i, &pTabNode);
		if (FAILED(pTabNode.QueryInterface(&pTabElement))) continue;

		std::shared_ptr<TabData>	tabData(new TabData(strDefaultShell, strDefaultInitialDir));
		tabData->nIndex = i + 1;

		CComPtr<IXMLDOMElement>	pConsoleElement;
		CComPtr<IXMLDOMElement>	pCursorElement;
		CComPtr<IXMLDOMElement>	pBackgroundElement;

		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"title"), tabData->strTitle, L"ConsoleZ");
		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"icon"), tabData->strIcon, L"");
		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"use_default_icon"), tabData->bUseDefaultIcon, false);

		tabDataVector.push_back(tabData);

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"console"), pConsoleElement)))
		{
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), tabData->strShell, strDefaultShell);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), tabData->strInitialDir, strDefaultInitialDir);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"priority"), tabData->dwBasePriority, 2);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"run_as_user"), tabData->bRunAsUser, false);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"user"), tabData->strUser, L"");
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"net_only"), tabData->bNetOnly, false);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"run_as_admin"), tabData->bRunAsAdministrator, false);

			CComPtr<IXMLDOMNodeList> pEnvNodes;
			hr = pConsoleElement->selectNodes(CComBSTR(L"env"), &pEnvNodes);
			if (SUCCEEDED(hr))
			{
				long lListLength;
				pEnvNodes->get_length(&lListLength);

				for (long i = 0; i < lListLength; ++i)
				{
					CComPtr<IXMLDOMNode>    pEnvNode;
					CComPtr<IXMLDOMElement> pEnvElement;

					pEnvNodes->get_item(i, &pEnvNode);
					if (FAILED(pEnvNode.QueryInterface(&pEnvElement))) continue;

					std::shared_ptr<VarEnv> varenv (new VarEnv);

					XmlHelper::GetAttribute(pEnvElement, CComBSTR(L"var"),      varenv->strEnvVariable, L"");
					XmlHelper::GetAttribute(pEnvElement, CComBSTR(L"value"),    varenv->strEnvValue,    L"");
					XmlHelper::GetAttribute(pEnvElement, CComBSTR(L"check"),    varenv->bEnvChecked,    true);

					if( !varenv->strEnvVariable.empty() )
						tabData->environmentVariables.push_back(varenv);
				}
			}
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"cursor"), pCursorElement)))
		{
			XmlHelper::GetAttribute(pCursorElement, CComBSTR(L"style"), tabData->dwCursorStyle, 0);
			XmlHelper::GetRGBAttribute(pCursorElement, tabData->crCursorColor, RGB(255, 255, 255));
			tabData->bInheritedCursor = false;
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background"), pBackgroundElement)))
		{
			DWORD dwBackgroundImageType = 0;

			XmlHelper::GetAttribute(pBackgroundElement, CComBSTR(L"type"), dwBackgroundImageType, 0);
			tabData->backgroundImageType = static_cast<BackgroundImageType>(dwBackgroundImageType);

			if (tabData->backgroundImageType == bktypeNone)
			{
				XmlHelper::GetRGBAttribute(pBackgroundElement, tabData->crBackgroundColor, RGB(0, 0, 0));
			}
			else
			{
				tabData->crBackgroundColor = RGB(0, 0, 0);

				// load image settings and let ImageHandler return appropriate bitmap
				CComPtr<IXMLDOMElement>	pImageElement;
				CComPtr<IXMLDOMElement>	pTintElement;

				if (FAILED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image"), pImageElement))) return false;

				if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image/tint"), pTintElement)))
				{
					XmlHelper::GetRGBAttribute(pTintElement, tabData->imageData.crTint, RGB(0, 0, 0));
					XmlHelper::GetAttribute(pTintElement, CComBSTR(L"opacity"), tabData->imageData.byTintOpacity, 0);
				}

				if (tabData->backgroundImageType == bktypeImage)
				{
					DWORD dwImagePosition = 0;

					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"file"), tabData->imageData.strFilename, wstring(L""));
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"relative"), tabData->imageData.bRelative, false);
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"extend"), tabData->imageData.bExtend, false);
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"position"), dwImagePosition, 0);

					tabData->imageData.imagePosition = static_cast<ImagePosition>(dwImagePosition);
				}
			}

			tabData->bInheritedBackground = false;
		}

		CComPtr<IXMLDOMElement> pColors;
		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"colors"), pColors)))
		{
			tabData->bInheritedColors = !XmlHelper::LoadColors(pTabElement, tabData->consoleColors, tabData->backgroundTextOpacity);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>		pTabsElement;
	CComPtr<IXMLDOMNodeList>	pTabChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"tabs"), pTabsElement))) return false;

	if (FAILED(pTabsElement->get_childNodes(&pTabChildNodes))) return false;

	long	lListLength;
	pTabChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pTabChildNode;
		CComPtr<IXMLDOMNode>	pRemovedTabNode;
		if (FAILED(pTabChildNodes->get_item(i, &pTabChildNode))) continue;

		pTabsElement->removeChild(pTabChildNode, &pRemovedTabNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;
	TabDataVector::iterator		itTab;
	TabDataVector::iterator		itLastTab = tabDataVector.end() - 1;

	pTabsElement->get_ownerDocument(&pSettingsDoc);

	long nIndex = 0;
	for (itTab = tabDataVector.begin(); itTab != tabDataVector.end(); ++itTab)
	{
		CComPtr<IXMLDOMElement>	pNewTabElement;
		CComPtr<IXMLDOMNode>	pNewTabOut;

		pSettingsDoc->createElement(CComBSTR(L"tab"), &pNewTabElement);

		// reindex
		(*itTab)->nIndex = ++nIndex;

		// set tab attributes
		if ((*itTab)->strTitle.length() > 0)
		{
			XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"title"), (*itTab)->strTitle);
		}

		if ((*itTab)->strIcon.length() > 0)
		{
			XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"icon"), (*itTab)->strIcon);
		}

		XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"use_default_icon"), (*itTab)->bUseDefaultIcon);

		// add <console> tag
		CComPtr<IXMLDOMElement>	pNewConsoleElement;
		CComPtr<IXMLDOMNode>	pNewConsoleOut;

		pSettingsDoc->createElement(CComBSTR(L"console"), &pNewConsoleElement);

		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"shell"), (*itTab)->strShell);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"init_dir"), (*itTab)->strInitialDir);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"priority"), (*itTab)->dwBasePriority);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"run_as_user"), (*itTab)->bRunAsUser);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"user"), (*itTab)->strUser);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"net_only"), (*itTab)->bNetOnly);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"run_as_admin"), (*itTab)->bRunAsAdministrator);

		// add <env> tag
		if(! (*itTab)->environmentVariables.empty() )
		{
			for(size_t i = 0; i < (*itTab)->environmentVariables.size(); ++i)
			{
				CComPtr<IXMLDOMElement> pNewEnvElement;
				CComPtr<IXMLDOMNode>    pNewEnvOut;

				pSettingsDoc->createElement(CComBSTR(L"env"), &pNewEnvElement);

				XmlHelper::SetAttribute(pNewEnvElement, CComBSTR(L"var"),   (*itTab)->environmentVariables[i]->strEnvVariable);
				XmlHelper::SetAttribute(pNewEnvElement, CComBSTR(L"value"), (*itTab)->environmentVariables[i]->strEnvValue);
				XmlHelper::SetAttribute(pNewEnvElement, CComBSTR(L"check"), (*itTab)->environmentVariables[i]->bEnvChecked);

				XmlHelper::AddTextNode(pNewConsoleElement, CComBSTR(L"\n\t\t\t\t"));
				pNewConsoleElement->appendChild(pNewEnvElement, &pNewEnvOut);
			}
			XmlHelper::AddTextNode(pNewConsoleElement, CComBSTR(L"\n\t\t\t"));
		}

		XmlHelper::AddTextNode(pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewConsoleElement, &pNewConsoleOut);

		if (!(*itTab)->bInheritedCursor)
		{
			// add <cursor> tag
			CComPtr<IXMLDOMElement>	pNewCursorElement;
			CComPtr<IXMLDOMNode>	pNewCursorOut;

			pSettingsDoc->createElement(CComBSTR(L"cursor"), &pNewCursorElement);

			XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"style"), (*itTab)->dwCursorStyle);
			XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"r"), GetRValue((*itTab)->crCursorColor));
			XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"g"), GetGValue((*itTab)->crCursorColor));
			XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"b"), GetBValue((*itTab)->crCursorColor));


			XmlHelper::AddTextNode(pNewTabElement, CComBSTR(L"\n\t\t\t"));
			pNewTabElement->appendChild(pNewCursorElement, &pNewCursorOut);
		}

		if (!(*itTab)->bInheritedBackground)
		{
			// add <background> tag
			CComPtr<IXMLDOMElement>	pNewBkElement;
			CComPtr<IXMLDOMNode>	pNewBkOut;

			pSettingsDoc->createElement(CComBSTR(L"background"), &pNewBkElement);

			XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"type"), (*itTab)->backgroundImageType);
			XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"r"), GetRValue((*itTab)->crBackgroundColor));
			XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"g"), GetGValue((*itTab)->crBackgroundColor));
			XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"b"), GetBValue((*itTab)->crBackgroundColor));


			// add <image> tag
			CComPtr<IXMLDOMElement>	pNewImageElement;
			CComPtr<IXMLDOMNode>	pNewImageOut;

			pSettingsDoc->createElement(CComBSTR(L"image"), &pNewImageElement);

			if ((*itTab)->backgroundImageType == bktypeImage)
			{
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"file"), (*itTab)->imageData.strFilename);
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"relative"), (*itTab)->imageData.bRelative ? true : false);
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"extend"), (*itTab)->imageData.bExtend ? true : false);
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"position"), static_cast<DWORD>((*itTab)->imageData.imagePosition));
			}
			else
			{
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"file"), wstring(L""));
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"relative"), false);
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"extend"), false);
				XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"position"), 0);
			}

			// add <tint> tag
			CComPtr<IXMLDOMElement>	pNewTintElement;
			CComPtr<IXMLDOMNode>	pNewTintOut;

			pSettingsDoc->createElement(CComBSTR(L"tint"), &pNewTintElement);

			XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"opacity"), (*itTab)->imageData.byTintOpacity);
			XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"r"), GetRValue((*itTab)->imageData.crTint));
			XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"g"), GetGValue((*itTab)->imageData.crTint));
			XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"b"), GetBValue((*itTab)->imageData.crTint));


			XmlHelper::AddTextNode(pNewImageElement, CComBSTR(L"\n\t\t\t\t\t"));
			pNewImageElement->appendChild(pNewTintElement, &pNewTintOut);
			XmlHelper::AddTextNode(pNewImageElement, CComBSTR(L"\n\t\t\t\t"));
			XmlHelper::AddTextNode(pNewBkElement, CComBSTR(L"\n\t\t\t\t"));
			pNewBkElement->appendChild(pNewImageElement, &pNewImageOut);
			XmlHelper::AddTextNode(pNewBkElement, CComBSTR(L"\n\t\t\t"));
			XmlHelper::AddTextNode(pNewTabElement, CComBSTR(L"\n\t\t\t"));
			pNewTabElement->appendChild(pNewBkElement, &pNewBkOut);
		}

		if (!(*itTab)->bInheritedColors)
		{
			XmlHelper::AddTextNode(pNewTabElement, CComBSTR(L"\n\t\t\t"));
			XmlHelper::SaveColors(pNewTabElement, (*itTab)->consoleColors, (*itTab)->backgroundTextOpacity);
		}
		XmlHelper::AddTextNode(pNewTabElement, CComBSTR(L"\n\t\t"));

		pTabsElement->appendChild(pNewTabElement, &pNewTabOut);

		// this is just for pretty printing
		if (itTab == itLastTab)
		{
			XmlHelper::AddTextNode(pTabsElement, CComBSTR(L"\n\t"));
		}
		else
		{
			XmlHelper::AddTextNode(pTabsElement, CComBSTR(L"\n\t\t"));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void TabSettings::SetDefaults(const wstring& defaultShell, const wstring& defaultInitialDir)
{
	strDefaultShell		= defaultShell;
	strDefaultInitialDir= defaultInitialDir;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SettingsHandler::SettingsHandler()
: m_pSettingsDocument()
, m_strSettingsPath(L"")
, m_strSettingsFileName(L"")
, m_settingsDirType(dirTypeExe)
, m_consoleSettings()
, m_appearanceSettings()
, m_behaviorSettings()
, m_behaviorSettings2()
, m_hotKeys()
, m_mouseSettings()
, m_tabSettings()
{
}

SettingsHandler::~SettingsHandler()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SettingsHandler::LoadSettings(const wstring& strSettingsFileName)
{
	HRESULT hr = S_OK;

	size_t pos = strSettingsFileName.rfind(L'\\');

	if (pos == wstring::npos)
	{
		// no path, first try with user's APPDATA dir

		m_strSettingsFileName = strSettingsFileName;

		{
			SetUserDataDir(dirTypeUser);

			hr = XmlHelper::OpenXmlDocument(
								GetSettingsFileName(), 
								m_pSettingsDocument, 
								m_pSettingsRoot);
		}

		if (FAILED(hr))
		{
			SetUserDataDir(dirTypeExe);

			hr = XmlHelper::OpenXmlDocument(
								GetSettingsFileName(), 
								m_pSettingsDocument, 
								m_pSettingsRoot);
		}

		if (FAILED(hr))
		{
			m_strSettingsPath	= L"res://" + Helpers::GetModuleFileName(NULL) + L"/";

			hr = XmlHelper::OpenXmlDocumentFromResource(
				L"CONSOLE.XML",
				m_pSettingsDocument,
				m_pSettingsRoot);

			SetUserDataDir(dirTypeUser);
		}

		if (FAILED(hr)) return false;
	}
	else
	{
		// settings file name with a path
		m_strSettingsPath		= strSettingsFileName.substr(0, pos+1);
		m_strSettingsFileName	= strSettingsFileName.substr(pos+1);

		wchar_t wszAppData[32767];
		::ZeroMemory(wszAppData, sizeof(wszAppData));
		::GetEnvironmentVariable(L"APPDATA", wszAppData, _countof(wszAppData));

		if (equals(m_strSettingsPath, wstring(wszAppData) + wstring(L"\\Console\\"), is_iequal()))
		{
			m_settingsDirType = dirTypeUser;
		}
		else if (equals(m_strSettingsPath, Helpers::GetModulePath(NULL), is_iequal()))
		{
			m_settingsDirType = dirTypeExe;
		}
		else
		{
			m_settingsDirType = dirTypeCustom;
		}

		hr = XmlHelper::OpenXmlDocument(
							strSettingsFileName, 
							m_pSettingsDocument, 
							m_pSettingsRoot);

		if (FAILED(hr)) return false;
	}

	// load settings' sections
	m_consoleSettings.Load(m_pSettingsRoot);
	m_appearanceSettings.Load(m_pSettingsRoot);
	m_behaviorSettings.Load(m_pSettingsRoot);
	m_behaviorSettings2.Load(m_pSettingsRoot);
	m_hotKeys.Load(m_pSettingsRoot);
	m_mouseSettings.Load(m_pSettingsRoot);

	m_tabSettings.SetDefaults(m_consoleSettings.strShell, m_consoleSettings.strInitialDir);
	m_tabSettings.Load(m_pSettingsRoot);

	for(auto iterTabData = m_tabSettings.tabDataVector.begin(); iterTabData != m_tabSettings.tabDataVector.end(); ++iterTabData)
	{
		iterTabData->get()->SetColors(m_consoleSettings.consoleColors, m_consoleSettings.backgroundTextOpacity, false);
		iterTabData->get()->SetCursor(m_consoleSettings.dwCursorStyle, m_consoleSettings.crCursorColor, false);
		iterTabData->get()->SetBackground(m_consoleSettings.backgroundImageType, m_consoleSettings.crBackgroundColor, m_consoleSettings.imageData, false);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SettingsHandler::SaveSettings()
{
	m_consoleSettings.Save(m_pSettingsRoot);
	m_appearanceSettings.Save(m_pSettingsRoot);
	m_behaviorSettings.Save(m_pSettingsRoot);
	m_behaviorSettings2.Save(m_pSettingsRoot);
	m_hotKeys.Save(m_pSettingsRoot);
	m_mouseSettings.Save(m_pSettingsRoot);
	m_tabSettings.Save(m_pSettingsRoot);

	HRESULT hr = m_pSettingsDocument->save(CComVariant(GetSettingsFileName().c_str()));

	return SUCCEEDED(hr) ? true : false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::SetUserDataDir(SettingsDirType settingsDirType)
{
	if (settingsDirType == dirTypeExe)
	{
		m_strSettingsPath = Helpers::GetModulePath(NULL);
	}
	else if (settingsDirType == dirTypeUser)
	{
		wchar_t wszAppData[32767];
		::ZeroMemory(wszAppData, sizeof(wszAppData));
		::GetEnvironmentVariable(L"APPDATA", wszAppData, _countof(wszAppData));

		m_strSettingsPath = wstring(wszAppData) + wstring(L"\\Console\\");
		::CreateDirectory(m_strSettingsPath.c_str(), NULL);
	}

	m_settingsDirType = settingsDirType;
}

//////////////////////////////////////////////////////////////////////////////
