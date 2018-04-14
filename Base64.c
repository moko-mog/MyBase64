#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#define BASE_NUM 64

#define UPPER_NUM 26
#define LOWER_NUM 26
#define NUM_NUM 10

#define ENC_BLOCK 4
#define ENC_BIT_WIDTH 6
#define BIT_PER_BYTE 8

static char* create_table(void){
	static char table[BASE_NUM];
	int bias = 0;
	int i = 0;
	
	for(i = 0 ; i < UPPER_NUM ; ++i){
		table[bias + i] = 'A' + (char)i;
	}
	bias += UPPER_NUM;
	
	for(i = 0 ; i < LOWER_NUM ; ++i){
		table[bias + i] = 'a' + (char)i;
	}
	bias += LOWER_NUM;
	
	for(i = 0 ; i < LOWER_NUM ; ++i){
		table[bias + i] = '0' + (char)i;
	}
	bias += NUM_NUM;
	
	table[bias] = '+'; ++bias;
	table[bias] = '/';
	
	return table;
}

int get_padding_bit(size_t bytes){
	
	size_t bits = bytes * BIT_PER_BYTE;
	int R = (int)(bits % ENC_BIT_WIDTH);
	
	if(R == 0){
		return 0;
	}
	
	return (ENC_BIT_WIDTH - R);
}

char* one_byte_encode(char* buf , uint8_t top , int padding , char* table){
	
	static char enc[4];
	int b[2];
	uint32_t one_byte;
	int i;
	
	one_byte = 0;
	one_byte = one_byte * 0x100 + top;
	one_byte <<= padding;
	
	b[0] = (int)(one_byte & 0xFC0) >> 6;
	b[1] = (int)(one_byte & 0x3F);
	
	for(i = 0 ; i < 2 ; ++i){
		enc[i] = table[b[i]];
	}
	
	for(i = 2 ; i < ENC_BLOCK ; ++i){
		enc[i] = '=';
	}
	
	buf = enc;
	return enc;
}

char* twin_byte_encode(char* buf , uint8_t top , uint8_t btm , int padding , char* table){
	
	static char enc[4];
	int b[3];
	uint32_t twin_byte;
	int i;
	
	twin_byte = 0;
	twin_byte = twin_byte * 0x100 + top;
	twin_byte = twin_byte * 0x100 + btm;
	twin_byte <<= padding;
	
	b[0] = (int)(twin_byte & 0x3F000) >> 12;
	b[1] = (int)(twin_byte & 0xFC0) >> 6;
	b[2] = (int)(twin_byte & 0x3F);
	
	for(i = 0 ; i < 3 ; ++i){
		enc[i] = table[b[i]];
	}
	
	for(i = 3 ; i < ENC_BLOCK ; ++i){
		enc[i] = '=';
	}
	
	buf = enc;
	return enc;
}

char* tri_byte_encode(char* buf , uint8_t top , uint8_t mid , uint8_t btm , char* table){
	
	static char enc[4];
	int b[4];
	uint32_t tri_byte;
	int i;
	
	tri_byte = 0;
	tri_byte = tri_byte * 0x100 + top;
	tri_byte = tri_byte * 0x100 + mid;
	tri_byte = tri_byte * 0x100 + btm;
	
	b[0] = (int)(tri_byte & 0xFC0000) >> 18;
	b[1] = (int)(tri_byte & 0x3F000) >> 12;
	b[2] = (int)(tri_byte & 0xFC0) >> 6;
	b[3] = (int)(tri_byte & 0x3F);
	
	for(i = 0 ; i < 4 ; ++i){
		enc[i] = table[b[i]];
	}
	
	buf = enc;
	return enc;
}

char* encode(char* enc_data , uint8_t* plane_data , size_t data_size , char* table){

	char* tmp_enc_data = NULL;
	int appendix , padding;
	int e_block;
	int buf_size = 0;
	int bias = 0;
	int i , j , k;
	
	
	appendix = data_size % 3;
	e_block = data_size / 3;
	
	padding = get_padding_bit(data_size);

	buf_size = e_block * ENC_BLOCK;
	buf_size += padding == 0 ? 1 : 2; 
	
	enc_data = (char*)malloc(sizeof(char) * buf_size);
	if(enc_data == NULL){
		return NULL;
	}
	
	j = 0;
	for(i = 0 ; i < e_block ; ++i){
		tmp_enc_data = tri_byte_encode(tmp_enc_data , plane_data[j] , plane_data[j + 1] , plane_data[j + 2] , table);
		for(k = 0 ; k < 4 ; ++k){
			enc_data[bias + k] = tmp_enc_data[k];
		}
		bias += 4;
		j += 3;
	}
	
	switch(appendix){
		case 1:
			tmp_enc_data = one_byte_encode(tmp_enc_data , plane_data[j] , padding , table);
			for(k = 0 ; k < 4 ; ++k){
				enc_data[bias + k] = tmp_enc_data[k];
			}
			bias += 4;
			break;
		case 2:
			tmp_enc_data = twin_byte_encode(tmp_enc_data , plane_data[j] , plane_data[j + 1] , padding , table);
			for(k = 0 ; k < 4 ; ++k){
				enc_data[bias + k] = tmp_enc_data[k];
			}
			bias += 4;
			break;
		default:
			break;
	}
	
	enc_data[bias] = '\0';
	return enc_data;
}

int main(int argc , char* argv[]){
	
	uint8_t* data;
	char* enc_data = NULL;
	char* table;
	int data_size;

	data_size = 8;	
	data = (uint8_t*)malloc(sizeof(uint8_t) * data_size);
	if(data == NULL){
		fprintf(stderr , "Cannot alloc memory");
		return 1;
	}
	data[0] = 'A';
	data[1] = 'B';
	data[2] = 'C';
	data[3] = 'D';
	data[4] = 'E';
	data[5] = 'F';
	data[6] = 'G';
	data[7] = '\0';
	data_size = strlen((char*)data);

	
	if(argc >= 2){
		free((void*)data);
		data_size = strlen(argv[1]);
		data = (uint8_t*)malloc(sizeof(uint8_t) * data_size + 1);
		if(data == NULL){
			fprintf(stderr , "Cannot alloc memory");
			return 1;
		}
		memcpy(data , argv[1] , data_size + 1);
	}
	
	table = create_table();
	enc_data = encode(enc_data , data , data_size , table);
	
	printf("plane  : %s\n" , (char*)data);
	printf("Base64 : %s\n" , enc_data);

	
	free((void*)enc_data);
	free((void*)data);
	
	
	return 0;
}