#include <cmath>
#include <algorithm>
#include <cstring>

#include "feature_extractor.hpp"


float FeatureExtractor::hz_to_mel(float hz) {
    return 2595.0f * std::log10(1.0f + hz / 700.0f);
}

float FeatureExtractor::mel_to_hz(float mel) {
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}


FeatureExtractor::FeatureExtractor(const FeatureExtractorConfig& c) : config(c) {
    if (config.fmax < 0.0f) config.fmax = 0.5f * config.sample_rate;

    window.resize(config.fft_size);
    fft_input.resize(config.fft_size, 0.0f);
    fft_output.resize(config.fft_size / 2 + 1);
    power_spectrum.resize(config.fft_size / 2 + 1);

    overlap_buffer.reserve(config.fft_size);

    build_hann_window();
    build_mel_filterbank();

    fft_config = kiss_fftr_alloc(config.fft_size, 0, nullptr, nullptr);
}


FeatureExtractor::~FeatureExtractor() {
    free(fft_config);
}


void FeatureExtractor::build_hann_window() {
    for (uint32_t i = 0; i < config.fft_size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (config.fft_size - 1)));
    }
}


void FeatureExtractor::build_mel_filterbank() {
    mel_filterbank.assign(config.num_mels, std::vector<float>(config.fft_size / 2 + 1, 0.0f));

    float mel_min = hz_to_mel(config.fmin);
    float mel_max = hz_to_mel(config.fmax);

    std::vector<float> mel_points(config.num_mels + 2);
    std::vector<int> bin_points(config.num_mels + 2);

    int max_bin = config.fft_size / 2;

    for (uint32_t i = 0; i < mel_points.size(); ++i) {
        mel_points[i] = mel_min + (mel_max - mel_min) * i / (config.num_mels + 1);

        float hz = mel_to_hz(mel_points[i]);
        bin_points[i] = std::min(max_bin, int(std::floor((config.fft_size + 1) * hz / config.sample_rate)));
    }

    for (uint32_t m = 1; m <= config.num_mels; ++m) {
        for (int k = bin_points[m - 1]; k < bin_points[m]; ++k) {
            mel_filterbank[m - 1][k] = float(k - bin_points[m - 1]) / (bin_points[m] - bin_points[m - 1]);
        }

        for (int k = bin_points[m]; k < bin_points[m + 1]; ++k) {
            mel_filterbank[m - 1][k] = float(bin_points[m + 1] - k) / (bin_points[m + 1] - bin_points[m]);
        }

        float sum = 0.0f;
        for (float v : mel_filterbank[m - 1]) {
            sum += v;
        } 
        if (sum > 0.0f) {
            for (float& v : mel_filterbank[m - 1]) {
                v /= sum;
            }
        } 
    }
}


std::vector<float> FeatureExtractor::compute_frame(const float* frame) {
    float energy_sum = 0.0f;

    for (uint32_t i = 0; i < config.fft_size; ++i) {
        fft_input[i] = frame[i] * window[i];
        energy_sum += std::abs(frame[i]);
    }

    std::vector<float> mel(config.num_mels, -config.top_db);

    if (energy_sum < config.silence_threshold)
        return mel;

    kiss_fftr(fft_config, fft_input.data(), fft_output.data());

    for (size_t i = 0; i < power_spectrum.size(); ++i) {
        float re = fft_output[i].r;
        float im = fft_output[i].i;
        power_spectrum[i] = re * re + im * im;
    }

    float max_db = -1e9f;

    for (uint32_t m = 0; m < config.num_mels; ++m) {
        float sum = 0.0f;
        for (size_t k = 0; k < power_spectrum.size(); ++k)
            sum += power_spectrum[k] * mel_filterbank[m][k];

        float db = 10.0f * std::log10(sum + config.eps);
        mel[m] = db;
        max_db = std::max(max_db, db);
    }

    float min_db = max_db - config.top_db;
    for (float& v : mel)
        v = std::max(v, min_db);

    return mel;
}


std::vector<std::vector<float>> FeatureExtractor::process_samples(const float* input, uint32_t num_samples) {
    std::vector<std::vector<float>> frames;

    overlap_buffer.insert(overlap_buffer.end(), input, input + num_samples);

    while (overlap_buffer.size() >= config.fft_size) {
        frames.push_back(compute_frame(overlap_buffer.data()));

        overlap_buffer.erase(overlap_buffer.begin(), overlap_buffer.begin() + config.hop_size);
    }

    return frames;
}
