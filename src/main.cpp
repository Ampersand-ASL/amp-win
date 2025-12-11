#include <iostream>

#include <process.h>

#include <curl/curl.h>

#include "MainWindow.h"
#include "amp-thread.h"
#include "service-thread.h"

using namespace std;
using namespace kc1fsz;

/*
Development:

export AMP_NODE0_NUMBER=672730
export AMP_NODE0_PASSWORD=microlink2
export AMP_NODE0_MGR_PORT=5038
export AMP_IAX_PORT=4568
export AMP_IAX_PROTO=IPV4
export AMP_ASL_REG_URL=https://register.allstarlink.org
export AMP_NODE0_USBSOUND="vendorname:\"C-Media Electronics, Inc.\""
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int nCmdShow) {

    // TODO: CRASH DISPLAY
    // TODO: MTLog

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
      
    // Get the AMP thread running
    _beginthread(amp_thread, 0, (void*)777);
    // Get the Service thread running
    _beginthread(service_thread, 0, 0);

    MainWindow w(hInstance, "61057", MsgQueue);
    w.show(nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
