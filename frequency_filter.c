#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "wav.h"

#define SAMPLE_RATE 44100
#define N 4410  // Block size

int main(int argc, char *argv[]) {
     if (argc != 2) {
        printf("Usage: %s <input_wav_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    FILE *infile = fopen(filename, "rb");
    if (!infile) {
        printf("Error: Could not open %s.?\n", filename);
        return 1;
    }

    // Read the incoming header
    WAVHeader header;
    fread(&header, sizeof(WAVHeader), 1, infile);

    // Setup the output file and write an identical header structure
    FILE *outfile = fopen("clean_reconstructed.wav", "wb");
    if (!outfile) {
        printf("Error: Could not create output file.\n");
        fclose(infile);
        return 1;
    }
    fwrite(&header, sizeof(WAVHeader), 1, outfile);

    uint32_t total_samples = header.subchunk2_size / sizeof(int16_t);
    int num_blocks = total_samples / N;

    printf("Processing %d audio blocks of size %d...\n", num_blocks, N);

    double x[N];          // Time-domain input
    double real[N];       // Frequency-domain Real
    double imag[N];       // Frequency-domain Imaginary
    double clean_x[N];    // Reconstructed clean time-domain output
    int16_t raw_buffer[N];

    // Block-by-block processing loop
    for (int b = 0; b < num_blocks; b++) {
        // 1. Read a block of raw samples and normalize
        if (fread(raw_buffer, sizeof(int16_t), N, infile) != N) {
            break;
        }
        for (int n = 0; n < N; n++) {
            x[n] = (double)raw_buffer[n] / 32768.0;
        }

        // 2. Perform DFT (Forward Transform)
        for (int k = 0; k < N; k++) {
            real[k] = 0.0;
            imag[k] = 0.0;
            for (int n = 0; n < N; n++) {
                double angle = (2.0 * M_PI * k * n) / N;
                real[k] += x[n] * cos(angle);
                imag[k] -= x[n] * sin(angle);
            }
        }

        // 3. APPLY FILTER: Zero out bins around 3000 Hz (Bin 70)
        // We zero a small window (bins 67 to 73) to catch any spectral leakage.
        // Crucial: We must also zero the symmetric mirror bins (N - k) on the upper half!
        int noise_bin = 70;
        int window_radius = 10;
        for (int i = -window_radius; i <= window_radius; i++) {
            int target_bin = noise_bin + i;
            int mirror_bin = N - target_bin;
            
            real[target_bin] = 0.0;
            imag[target_bin] = 0.0;
            
            real[mirror_bin] = 0.0;
            imag[mirror_bin] = 0.0;
        }

        // 4. Perform IDFT (Inverse Transform) on our modified spectrum
        for (int n = 0; n < N; n++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                double angle = (2.0 * M_PI * k * n) / N;
                // Accumulate the real part of (Real + i*Imag) * (cos + i*sin)
                sum += (real[k] * cos(angle) - imag[k] * sin(angle));
            }
            clean_x[n] = sum / N; // Scale by 1/N
        }

        // 5. Convert back to raw 16-bit integers and write to disk
        for (int n = 0; n < N; n++) {
            raw_buffer[n] = (int16_t)(clean_x[n] * 32767.0);
        }
        fwrite(raw_buffer, sizeof(int16_t), N, outfile);
        
        if (b % 10 == 0) {
            printf("Progress: Block %d/%d processed.\n", b, num_blocks);
        }
    }

    fclose(infile);
    fclose(outfile);
    printf("\nSuccess! Saved 'clean_reconstructed.wav'.\n");
    return 0;
}
