#include <stdio.h>
#include <malloc.h>
#include <math.h>

// -------------------------------------------------- [ Section: Endianness ] -
int isBigEndian() {
    int test = 1;
    char *p = (char*)&test;

    return p[0] == 0;
}
void reverseEndianness(const long long int size, void* value){
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
    for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}
void toBigEndian(const long long int size, void* value){
    char needsFix = !( (1 && isBigEndian()) || (0 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}
void toLittleEndian(const long long int size, void* value){
    char needsFix = !( (0 && isBigEndian()) || (1 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}

// ------------------------------------------------- [ Section: Wave Header ] -
typedef struct WaveHeader {
    // Riff Wave Header
    char chunkId[4];
    int  chunkSize;
    char format[4];

    // Format Subchunk
    char subChunk1Id[4];
    int  subChunk1Size;
    short int audioFormat;
    short int numChannels;
    int sampleRate;
    int byteRate;
    short int blockAlign;
    short int bitsPerSample;
    //short int extraParamSize;

    // Data Subchunk
    char subChunk2Id[4];
    int  subChunk2Size;

} WaveHeader;

WaveHeader makeWaveHeader(int const sampleRate, short int const numChannels, short int const bitsPerSample ){
    WaveHeader myHeader;

    // RIFF WAVE Header
    myHeader.chunkId[0] = 'R';
    myHeader.chunkId[1] = 'I';
    myHeader.chunkId[2] = 'F';
    myHeader.chunkId[3] = 'F';
    myHeader.format[0] = 'W';
    myHeader.format[1] = 'A';
    myHeader.format[2] = 'V';
    myHeader.format[3] = 'E';

    // Format subchunk
    myHeader.subChunk1Id[0] = 'f';
    myHeader.subChunk1Id[1] = 'm';
    myHeader.subChunk1Id[2] = 't';
    myHeader.subChunk1Id[3] = ' ';
    myHeader.audioFormat = 1; // FOR PCM
    myHeader.numChannels = numChannels; // 1 for MONO, 2 for stereo
    myHeader.sampleRate = sampleRate; // ie 44100 hertz, cd quality audio
    myHeader.bitsPerSample = bitsPerSample; // 
    myHeader.byteRate = myHeader.sampleRate * myHeader.numChannels * myHeader.bitsPerSample / 8;
    myHeader.blockAlign = myHeader.numChannels * myHeader.bitsPerSample/8;

    // Data subchunk
    myHeader.subChunk2Id[0] = 'd';
    myHeader.subChunk2Id[1] = 'a';
    myHeader.subChunk2Id[2] = 't';
    myHeader.subChunk2Id[3] = 'a';

    // All sizes for later:
    // chuckSize = 4 + (8 + subChunk1Size) + (8 + subChubk2Size)
    // subChunk1Size is constanst, i'm using 16 and staying with PCM
    // subChunk2Size = nSamples * nChannels * bitsPerSample/8
    // Whenever a sample is added:
    //    chunkSize += (nChannels * bitsPerSample/8)
    //    subChunk2Size += (nChannels * bitsPerSample/8)
    //myHeader.chunkSize = 4+8+16+8+0;
    myHeader.chunkSize = 4+8+16+8+0;
    myHeader.subChunk1Size = 16;
    myHeader.subChunk2Size = 0;
    
    return myHeader;
}

// -------------------------------------------------------- [ Section: Wave ] -
typedef struct Wave {
    WaveHeader header;
    char* data;
    long long int index;
    long long int size;
    long long int nSamples;
} Wave;

Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample){
    Wave myWave;
    myWave.header = makeWaveHeader(sampleRate,numChannels,bitsPerSample);
    return myWave;
}
void waveDestroy( Wave* wave ){
    free( wave->data );
}
void waveSetDuration( Wave* wave, const float seconds ){
    long long int totalBytes = (long long int)(wave->header.byteRate*seconds);
    wave->data = (char*)malloc(totalBytes);
    wave->index = 0;
    wave->size = totalBytes;
    wave->nSamples = (long long int) wave->header.numChannels * wave->header.sampleRate * seconds;
    wave->header.chunkSize = 4+8+16+8+totalBytes;
    wave->header.subChunk2Size = totalBytes;
}
void waveAddSample( Wave* wave, const float* samples ){
    int i;
    short int sample8bit;
    int sample16bit;
    long int sample32bit;
    char* sample;
    if( wave->header.bitsPerSample == 8 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample8bit = (short int) (127+127.0*samples[i]);
            toLittleEndian(1, (void*) &sample8bit);
            sample = (char*)&sample8bit;
            (wave->data)[ wave->index ] = sample[0];
            wave->index += 1;
        }
    }
    if( wave->header.bitsPerSample == 16 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample16bit = (int) (32767*samples[i]);
            //sample = (char*)&litEndianInt( sample16bit );
            toLittleEndian(2, (void*) &sample16bit);
            sample = (char*)&sample16bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->index += 2;
        }
    }
    if( wave->header.bitsPerSample == 32 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample32bit = (long int) ((pow(2,32-1)-1)*samples[i]);
            //sample = (char*)&litEndianLong( sample32bit );
            toLittleEndian(4, (void*) &sample32bit);
            sample = (char*)&sample32bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->data[ wave->index + 2 ] = sample[2];
            wave->data[ wave->index + 3 ] = sample[3];
            wave->index += 4;
        }
    }
}
void waveToFile( Wave* wave, const char* filename ){

    // First make sure all numbers are little endian
    toLittleEndian(sizeof(int), (void*)&(wave->header.chunkSize));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk1Size));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk2Size));

    // Open the file, write header, write data
    FILE *file;
    file = fopen(filename, "wb");
    fwrite( &(wave->header), sizeof(WaveHeader), 1, file );
    fwrite( (void*)(wave->data), sizeof(char), wave->size, file );
    fclose( file );

    // Convert back to system endian-ness
    toLittleEndian(sizeof(int), (void*)&(wave->header.chunkSize));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk1Size));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.subChunk2Size));
}

Wave wave_load(const char* name) {
    FILE* wave = fopen(name, "rb");

    fseek(wave,4,SEEK_SET);
    unsigned int chunkSize = 0;
    fread(&chunkSize,sizeof(unsigned int),1,wave);

    fseek(wave,4*5+2,SEEK_SET);
    unsigned short int numChannels = 0;
    fread(&numChannels,sizeof(unsigned short int),1,wave);
    //printf("%d\n",numChannels);

    fseek(wave,4*6,SEEK_SET);
    unsigned int sampleRate = 0;
    fread(&sampleRate,sizeof(unsigned int),1,wave);
    
    fseek(wave,4*8+2,SEEK_SET);
    unsigned short int bitsPerSample = 0;
    fread(&bitsPerSample,sizeof(unsigned short int),1,wave);
    //printf("%d\n",bitsPerSample);

    fseek(wave,4*7,SEEK_SET);
    unsigned int byteRate = 0;
    fread(&byteRate,sizeof(unsigned int),1,wave);

    float duration = (float) (chunkSize-36) / byteRate;

    int nSamples = (int)(duration*sampleRate);

    Wave w = makeWave(sampleRate,numChannels,bitsPerSample);

    waveSetDuration(&w,duration);

    fseek(wave,4*11,SEEK_SET);

    float samples[numChannels];
    long size_sample = w.header.numChannels * w.header.bitsPerSample / 8;
    long bytes_per_channel = size_sample / numChannels;
    printf("bytes_per_channel:%ld\n",bytes_per_channel);
    char data_buffer[size_sample];
    printf("size_sample:%ld\n",size_sample);
    int i = 0;
    int j = 0;
    for(i = 0; i < nSamples; i++) {
        fread(data_buffer,sizeof(data_buffer),1,wave);
        for(j = 0; j < numChannels; j++) {
            if (bytes_per_channel == 1) {
                samples[j] = data_buffer[j];
                if (samples[j] < 0)
                    samples[j] += 256;
                samples[j] -= 127;
                samples[j] /= 127;
            }
            if (bytes_per_channel == 2) {
                int tmp = ((int)(char)data_buffer[2*j+1])<<8 
                | ((unsigned int)(unsigned char)data_buffer[2*j]);
                samples[j] = tmp / (pow(2,16-1)-1);
                //printf("%f /// %f\n",cos(600.0*(float)i*3.14159/sampleRate),samples[j]);
            }
            if (bytes_per_channel == 4) {
                int tmp = ((int)(char)data_buffer[4*j+3])<<24 
                | ((unsigned int)(unsigned char)data_buffer[4*j+2]<<16)
                | ((unsigned int)(unsigned char)data_buffer[4*j+1]<<8)
                | ((unsigned int)(unsigned char)data_buffer[4*j]);
                samples[j] = tmp / (pow(2,32-1)-1);
                //printf("%f /// %f\n",cos(600.0*(float)i*3.14159/sampleRate),samples[j]);
            }
        }
        waveAddSample(&w,samples);
    }

    fclose(wave);
    return w;
}

// -------------------------------------------------------- [ Section: Main ] -
int main(){
    // Define some variables for the sound
    float sampleRate = 44100.0; // hertz
    float freq = 600.0;         // hertz
    float duration = 1;       // seconds
    int channels = 2;
    int nSamples = (int)(duration*sampleRate);
    
    // Create a mono (1), 32-bit sound and set the duration
    Wave mySound = makeWave((int)sampleRate,channels,16);
    waveSetDuration( &mySound, duration );

    // Add all of the data
    int i;
    float frameData[channels];
    for(i=0; i<nSamples; i+=1 ){
        frameData[0] = cos(freq*(float)i*3.14159/sampleRate);
        frameData[1] = cos(3*freq*(float)i*3.14159/sampleRate);
        //printf("%f /// %f\n",frameData[0],frameData[1]);
        waveAddSample( &mySound, frameData );
    }

    //Write it to a file and clean up when done
    waveToFile( &mySound, "mono-16bit.wav");
    waveDestroy( &mySound );
    Wave w = wave_load("mono-16bit.wav");
    waveToFile(&w,"mono-16bit2.wav");
    return 0;
}