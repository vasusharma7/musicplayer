#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "wav.h"

#define SAMPLE_RATE 44100
#define N 1024  // Window size for our DFT to prevent N^2 explosion in computation time

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_wav_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open %s. Did you run Step 2?\n", filename);
        return 1;
    }

    // Skip the header (44 bytes) to get straight to raw audio data
    fseek(file, sizeof(WAVHeader), SEEK_SET);

    // 1. Read exactly N samples into our time-domain array
    double x[N];
    int16_t raw_sample;
    for (int n = 0; n < N; n++) {
        if (fread(&raw_sample, sizeof(int16_t), 1, file) != 1) {
            printf("Error: Failed to read enough samples.\n");
            fclose(file);
            return 1;
        }
        x[n] = (double)raw_sample / 32768.0; // Normalize
    }
    fclose(file);

    // 2. Allocate arrays for our Frequency Domain output
    double real[N] = {0};
    double imag[N] = {0};
    double magnitude[N] = {0};

    printf("Running $O(N^2)$ DFT on %d samples...\n", N);

    // 3. The DFT loop
    for (int k = 0; k < N; k++) {
        for (int n = 0; n < N; n++) {
            double angle = (2.0 * M_PI * k * n) / N;
            real[k] += x[n] * cos(angle);
            imag[k] -= x[n] * sin(angle); // Negative sign from Euler's Formula
        }
        // Calculate magnitude
        magnitude[k] = sqrt(real[k] * real[k] + imag[k] * imag[k]);
    }

    // 4. Print the results (Only first N/2 bins, as the second half is a mirror image)
    // This is because for real-valued signals, the DFT is symmetric.
    // If we were in imaginary space, we would need to consider the full N bins.
    printf("\n%-10s | %-15s | %-10s\n", "Bin (k)", "Frequency (Hz)", "Magnitude");
    printf("---------------------------------------------\n");
    
    for (int k = 0; k < N / 2; k++) {
        double freq = k * ((double)SAMPLE_RATE / N);
        
        // Only print frequencies that have significant energy to avoid terminal spam
        if (magnitude[k] > 5.0) {
            printf("%-10d | %-15.2f | %-10.2f ", k, freq, magnitude[k]);
            
            // Print a simple ASCII bar chart to visualize the spike
            int bar_length = (int)(magnitude[k] / 3.0);
            for (int i = 0; i < bar_length; i++) printf("#");
            printf("\n");
        }
    }

    return 0;
}