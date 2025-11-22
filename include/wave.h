// WAVE file header format https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/ 
// WAVE file reading structure https://gist.github.com/SteelPh0enix/e44d4a030dd8816309af84809ed75604 

#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <stdint.h>

// header structs
struct riff_header {
    char chunk_id[5]; // should be "RIFF" + null
    uint32_t chunk_size;
    char format[5];   // should be "WAVE" + null
};

struct fmt_subchunk {
    char subchunk1_id[5];    // should be "fmt " + null
    uint32_t subchunk1_size; // usually 16
    uint16_t audio_format;   // PCM = 1
    uint16_t num_channels;   // 1 = mono, 2 = stereo
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct data_subchunk {
    char subchunk2_id[5]; // should be "data" + null
    uint32_t subchunk2_size;
};

struct wav_header {
    struct riff_header riff_header;
    struct fmt_subchunk fmt_subchunk;
    struct data_subchunk data_subchunk;
};

// WAV file struct
typedef struct {
    struct wav_header header;
    unsigned char* data; // audio samples ****
} WAVFile;

// parser functions
WAVFile WAV_ParseFile(const char* filename);
void WAV_Free(WAVFile* wav);

// helper functions
struct riff_header read_riff_header(FILE* fp);
struct fmt_subchunk read_fmt_subchunk(FILE* fp);
struct data_subchunk read_data_subchunk(FILE* fp);
void print_wav_header(const struct wav_header* wh);

// endianness helper functions
uint16_t read_u16(FILE* fp);
uint32_t read_u32(FILE* fp);

#endif // WAVE_H