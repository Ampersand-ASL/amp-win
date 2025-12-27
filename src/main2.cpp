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
#include <fstream>
#include <filesystem>

#include <process.h>

#include <curl/curl.h>

#include "kc1fsz-tools/win32/Win32MTLog.h"
#include "kc1fsz-tools/fixedstring2.h"

#include "amp-thread.h"
#include "service-thread.h"

using namespace std;
using namespace kc1fsz;

static const string DEFAULT_CONFIG = "{\"aslDnsRoot\":\"nodes.allstarlink.org\",\"aslRegUrl\":\"https://register.allstarlink.org\",\"aslStatUrl\":\"http://stats.allstarlink.org/uhandler\",\"audioDeviceType\":\"\",\"audioDeviceUsbBus\":\"\",\"audioDeviceUsbPort\":\"\",\"call\":\"\",\"iaxPort4\":\"4568\",\"lastUpdateMs\":0,\"node\":\"\",\"password\":\"\",\"privateKey\":\"\"}";

int main(int, const char**) {

    // TODO: CRASH DISPLAY

    Win32MTLog log;

    // Winsock init
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

    // Check to see if the config file exists, create if not
    string cfgFileName = "./config.json";
    log.info("Using configuration file %s", cfgFileName.c_str());

    if (!filesystem::exists(cfgFileName)) {
        log.info("Creating default configuration");
        ofstream cfg(cfgFileName);
        if (cfg.is_open()) {
            cfg << DEFAULT_CONFIG << endl;
            cfg.close();
        } else {
            log.error("Unable to create default configuration");
        }
    }

    // Get the Service thread running
    service_thread_args args1;
    args1.cfgFileName = cfgFileName;
    args1.log = &log;

    _beginthread(service_thread, 0, &args1);

    // Amp thread runs on main thread
    amp_thread_args args2;
    args2.cfgFileName = cfgFileName;
    args2.log = &log;

    amp_thread(&args2);

    return 0;
}


