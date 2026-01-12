#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <thread>
#include <chrono>

#include "audio_engine.hpp"

/**
 * hardware integration test
 * verifies both the OS and microphone are accessible and delivering data
 */
TEST_CASE("AudioEngine Hardware Liveliness", "[audio]") {
    AudioEngine engine;
    
    // check for successful engine initialization
    REQUIRE(engine.init() == AudioEngine::InitResult::success);
    
    // start the audio stream and allow the buffer to fill
    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    engine.stop();

    // check that signal data was moved to the ring buffer
    ma_pcm_rb* rb = engine.get_ring_buffer();
    ma_uint32 available = ma_pcm_rb_available_read(rb);
    
    CHECK(available > 0);
}

/**
 * callback logic test 
 * verifies that raw memory is routed to the class instance
 */ 
TEST_CASE("AudioEngine Callback Reality Check", "[audio]") {
    AudioEngine engine;
    REQUIRE(engine.init() == AudioEngine::InitResult::success);

    // fake 100-frames of audio input
    const int frame_count = 100;
    std::vector<float> fake_input(frame_count, 0.8f);

    // fake audio device
    ma_device dummy_device;
    dummy_device.pUserData = &engine; 

    // trigger the callback as if recording is finished
    AudioEngine::data_callback(&dummy_device, nullptr, fake_input.data(), frame_count);

    // verify data is correctly stored in buffer 
    ma_uint32 available = ma_pcm_rb_available_read(engine.get_ring_buffer());
    REQUIRE(available == frame_count);

    float* buffer_in;
    ma_uint32 frames_to_read = available;
    ma_pcm_rb_acquire_read(engine.get_ring_buffer(), &frames_to_read, (void**)&buffer_in);
    
    // check that the data comes out exactly as it went in
    CHECK(buffer_in[0] == 0.8f);
    ma_pcm_rb_commit_read(engine.get_ring_buffer(), frames_to_read);
}