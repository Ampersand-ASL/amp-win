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
//#include <Windows.h>

#include "LineRadio.h"

namespace kc1fsz {

class LineRadioWin : public LineRadio {
public:

    LineRadioWin(Log&, Clock&, MessageConsumer& consumer, unsigned busId, unsigned callId,
        unsigned destBusId, unsigned destCallId);

    virtual void consume(const Message& frame);

    int open(const char* deviceName, const char* hidName);    
    void close();
    
    // ----- Runnable ---------------------------------------------------------

    virtual bool run2();
    virtual void audioRateTick();
    virtual void oneSecTick();

private:

    uint32_t _maxTime = 0;

    void _play(const Message& msg);
    bool _progressPlay();
    void _checkTimeouts();

    bool _playing = false;
    // ### TODO: MOVE UP TO BASE
    uint32_t _lastPlayedFrameMs = 0;
    // If we go silent for this amount of time the playback is assumed
    // to have ended. 
    uint32_t _playSilenceIntervalMs = 20 * 4;

    // Circular buffer
    bool _isPlaying = false;
    unsigned _nextQueuePtr = 0;
    unsigned _nextPlayPtr = 0;
    unsigned _currentPlayPtr = 0;
    unsigned _spurtCount = 0;

    static const unsigned _queueSize = 32;
    int16_t _waveData[_queueSize][BLOCK_SIZE_48K];
    WAVEHDR _waveHdr[_queueSize];

    HWAVEOUT _waveOut;

};

}
