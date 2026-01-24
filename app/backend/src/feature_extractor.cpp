#include <cmath>
#include <algorithm>
#include <cstring> 

#include "feature_extractor.hpp"

// helper functions to convert between hertz and mel scale to match librosa
inline float FeatureExtractor::hz_to_mel(float hz) {
    return 2595.0f * std::log10(1.0f + hz / 700.0f);
}

inline float FeatureExtractor::mel_to_hz(float mel) {
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}


FeatureExtractor::FeatureExtractor(const FeatureExtractorConfig& c) : config(c) {
    if (config.fmax < 0.0f) config.fmax = 0.5f * config.sample_rate;

    window.resize(config.chunk_size);
    fft_input.resize(config.fft_size, 0.0f);
    fft_output.resize(config.fft_size / 2 + 1);
    power_spectrum.resize(config.fft_size / 2 + 1);
    mel_output.resize(config.num_mels);

    build_hann_window();
    build_mel_filterbank();

    fft_config = kiss_fftr_alloc(config.fft_size, 0, nullptr, nullptr);
}


// destroy the feature extractor and release all resources used
FeatureExtractor::~FeatureExtractor() {
    free(fft_config);
}


// hann window: w[n] = 0.5 * (1 - cos(2*pi*n/(N-1)))
// used to reduce spectral leakage
void FeatureExtractor::build_hann_window() {
    for (uint32_t i = 0; i < config.chunk_size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (config.chunk_size - 1)));
    }
}


// 
void FeatureExtractor::build_mel_filterbank() {
    mel_filterbank.resize(config.num_mels);

    float mel_min = hz_to_mel(config.fmin);
    float mel_max = hz_to_mel(config.fmax);

    std::vector<float> mel_points(config.num_mels + 2);
    std::vector<int> bin_points(config.num_mels + 2);

    for (uint32_t i = 0; i < config.num_mels + 2; ++i) {
        mel_points[i] = mel_min + i * (mel_max - mel_min) / (config.num_mels + 1);
        float hz = mel_to_hz(mel_points[i]);
        bin_points[i] = static_cast<int>(std::floor((config.fft_size + 1) * hz / config.sample_rate));
    }

    for (uint32_t m = 1; m <= config.num_mels; ++m) {
        mel_filterbank[m - 1].assign(config.fft_size / 2 + 1, 0.0f);

        for (int k = bin_points[m - 1]; k < bin_points[m]; ++k) {
            mel_filterbank[m - 1][k] = float(k - bin_points[m - 1]) / (bin_points[m] - bin_points[m - 1]);
        }

        for (int k = bin_points[m]; k < bin_points[m + 1]; ++k) {
            mel_filterbank[m - 1][k] = float(bin_points[m + 1] - k) / (bin_points[m + 1] - bin_points[m]);
        }
    }
}


const std::vector<float>& FeatureExtractor::compute_mel_spectrogram(const float* audio_chunk) {
    std::fill(fft_input.begin(), fft_input.end(), 0.0f);
    for (uint32_t i = 0; i < config.chunk_size; ++i) {
        fft_input[i] = audio_chunk[i] * window[i];
    }

    kiss_fftr(fft_config, fft_input.data(), fft_output.data());

    for (size_t i = 0; i < power_spectrum.size(); ++i) {
        float re = fft_output[i].r;
        float im = fft_output[i].i;
        power_spectrum[i] = re * re + im * im;
    }

    float max_db = -1e9f;
    for (uint32_t m = 0; m < config.num_mels; ++m) {
        float energy = 0.0f;
        for (size_t k = 0; k < power_spectrum.size(); ++k) {
            energy += power_spectrum[k] * mel_filterbank[m][k];
        }

        mel_output[m] = 10.0f * std::log10(std::max(energy, config.eps));
        max_db = std::max(max_db, mel_output[m]);
    }

    float min_db = max_db - config.top_db;
    for (float& v : mel_output) v = std::max(v, min_db);

    return mel_output;
}
