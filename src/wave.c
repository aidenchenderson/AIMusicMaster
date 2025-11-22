#include "wave.h"
#include <stdlib.h>
#include <string.h>

// endianness helper functions
uint16_t read_u16(FILE* fp) {
    uint16_t value;
    fread(&value, sizeof(uint16_t), 1, fp);
    // swap bytes if on big-endian system
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
    #endif
    return value;
}

uint32_t read_u32(FILE* fp) {
    uint32_t value;
    fread(&value, sizeof(uint32_t), 1, fp);
    // swap bytes if on big-endian system
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        value = ((value & 0xFF) << 24) | 
                ((value & 0xFF00) << 8) |
                ((value & 0xFF0000) >> 8) |
                ((value & 0xFF000000) >> 24);
    #endif
    return value;
}


//read chunks

struct riff_header read_riff_header(FILE* fp) {
    struct riff_header rh = {{0}, 0, {0}};
    fread(rh.chunk_id, 1, 4, fp);
    rh.chunk_id[4] = '\0';
    rh.chunk_size = read_u32(fp);
    fread(rh.format, 1, 4, fp);
    rh.format[4] = '\0';
    return rh;
}

struct fmt_subchunk read_fmt_subchunk(FILE* fp) {
    struct fmt_subchunk fs = {{0}, 0, 0, 0, 0, 0, 0, 0};
    fread(fs.subchunk1_id, 1, 4, fp);
    fs.subchunk1_id[4] = '\0';

    fs.subchunk1_size = read_u32(fp);
    fs.audio_format = read_u16(fp);
    fs.num_channels = read_u16(fp);
    fs.sample_rate = read_u32(fp);
    fs.byte_rate = read_u32(fp);
    fs.block_align = read_u16(fp);
    fs.bits_per_sample = read_u16(fp);

    // skip extra bytes
    if (fs.subchunk1_size > 16) {
        fseek(fp, fs.subchunk1_size - 16, SEEK_CUR);
    }

    return fs;
}

struct data_subchunk read_data_subchunk(FILE* fp) {
    struct data_subchunk ds = {{0}, 0};
    fread(ds.subchunk2_id, 1, 4, fp);
    ds.subchunk2_id[4] = '\0';
    ds.subchunk2_size = read_u32(fp);
    return ds;
}

// print function
void print_wav_header(const struct wav_header* wh) {
    printf("ChunkID: %s\n", wh->riff_header.chunk_id);
    printf("ChunkSize: %u\n", wh->riff_header.chunk_size);
    printf("Format: %s\n", wh->riff_header.format);

    printf("Subchunk1ID: %s\n", wh->fmt_subchunk.subchunk1_id);
    printf("Subchunk1Size: %u\n", wh->fmt_subchunk.subchunk1_size);
    printf("AudioFormat: %u\n", wh->fmt_subchunk.audio_format);
    printf("NumChannels: %u\n", wh->fmt_subchunk.num_channels);
    printf("SampleRate: %u\n", wh->fmt_subchunk.sample_rate);
    printf("ByteRate: %u\n", wh->fmt_subchunk.byte_rate);
    printf("BlockAlign: %u\n", wh->fmt_subchunk.block_align);
    printf("BitsPerSample: %u\n", wh->fmt_subchunk.bits_per_sample);

    printf("Subchunk2ID: %s\n", wh->data_subchunk.subchunk2_id);
    printf("Subchunk2Size: %u\n", wh->data_subchunk.subchunk2_size);
}

// main parser
WAVFile WAV_ParseFile(const char* filename) {
    WAVFile wav = {{0}, NULL};
    FILE* fp = fopen(filename, "rb");
    if (!fp) { 
        fprintf(stderr, "Failed to open file: %s\n", filename); 
        return wav; 
    }

    // read header
    wav.header.riff_header = read_riff_header(fp);
    
    // verify it's a WAV file
    if (strncmp(wav.header.riff_header.chunk_id, "RIFF", 4) != 0 ||
        strncmp(wav.header.riff_header.format, "WAVE", 4) != 0) {
        fprintf(stderr, "Not a valid WAV file\n");
        fclose(fp);
        return wav;
    }

    char chunk_id[5];
    uint32_t chunk_size;

    // Read chunks until we find data chunk
    while (fread(chunk_id, 1, 4, fp) == 4) {
        chunk_id[4] = '\0';
        chunk_size = read_u32(fp);

        if (strncmp(chunk_id, "fmt ", 4) == 0) {
            // Go back 4 bytes because read_fmt_subchunk expects to read the chunk ID
            fseek(fp, -8, SEEK_CUR);
            wav.header.fmt_subchunk = read_fmt_subchunk(fp);
        } else if (strncmp(chunk_id, "data", 4) == 0) {
            // Go back 4 bytes because read_data_subchunk expects to read the chunk ID  
            fseek(fp, -8, SEEK_CUR);
            wav.header.data_subchunk = read_data_subchunk(fp);
            
            // Now read the actual audio data
            wav.data = malloc(wav.header.data_subchunk.subchunk2_size);
            if (!wav.data) { 
                fprintf(stderr, "Failed to allocate memory for audio data\n");
                fclose(fp); 
                return wav; 
            }
            
            size_t bytes_read = fread(wav.data, 1, wav.header.data_subchunk.subchunk2_size, fp);
            if (bytes_read != wav.header.data_subchunk.subchunk2_size) {
                fprintf(stderr, "Failed to read all audio data\n");
                free(wav.data);
                wav.data = NULL;
            }
            break;
        } else {
            // Skip unknown chunks
            fseek(fp, chunk_size, SEEK_CUR);
        }
    }

    fclose(fp);
    return wav;
}

// Free WAV data (unchanged)
void WAV_Free(WAVFile* wav) {
    if (wav->data) {
        free(wav->data);
        wav->data = NULL;
    }
}