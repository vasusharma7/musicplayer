#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"

// Function helper to verify 4-character identifiers
int verify_id(const char *id, const char *expected) {
    return strncmp(id, expected, 4) == 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_wav_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return 1;
    }

    // 1. Read the header
    WAVHeader header;
    if (fread(&header, sizeof(WAVHeader), 1, file) != 1) {
        printf("Error: Could not read WAV header.\n");
        fclose(file);
        return 1;
    }

    // 2. Validate the WAV header signatures
    if (!verify_id(header.chunk_id, "RIFF") || 
        !verify_id(header.format, "WAVE") || 
        !verify_id(header.subchunk1_id, "fmt ") || 
        !verify_id(header.subchunk2_id, "data")) {
        printf("Error: Invalid or unsupported WAV format.\n");
        fclose(file);
        return 1;
    }

    // Ensure we are dealing with 16-bit Mono PCM
    if (header.audio_format != 1 || header.num_channels != 1 || header.bit_depth != 16) {
        printf("Error: Only 16-bit Mono PCM WAV files are supported.\n");
        fclose(file);
        return 1;
    }

    // 3. Calculate sample counts
    uint32_t total_samples = header.subchunk2_size / sizeof(int16_t);
    printf("--- WAV File Metadata ---\n");
    printf("File Name:      %s\n", filename);
    printf("Sample Rate:    %u Hz\n", header.sample_rate);
    printf("Total Samples:  %u\n", total_samples);
    printf("Duration:       %.2f seconds\n", (double)total_samples / header.sample_rate);
    printf("-------------------------\n");

    // 4. Allocate memory for normalized samples
    double *samples = (double *)malloc(total_samples * sizeof(double));
    if (!samples) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    // 5. Read, normalize, and load samples
    int16_t raw_buffer;
    uint32_t loaded_samples = 0;
    double max_val = -1.0;
    double min_val = 1.0;

    for (uint32_t i = 0; i < total_samples; i++) {
        if (fread(&raw_buffer, sizeof(int16_t), 1, file) != 1) {
            break; // Unexpected end of file
        }
        
        // Normalize 16-bit signed integer to [-1.0, 1.0] double
        // Reverse of what we do when writing: sample = (int16_t)(raw_sample * 32767.0 * VOLUME);
        samples[i] = (double)raw_buffer / 32768.0;
        loaded_samples++;

        // Track peak values for verification
        if (samples[i] > max_val) max_val = samples[i];
        if (samples[i] < min_val) min_val = samples[i];
    }

    fclose(file);

    printf("Successfully loaded %u samples into memory.\n", loaded_samples);
    printf("Signal Bounds:  Min = %f, Max = %f\n", min_val, max_val);

    // Clean up
    free(samples);
    return 0;
}