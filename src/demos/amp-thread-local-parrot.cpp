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
#include "BridgeCall.h"
#include "MultiRouter.h"

#include "demos/amp-thread-local-parrot.h"

using namespace std;
using namespace kc1fsz;

// #### TODO: Pull this out of the global scope
// NOTE: NOT USED SINCE THERE IS NO NETWORK CONNECTION 
threadsafequeue<Message> MsgQueueIn;

void amp_thread(void* ud) {

    Log& log = *((Log*)ud);
    log.info("amp_thread start");
    StdClock clock;

    MultiRouter router;

    amp::Bridge bridge10(log, clock, amp::BridgeCall::Mode::PARROT);
    bridge10.setSink(&router);
    router.addRoute(&bridge10, 10);

    LineRadioWin radio2(log, clock, router, 2, 1, 10, 1);
    router.addRoute(&radio2, 2);

    Sleep(1000);

    log.info("Opening radio connection");
    
    int rc = radio2.open("","", false);
    if (rc < 0) {
        log.error("Failed to open radio connection %d", rc);
        return;
    }

    // Main loop        
    Runnable2* tasks2[] = { &radio2, &bridge10 };
    EventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), 0, true);
}
