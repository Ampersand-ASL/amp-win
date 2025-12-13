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
#include <atomic>

#include "kc1fsz-tools/threadsafequeue.h"

#include "PCM16Frame.h"
#include "LineRadio.h"

namespace kc1fsz {

/**
 * An audio interface line for MSFT Windows. Uses the WASAPI audio API.
 */
class LineRadioWin : public LineRadio {
public:

    LineRadioWin(Log&, Clock&, MessageConsumer& consumer, unsigned busId, unsigned callId,
        unsigned destBusId, unsigned destCallId);
    ~LineRadioWin();

    int open(const char* deviceName, const char* hidName);    
    void close();

    void setCos(bool cos);

    // ----- From MessageConsumer ---------------------------------------------

    void consume(const Message& frame);

    // ----- Runnable ---------------------------------------------------------

    bool run2();
    void audioRateTick();

private:

    static unsigned _playThread(void*);
    unsigned _playThread();
    static unsigned _captureThread(void*);
    unsigned _captureThread();

    void _play(const Message& msg);
    void _checkTimeouts();

    std::atomic<bool> _run;
    std::atomic<bool> _captureEnabled;

    HANDLE _playThreadH = 0;
    bool _playing = false;
    // ### TODO: MOVE UP TO BASE
    uint32_t _lastPlayedFrameMs = 0;
    // If we go silent for this amount of time the playback is assumed
    // to have ended. 
    uint32_t _playSilenceIntervalMs = 20 * 4;
    // This queue passes play audio out to the audio thread
    threadsafequeue<PCM16Frame> _playQueueMTSafe;

    HANDLE _captureThreadH = 0;
    bool _capturing = false;
    uint64_t _captureStartUs = 0;
    uint32_t _captureCount = 0;
    uint32_t _lastCapturedFrameMs = 0;
    // If we go silent for this amount of time the capture is assumed to have ended. 
    uint32_t _captureSilenceIntervalMs = 20 * 4;
    // This queue passes capture audio out of the audio thread
    threadsafequeue<PCM16Frame> _captureQueueMTSafe;
    static const unsigned MAX_CAPTURE_BUFFER_SIZE = BLOCK_SIZE_48K * 4;
    int16_t _captureBuffer[MAX_CAPTURE_BUFFER_SIZE];
    unsigned _captureBufferSize = 0;
};

}
