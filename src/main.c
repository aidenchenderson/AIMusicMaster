#include <stdio.h>
#include "wave.h"

int main() {
    WAVFile wav = WAV_ParseFile("build/test.wav");
    if (!wav.data) return 1;

    // examples
    printf("Sample rate: %u\n", wav.header.fmt_subchunk.sample_rate);
    printf("Channels: %u\n", wav.header.fmt_subchunk.num_channels);
    printf("Bits per sample: %u\n", wav.header.fmt_subchunk.bits_per_sample);

    WAV_Free(&wav);
    return 0;
}
