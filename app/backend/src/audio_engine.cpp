#include <iostream>
#include <cstring>

#include "miniaudio.h"
#include "audio_engine.hpp"

AudioEngine::AudioEngine() {}

AudioEngine::InitResult AudioEngine::init() {
    ma_result res;

    // initialize the ring buffer to store mono audio data
    res = ma_pcm_rb_init(ma_format_f32, 1, 44100, NULL, NULL, &this->ring_buffer);
    if (res != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine: " << ma_result_description(res) << "\n";
        return AudioEngine::InitResult::ring_buffer_failure;
    }

    // set device configuration settings
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.sampleRate = 44100;
    config.dataCallback = data_callback;
    config.pUserData = this;

    // initialize the device with the configuration settings
    res = ma_device_init(NULL, &config, &this->audio_device);
    if (res != MA_SUCCESS) {
        ma_pcm_rb_uninit(&this->ring_buffer);
        std::cerr << "Failed to initialize audio engine: " << ma_result_description(res) << "\n";
        return AudioEngine::InitResult::device_failure;
    }

    this->init_result = AudioEngine::InitResult::success;
    return AudioEngine::InitResult::success;
}

void AudioEngine::start() {
    if (this->init_result != AudioEngine::InitResult::success) {
        std::cerr << "Failed to start audio engine: not initialized\n";
        return;
    }
    ma_device_start(&this->audio_device);
}

void AudioEngine::stop() {
    ma_device_stop(&this->audio_device);
}

ma_pcm_rb *AudioEngine::get_ring_buffer() {
    return &this->ring_buffer;
}

void AudioEngine::data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    AudioEngine* engine = (AudioEngine*) device->pUserData;

    ma_uint32 framesToWrite = frame_count;
    float* bufferOut;

    ma_result res = ma_pcm_rb_acquire_write(&engine->ring_buffer, &framesToWrite, (void**)&bufferOut);
    
    if (res == MA_SUCCESS && framesToWrite > 0) {
        memcpy(bufferOut, input, framesToWrite * sizeof(float));

        ma_pcm_rb_commit_write(&engine->ring_buffer, framesToWrite);
    }
}

AudioEngine::~AudioEngine() {
    // clean up
    if (this->init_result == AudioEngine::InitResult::success) {
        ma_device_uninit(&this->audio_device);
        ma_pcm_rb_uninit(&this->ring_buffer);
    }
}