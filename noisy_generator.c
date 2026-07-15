#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "wav.h"

#define SAMPLE_RATE 44100
#define DURATION 2.0

// Signal Parameters (The sound we want)
#define SIGNAL_FREQ 440.0
#define SIGNAL_VOLUME 0.4

// Noise Parameters (The interference we want to destroy later)
#define NOISE_FREQ 3000.0
#define NOISE_VOLUME 0.15


int main() {
    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * DURATION);
    uint32_t data_size = total_samples * sizeof(int16_t);

    WAVHeader header = {
        .chunk_id = {'R', 'I', 'F', 'F'},
        .chunk_size = 36 + data_size,
        .format = {'W', 'A', 'V', 'E'},
        .subchunk1_id = {'f', 'm', 't', ' '},
        .subchunk1_size = 16,
        .audio_format = 1,
        .num_channels = 1,
        .sample_rate = SAMPLE_RATE,
        .byte_rate = SAMPLE_RATE * 1 * sizeof(int16_t),
        .block_align = 1 * sizeof(int16_t),
        .bit_depth = 16,
        .subchunk2_id = {'d', 'a', 't', 'a'},
        .subchunk2_size = data_size
    };

    FILE *file = fopen("noisy_tone.wav", "wb");
    if (!file) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    fwrite(&header, sizeof(WAVHeader), 1, file);

    printf("Generating %f seconds of mixed signal (clean: %f Hz, noise: %f Hz)...\n", 
           DURATION, SIGNAL_FREQ, NOISE_FREQ);

    for (uint32_t n = 0; n < total_samples; n++) {
        double t = (double)n / SAMPLE_RATE;
        
        // 1. Generate the clean signal
        double clean_wave = sin(2.0 * M_PI * SIGNAL_FREQ * t) * SIGNAL_VOLUME;
        
        // 2. Generate the high-frequency noise
        double noise_wave = sin(2.0 * M_PI * NOISE_FREQ * t) * NOISE_VOLUME;
        
        // 3. Superimpose (add) them together
        double mixed_wave = clean_wave + noise_wave;

        // Ensure the mixed wave is within [-1.0, 1.0] to avoid clipping
        // Here, signal_volume + noise_volume = 0.4 + 0.15 = 0.55, which is safe
        
        // 4. Scale to 16-bit signed integer
        int16_t sample = (int16_t)(mixed_wave * 32767.0);
        
        fwrite(&sample, sizeof(int16_t), 1, file);
    }

    fclose(file);
    printf("File 'noisy_tone.wav' written successfully!\n");
    return 0;
}