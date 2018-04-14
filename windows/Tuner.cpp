////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A Tuner written in C++.
//
//  Copyright (C) 2009  Bill Farmer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include "Tuner.h"

// Application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Check for a previous instance of this app
    if (!hPrevInstance)
	if (!RegisterMainClass(hInstance))
	    return FALSE;

    // Save the application-instance handle.
    hInst = hInstance;

    // Initialize common controls to get the new style controls, also
    // dependent on manifest file
    InitCommonControls();

    // Start Gdiplus
    GdiplusStartup(&token, &input, NULL);

    // Get saved status
    GetSavedStatus();

    // Create the main window.
    window.hwnd =
	CreateWindow(WCLASS, L"Tuner",
		     WS_OVERLAPPED | WS_MINIMIZEBOX |
		     WS_SIZEBOX | WS_SYSMENU,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     NULL, 0, hInst, NULL);

    // If the main window cannot be created, terminate
    // the application.
    if (!window.hwnd)
	return FALSE;

    // Show the window and send a WM_PAINT message to the window
    // procedure.
    ShowWindow(window.hwnd, nCmdShow);
    UpdateWindow(window.hwnd);

    // Process messages
    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Register class
BOOL RegisterMainClass(HINSTANCE hInst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.
    WNDCLASS wc = 
	{CS_HREDRAW | CS_VREDRAW,
	 MainWndProc, 0, 0, hInst,
	 LoadIcon(hInst, L"Tuner"),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.
    return RegisterClass(&wc);
}

VOID GetSavedStatus()
{
    HKEY hkey;
    LONG error;
    DWORD value;
    int size = sizeof(value);

    // Initial values
    strobe.enable = TRUE;
    audio.filter = TRUE;
    spectrum.expand = 1;

    // Reference initial value
    audio.reference = A5_REFNCE;

    // Open user key
    error = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			 KEY_READ, &hkey);

    if (error == ERROR_SUCCESS)
    {
	// Spectrum zoom
	error = RegQueryValueEx(hkey, L"Zoom", NULL, NULL,
				(LPBYTE)&value, (LPDWORD)&size);
	// Update value
	if (error == ERROR_SUCCESS)
	    spectrum.zoom = value;

	// Strobe enable
	error = RegQueryValueEx(hkey, L"Strobe", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value
	if (error == ERROR_SUCCESS)
	    strobe.enable = value;

	// Strobe colours
	error = RegQueryValueEx(hkey, L"Colours", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value
	if (error == ERROR_SUCCESS)
	    strobe.colours = value;

	// Filter
	error = RegQueryValueEx(hkey, L"Filter", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value
	if (error == ERROR_SUCCESS)
	    audio.filter = value;

	// Reference
	error = RegQueryValueEx(hkey, L"Reference", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value
	if (error == ERROR_SUCCESS)
	    audio.reference = value / 10.0;

	// Close key
	RegCloseKey(hkey);
    }
}

// Main window procedure
LRESULT CALLBACK MainWndProc(HWND hWnd,
			     UINT uMsg,
			     WPARAM wParam,
			     LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            // Get the window and client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            // Calculate desired window width and height
            int border = (window.wind.right - window.wind.left) -
                window.rect.right;
            int header = (window.wind.bottom - window.wind.top) -
                window.rect.bottom;
            int width  = WIDTH + border;
            int height = HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            width = window.rect.right;
            height = window.rect.bottom;

            // Create tooltip
            tooltip.hwnd =
                CreateWindow(TOOLTIPS_CLASS, NULL,
                             WS_POPUP | TTS_ALWAYSTIP,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             hWnd, NULL, hInst, NULL);

            SetWindowPos(tooltip.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            tooltip.info.cbSize = sizeof(tooltip.info);
            tooltip.info.hwnd = hWnd;
            tooltip.info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;

            // Create scope display
            scope.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, MARGIN, width - MARGIN * 2,
                             SCOPE_HEIGHT, hWnd,
                             (HMENU)SCOPE_ID, hInst, NULL);
            GetWindowRect(scope.hwnd, &scope.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&scope.rect, 2);

            // Add scope to tooltip
            tooltip.info.uId = (UINT_PTR)scope.hwnd;
            tooltip.info.lpszText = (LPWSTR)L"Scope, click to filter audio";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create spectrum display
            spectrum.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, scope.rect.bottom + SPACING,
                             width - MARGIN * 2,
                             SPECTRUM_HEIGHT, hWnd,
                             (HMENU)SPECTRUM_ID, hInst, NULL);
            GetWindowRect(spectrum.hwnd, &spectrum.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&spectrum.rect, 2);

            // Add spectrum to tooltip
            tooltip.info.uId = (UINT_PTR)spectrum.hwnd;
            tooltip.info.lpszText = (LPWSTR)L"Spectrum, click to zoom";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create display
            display.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, spectrum.rect.bottom + SPACING,
                             width - MARGIN * 2, DISPLAY_HEIGHT, hWnd,
                             (HMENU)DISPLAY_ID, hInst, NULL);
            GetWindowRect(display.hwnd, &display.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&display.rect, 2);

            // Add display to tooltip
            tooltip.info.uId = (UINT_PTR)display.hwnd;
            tooltip.info.lpszText = (LPWSTR)L"Display, click to lock";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create strobe
            strobe.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, display.rect.bottom + SPACING,
                             width - MARGIN * 2, STROBE_HEIGHT, hWnd,
                             (HMENU)STROBE_ID, hInst, NULL);
            GetWindowRect(strobe.hwnd, &strobe.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&strobe.rect, 2);

            // Create tooltip for strobe
            tooltip.info.uId = (UINT_PTR)strobe.hwnd;
            tooltip.info.lpszText = (LPWSTR)L"Strobe, click to disable/enable";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create meter
            meter.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, strobe.rect.bottom + SPACING,
                             width - MARGIN * 2, METER_HEIGHT, hWnd,
                             (HMENU)METER_ID, hInst, NULL);
            GetWindowRect(meter.hwnd, &meter.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&meter.rect, 2);

            // Add meter to tooltip
            tooltip.info.uId = (UINT_PTR)meter.hwnd;
            tooltip.info.lpszText = (LPWSTR)L"Cents, click to lock";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Start audio thread
            audio.thread = CreateThread(NULL, 0, AudioThread, hWnd,
                                        0, &audio.id);

            // Start meter timer
            CreateTimerQueueTimer(&meter.timer, NULL,
                                  (WAITORTIMERCALLBACK)MeterCallback,
                                  &meter.hwnd, METER_DELAY, METER_DELAY,
                                  WT_EXECUTEDEFAULT);

            // Start strobe timer
            CreateTimerQueueTimer(&strobe.timer, NULL,
                                  (WAITORTIMERCALLBACK)StrobeCallback,
                                  &strobe.hwnd, STROBE_DELAY, STROBE_DELAY,
                                  WT_EXECUTEDEFAULT);
        }
        break;

	// Colour static text
    case WM_CTLCOLORSTATIC:
    	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    	break;

	// Draw item
    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
	break;

	// Disable menus by capturing this message
    case WM_INITMENU:
	break;

	// Capture system character key to stop pop up menus and other
	// nonsense
    case WM_SYSCHAR:
	break;

	// Set the focus back to the window by clicking
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

        // Context menu
    case WM_RBUTTONDOWN:
	DisplayContextMenu(hWnd, MAKEPOINTS(lParam));
	break;

	// Buttons
    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	    // Scope
	case SCOPE_ID:
	    ScopeClicked(wParam, lParam);
	    break;

	    // Display
	case DISPLAY_ID:
	    DisplayClicked(wParam, lParam);
	    break;

	    // Spectrum
	case SPECTRUM_ID:
	    SpectrumClicked(wParam, lParam);
	    break;

	    // Strobe
	case STROBE_ID:
	    StrobeClicked(wParam, lParam);
	    break;

	    // Meter
	case METER_ID:
	    MeterClicked(wParam, lParam);
	    break;

	    // Zoom control
	case ZOOM_ID:
	    ZoomClicked(wParam, lParam);
	    break;

	    // Enable control
	case ENABLE_ID:
	    EnableClicked(wParam, lParam);
	    break;

	    // Filter control
	case FILTER_ID:
	    FilterClicked(wParam, lParam);
	    break;

	    // Downsample control
	case DOWN_ID:
	    DownClicked(wParam, lParam);
	    break;

	    // Lock control
	case LOCK_ID:
	    LockClicked(wParam, lParam);
	    break;

	    // Multiple control
	case MULTIPLE_ID:
	    MultipleClicked(wParam, lParam);
	    break;

	    // Options
	case OPTIONS_ID:
	    DisplayOptions(wParam, lParam);
	    break;

	    // Quit
	case QUIT_ID:
            Gdiplus::GdiplusShutdown(token);
	    waveInStop(audio.hwi);
	    waveInClose(audio.hwi);
	    PostQuitMessage(0);
	    break;
	}

	// Set the focus back to the window
	SetFocus(hWnd);
	break;

	// Char pressed
    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

	// Audio input data
    case MM_WIM_DATA:
	WaveInData(wParam, lParam);
	break;

	// Size
    case WM_SIZE:
	EnumChildWindows(hWnd, EnumChildProc, lParam);
	break;

	// Sizing
    case WM_SIZING:
	return WindowResizing(hWnd, wParam, lParam);
	break;

        // Process other messages.
    case WM_DESTROY:
        Gdiplus::GdiplusShutdown(token);
	waveInStop(audio.hwi);
	waveInClose(audio.hwi);
	PostQuitMessage(0);
	break;

	// Everything else
    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Enum child proc for window resizing
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);

    // Switch by id to resize tool windows.
    switch ((DWORD)GetWindowLongPtr(hWnd, GWLP_ID))
    {
	// Scope, resize it
    case SCOPE_ID:
	MoveWindow(hWnd, MARGIN, MARGIN, width - MARGIN * 2,
                   (height - TOTAL) * SCOPE_HEIGHT / TOTAL_HEIGHT,
                   FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &scope.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&scope.rect, 2);
	break;

	// Spectrum, resize it
    case SPECTRUM_ID:
	MoveWindow(hWnd, MARGIN, scope.rect.bottom + SPACING,
		   width - MARGIN * 2,
                   (height - TOTAL) * SPECTRUM_HEIGHT / TOTAL_HEIGHT,
                   FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &spectrum.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&spectrum.rect, 2);
	break;

	// Display, resize it
    case DISPLAY_ID:
	MoveWindow(hWnd, MARGIN, spectrum.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * DISPLAY_HEIGHT / TOTAL_HEIGHT,
                   FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &display.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&display.rect, 2);
	break;

	// Strobe, resize it
    case STROBE_ID:
	MoveWindow(hWnd, MARGIN, display.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * STROBE_HEIGHT / TOTAL_HEIGHT,
                   FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &strobe.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&strobe.rect, 2);
	break;

	// Meter, resize it
    case METER_ID:
	MoveWindow(hWnd, MARGIN, strobe.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * METER_HEIGHT / TOTAL_HEIGHT,
                   FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &meter.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&meter.rect, 2);
	break;
    }

    return TRUE;
}

// Window resizing
BOOL WindowResizing(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PRECT rectp = (PRECT)lParam;

    // Get the window and client dimensions
    GetWindowRect(hWnd, &window.wind);
    GetClientRect(hWnd, &window.rect);

    // Edges
    int border = (window.wind.right - window.wind.left) -
        window.rect.right;
    int header = (window.wind.bottom - window.wind.top) -
        window.rect.bottom;

    // Window minimum width and height
    int width  = WIDTH + border;
    int height = HEIGHT + header;

    // Minimum size
    if (rectp->right - rectp->left < width)
	rectp->right = rectp->left + width;

    if (rectp->bottom - rectp->top < height)
	rectp->bottom = rectp->top + height;

    // Maximum width
    if (rectp->right - rectp->left > STEP + border)
        rectp->right = rectp->left + STEP + border;

    // Offered width and height
    width = rectp->right - rectp->left;
    height = rectp->bottom - rectp->top;

    switch (wParam)
    {
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
        height = (((width - border) * HEIGHT) / WIDTH) + header;
        rectp->bottom = rectp->top + height;
        break;

    case WMSZ_TOP:
    case WMSZ_BOTTOM:
        width = (((height - header) * WIDTH) / HEIGHT) + border;
        rectp->right = rectp->left + width;
        break;

    default:
        width = (((height - header) * WIDTH) / HEIGHT) + border;
        rectp->right = rectp->left + width;
        break;
    }

    return TRUE;
}

// Draw item
BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    UINT state = lpdi->itemState;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    SetGraphicsMode(hdc, GM_ADVANCED);

    switch (wParam)
    {
	// Scope
    case SCOPE_ID:
	return DrawScope(hdc, rect);
	break;

	// Spectrum
    case SPECTRUM_ID:
	return DrawSpectrum(hdc, rect);
	break;

	// Strobe
    case STROBE_ID:
	return DrawStrobe(hdc, rect);
	break;

	// Display
    case DISPLAY_ID:
	return DrawDisplay(hdc, rect);
	break;

	// Meter
    case METER_ID:
	return DrawMeter(hdc, rect);
	break;
    }
}

// Display context menu
BOOL DisplayContextMenu(HWND hWnd, POINTS points)
{
    HMENU menu;
    POINT point;

    // Convert coordinates
    POINTSTOPOINT(point, points);
    ClientToScreen(hWnd, &point);

    // Create menu
    menu = CreatePopupMenu();

    // Add menu items
    AppendMenu(menu, spectrum.zoom? MF_STRING | MF_CHECKED:
	       MF_STRING, ZOOM_ID, L"Zoom spectrum");
    AppendMenu(menu, strobe.enable? MF_STRING | MF_CHECKED:
	       MF_STRING, ENABLE_ID, L"Display strobe");
    AppendMenu(menu, audio.filter? MF_STRING | MF_CHECKED:
	       MF_STRING, FILTER_ID, L"Audio filter");
    AppendMenu(menu, audio.downsample? MF_STRING | MF_CHECKED:
	       MF_STRING, DOWN_ID, L"Downsample");
    AppendMenu(menu, display.lock? MF_STRING | MF_CHECKED:
	       MF_STRING, LOCK_ID, L"Lock display");
    AppendMenu(menu, display.multiple? MF_STRING | MF_CHECKED:
	       MF_STRING, MULTIPLE_ID, L"Multiple notes");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, OPTIONS_ID, L"Options...");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, QUIT_ID, L"Quit");

    // Pop up the menu
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Display options
BOOL DisplayOptions(WPARAM wParam, LPARAM lParam)
{
    // Check if exists
    if (options.hwnd == NULL)
    {
	WNDCLASS wc = 
	    {CS_HREDRAW | CS_VREDRAW, PopupProc,
	     0, 0, hInst,
	     LoadIcon(NULL, IDI_WINLOGO),
	     LoadCursor(NULL, IDC_ARROW),
	     GetSysColorBrush(COLOR_WINDOW),
	     NULL, PCLASS};

	// Register the window class.
	RegisterClass(&wc);

	// Get the main window rect
	RECT rWnd;
	GetWindowRect(window.hwnd, &rWnd);

	// Create the window, offset right
	options.hwnd =
	    CreateWindow(PCLASS, L"Tuner Options",
			 WS_VISIBLE | WS_POPUP |
			 WS_CAPTION,
			 rWnd.right + 10, rWnd.top,
			 WIDTH, 320, window.hwnd,
			 (HMENU)NULL, hInst, NULL);
    }

    // Show existing window
    else
	ShowWindow(options.hwnd, TRUE);
}

// Popup Procedure
LRESULT CALLBACK PopupProc(HWND hWnd,
			   UINT uMsg,
			   WPARAM wParam,
			   LPARAM lParam)
{
    RECT cRect;
    static TCHAR s[64];

    // Get the client rect
    GetClientRect(hWnd, &cRect);
    int width = cRect.right;

    // Switch on message
    switch (uMsg)
    {
    case WM_CREATE:

	// Create group box
	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 2, width - 20, 156,
			 hWnd, NULL, hInst, NULL);

	// Create zoom tickbox
	zoom.hwnd =
	    CreateWindow(WC_BUTTON, L"Zoom spectrum:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 20, 20, 124, 24,
			 hWnd, (HMENU)ZOOM_ID, hInst, NULL);

	SendMessage(zoom.hwnd, BM_SETCHECK,
		    spectrum.zoom? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip
	tooltip.info.uId = (UINT_PTR)zoom.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Zoom spectrum, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create strobe enable tickbox
	enable.hwnd =
	    CreateWindow(WC_BUTTON, L"Display strobe:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 20, 124, 24,
			 hWnd, (HMENU)ENABLE_ID, hInst, NULL);

	SendMessage(enable.hwnd, BM_SETCHECK,
		    strobe.enable? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)enable.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Display strobe, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create filter tickbox
	filter.hwnd =
	    CreateWindow(WC_BUTTON, L"Audio filter:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 20, 54, 124, 24,
			 hWnd, (HMENU)FILTER_ID, hInst, NULL);

	SendMessage(filter.hwnd, BM_SETCHECK,
		    audio.filter? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip
	tooltip.info.uId = (UINT_PTR)filter.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Audio filter, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create downsample tickbox
	down.hwnd =
	    CreateWindow(WC_BUTTON, L"Downsample:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 54, 124, 24,
			 hWnd, (HMENU)DOWN_ID, hInst, NULL);

	SendMessage(down.hwnd, BM_SETCHECK,
		    audio.downsample? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip
	tooltip.info.uId = (UINT_PTR)lock.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Downsample, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create lock tickbox
	lock.hwnd =
	    CreateWindow(WC_BUTTON, L"Lock display:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 88, 124, 24,
			 hWnd, (HMENU)LOCK_ID, hInst, NULL);

	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip
	tooltip.info.uId = (UINT_PTR)lock.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Lock display, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create text
	text.hwnd =
	    CreateWindow(WC_STATIC, L"Ref:",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 126, 32, 20, hWnd,
			 (HMENU)TEXT_ID, hInst, NULL);

	// Create edit control
	wsprintf(s, L"%6.2lf", audio.reference);

	legend.reference.hwnd =
	    CreateWindow(WC_EDIT, s,
			 WS_VISIBLE | WS_CHILD |
			 WS_BORDER,
			 62, 124, 82, 20, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	// Add edit to tooltip
	tooltip.info.uId = (UINT_PTR)legend.reference.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Reference, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create up-down control
	reference.hwnd =
	    CreateWindow(UPDOWN_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 UDS_AUTOBUDDY | UDS_ALIGNRIGHT,
			 0, 0, 0, 0, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	SendMessage(reference.hwnd, UDM_SETRANGE32, MIN_REF, MAX_REF);
	SendMessage(reference.hwnd, UDM_SETPOS32, 0, audio.reference * 10);

	// Add updown to tooltip
	tooltip.info.uId = (UINT_PTR)reference.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Reference, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create multiple tickbox
	multiple.hwnd =
	    CreateWindow(WC_BUTTON, L"Multiple notes:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 122, 124, 24,
			 hWnd, (HMENU)MULTIPLE_ID, hInst, NULL);

	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip
	tooltip.info.uId = (UINT_PTR)multiple.hwnd;
	tooltip.info.lpszText = (LPWSTR)L"Display multiple notes, "
	    L"click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create group box
	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 160, width - 20, 124,
			 hWnd, NULL, hInst, NULL);

	// Create save button
	button.save.hwnd =
	    CreateWindow(WC_BUTTON, L"Save",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 177, 85, 26,
			 hWnd, (HMENU)SAVE_ID, hInst, NULL);

	// Create text
	text.hwnd =
	    CreateWindow(WC_STATIC,
			 L"Use correction if your sound card clock "
			 L"is significantly inaccurate.",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 210, width - 40, 40,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create text
	wsprintf(s, L"Sample rate:  %6.1lf",
		(double)SAMPLE_RATE);

	legend.sample.hwnd =
	    CreateWindow(WC_STATIC, s,
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 252, 152, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create close button

	button.close.hwnd =
	    CreateWindow(WC_BUTTON, L"Close",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 247, 85, 26,
			 hWnd, (HMENU)CLOSE_ID, hInst, NULL);

	break;

	// Colour static text
    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	break;

	// Draw item
    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
	break;

	// Updown control
    case WM_VSCROLL:
	switch (GetWindowLongPtr((HWND)lParam, GWLP_ID))
	{
	case REFERENCE_ID:
	    ChangeReference(wParam, lParam);
	    break;
	}

	// Set the focus back to the window
	SetFocus(hWnd);
	break;

	// Set the focus back to the window by clicking
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

	// Display options menu
    case WM_RBUTTONDOWN:
	DisplayOptionsMenu(hWnd, MAKEPOINTS(lParam));
	break;

	// Commands
    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	    // Zoom control
	case ZOOM_ID:
	    ZoomClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Enable control
	case ENABLE_ID:
	    EnableClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Filter control
	case FILTER_ID:
	    FilterClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Downsample
	case DOWN_ID:
	    DownClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Lock control
	case LOCK_ID:
	    LockClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Multiple control
	case MULTIPLE_ID:
	    MultipleClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

	    // Reference
	case REFERENCE_ID:
	    EditReference(wParam, lParam);
	    break;

	    // Close
	case CLOSE_ID:
	    ShowWindow(hWnd, FALSE);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;
	}
	break;

	// Char pressed
    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

	// Everything else
    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Zoom clicked
BOOL ZoomClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	SpectrumClicked(wParam, lParam);
    }
}

// Enable clicked

BOOL EnableClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	StrobeClicked(wParam, lParam);
    }
}

// Filter clicked
BOOL FilterClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.filter = !audio.filter;
	break;

    default:
	return FALSE;
    }

    if (filter.hwnd != NULL)
	SendMessage(filter.hwnd, BM_SETCHECK,
		    audio.filter? BST_CHECKED: BST_UNCHECKED, 0);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, L"Filter", 0, REG_DWORD,
		      (LPBYTE)&audio.filter, sizeof(audio.filter));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);

	return FALSE;
    }

    return TRUE;
}

// Colour clicked
BOOL ColourClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (strobe.enable)
	{
	    strobe.colours++;

	    if (strobe.colours > MAGENTA)
		strobe.colours = BLUE;

	    strobe.changed = TRUE;
	}

	break;

    default:
	return FALSE;
    }

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, L"Colours", 0, REG_DWORD,
		      (LPBYTE)&strobe.colours, sizeof(strobe.colours));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);

	return FALSE;
    }

    return TRUE;
}

// Down clicked
BOOL DownClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.downsample = !audio.downsample;
	break;

    default:
	return FALSE;
    }

    if (down.hwnd != NULL)
	SendMessage(down.hwnd, BM_SETCHECK,
		    audio.downsample? BST_CHECKED: BST_UNCHECKED, 0);
    return TRUE;
}

// Expand clicked
BOOL ExpandClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;

	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Contract clicked
BOOL ContractClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;

	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Lock clicked
BOOL LockClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.lock = !display.lock;
	break;

    default:
	return FALSE;
    }

    InvalidateRgn(display.hwnd, NULL, TRUE);

    if (lock.hwnd != NULL)
	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);
    return TRUE;
}

// Multiple clicked
BOOL MultipleClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.multiple = !display.multiple;
	break;

    default:
	return FALSE;
    }

    if (multiple.hwnd != NULL)
	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    return TRUE;
}

// Display options menu
BOOL DisplayOptionsMenu(HWND hWnd, POINTS points)
{
    HMENU menu;
    POINT point;

    POINTSTOPOINT(point, points);
    ClientToScreen(hWnd, &point);

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, CLOSE_ID, L"Close");

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Char pressed
BOOL CharPressed(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
	// Copy display
    case 'C':
    case 'c':
    case 0x3:
	CopyDisplay(wParam, lParam);
	break;

	// Downsample

    case 'D':
    case 'd':
	DownClicked(wParam, lParam);
	break;

	// Filter
    case 'F':
    case 'f':
	FilterClicked(wParam, lParam);
	break;

	// Colour
    case 'K':
    case 'k':
	ColourClicked(wParam, lParam);
	break;

	// Lock
    case 'L':
    case 'l':
	LockClicked(wParam, lParam);
	break;

	// Options
    case 'O':
    case 'o':
	DisplayOptions(wParam, lParam);
	break;

	// Strobe
    case 'S':
    case 's':
	EnableClicked(wParam, lParam);
	break;

	// Multiple
    case 'M':
    case 'm':
	MultipleClicked(wParam, lParam);
	break;

	// Zoom
    case 'Z':
    case 'z':
	ZoomClicked(wParam, lParam);
	break;

	// Expand
    case '+':
	ExpandClicked(wParam, lParam);
	break;

	// Contract
    case '-':
	ContractClicked(wParam, lParam);
	break;
    }
}

// Copy display
BOOL CopyDisplay(WPARAM wParam, LPARAM lParam)
{
    // Memory size
    enum
    {MEM_SIZE = 1024};

    static TCHAR s[64];

    static const TCHAR *notes[] =
	{L"C", L"C#", L"D", L"Eb", L"E", L"F",
	 L"F#", L"G", L"Ab", L"A", L"Bb", L"B"};

    // Open clipboard
    if (!OpenClipboard(window.hwnd))
	return FALSE;

    // Empty clipboard
    EmptyClipboard(); 

    // Allocate memory
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, MEM_SIZE);

    if (mem == NULL)
    {
	CloseClipboard();
	return FALSE;
    }

    // Lock the memory
    LPTSTR text = (LPTSTR)GlobalLock(mem);

    // Check if multiple
    if (display.multiple && display.count > 0)
    {
	// For each set of values
	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    // Print the text
	    wsprintf(s, L"%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		    notes[n % Length(notes)], n / 12,
		    c * 100.0, fr, f, f - fr);

	    // Copy to the memory
	    if (i == 0)
		lstrcpy(text, s);

	    else
		lstrcat(text, s);
	}
    }

    else
    {
	// Print the values
	wsprintf(s, L"%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		notes[display.n % Length(notes)], display.n / 12,
		display.c * 100.0, display.fr, display.f,
		display.f - display.fr);

	// Copy to the memory
	lstrcpy(text, s);
    }

    // Place in clipboard
    GlobalUnlock(text);
    SetClipboardData(CF_TEXT, mem);
    CloseClipboard(); 
 
    return TRUE;
}

// Meter callback
VOID CALLBACK MeterCallback(PVOID lpParam, BOOL TimerFired)
{
    METERP meter = (METERP)lpParam;

    static float mc;

    // Do calculation
    mc = ((mc * 19.0) + meter->c) / 20.0;

    int value = round(mc * MAX_METER) + REF_METER;

    // Update meter
    // SendMessage(meter->slider.hwnd, TBM_SETPOS, TRUE, value);
}

// Strobe callback
VOID CALLBACK StrobeCallback(PVOID lpParam, BOOL TimerFired)
{
    STROBEP strobe = (STROBEP)lpParam;

    // Update strobe
    if (strobe->enable)
    	InvalidateRgn(strobe->hwnd, NULL, TRUE);
}

// Draw scope
BOOL DrawScope(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::Graphics;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 L""};

    // Create font
    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, GetStockObject(DC_PEN));

	// Select font
	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    // SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct =
        {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Dark green graticule
    SetDCPenColor(hbdc, RGB(0, 64, 0));

    // Draw graticule
    for (int i = 4; i < width; i += 5)
    {
        MoveToEx(hbdc, i, 0, NULL);
        LineTo(hbdc, i, height);
    }

    for (int i = 4; i < height; i += 5)
    {
        MoveToEx(hbdc, 0, i, NULL);
        LineTo(hbdc, width, i);
    }

    // Don't attempt the trace until there's a buffer
    if (scope.data == NULL)
    {
	// Copy the bitmap
	BitBlt(hdc, rect.left, rect.top, width, height,
	       hbdc, 0, 0, SRCCOPY);

	return TRUE;
    }

    // Initialise sync
    int maxdx = 0;
    int dx = 0;
    int n = 0;

    for (int i = 1; i < width; i++)
    {
	dx = scope.data[i] - scope.data[i - 1];
	if (maxdx < dx)
	{
	    maxdx = dx;
	    n = i;
	}

	if (maxdx > 0 && dx < 0)
	    break;
    }

    static float max;

    if (max < 4096)
	max = 4096;

    float yscale = max / (height / 2);

    max = 0.0;
    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    // Move the origin
    graphics.TranslateTransform(0.0, height / 2.0);
    // Green pen for scope trace
    Pen pen(Color(0, 255, 0), 1);

    // Draw the trace
    PointF last(-1.0, 0.0);
    for (int i = 0; i < width && i < scope.length; i++)
    {
	if (max < abs(scope.data[n + i]))
	    max = abs(scope.data[n + i]);

	float y = -scope.data[n + i] / yscale;
        PointF point(i, -scope.data[n + i] / yscale);
        graphics.DrawLine(&pen, last, point);

        last = point;
    }

    // Move the origin back
    // SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Show F if filtered
    if (audio.filter)
    {
	// Yellow text
	SetTextColor(hbdc, RGB(255, 255, 0));
	TextOut(hbdc, 0, height + 1, L"F", 1);
    }

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw spectrum
BOOL DrawSpectrum(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush;
    using Gdiplus::Graphics;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 L""};

    static TCHAR s[16];

    // Create font
    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge
    DrawEdge(hdc, &rect, EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, GetStockObject(DC_PEN));

	// Select font
	SelectObject(hbdc, font);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Dark green graticule
    SetDCPenColor(hbdc, RGB(0, 64, 0));

    // Draw graticule
    for (int i = 4; i < width; i += 5)
    {
	MoveToEx(hbdc, i, 0, NULL);
	LineTo(hbdc, i, height);
    }

    for (int i = 4; i < height; i += 5)
    {
	MoveToEx(hbdc, 0, i, NULL);
	LineTo(hbdc, width, i);
    }

    // Don't attempt the trace until there's a buffer
    if (spectrum.data == NULL)
    {
	// Copy the bitmap
	BitBlt(hdc, rect.left, rect.top, width, height,
	       hbdc, 0, 0, SRCCOPY);

	return TRUE;
    }

    // Move the origin
    // SetViewportOrgEx(hbdc, 0, height - 1, NULL);

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling
    float yscale = (float)height / max;

    max = 0.0;

    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.TranslateTransform(0.0, height - 1.0);
    // Green pen for spectrum trace
    Pen pen(Color(0, 255, 0), 1);
    // Transparent green brush for spectrum fill
    SolidBrush brush(Color(63, 0, 255, 0));

    // Draw the spectrum
    PointF lastp(0.0, 0.0);
    GraphicsPath path;

    if (spectrum.zoom)
    {
	// Calculate scale
	float xscale = ((float)width / (spectrum.r - spectrum.l)) / 2.0;

	for (int i = floor(spectrum.l); i <= ceil(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		float value = spectrum.data[i];

		if (max < value)
		    max = value;

                PointF point((i - spectrum.l) * xscale, -value * yscale);
                path.AddLine(lastp, point);
                lastp = point;
	    }
	}

        graphics.DrawPath(&pen, &path);
        path.AddLine(lastp, PointF(width, 0.0));
        path.CloseFigure();
        graphics.FillPath(&brush, &path);

        SetViewportOrgEx(hbdc, 0, height - 1, NULL);
	SetDCPenColor(hbdc, RGB(0, 255, 0));
	MoveToEx(hbdc, width / 2, 0, NULL);
	LineTo(hbdc, width / 2, -height);

	// Yellow pen for frequency trace
	SetDCPenColor(hbdc, RGB(255, 255, 0));
	SetTextColor(hbdc, RGB(255, 255, 0));
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);

	// Draw lines for each frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each that are in range
	    if (spectrum.values[i].f > spectrum.l &&
		spectrum.values[i].f < spectrum.h)
	    {
		int x = round((spectrum.values[i].f - spectrum.l) * xscale);
		MoveToEx(hbdc, x, 0, NULL);
		LineTo(hbdc, x, -height);

		double f = display.maxima[i].f;

		// Reference freq
		double fr = display.maxima[i].fr;
		double c = -12.0 * log2(fr / f);

		// Ignore silly values
		if (!isfinite(c))
		    continue;

		swprintf(s, sizeof(s), L"%+0.0f", c * 100.0);
		TextOut(hbdc, x, 2, s, lstrlen(s));
	    }
	}
    }

    else
    {
	float xscale = log((float)spectrum.length /
			   (float)spectrum.expand) / width;
	int last = 1;
	for (int x = 0; x < width; x++)
	{
	    float value = 0.0;

	    int index = (int)round(pow(M_E, x * xscale));
	    for (int i = last; i <= index; i++)
	    {
		// Don't show DC component
		if (i > 0 && i < spectrum.length)
		{
		    if (value < spectrum.data[i])
			value = spectrum.data[i];
		}
	    }

	    // Update last index
	    last = index + 1;

	    if (max < value)
		max = value;

	    PointF point(x, -value * yscale);
            path.AddLine(lastp, point);

            lastp = point;
	}

        graphics.DrawPath(&pen, &path);
        path.AddLine(lastp, PointF(width, 0.0));
        path.CloseFigure();
        graphics.FillPath(&brush, &path);

	// Yellow pen for frequency trace
        SetViewportOrgEx(hbdc, 0, height - 1, NULL);
	SetDCPenColor(hbdc, RGB(255, 255, 0));
	SetTextColor(hbdc, RGB(255, 255, 0));
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);

	// Draw lines for each frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each
	    int x = round(log(spectrum.values[i].f) / xscale);
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, -height);

	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;
	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    swprintf(s, sizeof(s),  L"%+0.0f", c * 100.0);
	    TextOut(hbdc, x, 2, s, lstrlen(s));
	}

	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);

	if (spectrum.expand > 1)
	{
	    wsprintf(s, L"x%d", spectrum.expand);
	    TextOut(hbdc, 0, 2, s, lstrlen(s));
	}
    }

    // D for downsample
    if (audio.downsample)
    {
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	TextOut(hbdc, 0, 10 - height, L"D", 1);
    }

    // Move the origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw display
BOOL DrawDisplay(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT medium;
    static HFONT larger;
    static HFONT large;
    static HFONT decor;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    static const TCHAR *notes[] =
	{L"C", L"C", L"D", L"E", L"E", L"F",
	 L"F", L"G", L"A", L"A", L"B", L"B"};

    static const TCHAR *sharps[] =
	{L"", L"\u266F", L"", L"\u266D", L"", L"",
	 L"\u266F", L"", L"\u266D", L"", L"\u266D", L""};

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 L"DejaVu Sans"};

    static TCHAR s[64];

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create new fonts if resized
    if (width != size.cx || height != size.cy)
    {
        if (font != NULL)
        {
            DeleteObject(font);
            DeleteObject(large);
            DeleteObject(larger);
            DeleteObject(medium);
            DeleteObject(decor);
        }

	lf.lfHeight = height / 3;
        lf.lfWeight = FW_BOLD;
	large = CreateFontIndirect(&lf);

	lf.lfHeight = height / 2;
	larger = CreateFontIndirect(&lf);

	lf.lfHeight = height / 5;
	medium = CreateFontIndirect(&lf);

	lf.lfHeight = height / 4;
	decor = CreateFontIndirect(&lf);

        lf.lfHeight = height / 8;
        lf.lfWeight = FW_NORMAL;
	font = CreateFontIndirect(&lf);
    }

    // Create DC
    if (hbdc == NULL)
	hbdc = CreateCompatibleDC(hdc);

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    if (display.multiple)
    {
	// Select font
	SelectObject(hbdc, font);
        int font_height = height / 9;

	// Set text align
	SetTextAlign(hbdc, TA_TOP);

	if (display.count == 0)
	{
            int x = 4;

	    // Display note
	    swprintf(s, sizeof(s), L"%s%s%d", notes[display.n % Length(notes)],
		    sharps[display.n % Length(notes)], display.n / 12);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display cents
	    swprintf(s, sizeof(s), L"%+4.2lf¢", display.c * 100.0);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display reference
	    swprintf(s, sizeof(s), L"%4.2lfHz", display.fr);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display frequency
	    swprintf(s, sizeof(s), L"%4.2lfHz", display.f);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display difference
	    swprintf(s, sizeof(s), L"%+4.2lfHz", display.f - display.fr);
	    TextOut(hbdc, x, 0, s, lstrlen(s));
	}

        int y = 0;

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

            int x = 4;

	    // Display note
	    swprintf(s, sizeof(s), L"%S%S%d", notes[n % Length(notes)],
		    sharps[n % Length(notes)], n / 12);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display cents
	    swprintf(s, sizeof(s), L"%+4.2lf¢", c * 100.0);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display reference
	    swprintf(s, sizeof(s), L"%4.2lfHz", fr);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display frequency
	    swprintf(s, sizeof(s), L"%4.2lfHz", f);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display difference
	    swprintf(s, sizeof(s), L"%+4.2lfHz", f - fr);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            y += size.cy;
        }
    }

    else
    {
	// Select larger font
	SelectObject(hbdc, larger);

	// Text size
	SIZE size = {0};

	// Set text align
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);
	SetBkMode(hbdc, TRANSPARENT);

	// Display note
	swprintf(s, sizeof(s), L"%S", notes[display.n % Length(notes)]);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);

        int y = size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	int x = size.cx + 8;

	// Select decor font
	SelectObject(hbdc, decor);

	swprintf(s, sizeof(s), L"%d", display.n / 12);
	TextOut(hbdc, x, y, s, lstrlen(s));

	// Select decor font
	SelectObject(hbdc, decor);

	swprintf(s, sizeof(s), L"%S", sharps[display.n % Length(sharps)]);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	TextOut(hbdc, x, y - size.cy, s, lstrlen(s));

	// Select large font
	SelectObject(hbdc, large);
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);

	// Display cents
	swprintf(s, sizeof(s), L"%+4.2f¢", display.c * 100.0);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));

	// Select medium font
	SelectObject(hbdc, medium);
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);

	// Display reference frequency
	swprintf(s, sizeof(s), L"%4.2fHz", display.fr);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	y += size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	// Display actual frequency
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);
	swprintf(s, sizeof(s), L"%4.2fHz", display.f);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));


	// Display reference
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);
	swprintf(s, sizeof(s), L"%4.2fHz", audio.reference);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	y += size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	// Display frequency difference
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);
	swprintf(s, sizeof(s), L"%+4.2lfHz", display.f - display.fr);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));
    }

    // Show lock
    if (display.lock)
	DrawLock(hbdc, -1, height + 1);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw lock
BOOL DrawLock(HDC hdc, int x, int y)
{
    POINT point;
    POINT body[] =
	{{2, -3}, {8, -3}, {8, -8}, {2, -8}, {2, -3}};

    SetViewportOrgEx(hdc, x, y, &point);

    Polyline(hdc, body, Length(body));

    MoveToEx(hdc, 3, -8, NULL);
    LineTo(hdc, 3, -11);

    MoveToEx(hdc, 7, -8, NULL);
    LineTo(hdc, 7, -11);

    MoveToEx(hdc, 4, -11, NULL);
    LineTo(hdc, 7, -11);

    SetPixel(hdc, 3, -11, RGB(255, 170, 85));
    SetPixel(hdc, 6, -10, RGB(255, 170, 85));

    SetPixel(hdc, 4, -10, RGB(85, 170, 255));
    SetPixel(hdc, 7, -11, RGB(85, 170, 255));

    SetPixel(hdc, 7, -7, RGB(255, 170, 85));
    SetPixel(hdc, 7, -4, RGB(255, 170, 85));

    SetPixel(hdc, 3, -7, RGB(85, 170, 255));
    SetPixel(hdc, 3, -4, RGB(85, 170, 255));

    SetViewportOrgEx(hdc, point.x, point.y, NULL);
    return TRUE;
}

// Draw meter
BOOL DrawMeter(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    // Plain vanilla font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 L""};

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	// Create DC
	hbdc = CreateCompatibleDC(hdc);
	SetTextAlign(hbdc, TA_CENTER);
    }

    // Create new font if resized
    if (width != size.cx || height != size.cy)
    {
        if (font != NULL)
            DeleteObject(font);

        lf.lfHeight = height / 3;
	font = CreateFontIndirect(&lf);
	SelectObject(hbdc, font);
    }

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Move origin
    SetViewportOrgEx(hbdc, width / 2, 0, NULL);

    // Draw the meter scale
    for (int i = 0; i < 6; i++)
    {
	int x = width / 11 * i;
	static TCHAR s[16];

	swprintf(s, sizeof(s), L"%d", i * 10);
	TextOut(hbdc, x + 1, 0, s, lstrlen(s));
	TextOut(hbdc, -x + 1, 0, s, lstrlen(s));

	MoveToEx(hbdc, x, height / 3, NULL);
	LineTo(hbdc, x, height / 2);
	MoveToEx(hbdc, -x, height / 3, NULL);
	LineTo(hbdc, -x, height / 2);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		MoveToEx(hbdc, x + j * width / 55, height * 3 / 8, NULL);
		LineTo(hbdc, x + j * width / 55, height / 2);
	    }

	    MoveToEx(hbdc, -x + j * width / 55, height * 3 / 8, NULL);
	    LineTo(hbdc, -x + j * width / 55, height / 2);
	}
    }

    // Move origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw strobe
BOOL DrawStrobe(HDC hdc, RECT rect)
{
    static float mc = 0.0;
    static float mx = 0.0;

    static HBRUSH sbrush;
    static HBRUSH sshade;
    static HBRUSH mbrush;
    static HBRUSH mshade;
    static HBRUSH lbrush;
    static HBRUSH lshade;
    static HBRUSH ebrush;

    static SIZE size;

    // Colours
    static int colours[][2] =
	{{RGB(63, 63, 255), RGB(63, 255, 255)},
	 {RGB(111, 111, 0), RGB(191, 255, 191)},
	 {RGB(255, 63, 255), RGB(255, 255, 63)}};

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int block = height / 4;

    // Create brushes
    int foreground = colours[strobe.colours][0];
    int background = colours[strobe.colours][1];

    if (sbrush == NULL || strobe.changed ||
        size.cx != width || size.cy != height)
    {
	if (sbrush != NULL)
	    DeleteObject(sbrush);

	if (sshade != NULL)
	    DeleteObject(sshade);

	if (mbrush != NULL)
	    DeleteObject(mbrush);

	if (mshade != NULL)
	    DeleteObject(mshade);

	if (lbrush != NULL)
	    DeleteObject(lbrush);

	if (lshade != NULL)
	    DeleteObject(lshade);

	if (ebrush != NULL)
	    DeleteObject(ebrush);

	HDC hbdc = CreateCompatibleDC(hdc);

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 2, block));
	SelectObject(hbdc, GetStockObject(DC_PEN));
	SelectObject(hbdc, GetStockObject(DC_BRUSH));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block, 0, block * 2, block);

	sbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 2, block));

	TRIVERTEX vertex[] =
	    {{0, 0,
	      (COLOR16)(GetRValue(foreground) << 8),
	      (COLOR16)(GetGValue(foreground) << 8),
	      (COLOR16)(GetBValue(foreground) << 8),
	      0},
	     {block, block,
	      (COLOR16)(GetRValue(background) << 8),
	      (COLOR16)(GetGValue(background) << 8),
	      (COLOR16)(GetBValue(background) << 8),
	      0},
	     {block * 2, 0,
	      (COLOR16)(GetRValue(foreground) << 8),
	      (COLOR16)(GetGValue(foreground) << 8),
	      (COLOR16)(GetBValue(foreground) << 8),
	      0}};

	GRADIENT_RECT gradient[] =
	    {{0, 1}, {1, 2}};

	GradientFill(hbdc, vertex, Length(vertex),
		     gradient, Length(gradient), GRADIENT_FILL_RECT_H);

	sshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 4, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 2, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 2, 0, block * 4, block);

	mbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 4, block));

	vertex[1].x = block * 2;
	vertex[2].x = block * 4;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	mshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 8, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 4, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 4, 0, block * 8, block);

	lbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 8, block));

	vertex[1].x = block * 4;
	vertex[2].x = block * 8;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	lshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 16, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 8, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 8, 0, block * 16, block);

	ebrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	DeleteDC(hbdc);
	strobe.changed = FALSE;
    }

    if (strobe.enable)
    {
        // Transform viewport
        SetViewportOrgEx(hdc, rect.left, rect.top, NULL);

    	mc = ((19.0 * mc) + strobe.c) / 20.0;
    	mx += mc * 50.0;

    	if (mx > block * 16.0)
    	    mx = 0.0;

    	if (mx < 0.0)
    	    mx = block * 16.0;

    	int rx = round(mx - block * 16.0);
	SetBrushOrgEx(hdc, rx, 0, NULL);
	SelectObject(hdc, GetStockObject(NULL_PEN));

	if (fabs(mc) > 0.4)
	{
	    SelectObject(hdc, GetStockObject(DC_BRUSH));
	    SetDCBrushColor(hdc, background);
	}

	else if (fabs(mc) > 0.2)
	    SelectObject(hdc, sshade);

	else
	    SelectObject(hdc, sbrush);
	Rectangle(hdc, 0, 0, width, block);

	if (fabs(mc) > 0.3)
	    SelectObject(hdc, mshade);

	else
	    SelectObject(hdc, mbrush);
	Rectangle(hdc, 0, block, width, block * 2);

	if (fabs(mc) > 0.4)
	    SelectObject(hdc, lshade);

	else
	    SelectObject(hdc, lbrush);
	Rectangle(hdc, 0, block * 2, width, block * 3);

	SelectObject(hdc, ebrush);
	Rectangle(hdc, 0, block * 3, width, block * 4);
    }

    else
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    return TRUE;
}

// Display clicked
BOOL DisplayClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.lock = !display.lock;
	break;

    default:
	return FALSE;
    }

    if (lock.hwnd != NULL)
	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    return TRUE;
}

// Scope clicked
BOOL ScopeClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	FilterClicked(wParam, lParam);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Spectrum clicked
BOOL SpectrumClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	spectrum.zoom = !spectrum.zoom;
	break;

    default:
	return FALSE;
    }

    if (zoom.hwnd != NULL)
	SendMessage(zoom.hwnd, BM_SETCHECK,
		    spectrum.zoom? BST_CHECKED: BST_UNCHECKED, 0);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, L"Zoom", 0, REG_DWORD,
		      (LPBYTE)&spectrum.zoom, sizeof(spectrum.zoom));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Strobe clicked
BOOL StrobeClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	strobe.enable = !strobe.enable;
	break;

    default:
	return FALSE;
    }

    InvalidateRgn(strobe.hwnd, NULL, TRUE);

    if (enable.hwnd != NULL)
	SendMessage(enable.hwnd, BM_SETCHECK,
		    strobe.enable? BST_CHECKED: BST_UNCHECKED, 0);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, L"Strobe", 0, REG_DWORD,
		      (LPBYTE)&strobe.enable, sizeof(strobe.enable));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Meter clicked
BOOL MeterClicked(WPARAM wParam, LPARAM lParam)
{
    return DisplayClicked(wParam, lParam);
}

// Edit reference
BOOL EditReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    if (audio.reference == 0)
	return FALSE;

    switch (HIWORD(wParam))
    {
    case EN_KILLFOCUS:
	GetWindowText(legend.reference.hwnd, s, sizeof(s));
	audio.reference = wcstof(s, NULL);

	SendMessage(reference.hwnd, UDM_SETPOS32, 0,
		    audio.reference * 10);
	break;

    default:
	return FALSE;
    }

    InvalidateRgn(display.hwnd, NULL, TRUE);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	int value = audio.reference * 10;

	RegSetValueEx(hkey, L"Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Change reference
BOOL ChangeReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    long value = SendMessage(reference.hwnd, UDM_GETPOS32, 0, 0);
    audio.reference = (double)value / 10.0;

    swprintf(s, sizeof(s), L"%6.2lf", audio.reference);
    SetWindowText(legend.reference.hwnd, s);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, L"Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, L"RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Tooltip show
VOID TooltipShow(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    switch (GetDlgCtrlID((HWND)pnmh->idFrom))
    {
    case VOLUME_ID:
	SetWindowText(status.hwnd, L" Microphone volume");
	break;

    case SCOPE_ID:
	SetWindowText(status.hwnd, L" Scope, click to filter audio");
	break;

    case SPECTRUM_ID:
	SetWindowText(status.hwnd, L" Spectrum, click to zoom");
	break;

    case DISPLAY_ID:
	SetWindowText(status.hwnd, L" Display, click to lock");
	break;

    case STROBE_ID:
	SetWindowText(status.hwnd, L" Strobe, click to disable/enable");
	break;

    case METER_ID:
    case SLIDER_ID:
	SetWindowText(status.hwnd, L" Cents, click to lock");
	break;
    }
}

// Audio thread
DWORD WINAPI AudioThread(LPVOID lpParameter)
{
    // Create wave format structure
    static WAVEFORMATEX wf =
	{WAVE_FORMAT_PCM, CHANNELS,
	 SAMPLE_RATE, SAMPLE_RATE * BLOCK_ALIGN,
	 BLOCK_ALIGN, BITS_PER_SAMPLE, 0};

    MMRESULT mmr;

    // Open a waveform audio input device
    mmr = waveInOpen(&audio.hwi, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id,  (DWORD_PTR)NULL, CALLBACK_THREAD);

    if (mmr != MMSYSERR_NOERROR)
    {
	static TCHAR s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, L"WaveInOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Create the waveform audio input buffers and structures
    static short data[4][STEP];
    static WAVEHDR hdrs[4] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0},
	 {(LPSTR)data[2], sizeof(data[2]), 0, 0, 0, 0},
	 {(LPSTR)data[3], sizeof(data[3]), 0, 0, 0, 0}};

    for (int i = 0; i < Length(hdrs); i++)
    {
	// Prepare a waveform audio input header
	mmr = waveInPrepareHeader(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static TCHAR s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, L"WaveInPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}

	// Add a waveform audio input buffer
	mmr = waveInAddBuffer(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static TCHAR s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, L"WaveInAddBuffer",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Start the waveform audio input
    mmr = waveInStart(audio.hwi);
    if (mmr != MMSYSERR_NOERROR)
    {
	static TCHAR s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, L"WaveInStart", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Set up reference value
    if (audio.reference == 0)
	audio.reference = A5_REFNCE;

    // Create a message loop for processing thread messages
    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

	// Process messages
	switch (msg.message)
	{
	    // Audio input opened
	case MM_WIM_OPEN:
	    // Not used
	    break;

	    // Audio input data
	case MM_WIM_DATA:
	    WaveInData(msg.wParam, msg.lParam);
	    break;

	    // Audio input closed
	case MM_WIM_CLOSE:
	    // Not used
	    break;
	}
    }

    return msg.wParam;
}

VOID WaveInData(WPARAM wParam, LPARAM lParam)
{
    enum
    {TIMER_COUNT = 16};

    // Create buffers for processing the audio data
    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double x2[RANGE / 2];
    static double x3[RANGE / 3];
    static double x4[RANGE / 4];
    static double x5[RANGE / 5];

    static double dx[RANGE];

    static maximum maxima[MAXIMA];
    static value   values[MAXIMA];

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

    // initialise data structs
    if (scope.data == NULL)
    {
	scope.length = STEP;

	spectrum.data = xa;
	spectrum.length = RANGE;

	spectrum.values = values;
	display.maxima = maxima;
    }

    // Copy the input data
    memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(double));

    short *data = (short *)((WAVEHDR *)lParam)->lpData;

    // Butterworth filter, 3dB/octave
    for (int i = 0; i < STEP; i++)
    {
	static double G = 3.023332184e+01;
	static double K = 0.9338478249;

	static double xv[2];
	static double yv[2];

	xv[0] = xv[1];
	xv[1] = (double)data[i] / G;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	// Choose filtered/unfiltered data
	buffer[(SAMPLES - STEP) + i] =
	    audio.filter? yv[1]: (double)data[i];
    }

    // Give the buffer back
    waveInAddBuffer(audio.hwi, (WAVEHDR *)lParam, sizeof(WAVEHDR));

    // Maximum data value
    static double dmax;

    if (dmax < 4096.0)
	dmax = 4096.0;

    // Calculate normalising value
    double norm = dmax;
    dmax = 0.0;

    // Copy data to FFT input arrays for tuner
    for (int i = 0; i < SAMPLES; i++)
    {
    	// Find the magnitude
    	if (dmax < fabs(buffer[i]))
    	    dmax = fabs(buffer[i]);

	// Calculate the window
	double window =
	    0.5 - 0.5 * cos(2.0 * M_PI *
			    i / SAMPLES);

	// Normalise and window the input data
	x[i].r = (double)buffer[i] / norm * window;
    }

    // do FFT for tuner
    fftr(x, SAMPLES);

    // Process FFT output for tuner
    for (int i = 1; i < RANGE; i++)
    {
	double real = x[i].r;
	double imag = x[i].i;

	xa[i] = hypot(real, imag);

#ifdef NOISE

	// Do noise cancellation
	xm[i] = (xa[i] + (xm[i] * 19.0)) / 20.0;

	if (xm[i] > xa[i])
	    xm[i] = xa[i];

	xd[i] = xa[i] - xm[i];

#endif

	// Do frequency calculation
	double p = atan2(imag, real);
	double dp = xp[i] - p;
	xp[i] = p;

	// Calculate phase difference
	dp -= (double)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (double)qpd;

	// Calculate frequency difference
	double df = OVERSAMPLE * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference and correction value
	xf[i] = (i * fps + df * fps);

	// Calculate differences for finding maxima
	dx[i] = xa[i] - xa[i - 1];
    }

    // Downsample
    if (audio.downsample)
    {
	// x2 = xa << 2
	for (int i = 0; i < Length(x2); i++)
	{
	    x2[i] = 0.0;

	    for (int j = 0; j < 2; j++)
		x2[i] += xa[(i * 2) + j] / 2.0;
	}

	// x3 = xa << 3
	for (int i = 0; i < Length(x3); i++)
	{
	    x3[i] = 0.0;

	    for (int j = 0; j < 3; j++)
		x3[i] += xa[(i * 3) + j] / 3.0;
	}

	// x4 = xa << 4
	for (int i = 0; i < Length(x4); i++)
	{
	    x4[i] = 0.0;

	    for (int j = 0; j < 4; j++)
		x2[i] += xa[(i * 4) + j] / 4.0;
	}

	// x5 = xa << 5
	for (int i = 0; i < Length(x5); i++)
	{
	    x5[i] = 0.0;

	    for (int j = 0; j < 5; j++)
		x5[i] += xa[(i * 5) + j] / 5.0;
	}

	// Add downsamples
	for (int i = 1; i < Length(xa); i++)
	{
	    if (i < Length(x2))
		xa[i] += x2[i];

	    if (i < Length(x3))
		xa[i] += x3[i];

	    if (i < Length(x4))
		xa[i] += x4[i];

	    if (i < Length(x5))
		xa[i] += x5[i];

	    // Recalculate differences
	    dx[i] = xa[i] - xa[i - 1];
	}
    }

    // Maximum FFT output
    double max = 0.0;
    double f = 0.0;

    int count = 0;
    int limit = RANGE - 1;

    // Find maximum value, and list of maxima
    for (int i = 1; i < limit; i++)
    {
	if (xa[i] > max)
	{
	    max = xa[i];
	    f = xf[i];
	}

	// If display not locked, find maxima and add to list
	if (!display.lock && count < Length(maxima) &&
	    xa[i] > MIN && xa[i] > (max / 4.0) &&
	    dx[i] > 0.0 && dx[i + 1] < 0.0)
	{
	    maxima[count].f = xf[i];

	    // Cents relative to reference
	    double cf =
	    	-12.0 * log2(audio.reference / xf[i]);

	    // Reference note
	    maxima[count].fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    // Note number
	    maxima[count].n = round(cf) + C5_OFFSET;

	    // Set limit to octave above
	    if (!audio.downsample && (limit > i * 2))
		limit = i * 2 - 1;

	    count++;
	}
    }

    // Reference note frequency and lower and upper limits
    double fr = 0.0;
    double fl = 0.0;
    double fh = 0.0;

    // Note number
    int n = 0;

    // Found flag and cents value
    BOOL found = FALSE;
    double c = 0.0;

    // Do the note and cents calculations
    if (max > MIN)
    {
	found = TRUE;

	// Frequency
	if (!audio.downsample)
	    f = maxima[0].f;

	// Cents relative to reference
	double cf =
	    -12.0 * log2(audio.reference / f);

	// Reference note
	fr = audio.reference * pow(2.0, round(cf) / 12.0);

	// Lower and upper freq
	fl = audio.reference * pow(2.0, (round(cf) - 0.55) / 12.0);
	fh = audio.reference * pow(2.0, (round(cf) + 0.55) / 12.0);

	// Note number
	n = round(cf) + C5_OFFSET;

	if (n < 0)
	    found = FALSE;

	// Find nearest maximum to reference note
	double df = 1000.0;

	for (int i = 0; i < count; i++)
	{
	    if (fabs(maxima[i].f - fr) < df)
	    {
		df = fabs(maxima[i].f - fr);
		f = maxima[i].f;
	    }
	}

	// Cents relative to reference note
	c = -12.0 * log2(fr / f);

	// Ignore silly values
	if (!isfinite(c))
	    c = 0.0;

	// Ignore if not within 50 cents of reference note
	if (fabs(c) > 0.5)
	    found = FALSE;
    }

    // If display not locked
    if (!display.lock)
    {
	// Update scope window
	scope.data = data;
	InvalidateRgn(scope.hwnd, NULL, TRUE);

	// Update spectrum window
	for (int i = 0; i < count; i++)
	    values[i].f = maxima[i].f / fps;

	spectrum.count = count;

	if (found)
	{
	    spectrum.f = f  / fps;
	    spectrum.r = fr / fps;
	    spectrum.l = fl / fps;
	    spectrum.h = fh / fps;
	}

	InvalidateRgn(spectrum.hwnd, NULL, TRUE);
    }

    static long timer;

    if (found)
    {
	// If display not locked
	if (!display.lock)
	{
	    // Update the display struct
	    display.f = f;
	    display.fr = fr;
	    display.c = c;
	    display.n = n;
	    display.count = count;

	    // Update meter
	    meter.c = c;

	    // Update strobe
	    strobe.c = c;
	}

	// Update display
	InvalidateRgn(display.hwnd, NULL, TRUE);

	// Reset count;
	timer = 0;
    }

    else
    {
	// If display not locked
	if (!display.lock)
	{

	    if (timer > TIMER_COUNT)
	    {
		display.f = 0.0;
		display.fr = 0.0;
		display.c = 0.0;
		display.n = 0;
		display.count = 0;

		// Update meter
		meter.c = 0.0;

		// Update strobe
		strobe.c = 0.0;

		// Update spectrum
		spectrum.f = 0.0;
		spectrum.r = 0.0;
		spectrum.l = 0.0;
		spectrum.h = 0.0;
	    }

	    // Update display
	    InvalidateRgn(display.hwnd, NULL, TRUE);
	}
    }

    timer++;
}

// Real to complex FFT, ignores imaginary values in input array
VOID fftr(complex a[], int n)
{
    double norm = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].r * norm;

	    a[j].r = a[i].r * norm;
	    a[j].i = 0.0;

	    a[i].r = tr;
	    a[i].i = 0.0;
	}

	int m = n / 2;
	while (m >= 1 && j >= m)
	{
	    j -= m;
	    m /= 2;
	}
	j += m;
    }
    
    for (int mmax = 1, istep = 2 * mmax; mmax < n;
	 mmax = istep, istep = 2 * mmax)
    {
	double delta = (M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].r - wi * a[j].i;
		double ti = wr * a[j].i + wi * a[j].r;
		a[j].r = a[i].r - tr;
		a[j].i = a[i].i - ti;
		a[i].r += tr;
		a[i].i += ti;
	    }
	}
    }
}