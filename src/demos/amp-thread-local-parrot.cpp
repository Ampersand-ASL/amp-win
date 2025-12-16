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
#include "MessageBus.h"
#include "EventLoop.h"
#include "Bridge.h"

#include "demos/amp-thread-local-parrot.h"

using namespace std;
using namespace kc1fsz;

// #### TODO: Pull this out of the global scope
threadsafequeue<Request> MsgQueue;

class QueueWatcher : public Runnable2 {
public: 

    QueueWatcher(LineRadioWin& radio) : _radio(radio) { }

    virtual bool run2() {
        Request req;
        if (MsgQueue.try_pop(req)) {
            if (req.cmd == "ptton")
                _radio.setCos(true);
            else if (req.cmd == "pttoff")
                _radio.setCos(false);
            return true;
        }
        else {
            return false;
        }
    }

private:

    LineRadioWin& _radio;
};

void amp_thread(void* ud) {

    Log& log = *((Log*)ud);
    log.info("amp_thread start");
    StdClock clock;

    amp::Bridge bridge10(log, clock);

    LineRadioWin radio0(log, clock, bridge10, 2, 1, 10, 1);
    bridge10.setSink(&radio0);

    Sleep(1000);

    log.info("Opening radio connection");
    
    int rc = radio0.open("","");
    if (rc < 0) {
        log.error("Failed to open radio connection %d", rc);
        return;
    }

    QueueWatcher watcher0(radio0);
   
    // Main loop        
    const unsigned task2Count = 3;
    Runnable2* tasks2[task2Count] = { &radio0, &bridge10, &watcher0 };
    EventLoop::run(log, clock, 0, 0, tasks2, task2Count, 0, true);
}
