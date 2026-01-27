#include <iostream>
#include <cstring>

#include "miniaudio.h"
#include "audio_engine.hpp"


AudioEngine::AudioEngine(ma_uint32 capture_device_index) : device_index(capture_device_index) {}


/**
 * Initializes the audio engine.
 * 
 * What is initialized:
 *  - miniaudio context for device discovery
 *  - ring buffer for decoupling capture from processing
 *  - audio device for reading mono float32 samples
 */
AudioEngine::InitResult AudioEngine::init() {
    // initialize audio context, for querying available devices and backend state
    if (ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS) {
        std::cerr << "failed to initialize audio context \n";
        return InitResult::context_failure;
    }
    context_initialized = true;

    // initialize a ring buffer to decouple real-time audio capture from downstream processing
    ma_result rb_res = ma_pcm_rb_init(ma_format_f32, CHANNELS, SAMPLE_RATE * RECORD_SEC, nullptr, nullptr, &ring_buffer);
    if (rb_res != MA_SUCCESS) {
        std::cerr << "ma_pcm_rb_init failed: " << ma_result_description(rb_res) << "\n";
        return InitResult::ring_buffer_failure;
    }
    ring_buffer_initialized = true;

    // query all detected audio devices, for selecting the capture device by index
    ma_device_info* capture_infos;
    ma_uint32 capture_count;
    ma_context_get_devices(&context, nullptr, nullptr, &capture_infos, &capture_count);

    if (device_index >= capture_count) {
        std::cerr << "Invalid capture device index: " << device_index << "\n";
        return InitResult::device_failure;
    }

    // device settings (mono, float32)
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = CHANNELS;
    config.capture.pDeviceID = &capture_infos[device_index].id;
    config.sampleRate = SAMPLE_RATE;

    // callback is called on real-time audio thread, use pUserData to pass the engine instance to the callback
    config.dataCallback = data_callback;
    config.pUserData = this;

    // create the device based on the configured parameters
    ma_result dev_res = ma_device_init(&context, &config, &audio_device);
    if (dev_res != MA_SUCCESS) {
        std::cerr << "ma_device_init failed: " << ma_result_description(dev_res) << "\n";
        return InitResult::device_failure;
    }
    device_initialized = true;

    init_result = InitResult::success;
    return init_result;
}


/**
 * Start capturing audio.
 * - invokes the audio callback on a real-time audio thread
 */
void AudioEngine::start() {
    if (init_result == InitResult::success) {
        ma_device_start(&audio_device);
    }
}


/**
 * Stop capturing audio.
 * - no more callbacks once executed
 */
void AudioEngine::stop() {
    ma_device_stop(&audio_device);
}


/**
 * Retrieves a pointer to the ring buffer
 * - used for diagnostics or to extract straight from the rb, but prefer reading chunks
 */
ma_pcm_rb *AudioEngine::get_ring_buffer() {
    return &ring_buffer;
}


/**
 * Audio callback invoked on a real-time audio thread.
 * - copy captured audio into the ring buffer
 */
void AudioEngine::data_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
    // capture only, explicitly suppress unused output variable warnings
    (void)output;
    if (!input) return;

    // retrieve the audio engine associated with the device on this thread
    AudioEngine *engine = static_cast<AudioEngine*>(device->pUserData);

    ma_uint32 frames = frame_count;
    float *dest = nullptr;

    // write frames to the ring buffer based on the amount of space it currently has available
    if (ma_pcm_rb_acquire_write(&engine->ring_buffer, &frames, (void **)&dest) == MA_SUCCESS && frames > 0) {
        memcpy(dest, input, frames * sizeof(float));
        ma_pcm_rb_commit_write(&engine->ring_buffer, frames);
    }
}


/**
 * Read a fixed-size audio chunk from the ring buffer.
 * - returns true only when exactly `frames` samples are available
 * - partial reads are discarded to preserve stable processing windows
 */
bool AudioEngine::read_chunk(float* out, ma_uint32 frames)
{
    ma_uint32 available = frames;
    float* src = nullptr;

    ma_result res = ma_pcm_rb_acquire_read(&ring_buffer, &available, (void**)&src);

    if (res != MA_SUCCESS || available < frames) {
        if (res == MA_SUCCESS && available > 0) {
            ma_pcm_rb_commit_read(&ring_buffer, available);
        }
        return false;
    }

    memcpy(out, src, frames * sizeof(float));
    ma_pcm_rb_commit_read(&ring_buffer, frames);

    return true;
}


/**
 * Device enumeration (static)
 * - Lists all detected audio capture devices without needing an engine instance
 */
std::vector<std::string> AudioEngine::get_capture_devices() {
    ma_context ctx;
    std::vector<std::string> devices;

    if (ma_context_init(nullptr, 0, nullptr, &ctx) != MA_SUCCESS) return devices; 

    ma_device_info* infos;
    ma_uint32 count;
    ma_context_get_devices(&ctx, nullptr, nullptr, &infos, &count);

    for (ma_uint32 i = 0; i < count; ++i) {
        devices.emplace_back(infos[i].name);
    }

    ma_context_uninit(&ctx);
    return devices;
}


/**
 * Destroy the audio engine and release all resources used
 */
AudioEngine::~AudioEngine() {
    if (device_initialized) ma_device_uninit(&audio_device);
    if (ring_buffer_initialized) ma_pcm_rb_uninit(&ring_buffer);
    if (context_initialized) ma_context_uninit(&context);
}
