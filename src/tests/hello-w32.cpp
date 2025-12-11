/*
A demonstration of the basic capabilities of the Win32 API.
Copyright (C) 2005, Bruce MacKinnon KC1FSZ
*/
#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <iostream>

#define IDC_STATIC_TEXT 1000
#define IDC_EDIT_INPUT 1001
#define IDC_BUTTON_0 1002

using namespace std;

static HBRUSH whiteBrush = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int nCmdShow) {

    // Register the window class.
    const wchar_t WINDOW_CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        WINDOW_CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        // This data will be passed into the WM_CREATE event (see below)
        (LPVOID)777
        );
    if (hwnd == NULL) {
        return 0;
    }

    // Demonstration: attach user data to window (like an object pointer)
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)888);

    HWND hWndStatic = CreateWindowEx(
        0,                      // Optional window extended style
        L"STATIC",              // The predefined static control class name
        L"Some static text",    // The text to display
        WS_CHILD | WS_VISIBLE | SS_LEFT, // Styles (child, visible, left-aligned)
        10, 10,                 // X, Y position
        200, 20,                // Width, Height
        hwnd,                   // Handle to the parent window
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
        hwnd,             // Parent window handle
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
        hwnd,             // Parent window handle
        (HMENU)IDC_BUTTON_0,      // Button's unique identifier (ID)
        hInstance,        // Application instance handle
        NULL              // Additional app data
    );

    whiteBrush = CreateSolidBrush(RGB(255, 255, 255));

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Pull out the user data for the window
    auto ud = (unsigned long long)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    cout << "Message for window " << ud << endl;

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
            return (LRESULT)whiteBrush;
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

