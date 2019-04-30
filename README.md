ConsoleZ
========

[![Join the chat at https://gitter.im/cbucher/console](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/cbucher/console?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

![screenshot005](https://github.com/cbucher/console/wiki/screenshot001.png)

This is a modified version of Console 2 for a better experience under Windows Vista/7/8/10 and a better visual rendering.

This fork supports:

* Splitting Tabs into views (horizontally and vertically)
* Grouping views (so input sent to one goes to all of them)
* Windows Vista aero glass theme
* Windows 7 jump list
* Windows 7 wallpaper positions and slideshow
* Windows 8 wallpapers on dual screen
* Zooming with Ctrl-Mouse
* Quake style console animation
* Strict monospace font rendering
* Settable opacity of text background color
* Full screen
* High DPI (per monitor)
* Localization
* Snippets
* Workspaces
* and more...

Downloads
=========

See the [download wiki page](https://github.com/cbucher/console/wiki/Downloads).

Changelog
=========

    ! changed
    - removed
    + added
    * fixed

Changes in 1.19.0 (14 Apr 2019)

    ! Ignore tab flashing if console view age is less than 3 seconds.
    + Named instances.
    + "-attach" command line switch.
    + Move current view into a new tab.
    + Record which tab had focus when saving workspace.
    + Clone the whole current tab.
    + Organize tabs within submenus.
    * Selection length was not refreshed in status bar.
    * Fix tab bar showing in full screen mode.
    * Fix crash when spliting then selecting another pane.
    * Tab background position was not preserved.
    * Tabs menu doesn't switch to a tab after the 10th choice.
    * If OpenClipboard fails, retry for 10 times with a 100ms delay.

Changes in 1.18.3 (30 Apr 2018)

    ! WTL 10.0.7336
    ! GDI+ replaces FreeImage library in aero version.
    + Maximize/Restore a view in a tab.
    + Support for animated background pictures.
    + Support for alternate desktop wallpaper pictures.
    * Fix duplicated "Switch to next tab" and "Switch to previous tab"
      commands in hotkeys settings.

Changes in 1.18.2 (29 Sep 2017)

    ! Display an error dialog when seeking settings of an undefined tab.
    ! Detect if ConsoleZ is hooked (ConsoleHook/ConsoleHook32).
    + "New Tab" button in tabs area.
    + Save "Switch On/Off Transparency" state.
    * Fix startup directory consistency.
    * Fix retrieving of last process ID attached to the console.
    * Fix sliders in transparency settings dialog.
    * Refresh tab title when last process ID attached to the console changes.
    * Fix bing image copyright displayed into status bar.
    * Fix external command saving into settings file.

Changes in 1.18.1 (27 Mar 2017)

    + Transparency settings per windowed/full screen mode.
    + Print code page (for each shell) in diagnostic report.
    * Fix ALT-SPACE menu.
    * Fix "Attach Consoles" failure.
    * Fix split bar size.
    * Start a safe console when no console started.
    * Standardize hotkeys modifiers and order (mouse).

Changes in 1.18.0 (17 Feb 2017)

    ! Update tab title current directory in realtime.
    ! Renaming a tab title to empty is no longer allowed
      (empty string is replaced by tab title from settings).
    ! Backup settings file using a timestamp (unique backup filename).
    ! Tabbing framework localization.
    + Workspaces.
    + Option to hide the "close tab" button.
    + Split bar size is configurable.
    + Allow merging tabs (horizontally or vertically).
    + Allow view keeping background, colors and cursor
      when it moved into another tab.
    + Support for reversed video (swap foreground and background).
    + Allow mouse events forwarding.
    + Show Bing image copyright in status bar.
    * GDI leaks (thanks to char101).
    * Allow choosing environment variables inheritance method.
    * Improve Windows 10 glass transparency.
    * PowerShell progress bar is fixed on Windows 10.

Changes in 1.17.2 (18 Nov 2016)

    ! New Icons (a gift from BlazenBundy).
    ! Use icon defined in appearance settings as default tab icon.
    + Allow configuration of icon size.
    + Option to prevent exiting ConsoleZ on closing of last tab.
    + Support for underlined text.
    * Fix "snap to edges" feature.
    * Fix rebar auto sizing bug.
    * Fix docking bug when showing/hiding bar.
    * Update window icon after settings edition.
    * Update tab icons after settings edition.
    * German localization (thanks to sskopnik).

Changes in 1.17.1 (22 Sep 2016)

    ! Handle XML parsing errors (configuration file and snippets).
    + "Toggle always on top" hotkey command.
    + New high dpi icons (250%, 300%, 400% and 500%).
    + Bing image of the day as background.
    * ConsoleZ no longer closes without explanation on bad config.
    * Start a safe console when no console started.
    * Fix scroll bars position in hidden tabs.
    * System environment variables no longer override local environment variables.
    * Japanese localization (thanks to ys-nuem).
    * Link files are also resolved when text is copied in shell text box.

Changes in 1.17.0 (08 may 2016)

    ! Rebuild FreeImage 3.17.0 (static link without OpenMP)
    - ConsoleZ no longer sets the current Windows console font to "Lucidia Console" size 5.
      (DBCS code page requires a specific font)
    + Snippets manager.
    + "Resume scrolling" command.
    + Configure initial window state (appearance settings).
    + Japanese localization (thanks to rosolino).
    * Mono-instance is no longer forced in jump lists.
    * User profile is fully created when running a shell with another user credentials.
    * Scrolling during selection does not properly refresh.

Changes in 1.16.1 (08 mar 2016)

    ! Upgrade FreeImage version from 3.15.4 to 3.17.0.
    ! Show BMP/DIB/GIF/JPEG/PNG files in Browse background image dialog.
    + Detect new docking positions with "snap to desktop edges"
      (top/bottom/left/right middle).
    + ConsoleZ sets the current Windows console font to "Lucidia Console" size 5.
      Windows console should support UNICODE characters.
      Number of rows/columns should not be limited.
    + Support UNICODE picture filename.
    + German localization (thanks to cruzzer).
    * Fix placement problem with "snap to desktop edges" when cursor is in another monitor.
    * Fix crash when wallpaper picture loading fails.
    * When you configured a background with relative picture then wallpaper was drawn.
    * Fix the word selection: right delimiters was not used.
    * "Click Link" does no longer launch an empty string.
      This was launching explorer in the current working directory.
    * Fix highlighting color setting saving.
    * Fix wallpaper display.

Changes in 1.16.0 (07 feb 2016)

    ! PowerShell progressbar detection is disabled.
    ! ConsoleZ hook can detect if the shell is not a Win32 console application.
    + Command-line switch to toggle window visibility.
    + Select text/column with keyboard.
    + Per-monitor DPI awareness.
    + Arrange tabs using keyboard.
    + Allow distinct controls settings for full screen
      and windowed application.
    + Environment inheritance is coming back.
      Inheritance works now with mono-instance.
      There is no inheritance when the shell run as administrator
      or whith another user credentials!
    + Glass effect transparency on Windows 10.
    + Specific ConsoleZxxx environment variables:
      ConsoleZBaseDir = folder containing console.exe
      ConsoleZVersion = current version of ConsoleZ
    * Hide, when loosing focus, only if the new foreground window
      is not owned by our application.
    * Fix crash when screen buffer size is reduced.
    * Fix dpi scaling of some GUI parts.
    * Fix activation by hotkey.

Changes in 1.15.0 (10 sep 2015)

    + Hide when loosing focus.
    + Support localization.
    + New command "paste selection".
    + Support %D (current directory leaf) in text formatting.
    + Support PowerShell progress bar.
    + Support typographic ligatures.
    + Support missing font smoothings: antialiased and "natural" ClearType.
    + Support localization.
    + French localization.
    + Russian localization.
    * Fix "Clone in new Tab" inverts initial directory and tab's title.
    * Fix incomplete shell title.
    * Fix touch scrolling broken on Win10.
    * Fix "Browse..." button for icon change causes program "Not Responding".
    * Fix repaint failure when background is a relative image
      and the application is minimized.
    * Fix hook can crash during resizing.

Changes in 1.14.0 (08 May 2015)

    ! Limit frame rate to avoid unresponsive application.
    ! Use IDesktopWallpaper to retrieve monitors' wallpaper (Win8).
    ! Update title even without text modification.
    ! Double-click on scroll buttons do nothing.
    + 'Tabs' entry in the menubar.
    + -n command arg to set startup tab title.
    * Fix bad cursor display in tabs settings.
    * Fix console cursor color.
    * Fix a crash in diagnostic report writing.
    * Call DoDataExchange(DDX_SAVE) before modifiying values
      in browse shell/initial dir buttons.
    * Prevent from infinite waiting when the shell is terminated
      during a mouse forwarding or a text selection.
    * Fix program does not respond by opening the setting dialog.
    * Fix toolbar does not work from remote desktop.

Changes in 1.13.0 (14 Feb 2015)

    ! Visual Studio 2013
    ! WTL 9.4140 final
    ! Legacy 32 bits version is compatible with Windows XP.
    ! Support high DPI.
    ! Create HOMEDRIVE, HOMEPATH and HOMESHARE environment variables
      when launching shell as different user.
    ! Expand environment strings using environment bloc
                                     + HOMEDRIVE/HOMEPATH/HOMESHARE
                                     + variables defined in tab.
    + Switch on/off transparency.
    + CTRL-C event can be generated by configured hotkey.
    + Diagnose and get font information.
    * "Snap to desktop edge" is ignored when Win key is down.
    * Active auto horizontal scroll in search combobox.
    * Fix restoring with "Snap to desktop edge".
    * Fix crash when attaching a console with a screen bigger than ConsoleZ's view.
    * Docking is ignored when ConsoleZ is full screen.
    * Fix elevated process detection.

Changes in 1.12.0 (09 Oct 2014)

    ! The tab's title is now modified when you drag & drop a shortcut.
      The title is set with shortcut filename.
    ! The default colors map is no longer displayed in Settings/Console
    ! Mouse command "drag window" has no effect when application is maximized or runs
      in fullscreen mode.
    + Tab's background can be inherited.
    + Tab's cursor can be inherited.
    + Tab Base Priority.
    + "Click link" mouse command.
    + Set environment variables per tabs.
    + Clear and Reset Scrollback.
    * Use impersonated user's environment block.

Changes in 1.11.0 (23 Jun 2014)

    ! Window columns limit is now 300.
    ! buffer columns limit is now 9999.
    ! Word selection is no longer limited to the window.
    ! Minimize to systray when taskbar button is unchecked and systray icon is checked.
    ! Changing docking position in settings no longer require restart.
    ! Redesign the tabbar (flat style).
    ! Allow starting a new selection when current selection is not cleared.
    + Attach consoles.
    + Detach view.
    + Add horizontal bar cursor.
    + Add %d to text formatting syntax: current directory.
    + Add %P to text formatting syntax: ID of last process attached to the console.
    + Support external commands (syntax uses title formatting).
    + Support credential providers (allowing integration of additional authentication
      mechanisms such as biometrics and smart cards).
    + New docking positions: top middle, bottom middle, left middle and right middle.
    + Quake like animation time is settable.
    + Add search bar.
    + Allow disabling RTF in settings.
    * Fix aero toolbar drawing.
    * Docking no longer affects maximized application.

Changes in 1.10.0 (30 Mar 2014)

    ! Tab number is now full part of the title.
    ! Main title format can be defined by user.
    ! Tab title format can be defined by user.
    ! Remove "Administrator:" prefix.
    + Input Method Editor (IME) support.
    + Allow selection by column.
    + Clone current view in a new tab.
    + Clone/Split using the current directory.
    * Fix "pinned to desktop" mode on Windows 7 and more.
    * Fix paste inconsistency on multiple lines.
    * Fix Quake like show/hide animation.
    * Tab text was hidden when less than 12 pixels.
    * Fix Quake like show/hide.
    * Fix some crashes.
    * Fix starting directory.
    * Fix VIM dead char.

Changes in 1.09.1 (17 Dec 2013)

    ! In settings, check if a hotkey is already assigned.
    * Fix the new keyboard handling.

Changes in 1.09.0 (01 Dec 2013)

    ! Title options are more consistent.
    ! Activate ConsoleZ (global) hotkey minimizes the application if active.
    ! Quake like: activates the previous application when hiding.
    ! Buffer rows limit increased to 32766.
    ! Can configure the number of spaces used to detect an end of line.
    + Focus follows mouse.
    + New actions in "File" menu:
      "Close All Tabs BUT This"
      "Close All Tabs to the Left"
      "Close All Tabs to the Right"
    + Right click on tabs: selects the tab and displays a popup menu.
    + New cursor that mimics the Windows console's one.
    + Can specify an initial application window size (in pixels).
    + Can save the application window size (in pixels) on exit.
    + Allow multiple instances (or single instance).
    + Blinking XTerm Cursor.
    + Support of drag and drop in settings.
    + Can hide icons in tabs bar.
    * Fix problem with dead chars sent to multiple consoles (new keyboard handling).
    * Fix icon extraction with negative index.
    * Fix a visual bug when caption and resize are removed.
    * Transmit the current working directory when application instance is reused.
    * Maximized application was restored at wrong position.
    * If the scroll bar is unnecessary, disable the scroll bar instead of removing it.

Changes in 1.08.0 (28 Sep 2013)

    ! prettier split bar (aero only).
    + displays intensified colors in bold.
    + displays intensified colors in italic.
    + user can configure an extra width added to the fixed width of the font.
    + shell can be launched with elevated priviledges (run as administrator).
    + supports shell link files (.lnk) in tabs settings.
      Icon, program's path, arguments, working directory and "run as administrator"
      option are extracted from the link.
    * Some cursors was hidden with black color.
    * Fade cursor was blinking when console was inactive.
    * Fix DBCS rendering.
    * Fix DBCS selection.
    * Fix DBCS cursor.
    * Fix hang when starting with multiple startup tabs.
    * Pressing alt key will switch the menu bar visibility only if menu bar is not
      permanently on.
    * Fix pasting to multiple views when views are grouped.
    * Inputs are no longer sent to views do not belong to the group.
    * Active view cannot be painted (and 100% cpu).

Changes in 1.07.1 (05 Jul 2013)

    * Sometimes the application could not start with multiple tabs.
      The delay between initial tabs was uninitialized.
    * Icons missing in tabs menu popup.

Changes in 1.07.0 (26 May 2013)

    ! Limits the maximized windows size on monitors with taskbar
      so ConsoleZ, with no border, no longer overlaps the taskbar when WIN+UP is pressed
    ! Changing tabbar position in settings no longer require restart.
    ! Settings edition no longer modifies the zoom factor
    + Full screen support
    + Full screen button in taskbar
    + Full screen hot key (default: F11)
    + Full screen settings: can start in full screen and select the full screen monitor
    + Left and right delimiters for word selection can be set
    + View can be closed from the menu
    + New behavior settings:
      "Allow closing the last tab/view"
      "Confirm before closing multiple tabs/views"
    + Color map per tab
    + Color map can be imported from another configuration file
    + 3 context menus can be configured in mouse settings:
      Context menu #1 = full menu
      Context menu #2 = configured tabs
      Context menu #3 = opened tabs
    + Zoom factor in status bar
    + New hotkeys:
      "Zoom 100%"      (default: CTRL+0 NUM KEYPAD)
      "Zoom increment" (default: CTRL++ NUM KEYPAD)
      "Zoom decrement" (default: CTRL+- NUM KEYPAD)
    * Caption was removed when border was unchecked
    * Display of hotkey '&' in menu was truncated
    * Press ALT key, with mouse action, no longer shows/hides the menu

Changes in 1.06.0 (24 Mar 2013)

    ! The cloned tab title is suffixed with (2).
    ! about dialog box improvement
    ! Hotkeys are displayed in the menu
    ! Restart is no longer needed for these appearance settings:
      "Caption"
      "Resizable"
      "Border"
      "Taskbar button"
    + new option "Net Only" to "Run as user"
      the user credentials are used only to access network resources
      the user is displayed in tab title surrounded with braces
    + "Run as user" supports UPN format (user@domain).
    + new buttons in the toolbar:
      "Rename tab"
      "Split Horizontally"
      "Split Vertically"
    + new hotkeys:
      "New Tab 11"            (default: CTRL+F11)
      "New Tab 12"            (default: CTRL+F12)
      "Switch to left view"   (default: ALT+LEFT)
      "Switch to right view"  (default: ALT+RIGHT)
      "Switch to top view"    (default: ALT+UP)
      "Switch to bottom view" (default: ALT+DOWN)
    + Windows key can be used in global hotkeys.
    + The glass frame extended in client area (toolbar and tabs) can be used to
      move the window.
    + The glass frame extended in client area (tabs only) can be used to
      maximize or restore the window by double clicking.
    * If the only one tab was cloned, the "delete" button remained disabled.
    * Under Windows 8, the call to GetTextMetrics function failed with some fonts.
      The return code wasn't checked and char metrics wasn't correctly initialized.
    * wrong tabs scrolling
    * close button drawing
    * background of tab was black with basic Windows theme
    * error handling and return code checking
    * Re-add tray icon if Windows Explorer restarts.
    * When files are dropped on a view, this view gets the focus.
    * Title was not updated when view changed.

Changes in 1.05.0 (2 Feb 2013)

    ! FreeImage 3.15.4
    ! Visual Studio 2012
    ! WTL 8.1.12085
    ! Forces monospace displaying
      by adjusting the size of chars larger than average width font.
    ! Limits vertical scrolling to the furthest buffer location viewed.
    + Windows 8 can use a wallpaper per monitor.
    + Displays a closing confirmation when there is only one tab
      but multiple views.
    + Improves the status bar with:
      console buffer size, console screen size,
      selection size and console PID
    * Dropping a file affects all views in a group.
    * Aero glass margins are resetted when composition change
      (hibernation disables DWM, at wakeup ConsoleZ was fully transparent).
