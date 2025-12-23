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
#include <string>
#include <vector>

#include "kc1fsz-tools/Log.h"

#include "MainWindow.h"
#include "amp-thread.h"

#define IDC_STATIC_TEXT 1000
#define IDC_EDIT_INPUT 1001
#define IDC_BUTTON_CONNECT 1002
#define IDC_BUTTON_DISCONNECTALL 1003
#define IDC_BUTTON_PTT 1004

using namespace std;

using namespace std;

namespace kc1fsz {

string getEditText(HWND hWndEdit);

static const wchar_t WINDOW_CLASS_NAME[] = L"MainWindow";    

void MainWindow::reg(HINSTANCE hInstance) {
    WNDCLASS wc = { };
    wc.lpfnWndProc = _windProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    RegisterClass(&wc);
}

MainWindow::MainWindow(HINSTANCE hInstance, Log& log, const char* localNodeNumber,
    threadsafequeue<Message>& q2, 
    unsigned networkDestLineId, unsigned radioDestLineId)
:   _log(log),
    _localNodeNumber(localNodeNumber),
    _msgQueue2(q2),
    _networkDestLineId(networkDestLineId),
    _radioDestLineId(radioDestLineId) {

    _whiteBrush = CreateSolidBrush(RGB(255, 255, 255));

    wchar_t nodeNameW[32];
    MultiByteToWideChar(CP_UTF8, 0, localNodeNumber, -1, nodeNameW, 32);
    wchar_t label[64];
    _snwprintf_s(label, 64, L"ASL Ampersand (Node: %s)", nodeNameW);

    _hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        WINDOW_CLASS_NAME,                     // Window class
        label,
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        100, 100, 500, 150,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL
        );
    SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);   

    CreateWindowEx(
        0,                      // Optional window extended style
        L"STATIC",              // The predefined static control class name
        L"ASL Ampersand - KC1FSZ bruce@mackinnon.com",    // The text to display
        WS_CHILD | WS_VISIBLE | SS_LEFT, // Styles (child, visible, left-aligned)
        10, 10,                 // X, Y position
        400, 20,                // Width, Height
        _hwnd,                   // Handle to the parent window
        (HMENU)IDC_STATIC_TEXT, // Child window identifier (ID)
        hInstance,              // Handle to the application instance
        NULL                    // Pointer to window creation data
    );

    _hEditNode = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Extended styles, gives a sunken border look
        L"EDIT",           // Predefined window class name for edit controls (L for Unicode)
        //L"61057",
        //L"55553",
        L"27339",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, // Window styles
        10, 40,           // X, Y position
        150, 25,          // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_EDIT_INPUT, // Control ID (cast to HMENU)
        hInstance, // Instance handle
        NULL              // Additional data
    );

    CreateWindow(
        TEXT("button"),   // Predefined class name
        TEXT("Connect"), // Button text
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        180, 40,           // X, Y position
        80, 25,           // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_BUTTON_CONNECT,      // Button's unique identifier (ID)
        hInstance,        // Application instance handle
        NULL              // Additional app data
    );

    CreateWindow(
        TEXT("button"),   // Predefined class name
        TEXT("Disconnect All"), // Button text
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        280, 40,           // X, Y position
        150, 25,           // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_BUTTON_DISCONNECTALL,      // Button's unique identifier (ID)
        hInstance,        // Application instance handle
        NULL              // Additional app data
    );

    _hPttButton = CreateWindow(
        TEXT("button"),   // Predefined class name
        TEXT("PTT (Currently Unkeyed)"), // Button text
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW, // Styles
        10, 80,           // X, Y position
        200, 25,           // Width, Height
        _hwnd,             // Parent window handle
        (HMENU)IDC_BUTTON_PTT,      // Button's unique identifier (ID)
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
            //const CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
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
            if (controlID == IDC_BUTTON_CONNECT && messageType == BN_CLICKED) {
                PayloadCall req;
                strcpyLimited(req.localNumber, _localNodeNumber.c_str(), sizeof(req.localNumber));
                strcpyLimited(req.targetNumber, getEditText(_hEditNode).c_str(), sizeof(req.targetNumber));
                Message msg(Message::Type::SIGNAL, Message::SignalType::CALL_NODE, 
                    sizeof(req), (const uint8_t*)&req, 0, 0);
                msg.setDest(_networkDestLineId, 1);
                _msgQueue2.push(msg);
            }
            else if (controlID == IDC_BUTTON_DISCONNECTALL && messageType == BN_CLICKED) {
                Message msg(Message::Type::SIGNAL, Message::SignalType::DROP_ALL_NODES, 
                    0, 0, 0, 0);
                msg.setDest(_networkDestLineId, 1);
                _msgQueue2.push(msg);
            }
            else if (controlID == IDC_BUTTON_PTT && messageType == BN_CLICKED) {
                if (!_pttToggle) {
                    
                    Message msg(Message::Type::SIGNAL, Message::SignalType::COS_ON, 
                        0, 0, 0, 0);
                    msg.setDest(_radioDestLineId, 1);
                    _msgQueue2.push(msg);

                    _pttToggle = true;
                    SendMessage(_hPttButton, WM_SETTEXT, 0, (LPARAM)TEXT("PTT (Currently Keyed)"));
                } else {

                    Message msg(Message::Type::SIGNAL, Message::SignalType::COS_OFF, 
                        0, 0, 0, 0);
                    msg.setDest(_radioDestLineId, 1);
                    _msgQueue2.push(msg);

                    _pttToggle = false;
                    SendMessage(_hPttButton, WM_SETTEXT, 0, (LPARAM)TEXT("PTT (Currently Unkeyed)"));
                }
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

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;

            // Special drawing for PTT button
            if (pDIS->hwndItem == _hPttButton) // Check if it's our specific button
            {
                // Your custom drawing code goes here
                // Use pDIS->hDC, pDIS->rcItem, pDIS->itemState

                const wchar_t* text;
                HBRUSH keyedBrush;
                if (_pttToggle) {
                   text = L"PTT (Keyed)";
                   keyedBrush = CreateSolidBrush(RGB(150, 150, 255));
                } else {
                   text = L"PTT (Currently Unkeyed)";
                   keyedBrush = CreateSolidBrush(RGB(0, 250, 0));
                }

                // Example: Draw a simple colored rectangle based on state
                HBRUSH hBrush;
                if (pDIS->itemState & ODS_SELECTED) {
                    hBrush = CreateSolidBrush(RGB(150, 150, 255));
                } else {
                    hBrush = keyedBrush;
                }
                FillRect(pDIS->hDC, &pDIS->rcItem, hBrush);
                DeleteObject(hBrush);

                // Draw text
                SetBkMode(pDIS->hDC, TRANSPARENT);
                DrawText(pDIS->hDC, text, -1, &pDIS->rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
                SetBkMode(pDIS->hDC, OPAQUE);

                // Draw focus rectangle if needed
                if (pDIS->itemState & ODS_FOCUS) {
                    DrawFocusRect(pDIS->hDC, &pDIS->rcItem);
                }
                return TRUE; // Handled the message
            }
        }
    }
    // By default
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
 * @returns The contents of a text box in normal encoding.
 */
string getEditText(HWND hWndEdit) {

    // 1. Determine text length
    int length = GetWindowTextLength(hWndEdit);
    if (length == 0) {
        cout << "Edit control is empty or an error occurred." << endl;
        return string();
    }
    // 2. Allocate buffer (use std::vector for robust memory management)
    // Add 1 for the null terminator
    std::vector<TCHAR> buffer(length + 1); 
    // 3. Retrieve the text
    // The third argument is the maximum number of characters to copy, including the null terminator.
    GetWindowText(hWndEdit, buffer.data(), length + 1); 
    // Convert to normal
    char contents[64];
    contents[0] = 0;
    int wSize = WideCharToMultiByte(CP_ACP, 0, buffer.data(), -1, NULL, 0, NULL, NULL);
    if (wSize > 0) {
        WideCharToMultiByte(CP_ACP, 0, buffer.data(), -1, contents, 64, NULL, NULL);
    }
    return string(contents);
}

}
