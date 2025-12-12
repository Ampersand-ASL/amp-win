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
#pragma once

#include <winsock2.h>

#include "kc1fsz-tools/threadsafequeue.h"

#include "PCM16Frame.h"
#include "LineRadio.h"

namespace kc1fsz {

class LineRadioWin : public LineRadio {
public:

    LineRadioWin(Log&, Clock&, MessageConsumer& consumer, unsigned busId, unsigned callId,
        unsigned destBusId, unsigned destCallId);
    ~LineRadioWin();

    int open(const char* deviceName, const char* hidName);    
    void close();

    // ----- From MessageConsumer ---------------------------------------------

    void consume(const Message& frame);

    // ----- Runnable ---------------------------------------------------------

    bool run2();
    void audioRateTick();
    void oneSecTick();

private:

    static unsigned _audioThread(void*);
    unsigned _audioThread();

    void _play(const Message& msg);
    void _checkTimeouts();

    HANDLE _threadH = 0;
    bool _run = false;

    bool _playing = false;
    // ### TODO: MOVE UP TO BASE
    uint32_t _lastPlayedFrameMs = 0;
    // If we go silent for this amount of time the playback is assumed
    // to have ended. 
    uint32_t _playSilenceIntervalMs = 20 * 4;
    // This queue passes play audio out to the audio thread
    threadsafequeue<PCM16Frame> _playQueue;
};

}
