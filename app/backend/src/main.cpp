#include <iostream>
#include <cstring>
#include <portaudio.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 512

typedef struct CallbackData {
    int input_channel_count;
} CallbackData;

inline float max(float a, float b) {
    return a > b ? a : b;
}

int pa_stream_callback(
    const void* input_buffer, 
    void* output_buffer, unsigned long frames_per_buffer, 
    const PaStreamCallbackTimeInfo* time_info, 
    PaStreamCallbackFlags status_flags, 
    void* user_data 
) {
    if (input_buffer == nullptr) {
        std::cout << "Input buffer is null";
        return 0;
    }
    
    const int16_t* input = static_cast<const int16_t*>(input_buffer);
    (void)output_buffer;

    CallbackData* data = static_cast<CallbackData*>(user_data);
    int input_channels = data->input_channel_count;

    int display_size = 100;
    std::cout << "\r";

    float left_volume = 0;
    float right_volume = 0;

    if (input_channels == 1) {
        left_volume = right_volume = 0;
        for (unsigned long i = 0; i < frames_per_buffer; i++) {
            float v = std::abs(input[i]) / 32768.0f;
            left_volume = max(left_volume, v);
            right_volume = max(right_volume, v);
        }
    } else if (input_channels == 2) {
        left_volume = right_volume = 0;
        for (unsigned long i = 0; i < frames_per_buffer * 2; i += 2) {
            float l = std::abs(input[i]) / 32768.0f;
            float r = std::abs(input[i+1]) / 32768.0f;
            left_volume  = max(left_volume, l);
            right_volume = max(right_volume, r);
        }
    }

    for (int i = 0; i < display_size; i++) {
        float proportion = i / static_cast<float>(display_size);
        if (proportion <= left_volume && proportion <= right_volume) {
            std::cout << "█";
        } else if (proportion <= left_volume) {
            std::cout << "▀";
        } else if (proportion <= right_volume) {
            std::cout << "▄";
        } else {
            std::cout << " ";
        }
    }

    fflush(stdout);

    return 0;
}

int main(int argc, char** argv) {
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cout << "error initializing port audio: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    int num_devices = Pa_GetDeviceCount();
    std::cout << "number of devices: " << num_devices << "\n";

    if (num_devices < 0) {
        std::cout << "error getting device count\n";
        return 1;
    } else if (num_devices == 0) {
        std::cout << "no audio devices detected\n";
        return 0;
    }

    const PaDeviceInfo* device_info;
    for (int i = 0; i < num_devices; i++) {
        device_info = Pa_GetDeviceInfo(i);
        std::cout << "device: " << i << "\n";
        std::cout << "\tname: " << device_info->name << "\n";
        std::cout << "\tmax input channels: " << device_info->maxInputChannels << "\n";
        std::cout << "\tmax output channels: " << device_info->maxOutputChannels << "\n";
        std::cout << "\tdefault sample rate: " << device_info->defaultSampleRate << "\n";
    }

    int device = 12;

    PaStreamParameters input_parameters;
    PaStreamParameters output_parameters;

    memset(&input_parameters, 0, sizeof(input_parameters));
    input_parameters.channelCount = 2;
    input_parameters.device = device;
    input_parameters.hostApiSpecificStreamInfo = NULL;
    input_parameters.sampleFormat = paInt16;
    input_parameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

    memset(&output_parameters, 0, sizeof(output_parameters));
    output_parameters.channelCount = Pa_GetDeviceInfo(device)->maxOutputChannels;
    output_parameters.device = device;
    output_parameters.hostApiSpecificStreamInfo = NULL;
    output_parameters.sampleFormat = paInt16;
    output_parameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

    CallbackData callback_data;
    callback_data.input_channel_count = 2;

    PaStream* stream;
    err = Pa_OpenStream(&stream, &input_parameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, pa_stream_callback, &callback_data);
    if (err != paNoError) {
        std::cout << "error opening audio stream: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cout << "error starting audio stream: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    Pa_Sleep(10 * 1000);

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cout << "error stopping audio stream: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cout << "error closing audio stream: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    err = Pa_Terminate();
    if (err != paNoError) {
        std::cout << "error terminating port audio: " << Pa_GetErrorText(err) << "\n";
        return 1;
    }

    return 0;
}