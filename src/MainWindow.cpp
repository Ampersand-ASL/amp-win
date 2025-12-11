/**
 * Copyright (C) 2025, Bruce MacKinnon KC1FSZ
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include "MainWindow.h"

#define IDC_STATIC_TEXT 1000
#define IDC_EDIT_INPUT 1001
#define IDC_BUTTON_0 1002

using namespace std;

using namespace std;

namespace kc1fsz {

static const wchar_t WINDOW_CLASS_NAME[] = L"MainWindow";    

void MainWindow::reg(HINSTANCE hInstance) {
    WNDCLASS wc = { };
    wc.lpfnWndProc = _windProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    RegisterClass(&wc);
}

MainWindow::MainWindow(HINSTANCE hInstance) {

    _whiteBrush = CreateSolidBrush(RGB(255, 255, 255));

    _hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        WINDOW_CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL
        );
    SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);   

    HWND hWndStatic = CreateWindowEx(
        0,                      // Optional window extended style
        L"STATIC",              // The predefined static control class name
        L"Some static text",    // The text to display
        WS_CHILD | WS_VISIBLE | SS_LEFT, // Styles (child, visible, left-aligned)
        10, 10,                 // X, Y position
        200, 20,                // Width, Height
        _hwnd,                   // Handle to the parent window
        (HMENU)IDC_STATIC_TEXT, // Child window identifier (ID)
        hInstance,              // Handle to the application instance
        NULL                    // Pointer to window creation data
    );

    HWND hEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Extended styles, gives a sunken border look
        L"EDIT",           // Predefined window class name for edit controls (L for Unicode)
        L"Enter text here", // Initial text (can be NULL or empty string L"")
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, // Window styles
        10, 50,           // X, Y position
        200, 25,          // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_EDIT_INPUT, // Control ID (cast to HMENU)
        hInstance, // Instance handle
        NULL              // Additional data
    );

    HWND hwndButton = CreateWindow(
        TEXT("button"),   // Predefined class name
        TEXT("Click Me"), // Button text
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        10, 100,           // X, Y position
        80, 25,           // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_BUTTON_0,      // Button's unique identifier (ID)
        hInstance,        // Application instance handle
        NULL              // Additional app data
    );
}

MainWindow::~MainWindow() {
    DestroyWindow(_hwnd);
}

LRESULT CALLBACK MainWindow::_windProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Pull out the user data for the window
    auto ud = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    return ud->_msg(hwnd, uMsg, wParam, lParam);
}

void MainWindow::show(int nCmdShow) {
    ShowWindow(_hwnd, nCmdShow);
}

LRESULT MainWindow::_msg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
        case WM_CREATE:
        {
            const CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            cout << "Got " << (unsigned long long)cs->lpCreateParams << endl;
            return 0;
        }
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            // Set the text color (optional, e.g., black)
            SetTextColor(hdcStatic, RGB(0, 0, 0));
            // Set the background color in the DC
            SetBkColor(hdcStatic, RGB(255, 255, 255));
            // Return the handle to the background brush
            return (LRESULT)_whiteBrush;
        }        
        case WM_COMMAND: 
        {
            int controlID = LOWORD(wParam);
            int messageType = HIWORD(wParam);
            if (controlID == IDC_BUTTON_0 && messageType == BN_CLICKED) {
                // Code to execute when the "Click Me" button is pressed
                MessageBox(hwnd, TEXT("Button was clicked!"), TEXT("Notification"), MB_OK);
            }
            break;
        }        
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    // By default
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

}
