#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define DURATION 2.0
#define FREQUENCY 440.0
#define VOLUME 0.5



// To prevent ther compiler from adding extra padding bits between the struct elements
// This preserves our header format in the disk by keeping it at size we want. 
#pragma pack(push, 1) 
typedef struct {
    // RIFF Chunk Descriptor
    char     chunk_id[4];        // "RIFF"
    uint32_t chunk_size;      // Size of entire file in bytes
    char     format[4];          // "WAVE"
    
    // "fmt" Sub-chunk (describes the audio format)
    char     subchunk1_id[4];    // "fmt " (with trailing space)
    uint32_t subchunk1_size;  // 16 for PCM
    uint16_t audio_format;    // 1 for PCM (uncompressed)
    uint16_t num_channels;    // 1 for Mono, 2 for Stereo
    uint32_t sample_rate;     // e.g., 44100
    uint32_t byte_rate;       // sample_rate * num_channels * (bit_depth / 8)
    uint16_t block_align;     // num_channels * (bit_depth / 8)
    uint16_t bit_depth;       // 16 bits per sample
    
    // "data" Sub-chunk (contains the actual sound samples)
    char     subchunk2_id[4];    // "data"
    uint32_t subchunk2_size;  // Number of bytes in the data section
} WAVHeader;
#pragma pack(pop)

int main() {
    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * DURATION);
    uint32_t data_size = total_samples * sizeof(int16_t); // 16-bit = 2 bytes per sample

    // 1. Populate the WAV Header
    WAVHeader header = {
        .chunk_id = {'R', 'I', 'F', 'F'},
        .chunk_size = 36 + data_size, // 36 bytes of this header metadata + data size
        .format = {'W', 'A', 'V', 'E'},
        .subchunk1_id = {'f', 'm', 't', ' '},
        .subchunk1_size = 16,        // Size of the rest of the fmt chunk
        .audio_format = 1,           // 1 = Pulse Code Modulation (Uncompressed)
        .num_channels = 1,           // 1 = Mono (I do not even have stereo speakers)
        .sample_rate = SAMPLE_RATE,
        .byte_rate = SAMPLE_RATE * 1 * sizeof(int16_t),
        .block_align = 1 * sizeof(int16_t),
        .bit_depth = 16,
        .subchunk2_id = {'d', 'a', 't', 'a'},
        .subchunk2_size = data_size
    };

    // 2. Open file for binary writing
    FILE *file = fopen("pure_tone.wav", "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }

    // 3. Write the 44-byte header
    fwrite(&header, sizeof(WAVHeader), 1, file);

    printf("Generating %f seconds of a %f Hz tone...\n", DURATION, FREQUENCY);

    // 4. Generate and stream audio samples directly to disk
    for (uint32_t n = 0; n < total_samples; n++) {
        double t = (double)n / SAMPLE_RATE;
        
        // Calculate raw wave value: limit to [-1.0, 1.0]
        double raw_sample = sin(2.0 * M_PI * FREQUENCY * t);
        
        // Scale to signed 16-bit range (-32768 to 32767)
        int16_t sample = (int16_t)(raw_sample * 32767.0 * VOLUME);
        
        // Write the 2-byte sample
        fwrite(&sample, sizeof(int16_t), 1, file);
    }

    fclose(file);
    printf("File 'pure_tone.wav' written successfully!\n");
    return 0;
}