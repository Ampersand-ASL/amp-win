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
//#include <process.h>

#include <iostream>
#include <cassert>

#include "kc1fsz-tools/Log.h"

#include "Message.h"
#include "LineRadioWin.h"
#include "Transcoder_SLIN_48K.h"

using namespace std;

namespace kc1fsz {

LineRadioWin::LineRadioWin(Log& log, Clock& clock, MessageConsumer& consumer, 
    unsigned busId, unsigned callId, unsigned destBusId, unsigned destCallId)
:   LineRadio(log, clock, consumer, busId, callId, destBusId, destCallId) {
    // Get the audio thread running
    _beginthread(_audioThread, 0, (void*)this);
}

int LineRadioWin::open(const char* deviceName, const char* hidName) {    
    return 0;
}

void LineRadioWin::close() {
}
    
bool LineRadioWin::run2() {
    return false;
}

void LineRadioWin::audioRateTick() {
     _checkTimeouts();
}

// ===== Play Related =========================================================

void LineRadioWin::consume(const Message& frame) {
    if (frame.getType() == Message::Type::AUDIO)
        _play(frame);
}

void LineRadioWin::_play(const Message& msg) {  

    // Detect transitions from silence to playing
    if (!_playing) {
        _playStart();
    }

    assert(msg.size() == BLOCK_SIZE_48K * 2);
    assert(msg.getFormat() == CODECType::IAX2_CODEC_SLIN_48K);

    // Convert the SLIN_48K LE into 16-bit PCM audio
    Transcoder_SLIN_48K transcoder;

    // #### TODO: STUFF A FRAME IF WE'RE COMING OUT OF SILENCE

    //transcoder.decode(msg.raw(), msg.size(), _waveData[_nextQueuePtr], BLOCK_SIZE_48K);
    
    _lastPlayedFrameMs = _clock.time();
    _playing = true;
}

void LineRadioWin::_checkTimeouts() {

    // Detect transitions from audio to silence
    if (_playing &&
        _clock.isPast(_lastPlayedFrameMs + _playSilenceIntervalMs)) {
        _playing = false;
        _log.info("Play End");
        _playEnd();
    }
    /*
    if (_capturing &&
        _clock.isPast(_lastCapturedFrameMs + _captureSilenceIntervalMs)) {
        _capturing = false;
        _captureEnd();
    }
    */
}

void LineRadioWin::oneSecTick() {
}

void _audioThread(void*);
void _audioThread();


}
