#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <string>
#include <vector>

#include "miniaudio.h"

static constexpr ma_uint32 SAMPLE_RATE = 48000;     // 48khz 
static constexpr ma_uint32 CHANNELS = 1;            // mono audio
static constexpr ma_uint32 RECORD_SEC = 10;         // 10 seconds of recording
static constexpr ma_uint32 CHUNK_FRAMES = 1024;     // audio samples to process at a time

class AudioEngine {
public:
    enum class InitResult {
        success = 0,
        ring_buffer_failure = 1,
        device_failure = 2,
        context_failure = 3,
    };

    AudioEngine(ma_uint32 capture_device_index);
    ~AudioEngine();

    InitResult init();
    void start();
    void stop();

    bool read_chunk(float* out, ma_uint32 frames);

    ma_pcm_rb* get_ring_buffer();

    static std::vector<std::string> get_capture_devices();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

private:
    static void data_callback(
        ma_device* device,
        void* output,
        const void* input,
        ma_uint32 frame_count
    );

private:
    ma_context context{};
    ma_device audio_device{};
    ma_pcm_rb ring_buffer{};
    ma_uint32 device_index;
    InitResult init_result = InitResult::context_failure;

    bool context_initialized = false;
    bool device_initialized = false;
    bool ring_buffer_initialized = false;
};

#endif // AUDIO_ENGINE_H