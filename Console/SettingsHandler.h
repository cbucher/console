#pragma once

#include "resource.h"

#include <msxml.h>

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct SettingsBase
{
	virtual bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot) = 0;
	virtual bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot) = 0;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum BackgroundImageType
{
	bktypeNone		= 0,
	bktypeImage		= 1,
	bktypeDesktop	= 2,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSettings : public SettingsBase
{
	ConsoleSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ConsoleSettings& operator=(const ConsoleSettings& other);

	wstring		strShell;
	wstring		strInitialDir;

	DWORD		dwRefreshInterval;
	DWORD		dwChangeRefreshInterval;
	DWORD		dwRows;
	DWORD		dwColumns;
	DWORD		dwBufferRows;
	DWORD		dwBufferColumns;

	bool		bStartHidden;
	bool		bSaveSize;

	COLORREF	defaultConsoleColors[16];
	COLORREF	consoleColors[16];
	BYTE		backgroundTextOpacity;

	DWORD		dwCursorStyle;
	COLORREF	crCursorColor;

	BackgroundImageType				backgroundImageType;
	COLORREF						crBackgroundColor;
	ImageData						imageData;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

enum FontSmoothing
{
	fontSmoothDefault          = 0,
	fontSmoothNone             = 1,
	fontSmoothCleartype        = 2,
	fontSmoothCleartypeNatural = 3,
	fontSmoothAntialiased      = 4,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FontSettings : public SettingsBase
{
	FontSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FontSettings& operator=(const FontSettings& other);

	wstring			strName;
	DWORD			dwSize;
	DWORD			dwExtraWidth;
	bool			bBold;
	bool			bItalic;
	FontSmoothing	fontSmoothing;
	bool			bBoldIntensified;
	bool			bItalicIntensified;
	bool			bLigature;
	bool			bUseColor;
	COLORREF		crFontColor;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct WindowSettings : public SettingsBase
{
	WindowSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	WindowSettings& operator=(const WindowSettings& other);

	wstring			strTitle;
	wstring			strIcon;
	wstring			strMainTitleFormat;
	wstring			strTabTitleFormat;
	bool			bUseTabIcon;
	bool			bUseTabTitles;
	DWORD			dwTrimTabTitles;
	DWORD			dwTrimTabTitlesRight;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FullScreenSettings : public SettingsBase
{
	FullScreenSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FullScreenSettings& operator=(const FullScreenSettings& other);

	bool      bStartInFullScreen;
	DWORD     dwFullScreenMonitor;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ControlsSettings2
{
	ControlsSettings2();

	bool Load(const CComPtr<IXMLDOMElement>& pCtrlsElement);
	bool Save(const CComPtr<IXMLDOMElement>& pCtrlsElement);

	ControlsSettings2& operator=(const ControlsSettings2& other);

	bool			bShowMenu;
	bool			bShowToolbar;
	bool			bShowSearchbar;
	bool			bShowStatusbar;
	bool			bShowTabs;
	bool			bHideSingleTab;
	bool			bTabsOnBottom;
	bool			bHideTabIcons;
	bool			bShowScrollbars;
};

struct ControlsSettings
{
	ControlsSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ControlsSettings& operator=(const ControlsSettings& other);

	bool         bIsFullScreen;
	inline bool& ShowMenu      (void) { return bIsFullScreen? controlsFullScreen.bShowMenu       : controlsWindowed.bShowMenu       ; }
	inline bool& ShowToolbar   (void) { return bIsFullScreen? controlsFullScreen.bShowToolbar    : controlsWindowed.bShowToolbar    ; }
	inline bool& ShowSearchbar (void) { return bIsFullScreen? controlsFullScreen.bShowSearchbar  : controlsWindowed.bShowSearchbar  ; }
	inline bool& ShowStatusbar (void) { return bIsFullScreen? controlsFullScreen.bShowStatusbar  : controlsWindowed.bShowStatusbar  ; }
	inline bool& ShowTabs      (void) { return bIsFullScreen? controlsFullScreen.bShowTabs       : controlsWindowed.bShowTabs       ; }
	inline bool& HideSingleTab (void) { return bIsFullScreen? controlsFullScreen.bHideSingleTab  : controlsWindowed.bHideSingleTab  ; }
	inline bool& TabsOnBottom  (void) { return bIsFullScreen? controlsFullScreen.bTabsOnBottom   : controlsWindowed.bTabsOnBottom   ; }
	inline bool& HideTabIcons  (void) { return bIsFullScreen? controlsFullScreen.bHideTabIcons   : controlsWindowed.bHideTabIcons   ; }
	inline bool& ShowScrollbars(void) { return bIsFullScreen? controlsFullScreen.bShowScrollbars : controlsWindowed.bShowScrollbars ; }

	ControlsSettings2 controlsFullScreen;
	ControlsSettings2 controlsWindowed;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct StylesSettings : public SettingsBase
{
	StylesSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	StylesSettings& operator=(const StylesSettings& other);

	bool			bCaption;
	bool			bResizable;
	bool			bTaskbarButton;
	bool			bBorder;
	bool			bTrayIcon;
	bool			bQuake;
	bool			bJumplist;
	bool			bIntegratedIME;
	bool			bHideWhenInactive;
	bool			bPerMonitorDpi;
	DWORD			dwInsideBorder;
	DWORD			dwQuakeAnimationTime;
	COLORREF		crSelectionColor;
	COLORREF		crHighlightColor;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum DockPosition
{
	dockNone	= -1,
	dockTL		= 0,
	dockTR		= 1,
	dockBL		= 2,
	dockBR		= 3,
	dockTM		= 4,
	dockBM		= 5,
	dockLM		= 6,
	dockRM		= 7,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum ZOrder
{
	zorderNormal	= 0,
	zorderOnTop		= 1,
	zorderOnBottom	= 2,
	zorderDesktop	= 3
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct PositionSettings : public SettingsBase
{
	PositionSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	PositionSettings& operator=(const PositionSettings& other);

	bool         bShowMaximized;
	bool         bSavePosition;
	bool         bSaveSize;
	int          nX;
	int          nY;
	int          nW;
	int          nH;
	ZOrder       zOrder;
	DockPosition dockPosition;
	int          nSnapDistance;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum TransparencyType
{
	transNone             = 0,
	transAlpha            = 1,
	transColorKey         = 2,
	transGlass            = 3,
	transAlphaAndColorKey = 4,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TransparencySettings : public SettingsBase
{
	TransparencySettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	TransparencySettings& operator=(const TransparencySettings& other);

	TransparencyType	transType;
	BYTE				byActiveAlpha;
	BYTE				byInactiveAlpha;
	COLORREF			crColorKey;

	static BYTE			minAlpha;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct AppearanceSettings : public SettingsBase
{
	AppearanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	AppearanceSettings& operator=(const AppearanceSettings& other);

	FontSettings         fontSettings;
	WindowSettings       windowSettings;
	ControlsSettings     controlsSettings;
	StylesSettings       stylesSettings;
	PositionSettings     positionSettings;
	TransparencySettings transparencySettings;
	FullScreenSettings   fullScreenSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct CopyPasteSettings : public SettingsBase
{
	CopyPasteSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	CopyPasteSettings& operator=(const CopyPasteSettings& other);

	bool			bCopyOnSelect;
	bool			bClearOnCopy;
	bool			bSensitiveCopy;
	bool			bNoWrap;
	bool			bTrimSpaces;
	bool			bRTF;
	bool			bIncludeLeftDelimiter;
	bool			bIncludeRightDelimiter;
	std::wstring strLeftDelimiters;
	std::wstring strRightDelimiters;

	CopyNewlineChar	copyNewlineChar;
	DWORD			dwEOLSpaces;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ScrollSettings : public SettingsBase
{
	ScrollSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ScrollSettings& operator=(const ScrollSettings& other);

	DWORD	dwPageScrollRows;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TabHighlightSettings : public SettingsBase
{
	TabHighlightSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	TabHighlightSettings& operator=(const TabHighlightSettings& other);

	DWORD	dwFlashes;
	bool	bStayHighlighted;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct CloseSettings : public SettingsBase
{
	CloseSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	CloseSettings& operator=(const CloseSettings& other);

	bool bAllowClosingLastView;
	bool bConfirmClosingMultipleViews;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FocusSettings : public SettingsBase
{
	FocusSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FocusSettings& operator=(const FocusSettings& other);

	bool bFollowMouse;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct InstanceSettings : public SettingsBase
{
	InstanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	InstanceSettings& operator=(const InstanceSettings& other);

	bool bAllowMultipleInstances;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct CloneSettings : public SettingsBase
{
	CloneSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	CloneSettings& operator=(const CloneSettings& other);

	bool bUseCurrentDirectory;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct SearchSettings : public SettingsBase
{
	SearchSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	SearchSettings& operator=(const SearchSettings& other);

	bool bMatchCase;
	bool bMatchWholeWord;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct RunAsUserSettings : public SettingsBase
{
	RunAsUserSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	RunAsUserSettings& operator=(const RunAsUserSettings& other);

	bool bUseCredentialProviders;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BehaviorSettings : public SettingsBase
{
	BehaviorSettings ();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	BehaviorSettings& operator=(const BehaviorSettings& other);

	CopyPasteSettings    copyPasteSettings;
	ScrollSettings       scrollSettings;
	TabHighlightSettings tabHighlightSettings;
	CloseSettings        closeSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BehaviorSettings2 : public SettingsBase
{
	BehaviorSettings2 ();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	BehaviorSettings2& operator=(const BehaviorSettings2& other);

	FocusSettings        focusSettings;
	InstanceSettings     instanceSettings;
	CloneSettings        cloneSettings;
	SearchSettings       searchSettings;
	RunAsUserSettings    runAsUserSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct HotKeys : public SettingsBase
{
	HotKeys();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	HotKeys& operator=(const HotKeys& other);

	struct CommandData
	{
		CommandData(const wstring& command, WORD commandID, const UINT descriptionID, bool global = false)
		: strCommand(command)
		, wCommandID(commandID)
		, strDescription(Helpers::LoadString(descriptionID))
		, bExtended(false)
		, bGlobal(global)
		, bWin(false)
		{
			::ZeroMemory(&accelHotkey, sizeof(ACCEL));
		}

    std::wstring GetHotKeyName(void)
    {
      std::wstring strHotKeyName;

      if( this->accelHotkey.fVirt || this->accelHotkey.key )
      {
        LONG    lScanCode;
        wchar_t szKeyName[32];

        if (this->accelHotkey.fVirt & FCONTROL)
        {
          lScanCode = ::MapVirtualKey(VK_CONTROL, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        if (this->accelHotkey.fVirt & FSHIFT)
        {
          lScanCode = ::MapVirtualKey(VK_SHIFT, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        if (this->accelHotkey.fVirt & FALT)
        {
          lScanCode = ::MapVirtualKey(VK_MENU, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        lScanCode = ::MapVirtualKey(this->accelHotkey.key, 0) << 16;
        if (this->bExtended) lScanCode |= 0x01000000L;

        if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
        {
          // it's used in menu, so we double &
          if (szKeyName[0] == L'&')
            strHotKeyName += L"&&";
          else
            strHotKeyName += szKeyName;
        }
      }

      return strHotKeyName;
    }

		wstring	strCommand;
		WORD	wCommandID;
		wstring	strDescription;
		ACCEL	accelHotkey;
		bool	bExtended;
		bool	bGlobal;
		bool	bWin;
	};

	struct command{};
	struct commandID{};

	typedef multi_index_container<
				std::shared_ptr<CommandData>,
				indexed_by
				<
					sequenced<>,
					ordered_unique<tag<command>,	member<CommandData, wstring, &CommandData::strCommand> >,
					ordered_unique<tag<commandID>,	member<CommandData, WORD, &CommandData::wCommandID> >
				> >									Commands;

	typedef nth_index<Commands,0>::type				CommandsSequence;
	typedef Commands::index<command>::type			CommandNameIndex;
	typedef Commands::index<commandID>::type		CommandIDIndex;

	Commands	commands;

	std::array<std::wstring, ID_EXTERNAL_COMMAND_1> externalCommands;

	bool		bUseScrollLock;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct MouseSettings : public SettingsBase
{
	enum Command
	{
		cmdNone         = 0,
		cmdCopy         = 1,
		cmdSelect       = 2,
		cmdPaste        = 3,
		cmdDrag         = 4,
		cmdMenu1        = 5,
		cmdMenu2        = 6,
		cmdMenu3        = 7,
		cmdColumnSelect = 8,
		cmdLink         = 9,
	};

	enum Button
	{
		btnNone		= 0,
		btnLeft		= 1,
		btnRight	= 2,
		btnMiddle	= 3,
		btn4th		= 4,
		btn5th		= 5,
	};

	enum ModifierKeys
	{
		mkNone	= 0x00,
		mkCtrl	= 0x01,
		mkShift	= 0x02,
		mkAlt	= 0x04
	};

	enum ClickType
	{
		clickNone	= 0,
		clickSingle	= 1,
		clickDouble	= 2
	};
	
	MouseSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	MouseSettings& operator=(const MouseSettings& other);

	struct Action
	{
		Action()
		: button(btnNone)
		, modifiers(mkNone)
		, clickType(clickNone)
		{
		}

		bool operator==(const Action& other) const
		{
			if ((button == other.button) &&
				(modifiers == other.modifiers) &&
				(clickType == other.clickType))
			{
				return true;
			}

			return false;
		}

		Button		button;
		DWORD		modifiers;
		ClickType	clickType;
	};

	struct CommandData
	{
		CommandData(Command cmd, wstring strCmd, const UINT descriptionID)
		: command(cmd)
		, strCommand(strCmd)
		, strDescription(Helpers::LoadString(descriptionID))
		{
		}

		Command	command;
		wstring	strCommand;
		wstring	strDescription;
		Action	action;
	};

	struct commandID{};
	struct commandName{};

	typedef multi_index_container<
				std::shared_ptr<CommandData>,
				indexed_by
				<
					sequenced<>,
					ordered_unique<tag<commandName>,		member<CommandData, wstring, &CommandData::strCommand> >,
					ordered_unique<tag<commandID>,			member<CommandData, Command, &CommandData::command> >
				> >									Commands;

	typedef nth_index<Commands,0>::type				CommandsSequence;
	// these typedefs seem to produce internal compiler errors with 7.1 compiler
	// we'll use them where needed in the code
/*
	typedef Commands::index<commandID>::type		CommandIDIndex;
	typedef Commands::index<commandName>::type		CommandNameIndex;
*/

	Commands	commands;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct VarEnv
{
	VarEnv()
		: strEnvVariable()
		, strEnvValue()
		, bEnvChecked(false)
	{
	}

	VarEnv(const std::wstring& strEnvVariable,
	       const std::wstring& strEnvValue)
		: strEnvVariable(strEnvVariable)
		, strEnvValue(strEnvValue)
		, bEnvChecked(true)
	{
	}

	std::wstring strEnvVariable;
	std::wstring strEnvValue;
	bool         bEnvChecked;
};

struct TabData
{
	TabData(const wstring& shell, const wstring& initialDir)
	: strTitle(L"ConsoleZ")
	, strIcon(L"")
	, bUseDefaultIcon(false)
	, hwnd(nullptr)
	, strShell(shell)
	, strInitialDir(initialDir)
	, dwBasePriority(2)
	, bRunAsUser(false)
	, strUser()
	, bNetOnly(false)
	, bRunAsAdministrator(false)
	, bCloneable(true)
	, bInheritedCursor(true)
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	, bInheritedBackground(true)
	, backgroundImageType(bktypeNone)
	, crBackgroundColor(RGB(0, 0, 0))
	, iconMenu()
	, imageData()
	, bInheritedColors(true)
	, backgroundTextOpacity(255)
	, nIndex(0)
	{
		consoleColors[0]  = 0x000000;
		consoleColors[1]  = 0x800000;
		consoleColors[2]  = 0x008000;
		consoleColors[3]  = 0x808000;
		consoleColors[4]  = 0x000080;
		consoleColors[5]  = 0x800080;
		consoleColors[6]  = 0x008080;
		consoleColors[7]  = 0xC0C0C0;
		consoleColors[8]  = 0x808080;
		consoleColors[9]  = 0xFF0000;
		consoleColors[10] = 0x00FF00;
		consoleColors[11] = 0xFFFF00;
		consoleColors[12] = 0x0000FF;
		consoleColors[13] = 0xFF00FF;
		consoleColors[14] = 0x00FFFF;
		consoleColors[15] = 0xFFFFFF;
	}

	// custom shell settings
	wstring							strTitle;
	wstring							strIcon;
	bool							bUseDefaultIcon;
	HWND							hwnd;

	wstring							strShell;
	wstring							strInitialDir;
	DWORD							dwBasePriority;
	bool							bRunAsUser;
	wstring							strUser;
	bool							bNetOnly;
	bool							bRunAsAdministrator;
	bool							bCloneable;

	bool							bInheritedCursor;
	DWORD							dwCursorStyle;
	COLORREF						crCursorColor;

	bool							bInheritedBackground;
	BackgroundImageType				backgroundImageType;
	COLORREF						crBackgroundColor;
	ImageData						imageData;

	CIcon							iconMenu;

	bool							bInheritedColors;
	COLORREF						consoleColors[16];
	BYTE                backgroundTextOpacity;

	std::vector<std::shared_ptr<VarEnv>> environmentVariables;

	long                nIndex;

	void SetColors(const COLORREF colors[16], const BYTE opacity, const bool bForced)
	{
		if (bInheritedColors || bForced)
		{
			::CopyMemory(consoleColors, colors, sizeof(consoleColors));
			backgroundTextOpacity = opacity;
		}
	}

	void SetCursor(const DWORD dwNewCursorStyle, const COLORREF crNewCursorColor, const bool bForced)
	{
		if (bInheritedCursor || bForced)
		{
			this->dwCursorStyle = dwNewCursorStyle;
			this->crCursorColor = crNewCursorColor;
		}
	}

	void SetBackground(const BackgroundImageType newBackgroundImageType, const COLORREF crNewBackgroundColor, const ImageData newImageData, const bool bForced)
	{
		if (bInheritedBackground || bForced)
		{
			this->backgroundImageType = newBackgroundImageType;
			this->crBackgroundColor   = crNewBackgroundColor;
			this->imageData           = newImageData;
		}
	}

	HICON GetMenuIcon(void)
	{
		if (iconMenu.IsNull())
		{
			// load small icon
			iconMenu.Attach(GetSmallIcon());
		}
		return iconMenu;
	}

	HICON GetBigIcon(void)
	{
		if( hwnd )
		{
			return reinterpret_cast<HICON>(::SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
		}
		else
		{
			return Helpers::LoadTabIcon(true, bUseDefaultIcon, strIcon, strShell);
		}
	}

	HICON GetSmallIcon(void)
	{
		if( hwnd )
		{
			return reinterpret_cast<HICON>(::SendMessage(hwnd, WM_GETICON, ICON_SMALL2, 0));
		}
		else
		{
			return Helpers::LoadTabIcon(false, bUseDefaultIcon, strIcon, strShell);
		}
	}

	static DWORD GetPriorityClass(DWORD dwPriority)
	{
		switch( dwPriority )
		{
		case 0:  return IDLE_PRIORITY_CLASS;
		case 1:  return BELOW_NORMAL_PRIORITY_CLASS;
		case 2:  return NORMAL_PRIORITY_CLASS;
		case 3:  return ABOVE_NORMAL_PRIORITY_CLASS;
		case 4:  return HIGH_PRIORITY_CLASS;
		case 5:  return REALTIME_PRIORITY_CLASS;
		default: return NORMAL_PRIORITY_CLASS;
		}
	}

	static DWORD StringToPriority(const wchar_t * p)
	{
		     if( _wcsicmp(L"Idle",        p) == 0 ) return 0;
		else if( _wcsicmp(L"BelowNormal", p) == 0 ) return 1;
		else if( _wcsicmp(L"Normal",      p) == 0 ) return 2;
		else if( _wcsicmp(L"AboveNormal", p) == 0 ) return 3;
		else if( _wcsicmp(L"High",        p) == 0 ) return 4;
		else if( _wcsicmp(L"Realtime",    p) == 0 ) return 5;

		return ULONG_MAX;
	}

	static const wchar_t * PriorityToString(DWORD dwPriority)
	{
		switch( dwPriority )
		{
		case 0:  return L"Idle";
		case 1:  return L"BelowNormal";
		case 2:  return L"Normal";
		case 3:  return L"AboveNormal";
		case 4:  return L"High";
		case 5:  return L"Realtime";
		default: return L"Normal";
		}
	}
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef vector<std::shared_ptr<TabData> >	TabDataVector;

struct TabSettings : public SettingsBase
{
	TabSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	void SetDefaults(const wstring& defaultShell, const wstring& defaultInitialDir);

	TabDataVector	tabDataVector;

private:

	wstring			strDefaultShell;
	wstring			strDefaultInitialDir;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SettingsHandler
{
	public:

		enum SettingsDirType
		{
			dirTypeExe		= 0,
			dirTypeUser		= 1,
			dirTypeCustom	=2
		};

	public:
		SettingsHandler();
		~SettingsHandler();

	public:

		bool LoadSettings(const wstring& strSettingsFileName);
		bool SaveSettings();

		wstring	GetSettingsFileName() const { return m_strSettingsPath+m_strSettingsFileName; }
		wstring	GetSettingsPath() const { return m_strSettingsPath; }
		wstring	GetSettingsTitle() const { return m_strSettingsFileName; }

		SettingsDirType GetSettingsDirType() const { return m_settingsDirType; }
		void SetUserDataDir(SettingsDirType settingsDirType);

		ConsoleSettings& GetConsoleSettings() { return m_consoleSettings; }
		AppearanceSettings& GetAppearanceSettings() { return m_appearanceSettings; }
		BehaviorSettings& GetBehaviorSettings() { return m_behaviorSettings; }
		BehaviorSettings2& GetBehaviorSettings2() { return m_behaviorSettings2; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		MouseSettings& GetMouseSettings() { return m_mouseSettings; }
		TabSettings& GetTabSettings() { return m_tabSettings; }

	private:

		CComPtr<IXMLDOMDocument>	m_pSettingsDocument;
		CComPtr<IXMLDOMElement>		m_pSettingsRoot;

	private:

		wstring				m_strSettingsPath;
		wstring				m_strSettingsFileName;

		SettingsDirType		m_settingsDirType;

		ConsoleSettings		m_consoleSettings;
		AppearanceSettings	m_appearanceSettings;
		BehaviorSettings	m_behaviorSettings;
		BehaviorSettings2	m_behaviorSettings2;
		HotKeys				m_hotKeys;
		MouseSettings		m_mouseSettings;

		TabSettings			m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////
