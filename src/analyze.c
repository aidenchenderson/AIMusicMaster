#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include "wave.h"

// FFT foundamental requirement is the equally time spacing of samples and their congruence
// might not be ideal

void analyze_wav(WAVFile *wav) {
    int num_samples = wav->header.data_subchunk.subchunk2_size / (wav->header.fmt_subchunk.bits_per_sample / 8);
    double *in = fftw_malloc(sizeof(double) * num_samples);
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * (num_samples/2 + 1));
    fftw_plan p = fftw_plan_dft_r2c_1d(num_samples, in, out, FFTW_ESTIMATE);

    // Convert audio samples to double [-1,1]
    for (int i = 0; i < num_samples; i++) {
        int16_t sample = ((int16_t*)wav->data)[i];
        in[i] = sample / 32768.0;
    }

    fftw_execute(p);

    // frequency calculation (chatgpt)
    // Find peaks in out[] → frequencies → notes
    for (int i = 0; i < num_samples/2 + 1; i++) {
        double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
        double freq = i * wav->header.fmt_subchunk.sample_rate / num_samples;
        if (magnitude > 0.1) printf("Frequency: %.1f Hz\n", freq);
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}
