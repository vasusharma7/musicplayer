#include <stdint.h>


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
