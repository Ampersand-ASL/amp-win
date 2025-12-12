
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h> // for sin()
#include <stdint.h>

#include <iostream>

#include "kc1fsz-tools/linux/StdClock.h"

using namespace std;
using namespace kc1fsz;

// Adapted from:
// https://gist.github.com/kevinmoran/2e673695058c7bc32fb5172848900db5
// (I see no copyright message on this file)

int main(int, const char**) {

    StdClock clock;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
    assert(hr == S_OK);

    IMMDeviceEnumerator* deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)(&deviceEnumerator));
    assert(hr == S_OK);

    IMMDevice* audioDevice;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);
    assert(hr == S_OK);

    deviceEnumerator->Release();

    IAudioClient2* audioClient;
    hr = audioDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr, (LPVOID*)(&audioClient));
    assert(hr == S_OK);

    audioDevice->Release();
    
    WAVEFORMATEX mixFormat = {};
    mixFormat.wFormatTag = WAVE_FORMAT_PCM;
    //mixFormat.nChannels = 2;
    mixFormat.nChannels = 1;
    mixFormat.nSamplesPerSec = 48000;
    mixFormat.wBitsPerSample = 16;
    mixFormat.nBlockAlign = (mixFormat.nChannels * mixFormat.wBitsPerSample) / 8;
    mixFormat.nAvgBytesPerSec = mixFormat.nSamplesPerSec * mixFormat.nBlockAlign;

    // This controls the size of the buffer, and also how long buffers get filled
    // before audio can be heard.

    const int64_t REFTIMES_PER_SEC = 5000000; // hundred nanoseconds
    REFERENCE_TIME requestedSoundBufferDuration = REFTIMES_PER_SEC;

    DWORD initStreamFlags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                            | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                            | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );

    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                 initStreamFlags, 
    // The buffer capacity as a time value. This parameter is of type REFERENCE_TIME 
    // and is expressed in 100-nanosecond units. This parameter contains the buffer 
    // size that the caller requests for the buffer that the audio application will 
    // share with the audio engine (in shared mode) or with the endpoint device (in 
    // exclusive mode). 
                                 requestedSoundBufferDuration, 
                                 0, 
                                 &mixFormat, 
                                 nullptr);
    assert(hr == S_OK);

    IAudioRenderClient* audioRenderClient;
    hr = audioClient->GetService(__uuidof(IAudioRenderClient), (LPVOID*)(&audioRenderClient));
    assert(hr == S_OK);

    // Get the size of the render buffer
    UINT32 bufferSize;
    hr = audioClient->GetBufferSize(&bufferSize);
    assert(hr == S_OK);
    cout << "Buffer Size " << bufferSize << endl;

    REFERENCE_TIME defP;
    REFERENCE_TIME minP;
    audioClient->GetDevicePeriod(&defP, &minP);
    cout << "Def (ms) " << defP / (10 * 1000) << endl;
    cout << "Min (ms) " << minP / (10 * 1000) << endl;

    long startTime = clock.timeUs();

    hr = audioClient->Start();
    assert(hr == S_OK);

    double playbackTime = 0.0;
    const float TONE_HZ = 440;
    const int16_t TONE_VOLUME = 3000;
    // How long the tone should play
    unsigned targetSamples = 4 * mixFormat.nSamplesPerSec;
    unsigned samples = 0;
   
    // 96000 / 4 = 24000 samples
    // 0.5 seconds at 48000

    while (samples < targetSamples)
    {
        // This method retrieves a padding value that indicates the amount of valid, 
        // unread data that the endpoint buffer currently contains. A rendering 
        // application can use the padding value to determine how much new data it can 
        // safely write to the endpoint buffer without overwriting previously written 
        // data that the audio engine has not yet read from the buffer.                 
        UINT32 bufferPadding;
        hr = audioClient->GetCurrentPadding(&bufferPadding);
        assert(hr == S_OK);

        // Calculate the space that we can write into
        UINT32 frameCount = bufferSize - bufferPadding;
        //if (frameCount > 0)
        //    cout << "Frame Count " << samples << " " << frameCount << endl;

        // Allocate a render buffer that matches the amount of free space available
        int16_t* buffer;
        hr = audioRenderClient->GetBuffer(frameCount, (BYTE**)(&buffer));
        assert(hr == S_OK);

        // Fill it up
        for(UINT32 frameIndex = 0; frameIndex < frameCount; frameIndex++)
        {
            float amplitude = (float)sin(playbackTime*2*M_PI*TONE_HZ);
            int16_t y = (int16_t)(TONE_VOLUME * amplitude);

            *buffer++ = y; // left
            //*buffer++ = y; // right

            playbackTime += 1.f / mixFormat.nSamplesPerSec;
            samples++;
        }

        // IMPORTANT: Release when finished
        hr = audioRenderClient->ReleaseBuffer(frameCount, 0);
        assert(hr == S_OK);

        /*
        // Get playback cursor position
        IAudioClock* audioClock;
        audioClient->GetService(__uuidof(IAudioClock), (LPVOID*)(&audioClock));
        UINT64 audioPlaybackFreq;
        UINT64 audioPlaybackPos;
        audioClock->GetFrequency(&audioPlaybackFreq);
        audioClock->GetPosition(&audioPlaybackPos, 0);
        audioClock->Release();
        UINT64 audioPlaybackPosInSeconds = audioPlaybackPos/audioPlaybackFreq;
        UINT64 audioPlaybackPosInSamples = audioPlaybackPosInSeconds*mixFormat.nSamplesPerSec;
        */
    }

    long endTime = clock.timeUs();
    long dur = endTime - startTime;
    cout << "Dur " << dur << endl; 

    // Drain the audio buffer so that we hear the whole thing
    while (true) {
        UINT32 bufferPadding;
        hr = audioClient->GetCurrentPadding(&bufferPadding);
        assert(hr == S_OK);
        if (bufferPadding == 0)
            break;
    }

    audioClient->Stop();
    audioClient->Release();
    audioRenderClient->Release();

    return 0;
}