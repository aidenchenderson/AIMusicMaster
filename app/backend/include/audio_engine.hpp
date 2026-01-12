#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <cstring>
#include "miniaudio.h"

class AudioEngine {
public:
    enum class InitResult {
        success = 0,
        ring_buffer_failure = 1,
        device_failure = 2,
    };

    AudioEngine();
    ~AudioEngine();

    InitResult init();
    void start();
    void stop();

    ma_pcm_rb* get_ring_buffer();

    static void data_callback(
        ma_device*  device, 
        void*       output, 
        const void* input, 
        ma_uint32   frame_count
    );

private:
    ma_device audio_device;
    ma_pcm_rb ring_buffer;
    InitResult init_result;
};

#endif // AUDIO_ENGINE_H