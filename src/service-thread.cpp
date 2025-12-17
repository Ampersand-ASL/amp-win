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

#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/linux/StdClock.h"

#include "RegisterTask.h"
#include "StatsTask.h"
#include "EventLoop.h"

using namespace std;
using namespace kc1fsz;

void service_thread(void* ud) {

    Log& log = *((Log*)ud);
    log.info("service_thread start");
    StdClock clock;

    RegisterTask registerTask(log, clock);
    registerTask.configure(getenv("AMP_ASL_REG_URL"), getenv("AMP_NODE0_NUMBER"), 
        getenv("AMP_NODE0_PASSWORD"), atoi(getenv("AMP_IAX_PORT")));

    StatsTask statsTask(log, clock, "1.0.0");
    statsTask.configure(getenv("AMP_ASL_STAT_URL"), getenv("AMP_NODE0_NUMBER"));

    // Main loop        
    Runnable2* tasks2[] = { &registerTask, &statsTask };
    EventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2));

    // #### TODO: NEED A CLEAN WAY TO EXIT THIS THREAD
}
