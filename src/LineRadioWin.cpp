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

#include "kc1fsz-tools/Log.h"

#include "Message.h"
#include "LineRadioWin.h"
#include "Transcoder_SLIN_48K.h"

using namespace std;

namespace kc1fsz {

LineRadioWin::LineRadioWin(Log& log, Clock& clock, MessageConsumer& consumer, 
    unsigned busId, unsigned callId, unsigned destBusId, unsigned destCallId)
:   LineRadio(log, clock, consumer, busId, callId, destBusId, destCallId) {

    // Specify audio parameters
    WAVEFORMATEX pFormat;
    pFormat.wFormatTag = WAVE_FORMAT_PCM;
    pFormat.nChannels = 1;
    pFormat.nSamplesPerSec = AUDIO_RATE;
    pFormat.nAvgBytesPerSec = AUDIO_RATE * 2;
    pFormat.nBlockAlign = 2;
    pFormat.wBitsPerSample = 16;
    pFormat.cbSize = 0;

    // Open the output channel - using the default output device (WAVE_MAPPER)
    MMRESULT result = waveOutOpen(&_waveOut, WAVE_MAPPER, &pFormat, (DWORD_PTR)0, 0L, 
        (WAVE_FORMAT_DIRECT | CALLBACK_NULL));
    if (result) {
        _log.error("Open failed");
        return;
    }

    // Set up and prepare two buffers/headers for output.
    for (unsigned i = 0; i < _queueSize; i++) {
        _waveHdr[i].lpData = (LPSTR)_waveData[i];
        _waveHdr[i].dwBufferLength = BLOCK_SIZE_48K * 2;
        _waveHdr[i].dwBytesRecorded = 0;
        _waveHdr[i].dwUser = 0L;
        _waveHdr[i].dwFlags = 0L;
        _waveHdr[i].dwLoops = 0L;
        result = waveOutPrepareHeader(_waveOut, &(_waveHdr[i]), sizeof(WAVEHDR));    
        if (result) {
            _log.error("Prepare failed");
            return;
        }
    }

     waveOutRestart(_waveOut);
}

int LineRadioWin::open(const char* deviceName, const char* hidName) {    
    return 0;
}

void LineRadioWin::close() {
}
    
bool LineRadioWin::run2() {
    return _progressPlay();
}

bool LineRadioWin::_progressPlay() {

    uint64_t startTime = _clock.timeUs();
    bool didSomething = false;

    // Anything waiting?
    if (_nextQueuePtr != _nextPlayPtr) {
        didSomething = true;
        // Start the new one
        MMRESULT result = waveOutWrite(_waveOut, 
            &(_waveHdr[_nextPlayPtr]), sizeof(WAVEHDR));
        if (result) {
            //_log.info("Write failed %d", _nextPlayPtr);
        }
        else {
            //_log.info("Played %d", _nextPlayPtr);
            // Increment and wrap
            _nextPlayPtr++;
            if (_nextPlayPtr == _queueSize)
                _nextPlayPtr = 0;
        }
    }

    uint64_t endTime = _clock.timeUs();
    uint64_t dur = endTime - startTime;

    if (dur > 500) {
        cout << "Dur " << dur << " " << _spurtCount << endl;
    }

    if (dur > _maxTime) {
        _maxTime = dur;
    }

    return didSomething;
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
        _spurtCount = 0;
    }

    assert(msg.size() == BLOCK_SIZE_48K * 2);
    assert(msg.getFormat() == CODECType::IAX2_CODEC_SLIN_48K);

    // Convert the SLIN_48K LE into 16-bit PCM audio
    Transcoder_SLIN_48K transcoder;

    // #### TODO: STUFF A FRAME IF WE'RE COMING OUT OF SILENCE

    transcoder.decode(msg.raw(), msg.size(), _waveData[_nextQueuePtr], BLOCK_SIZE_48K);
    //_log.info("Queued %d", _nextQueuePtr);
    // Increment and wrap
    _nextQueuePtr++;
    if (_nextQueuePtr == _queueSize) 
        _nextQueuePtr = 0;

    // Immediately try to write something
    _progressPlay();

    _lastPlayedFrameMs = _clock.time();
    _playing = true;
    _spurtCount++;
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
    cout << "max  " << _maxTime << endl;
    _maxTime = 0;
}


}
