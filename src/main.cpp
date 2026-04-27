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
#include <thread>

#include <process.h>

// 3rd party
#include <curl/curl.h>
// 3rd party command-line parser
#include <argparse/argparse.hpp>

#include "kc1fsz-tools/win32/Win32MTLog.h"
#include "kc1fsz-tools/linux/StdClock.h"
#include "kc1fsz-tools/fixedstring2.h"
#include "kc1fsz-tools/threadsafequeue2.h"
#include "kc1fsz-tools/copyableatomic.h"

// amp-core
#include "config-handler.h"
#include "service-thread.h"
#include "LineIAX2.h"
#include "EventLoop.h"
#include "Bridge.h"
#include "MultiRouter.h"
#include "ConfigPoller.h"
#include "WebUi.h"
#include "TraceLog.h"
#include "QueueConsumer.h"
#include "Message.h"

#include "config-handler.h"
#include "LineRadioWin.h"
#include "LocalRegistryStd.h"

#define MAX_CALLS (8)

// Line IDs
#define LINE_ID_IAX (1)
#define LINE_ID_BRIDGE (10)
#define LINE_ID_STATS (12)
#define LINE_ID_SIGNAL_OUT (31)

using namespace std;
using namespace kc1fsz;

// ### TODO: FIGURE OUT HOW TO MAKE THIS AUTOMATIC
static const char* VERSION = "20260427.0";
const char* const GIT_HASH = "?";
static const char* PUBLIC_USER = "radio";

// These are potentially large structure, so keeping it off the stack
static amp::BridgeCall callBank[MAX_CALLS];
static LineIAX2::Call iaxCallBank[MAX_CALLS];

int main(int argc, const char** argv) {

    // TODO: CRASH DISPLAY

    StdClock clock;
    Win32MTLog log;
    const unsigned traceLogSize = 64;
    std::string traceLogData[traceLogSize];
    TraceLog traceLog(clock, traceLogData, traceLogSize);

    log.info("AMP Windows");
    log.info("Powered by the Ampersand ASL Project https://github.com/Ampersand-ASL");
    log.info("Copyright (C) 2026, Bruce MacKinnon KC1FSZ");
    log.info("Version %s Git Hash %s", VERSION, GIT_HASH);
    log.info("----------------------------------------------------------------------");

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
      
    // Parse command line arguments
    argparse::ArgumentParser program("amp-server", VERSION);
    string cfgFileName;
    string defaultCfgFileName = getenv("USERPROFILE");
    defaultCfgFileName += "/amp-win.json";
    program.add_argument("--config")
        .help("Name of configuration file")
        .default_value(defaultCfgFileName)
        .store_into(cfgFileName);
    
    int uiPort = 8080;
    program.add_argument("--httpport")
        .store_into(uiPort)
        .default_value(8080)
        .help("Port number for HTTP UI server");

    string uiPwd;
    program.add_argument("--httppwd")
        .help("Password for HTTP UI/API authentication")
        .store_into(uiPwd);

    program.add_argument("--trace")
        .help("Turn on network tracing")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        log.error("Argument error: %s", err.what());
        std::exit(-2);
    }

    log.info("Using configuration file %s", cfgFileName.c_str());

    // Create a default/starting config file if this is the first time.
    if (!filesystem::exists(cfgFileName)) {
        log.info("Creating default configuration");
        ofstream cfg(cfgFileName);
        if (cfg.is_open()) 
            cfg << amp::ConfigPoller::DEFAULT_CONFIG << endl;
        else {
            log.error("Unable to create default configuration");
            std::exit(-3);
        }
    }

    // This is the router (aka "bus") that passes Message objects between the rest 
    // of the components in the system. You'll see that everything else below is
    // wired to the router one way or the other.
    threadsafequeue2<MessageCarrier> respQueue;
    // A wrapper that makes the response queue look like a MessageConsumer
    QueueConsumer respQueueConsumer(respQueue);

    MultiRouter router(respQueue);

    copyableatomic<std::string> pokeAddr;

    // Setup a way to pass messages over to the service thread
    threadsafequeue2<MessageCarrier> serviceThreadReqQueue;
    QueueConsumer serviceThreadReqQueueConsumer(serviceThreadReqQueue);
    // Pass message from local router up to the service thread
    router.addRoute(&serviceThreadReqQueueConsumer, LINE_ID_STATS);

    // Get the service thread running. This handles non-time-sensitive
    // stuff like registration, stats, etc.
    std::thread serviceThread(amp::service_thread, &cfgFileName, &log, VERSION, &pokeAddr,
        &serviceThreadReqQueue);

    // The Bridge is what provides the audio conference capability. The various 
    // Lines connect to the Bridge.
    amp::Bridge bridge10(log, traceLog, clock, router, amp::BridgeCall::Mode::NORMAL, 
        LINE_ID_BRIDGE, 
        0, 0, 0, LINE_ID_IAX, LINE_ID_STATS,
        callBank, MAX_CALLS);
    router.addRoute(&bridge10, LINE_ID_BRIDGE);

    // This is the Line that connects to the USB sound interface
    LineRadioWin radio2(log, clock, router, 2, 1, 10, 1, LINE_ID_SIGNAL_OUT);
    router.addRoute(&radio2, 2);

    // This is the Line that makes the IAX2 network connection
    LocalRegistryStd locReg;
    LineIAX2 iax2Channel1(log, traceLog, clock, 1, router, 0, 0, &locReg, 10, PUBLIC_USER,
        iaxCallBank, MAX_CALLS);
    router.addRoute(&iax2Channel1, 1);
    if (program["--trace"] == true)
        iax2Channel1.setTrace(true);

    // This is the HTTP server that provides the UI
    amp::WebUi webUi(log, clock, uiPort, 1, 2, cfgFileName.c_str(), 
        VERSION, traceLog);
    // This allow the WebUi to watch all traffic and pull out the things 
    // that are relevant for status display.
    router.addRoute(&webUi, MultiRouter::BROADCAST);
    webUi.setUiPWd(uiPwd);

    // Get the UI thread going
    std::thread webUiThread(amp::WebUi::uiThread, &webUi, &respQueueConsumer);

    // This is a poller that watches for changes to the configuration file
    // and applies those changes to everything on the main thread.
    amp::ConfigPoller cfgPoller(log, cfgFileName.c_str(), 
        // This function will be called on any update to the configuration document.
        [&log, &webUi, &iax2Channel1, &radio2, &bridge10]
        (const json& cfg) {

            log.info("Configuration change detected");
            cout << cfg.dump() << endl;

            try {
                amp::configHandler(log, cfg, webUi, iax2Channel1, radio2, bridge10);
            }
            // ### TODO MORE SPECIFIC
            catch (json::exception& ex) {
                log.error("Failed to process configuration change %s", ex.what());
            }
        }
    );

    // Setup a poller that looks at the bridge status and passes any updates
    // over to the web UI. We will get an event *AT LEAST* every 10 seconds.
    amp::BridgeStatusDocPoller statusPoller(log, clock, bridge10, 10 * 1000,
        [&webUi](const json& statusDoc) {
            webUi.setBridgeStatus(statusDoc);
        }
    );

    // Setup the EventLoop with all of the tasks that need to be run on this thread
    Runnable2* tasks[] = { &radio2, &iax2Channel1, &bridge10, &webUi, &cfgPoller,
        &statusPoller, &router };
    EventLoop::run(log, clock, 0, 0, tasks, std::size(tasks), nullptr, false);

    // #### TODO: At the moment there is no clean way to get out of the loop

    return 0;
}


