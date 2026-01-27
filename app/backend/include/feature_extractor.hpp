#ifndef H_FEATURE_EXTRACTOR
#define H_FEATURE_EXTRACTOR

#include <vector>
#include <cstdint>
#include "kiss_fftr.h"


struct FeatureExtractorConfig {
    uint32_t sample_rate = 48000;
    uint32_t fft_size = 1024;
    uint32_t hop_size = 512;
    uint32_t num_mels = 40;

    float fmin = 0.0f;
    float fmax = -1.0f;
    float top_db = 80.0f;
    float eps = 1e-10f;

    float silence_threshold = 1e-4f;
};


class FeatureExtractor {
public:
    FeatureExtractor(const FeatureExtractorConfig& config);
    ~FeatureExtractor();

    std::vector<std::vector<float>> process_samples(const float* input, uint32_t num_samples);

private:
    void build_hann_window();
    void build_mel_filterbank();
    std::vector<float> compute_frame(const float* frame);

    static float hz_to_mel(float hz);
    static float mel_to_hz(float mel);

private:
    FeatureExtractorConfig config;

    std::vector<float> window;
    std::vector<float> fft_input;
    std::vector<kiss_fft_cpx> fft_output;
    std::vector<float> power_spectrum;
    std::vector<std::vector<float>> mel_filterbank;

    std::vector<float> overlap_buffer;

    kiss_fftr_cfg fft_config;
};

#endif // H_FEATURE_EXTRACTOR
