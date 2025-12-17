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
#include "amp-thread.h"
#include "service-thread.h"

using namespace std;
using namespace kc1fsz;

/*
Development:

export AMP_NODE0_NUMBER=672730
export AMP_NODE0_PASSWORD=xxxxx
export AMP_NODE0_MGR_PORT=5038
export AMP_IAX_PORT=4568
export AMP_IAX_PROTO=IPV4
export AMP_ASL_REG_URL=https://register.allstarlink.org
export AMP_ASL_STAT_URL=http://stats.allstarlink.org/uhandler
export AMP_NODE0_USBSOUND="vendorname:\"C-Media Electronics, Inc.\""
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int nCmdShow) {

    // TODO: CRASH DISPLAY

    Win32MTLog log;

    MainWindow::reg(hInstance);
    
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int rc = WSAStartup(wVersionRequested, &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed with error: %d\n", rc);
        return -1;
    }

    // Get libcurl going
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res) {
        printf("Libcurl failed\n");
        return -1;
    }

    // COM initialization (needed for audio interface)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
    assert(hr == S_OK);
      
    // TODO GET HANDLE FOR SHUTDOWN

    // Get the AMP thread running
    _beginthread(amp_thread, 0, (void*)&log);
    // Get the Service thread running
    _beginthread(service_thread, 0, (void*)&log);

    //MainWindow w(hInstance, log, "61057", MsgQueue);
    MainWindow w(hInstance, log, "672730", MsgQueue);
    w.show(nCmdShow);

    log.info("Main loop");

    // Run the Windows message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
