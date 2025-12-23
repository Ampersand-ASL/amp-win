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
#include "TwoLineRouter.h"

#include "amp-thread.h"

using namespace std;
using namespace kc1fsz;

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

// #### TODO: Pull this out of the global scope
threadsafequeue<Request> MsgQueue;

class QueueWatcher : public Runnable2 {
public: 

    QueueWatcher(LineIAX2& iax, LineRadioWin& radio) : _iax(iax), _radio(radio) { }

    virtual bool run2() {
        Request req;
        if (MsgQueue.try_pop(req)) {
            if (req.cmd == "connect")
                _iax.call(req.localNodeNumber.c_str(), req.targetNodeNumber.c_str());
            else if (req.cmd == "disconnectall")
                _iax.disconnectAllNonPermanent();
            else if (req.cmd == "ptton")
                _radio.setCos(true);
            else if (req.cmd == "pttoff")
                _radio.setCos(false);
            return true;
        }
        else {
            return false;
        }
    }

    // #### TODO: REMOVE
    virtual void audioRateTick(uint32_t tickTimeMs) { }

private:

    LineIAX2& _iax;
    LineRadioWin& _radio;
};

void amp_thread(void* ud) {

    Log& log = *((Log*)ud);
    log.info("amp_thread start");
    StdClock clock;

    amp::Bridge bridge10(log, clock, amp::BridgeCall::Mode::NORMAL);
    
    LineRadioWin radio2(log, clock, bridge10, 2, 1, 10, 1);

    CallValidatorStd val;
    LocalRegistryStd locReg;
    LineIAX2 iax2Channel1(log, clock, 1, bridge10, &val, &locReg);
    //iax2Channel1.setTrace(true);
    iax2Channel1.setPrivateKey(getenv("AMP_PRIVATE_KEY"));
    iax2Channel1.setDNSRoot(getenv("AMP_ASL_DNS_ROOT"));

    TwoLineRouter router(iax2Channel1, 1, radio2, 2);
    bridge10.setSink(&router);

    QueueWatcher watcher(iax2Channel1, radio2);

    // #### TODO: UNDERSTAND THIS, POSSIBLE RACE CONDITION
    Sleep(500);

    int rc;
    rc = iax2Channel1.open(AF_INET, atoi(getenv("AMP_IAX_PORT")), "radio");
    if (rc < 0) {
        log.error("Failed to open IAX2 connection %d", rc);
        return;
    }
    
    rc = radio2.open("","");
    if (rc < 0) {
        log.error("Failed to open radio connection %d", rc);
        return;
    }

    // Main loop        
    const unsigned taskCount = 4;
    Runnable2* tasks[taskCount] = { &radio2, &iax2Channel1, &bridge10, &watcher };
    EventLoop::run(log, clock, 0, 0, tasks, taskCount, 0, false);
}
