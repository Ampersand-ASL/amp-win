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
#include <cassert>

#include <process.h>

#include <curl/curl.h>

#include "kc1fsz-tools/win32/Win32MTLog.h"

#include "MainWindow.h"
#include "amp-thread-local-parrot.h"

using namespace std;
using namespace kc1fsz;

/*
Development:

export AMP_NODE0_USBSOUND="vendorname:\"C-Media Electronics, Inc.\""
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int nCmdShow) {

    // TODO: CRASH DISPLAY

    Win32MTLog log;

    MainWindow::reg(hInstance);
    
    // COM initialization (needed for audio interface)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
    assert(hr == S_OK);
      
    // TODO GET HANDLE FOR SHUTDOWN

    // Get the AMP thread running
    _beginthread(amp_thread, 0, (void*)&log);

    MainWindow w(hInstance, log, "61057", MsgQueue);
    w.show(nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

