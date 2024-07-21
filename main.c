#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>

bool DEBUG = true;

typedef struct Audio {

    char*  data;
    long long len; 

} Audio ;

Audio readMusicFile(char *);
void printBits(Audio, long long, long long);
size_t getID3Size(Audio);
int ID3_sync_safe_to_int(char byte0, char byte1, char byte2, char byte3);

int main(int argv, char **argc){
    
    if( argv < 2) {
        printf("Please specify the music file to play !\n");
        exit(-1);
    }
    
    Audio audio = readMusicFile(argc[1]);
    
    if(DEBUG){
        printf("ID3 HEADER:\n");
        printBits(audio, 10 ,0 );    
    }

    size_t ID3size = getID3Size(audio);
    
    printBits(audio, 32, ID3size);

    free(audio.data);
    return 0;
}


size_t getID3Size(Audio audio){
    char *data = audio.data;
    // Fifth byte of the header contains flags and 4th bit if set, conveys that footer
    // if present. Footer = header (except for identification string which is 3DI in 
    // place of ID3 within the header (first 3 bytes i.e. 49 44 33 in ASCII Hex ) 
    bool footer = audio.data[5] & 0x10;
    // Bytes 7 to 10 represent size of rest of ID3 HEADER exlcuding footer if present
    // Hence, total size is obtained by adding 20/10 accordingly where 10 is the size
    // of HEADER = size of Footer.
    // Ref:https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-structure.html
    size_t size = ID3_sync_safe_to_int(data[6],data[7],data[8],data[9]) + (footer ? 20 : 10);
    printf("\nSize of ID3 = %zu\n", size );
    return size;
}


int ID3_sync_safe_to_int(char byte0, char byte1, char byte2, char byte3){
    return byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
}


void printBits(Audio audio, long long size, long long offset){
    char *data = audio.data;

    unsigned long long maxPow = 1 << 7;
    
    int i = offset, j = 1;
    if(size == 0) size = audio.len;
    
    for(; i < offset + size ; i++ , j++ ) {
        
       // print bit by bit
       // 
       // for( j = 0; j < 8; j++ ) {
       //     // print last bit and shift left.
       //     printf("%u",data[i] & maxPow  ? 1: 0 );
       //     data[i] <<= 1;
       // }
       // printf(" ");
        
        printf("%02x ", data[i] & 0xff); 

        // print in bunch of 32 bits at a time to gather 
        // headers from mp3 frames
        if (j % 4  == 0){
            printf("\n");
        }
    }
}

Audio readMusicFile(char *path){

    FILE *file;
    char *audio;
    unsigned long fileLen;

    file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file");
        exit(-2);
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    audio = (char *)malloc(fileLen+1);
    if (!audio)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(-3);
    }

    fread(audio, fileLen, 1, file);
    
    if(DEBUG) printf("Size of Music file: %ld\n", fileLen);
    
    fclose(file);

    Audio ret = {audio, fileLen};
    return ret;

}
