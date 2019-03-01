#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SIZE_HEADER_WAVE 44

typedef struct wave {
    char file_type_bloc_id[4];
    int32_t file_size;
    char file_format_id[4];
    char format_bloc_id[4];
    int32_t bloc_size;
    int16_t audio_format;
    int16_t nbr_channel;
    int32_t frequence;
    int32_t byte_per_sec;
    int16_t byte_per_bloc;
    int16_t byte_per_sample;
    char data_bloc_id[4];
    int32_t data_size;
    union {
        int8_t* data_ptr8;
        int16_t* data_ptr16;
        int32_t* data_ptr24;
    } data;
} wave_s;

wave_s* wave_load(const char* filename) {
    int read = 0;
    unsigned char buffer2[2];
    unsigned char buffer4[4];
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;

    wave_s* wave = malloc(sizeof(wave_s));

    // every information in th header of the first wave are copied into the second

    read = fread(wave->file_type_bloc_id, sizeof(wave->file_type_bloc_id), 1, file);

    read = fread(buffer4, sizeof(buffer4), 1, file);
    wave->file_size = buffer4[0] | (buffer4[1]<<8) | (buffer4[2]<<16) | (buffer4[3]<<24);

    read = fread(wave->file_format_id, sizeof(wave->file_format_id), 1, file);

    read = fread(wave->format_bloc_id, sizeof(wave->format_bloc_id), 1, file);

    read = fread(buffer4, sizeof(buffer4), 1, file);
    wave->bloc_size = buffer4[0] | (buffer4[1]<<8) | (buffer4[2]<<16) | (buffer4[3]<<24);

    read = fread(buffer2, sizeof(buffer2), 1, file);
    wave->audio_format = buffer2[0] | (buffer2[1]<<8);

    fread(buffer2, sizeof(buffer2), 1, file);
    wave->nbr_channel = buffer2[0] | (buffer2[1]<<8);

    fread(buffer4, sizeof(buffer4), 1, file);
    wave->frequence = buffer4[0] | (buffer4[1]<<8) | (buffer4[2]<<16) | (buffer4[3]<<24);

    fread(buffer4, sizeof(buffer4), 1, file);
    wave->byte_per_sec = buffer4[0] | (buffer4[1]<<8) | (buffer4[2]<<16) | (buffer4[3]<<24);

    fread(buffer2, sizeof(buffer2), 1, file);
    wave->byte_per_bloc = buffer2[0] | (buffer2[1]<<8);

    fread(buffer2, sizeof(buffer2), 1, file);
    wave->byte_per_sample = buffer2[0] | (buffer2[1]<<8);

    fread(wave->data_bloc_id, sizeof(wave->data_bloc_id), 1, file);

    fread(buffer4, sizeof(buffer4), 1, file);
    wave->data_size = buffer4[0] | (buffer4[1]<<8) | (buffer4[2]<<16) | (buffer4[3]<<24);        

    // writing datas
    if (wave->byte_per_sample == 8)
        wave->data.data_ptr8 = malloc(sizeof(int8_t) * wave->data_size);
    else if (wave->byte_per_sample == 16)
        wave->data.data_ptr16 = malloc(sizeof(int16_t) * wave->data_size);
    else if (wave->byte_per_sample == 24)
        wave->data.data_ptr24 = malloc(sizeof(int32_t) * wave->data_size);

    int nbr_sample = (8 * wave->data_size) / (wave->nbr_channel * wave->byte_per_sample);
    int length_sample = (wave->nbr_channel * wave->byte_per_sample) / 8;
    char data_buffer[length_sample];
    int i = 0;
    for (i = 0; i < nbr_sample; i++) {
        read = fread(data_buffer, sizeof(data_buffer), 1, file);
        if (read != 0) {
            int j = 0;
            for (j = 0; j < wave->nbr_channel; j++) {
                if (wave->byte_per_sample == 8)
                wave->data.data_ptr8[i * length_sample] = data_buffer[0];
                else if (wave->byte_per_sample == 16)
                    wave->data.data_ptr16[i * length_sample] = data_buffer[0] | data_buffer[1]<<8;
                else if (wave->byte_per_sample == 24)
                    wave->data.data_ptr24[i * length_sample] =
                data_buffer[0] | (data_buffer[1]<<8) | (data_buffer[2]<<16) | (data_buffer[3]<<24);
            }
        }
    }
    fclose(file);
    return wave;
}

bool wave_save(const char* filename, wave_s* wave) {

    FILE* file = fopen(filename, "wb");
    fwrite(wave, SIZE_HEADER_WAVE, 1, file);

    if (wave->byte_per_sample == 8)
        fwrite(wave->data.data_ptr8, wave->data_size, 1, file);
    else if (wave->byte_per_sample == 16)
        fwrite(wave->data.data_ptr16, wave->data_size, 1, file);
    else if (wave->byte_per_sample == 24)
        fwrite(wave->data.data_ptr24, wave->data_size, 1, file);

    fclose(file);
    return true;
}

int main() {
    wave_save("440-2.wav", wave_load("440.wav"));
    return EXIT_SUCCESS;
}