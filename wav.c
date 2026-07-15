#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "wav.h"

#define SAMPLE_RATE 44100
#define DURATION 2.0
#define FREQUENCY 440.0
#define VOLUME 0.5


int main() {
    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * DURATION);
    uint32_t data_size = total_samples * sizeof(int16_t); // 16-bit = 2 bytes per sample

    // Populate the Standard WAV Header
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

    // Open file for binary writing
    FILE *file = fopen("pure_tone.wav", "wb");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return 1;
    }

    // Write the 44-byte header
    fwrite(&header, sizeof(WAVHeader), 1, file);

    printf("Generating %f seconds of a %f Hz tone...\n", DURATION, FREQUENCY);

    // Generate and stream audio samples directly to disk
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