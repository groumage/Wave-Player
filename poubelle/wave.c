#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define SIZE_HEADER_WAVE 44

#define LA_3 440

typedef enum {
	do_3 = -9,
	do_3_d,
	re_3,
	re_3_d,
	mi_3,
	fa_3,
	fa_3_d,
	sol_3,
	sol_3_d,
	la_3,
	la_3_d,
	si_3,
} note_e;

typedef enum {
    bpe_8 = 8,
    bpe_16 = 16,
    bpe_24 = 24,
    bpe_32 = 32,
} bpe_e;

typedef enum {
	pcm = 1,
} audio_format_e;

typedef enum {
	mono = 1,
	stereo = 2,
	gcd = 3,
	gd_gd = 4,
	gdcs = 5,
	s = 6,
} audio_channel_e;

typedef enum {
	f_11k = 11025,
	f_22k = 22050,
	f_44k = 44100,
	f_48k = 48000,
	f_96k = 96000,
} freq_ech_e;

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
				data_buffer[0] | (data_buffer[1]<<8) | (data_buffer[2]<<16);
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
	//wave_load("Chlorine.wav");
	return EXIT_SUCCESS;
}
double freq_note(note_e n) {
	return LA_3 * pow(2.0, n/12.0);
}

bool is_big_endian() {
    int test = 1;
    char* p = (char*) &test;
    return p[0] == 0;
}

void reverse_endianness(int size, void* value){
    int i;
    char result[32];
    for (i = 0; i < size; i += 1)
        result[i] = ((char*)value)[size-i-1];
    for(i = 0; i < size; i+=1)
        ((char*)value)[i] = result[i];
}

// if the system is big endian, the function reverses bits each time it's called
// if the system is little endian, do nothing
void to_little_endian(int size, void* value){
    char needs_fix = !((0 && is_big_endian()) || (1 && !is_big_endian()));
    if(needs_fix)
        reverse_endianness(size, value);
}

wave_s* wave_new(uint32_t f, uint16_t p, uint16_t c, uint32_t B) {
	wave_s* wave = malloc(sizeof(wave_s));
	strncpy(wave->file_type_bloc_id, "RIFF", 4);
	strncpy(wave->file_format_id, "WAVE", 4);
	strncpy(wave->format_bloc_id, "fmt ", 4);
	wave->bloc_size = 16;
	wave->audio_format = pcm;
	wave->nbr_channel = c;
	wave->frequence = f;
	wave->byte_per_bloc = c * p / 8;
	wave->byte_per_sec = f * wave->byte_per_bloc;
	wave->byte_per_sample = p;
	strncpy(wave->data_bloc_id, "data", 4);
	wave->data_size = B * wave->byte_per_bloc;
	wave->file_size = wave->data_size + 44 - 8;
	if (wave->byte_per_sample == 8)
		wave->data.data_ptr8 = calloc(wave->file_size, 1);
	else if (wave->byte_per_sample == 16)
		wave->data.data_ptr16 = calloc(wave->file_size, 1);
	else if (wave->byte_per_sample == 24)
		wave->data.data_ptr24 = calloc(wave->file_size, 1);
	return wave;
}

void wave_delete(wave_s* wave) {
	if (wave->byte_per_sample == 8)
		free(wave->data.data_ptr8);
	else if (wave->byte_per_sample == 16)
		free(wave->data.data_ptr16);
	else if (wave->byte_per_sample == 24)
		free(wave->data.data_ptr24);
}
void wave_set(wave_s* w, uint32_t i, uint16_t j, int64_t a) {

}
/*
	to_little_endian(sizeof(int32_t), (void*)&wave->file_size);
	to_little_endian(sizeof(int32_t), (void*)&wave->bloc_size);
	to_little_endian(sizeof(int16_t), (void*)&wave->audio_format);
	to_little_endian(sizeof(int16_t), (void*)&wave->nbr_canaux);
	to_little_endian(sizeof(int32_t), (void*)&wave->frequence);
	to_little_endian(sizeof(int32_t), (void*)&wave->byte_per_sec);
	to_little_endian(sizeof(int16_t), (void*)&wave->byte_per_bloc);
	to_little_endian(sizeof(int16_t), (void*)&wave->byte_per_sample);
	to_little_endian(sizeof(int32_t), (void*)&wave->data_size);*/
/*
	to_little_endian(sizeof(int32_t), (void*)&wave->file_size);
	to_little_endian(sizeof(int32_t), (void*)&wave->bloc_size);
	to_little_endian(sizeof(int16_t), (void*)&wave->audio_format);
	to_little_endian(sizeof(int16_t), (void*)&wave->nbr_canaux);
	to_little_endian(sizeof(int32_t), (void*)&wave->frequence);
	to_little_endian(sizeof(int32_t), (void*)&wave->byte_per_sec);
	to_little_endian(sizeof(int16_t), (void*)&wave->byte_per_bloc);
	to_little_endian(sizeof(int16_t), (void*)&wave->byte_per_sample);
	to_little_endian(sizeof(int32_t), (void*)&wave->data_size);*/
/*if (bytes_per_channel == 1)
	data_in_channel = data_buffer[0];
else if (bytes_per_channel == 2)
	data_in_channel = data_buffer[0] | (data_buffer[1]<<8);
else if (bytes_per_channel == 4)
	data_in_channel = data_buffer[0] | (data_buffer[1]<<8) | (data_buffer[2]<<16) | (data_buffer[3]<<24);*/

//printf("%d\n", length_sample);

/*
int64_t wave_get(wave_s* w, uint32_t i, uint16_t j) {
	return NULL;
}
*/
// open a wav file with 1 channel, 44100 Hz, 16 bit per sample
/*wav_fmt_s* wav_open(const char* filename) {
	FILE* file = fopen(filename, "wb");
	if (file == NULL)
		return NULL;
	wav_header_s* header = malloc(sizeof(wav_header_s));
	strncpy(header->file_type_bloc_id, "RIFF",4);
	header->file_size = 0;
	strncpy(header->file_format_id, "WAVE", 4);
	wav_fmt_s* fmt = malloc(sizeof(wav_fmt_s));
	fmt->header = header;
	strncpy
	= {header, "fmt ", 16, pcm, 1, f_44k, f_44k * BIT_PER_SAMPLE / 8, BIT_PER_SAMPLE / 8, BIT_PER_SAMPLE}; 
	return NULL;
}*/

/*wave_fmt_s* wave_new(uint32_t f, uint16_t p, uint16_t c, uint32_t b) {
	wave_fmt_s* wave = malloc(sizeof(wave_fmt_s));
	strncpy(wave->header.file_type_bloc_id, "RIFF", 4);
	strncpy(wave->s_wave, "WAVE", 4);
	strncpy(wave->s_data, "data", 4);
	strncpy(wave->s_fmt, "fmt ", 4);
	wave->sixteen = 16;
	wave->audio_format = pcm;
	wave->nb_canaux = c;
	wave->freq = f;
	wave->octet_per_bloc = c * p / 8;
	wave->octet_per_sec = f * wave->octet_per_bloc;
	wave->accuracy = p;
	wave->data_size = b * wave->octet_per_sec;
	wave->file_size = wave->data_size + 44 - 8;
	wave->data = malloc(wave->data_size);
	return wave;
}

wave_s* create_sound(wave_s* w) {
	int16_t* data = w->data;
	int cpt;
	double val;
	for (cpt = 0, val = 0.0; cpt < w->file_size/2; cpt += 2, val += 0.0284951714612) {
        data[cpt] = sin(val) * 32267;
        data[cpt+1] = sin(val) * 32267;
    }
    return w;
}

void wave_delete(wave_s* w) {
	free(w->data);
}
*/
/*	wave_s* wave = wave_new(44100, bpe_16, stereo, 500);
	FILE* f = fopen("test.wav", "wb+");
	wave = create_sound(wave);
	// we create the structure ine the programm, but nothing is written in the file
	// so we write the header
	fwrite(wave, SIZE_HEADER_WAVE, 1, f);
	// datas are written in the file
	fwrite(wave->data, wave->data_size, 1, f);
	wave_delete(wave);
	fclose(f);
	printf("%f",freq_note(re_3));*/