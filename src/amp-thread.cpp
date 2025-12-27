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
#include <cmath> 
#include <queue>

#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/linux/StdClock.h"
#include "kc1fsz-tools/fixedqueue.h"
#include "kc1fsz-tools/NetUtils.h"

#include "LineIAX2.h"
#include "LineRadioWin.h"
#include "EventLoop.h"
#include "Bridge.h"
#include "MultiRouter.h"
#include "ConfigPoller.h"

#include "amp-thread.h"
#include "WebUi.h"

using namespace std;
using namespace kc1fsz;

#define WEB_UI_PORT 8080

/*
// Connects the manager to the IAX channel (TEMPORARY)
class ManagerSink : public ManagerTask::CommandSink {
public:

    ManagerSink(LineIAX2& ch) 
    :   _ch(ch) { }

    void execute(const char* cmd) { 
        _ch.processManagementCommand(cmd);
    }

private:

    LineIAX2& _ch;
};
*/

class CallValidatorStd : public CallValidator {
public:
    virtual bool isNumberAllowed(const char* targetNumber) const {
        return true;
    }
};

class LocalRegistryStd : public LocalRegistry {
public:
    virtual bool lookup(const char* destNumber, sockaddr_storage& addr) {
        /*
        //addr.ss_family = AF_INET6;
        addr.ss_family = AF_INET;
        //setIPAddr(addr, "::1");
        setIPAddr(addr, "127.0.0.1");
        setIPPort(addr, 4569);
        char temp[64];
        formatIPAddrAndPort((const sockaddr&)addr, temp, 64);
        return true;
        */
       return false;
    }
};

void amp_thread(void* ud) {

    // Pull out the thread startup arguments
    const amp_thread_args* args = (amp_thread_args*)ud;
    const string cfgFileName = args->cfgFileName;
    Log& log = *(args->log);

    log.info("amp_thread start");

    StdClock clock;

    MultiRouter router;

    amp::Bridge bridge10(log, clock, amp::BridgeCall::Mode::NORMAL);
    bridge10.setSink(&router);
    router.addRoute(&bridge10, 10);
    
    LineRadioWin radio2(log, clock, router, 2, 1, 10, 1);
    router.addRoute(&radio2, 2);

    CallValidatorStd val;
    LocalRegistryStd locReg;
    LineIAX2 iax2Channel1(log, clock, 1, router, &val, &locReg, 10);
    //iax2Channel1.setTrace(true);
    iax2Channel1.setPrivateKey(getenv("AMP_PRIVATE_KEY"));
    iax2Channel1.setDNSRoot(getenv("AMP_ASL_DNS_ROOT"));
    router.addRoute(&iax2Channel1, 1);

    // Instantiate the server for the web-based UI
    amp::WebUi webUi(log, clock, router, WEB_UI_PORT, 1, 2, cfgFileName.c_str());
    // This allow the WebUi to watch all traffic and pull out the things 
    // that are relevant for status display.
    router.addRoute(&webUi, MultiRouter::BROADCAST);

    // Setup the configuration poller for this thread
    amp::ConfigPoller cfgPoller(log, cfgFileName.c_str(), 
        // This function will be called on any update to the configuration document.
        [&log, &webUi, &iax2Channel1, &radio2, &bridge10](const json& cfg) {

            log.info("Configuration change detected");
            cout << cfg.dump() << endl;

            // Transfer the new configuration into the various places it is needed
            webUi.setConfig(cfg);

            int rc;
            rc = iax2Channel1.open(AF_INET, std::stoi(cfg["iaxPort4"].get<std::string>()), "radio");
            if (rc < 0) {
                log.error("Failed to open IAX2 connection %d", rc);
            }

            // #### TODO: Audio Device Selection
            rc = radio2.open("","");
            if (rc < 0) {
                log.error("Failed to open radio connection %d", rc);
                return;
            }
        }
    );

    // #### TODO: UNDERSTAND THIS, POSSIBLE RACE CONDITION
    //Sleep(500);

    // Main loop        
    Runnable2* tasks[] = { &radio2, &iax2Channel1, &bridge10, &router, &webUi, &cfgPoller };
    EventLoop::run(log, clock, 0, 0, tasks, std::size(tasks), 0, false);
}
