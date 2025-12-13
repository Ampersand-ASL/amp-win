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
// NOTE: There are no sockets here, but we do this to avoid a compiler
// warning: "Include winsock2 before windows.h"
#include <winsock2.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <iostream>
#include <cassert>
#include <cmath>

#include "kc1fsz-tools/Log.h"

#include "Message.h"
#include "LineRadioWin.h"
#include "Transcoder_SLIN_48K.h"

using namespace std;

namespace kc1fsz {

LineRadioWin::LineRadioWin(Log& log, Clock& clock, MessageConsumer& consumer, 
    unsigned busId, unsigned callId, unsigned destBusId, unsigned destCallId)
:   LineRadio(log, clock, consumer, busId, callId, destBusId, destCallId) {

    // Get the audio threads running
    _run = true;
    unsigned int threadID;
    _playThreadH = (HANDLE)_beginthreadex(NULL, 0, _playThread, (void*)this, 0, &threadID);
    _captureThreadH = (HANDLE)_beginthreadex(NULL, 0, _captureThread, (void*)this, 0, &threadID);
}

LineRadioWin::~LineRadioWin() {
    _run = false;
    WaitForSingleObject(_playThreadH, INFINITE);
    WaitForSingleObject(_captureThreadH, INFINITE);
}

int LineRadioWin::open(const char* deviceName, const char* hidName) {    
    return 0;
}

void LineRadioWin::close() {
}
    
void LineRadioWin::setCos(bool cos) {
    // #### TODO: THREAD SAFE?
    _cos = cos;
}

/**
 * This is activity that happens on the "main" thread.
 */
bool LineRadioWin::run2() {

    // Deal with captured audio streaming in from the audio thread
    unsigned frameCount = 0;
    PCM16Frame frame;

    while (_captureQueue.try_pop(frame)) {

        assert(frame.size() == BLOCK_SIZE_48K);

        uint64_t now = _clock.timeUs();
        if (_captureCount == 0)
            _captureStartUs = now;
        // #### TODO: WHAT IS THIS RELATIVE TO?
        uint64_t idealNow = _captureStartUs + (_captureCount * BLOCK_PERIOD_MS * 1000);
        _captureCount++;

        // Transition detect, the beginning of a capture "run"
        if (!_capturing) {
            _capturing = true;
            // Force a synchronization of the actual system clock and 
            // the timestamps that will be put on the generated frames.
            _captureStartUs = now;
            _captureCount = 0;
            idealNow = now;
            _captureStart();
        }

        _lastCapturedFrameMs = _clock.time();

        // Here is where statistics and possibly recording happens
        _analyzeCapturedAudio(frame.data(), BLOCK_SIZE_48K);

        // Here is where the actual processing of the new block happens
        _processCapturedAudio(frame.data(), BLOCK_SIZE_48K, now, idealNow);

        frameCount++;
    }

    return frameCount > 0;
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
    int16_t pcm48k_2[BLOCK_SIZE_48K];
    Transcoder_SLIN_48K transcoder;
    transcoder.decode(msg.raw(), msg.size(), pcm48k_2, BLOCK_SIZE_48K);

    // Here is where statistical analysis and/or local recording can take 
    // place for diagnostic purposes.
    _analyzePlayedAudio(pcm48k_2, BLOCK_SIZE_48K);

    // Pass the frame off to the worker thread
    _playQueue.push(PCM16Frame(pcm48k_2, BLOCK_SIZE_48K));

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
    if (_capturing &&
        _clock.isPast(_lastCapturedFrameMs + _captureSilenceIntervalMs)) {
        _capturing = false;
        _captureEnd();
    }
}

void LineRadioWin::oneSecTick() {
}

// ===== PLAY THREAD ==========================================================

unsigned LineRadioWin::_playThread(void* o) {    
    return ((LineRadioWin*)o)->_playThread();
}

// #### TODO: CLEAN UP ASSERTS
// #### TODO: MAKE IT SO WE CAN OPEN/CLOSE MULTIPLE TIMES (i.e. DEVICE CHANGE)

unsigned LineRadioWin::_playThread() {

    _log.info("LineRadioWin play thread start");

    HRESULT hr;

    IMMDeviceEnumerator* deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)(&deviceEnumerator));
    assert(hr == S_OK);

    IMMDevice* playAudioDevice;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &playAudioDevice);
    assert(hr == S_OK);

    deviceEnumerator->Release();

    IAudioClient2* playAudioClient;
    hr = playAudioDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr, (LPVOID*)(&playAudioClient));
    assert(hr == S_OK);

    playAudioDevice->Release();
    
    WAVEFORMATEX playMixFormat = {};
    playMixFormat.wFormatTag = WAVE_FORMAT_PCM;
    playMixFormat.nChannels = 1;
    playMixFormat.nSamplesPerSec = 48000;
    playMixFormat.wBitsPerSample = 16;
    playMixFormat.nBlockAlign = (playMixFormat.nChannels * playMixFormat.wBitsPerSample) / 8;
    playMixFormat.nAvgBytesPerSec = playMixFormat.nSamplesPerSec * playMixFormat.nBlockAlign;

    // This controls the size of the buffer, and also how long buffers get filled
    // before audio can be heard.

    // This is hundred nanoseconds
    const int64_t REFTIMES_PER_SEC = 50000000; 
    const int64_t bufferSizeUs = REFTIMES_PER_SEC / 10;
    const int64_t bufferSizeMs = bufferSizeUs / 1000;
    const REFERENCE_TIME requestedSoundBufferDuration = REFTIMES_PER_SEC;

    const DWORD initStreamFlags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                            | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                            | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );

    hr = playAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                 initStreamFlags, 
    // The buffer capacity as a time value. This parameter is of type REFERENCE_TIME 
    // and is expressed in 100-nanosecond units. This parameter contains the buffer 
    // size that the caller requests for the buffer that the audio application will 
    // share with the audio engine (in shared mode) or with the endpoint device (in 
    // exclusive mode). 
                                 requestedSoundBufferDuration, 
                                 0, 
                                 &playMixFormat, 
                                 nullptr);
    assert(hr == S_OK);

    IAudioRenderClient* renderClient;
    hr = playAudioClient->GetService(__uuidof(IAudioRenderClient), (LPVOID*)(&renderClient));
    assert(hr == S_OK);

    // Get the size of the render buffer
    UINT32 playBufferSize;
    hr = playAudioClient->GetBufferSize(&playBufferSize);
    assert(hr == S_OK);
    _log.info("Play buffer size %lld (ms) %u (samples)", bufferSizeMs, playBufferSize);

    hr = playAudioClient->Start();
    assert(hr == S_OK);

    // The thread event loop

    while (_run) {

        // Any outgoing audio?
        if (!_playQueue.empty()) {

            // If there is something waiting to play then check to see if there
            // is enough room in the audio hardware for it.
            //
            // This method retrieves a padding value that indicates the amount of valid, 
            // unread data that the endpoint buffer currently contains. A rendering 
            // application can use the padding value to determine how much new data it can 
            // safely write to the endpoint buffer without overwriting previously written 
            // data that the audio engine has not yet read from the buffer.
            //
            UINT32 bufferPadding;
            hr = playAudioClient->GetCurrentPadding(&bufferPadding);
            assert(hr == S_OK);
            // Calculate the space that we can write into
            UINT32 frameCount = playBufferSize - bufferPadding;
            //_log.info("Space used %d %", (100 * bufferPadding) / playBufferSize);

            // Is there room for a full frame in the hardware?
            if (frameCount < BLOCK_SIZE_48K) {
                break;
                // Do nothing
            }
            else {
                // Double-check that there is actually something to be played
                PCM16Frame frame;
                if (_playQueue.try_pop(frame)) {

                    unsigned blockSize = BLOCK_SIZE_48K;

                    // Allocate a render buffer in the hardware for a full block
                    int16_t* buffer = 0;
                    hr = renderClient->GetBuffer(blockSize, (BYTE**)(&buffer));
                    assert(hr == S_OK);

                    // NOTE: This is flowing directly into the hardware
                    for (unsigned i = 0; i < blockSize; i++)
                        *buffer++ = frame.data()[i];

                    // IMPORTANT: Release when finished
                    hr = renderClient->ReleaseBuffer(blockSize, 0);
                    assert(hr == S_OK);
                }
            }
        }
        else {
            Sleep(5);
        }
    }

    playAudioClient->Stop();
    playAudioClient->Release();

    return 0;
}

// ===== CAPTURE THREAD ========================================================

unsigned LineRadioWin::_captureThread(void* o) {    
    return ((LineRadioWin*)o)->_captureThread();
}

// #### TODO: CLEAN UP ASSERTS
// #### TODO: MAKE IT SO WE CAN OPEN/CLOSE MULTIPLE TIMES (i.e. DEVICE CHANGE)

unsigned LineRadioWin::_captureThread() {

    _log.info("LineRadioWin capture thread start");

    HRESULT hr;

    IMMDeviceEnumerator* deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)(&deviceEnumerator));
    assert(hr == S_OK);

    IMMDevice* captureAudioDevice;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &captureAudioDevice);
    assert(SUCCEEDED(hr));

    deviceEnumerator->Release();

    IAudioClient2* captureAudioClient;
    hr = captureAudioDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr, (LPVOID*)(&captureAudioClient));
    assert(hr == S_OK);

    captureAudioDevice->Release();
    
    WAVEFORMATEX captureMixFormat = {};
    captureMixFormat.wFormatTag = WAVE_FORMAT_PCM;
    captureMixFormat.nChannels = 1;
    captureMixFormat.nSamplesPerSec = 48000;
    captureMixFormat.wBitsPerSample = 16;
    captureMixFormat.nBlockAlign = (captureMixFormat.nChannels * captureMixFormat.wBitsPerSample) / 8;
    captureMixFormat.nAvgBytesPerSec = captureMixFormat.nSamplesPerSec * captureMixFormat.nBlockAlign;

    // This controls the size of the capture buffer, and also how much lag 
    // is built into the capture process.  We have left enough room for a 
    // few full frames in case things fall behind.

    // This is hundred nanoseconds
    const int64_t REFTIMES_PER_SEC = 400000; 
    const int64_t bufferSizeUs = REFTIMES_PER_SEC / 10;
    const int64_t bufferSizeMs = bufferSizeUs / 1000;
    const REFERENCE_TIME requestedSoundBufferDuration = REFTIMES_PER_SEC;

    const DWORD initStreamFlags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                            | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                            | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );

    hr = captureAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                 initStreamFlags, 
                                 requestedSoundBufferDuration, 
                                 0, 
                                 &captureMixFormat, 
                                 nullptr);
    assert(hr == S_OK); 

    IAudioCaptureClient* captureClient;
    hr = captureAudioClient->GetService(__uuidof(IAudioCaptureClient), (LPVOID*)(&captureClient));
    assert(hr == S_OK);

    UINT32 captureBufferSize;
    hr = captureAudioClient->GetBufferSize(&captureBufferSize);
    assert(hr == S_OK);
    _log.info("Capture buffer size %lld (ms) %u (samples)", bufferSizeMs, captureBufferSize);

    hr = captureAudioClient->Start();
    assert(hr == S_OK);
  
    while (_run) {

        // NOTE: Most of this work happens regardless of whether the COS 
        // signal is active. We do this to keep all of the audio capture buffers
        // clear. But most of the time the audio is just discarded, or possibly
        // just used for CTCSS/squelch detection.

        // Work hard until a complete frame is captured
        while (_captureBufferSize < BLOCK_SIZE_48K) {
       
            BYTE* captureBuffer;
            UINT32 nFrames = 0;
            DWORD flags;

            hr = captureClient->GetBuffer(&captureBuffer, &nFrames, &flags, NULL, NULL);
            assert(SUCCEEDED(hr));

            if (nFrames > 0) {
                // Not sure if this ever happens
                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    for (unsigned i = 0; i < nFrames; i++)
                        _captureBuffer[_captureBufferSize++] = 0;
                } else {
                    // #### TODO: NEED TO ENFORCE MAX_BUFFER_SIZE
                    // Pull the information out of the hardware
                    int16_t* pcm = (int16_t*)captureBuffer;
                    for (unsigned i = 0; i < nFrames; i++)
                        _captureBuffer[_captureBufferSize++] = pcm[i];
                }

                assert(_captureBufferSize <= MAX_CAPTURE_BUFFER_SIZE);
            }

            hr = captureClient->ReleaseBuffer(nFrames);
            assert(SUCCEEDED(hr));
        }

        // Send as many full frames back if possible
        while (_captureBufferSize >= BLOCK_SIZE_48K) {
            // #### TODO: THREAD SAFETY
            // #### TODO: MORE SOPHSITICATED COS
            // Send the oldest complete block of audio IF THE COS IS ENABLED
            if (_cos) {
                //_parrotQueue.push(PCM16Frame(_captureBuffer, BLOCK_SIZE_48K));
                _captureQueue.push(PCM16Frame(_captureBuffer, BLOCK_SIZE_48K));
            }

            // If there is anything left then shift left - overlapping move
            if (_captureBufferSize > BLOCK_SIZE_48K) {
                ::memmove(_captureBuffer, _captureBuffer + BLOCK_SIZE_48K, 
                    (_captureBufferSize - BLOCK_SIZE_48K) * sizeof(int16_t));
            }
            _captureBufferSize -= BLOCK_SIZE_48K;
        }

        // #### Temporary parrot
        if (_cos && !_previousCos) {
            _log.info("KEY DOWN");
        }
        else if (!_cos && _previousCos) {
            _log.info("KEY UP");
            //PCM16Frame r;
            //while (_parrotQueue.try_pop(r))
            //    _playQueue.push(r);
        }
        _previousCos = _cos;

        // WARNING: on Windows this will typically result in a 15-20ms sleep.
        // Buffers have been configured so that we can get away with sleeping
        // for a full audio frame period.
        Sleep(1);
    }

    captureAudioClient->Stop();
    captureAudioClient->Release();

    return 0;
}

}
