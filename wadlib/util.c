#include <stdio.h>
#include <stdint.h>

void PrintBits8(FILE * file, uint8_t bits) {
	int i;
	for(i=0; i<8; i++) {
		if (bits & 1)
			fprintf(file, "1");
		else
			fprintf(file, "0");
		bits >>= 1;
	}
}

void PrintBits16(FILE * file, uint16_t bits) {
	int i;
	for(i=0; i<16; i++) {
		if (bits & 1)
			fprintf(file, "1");
		else
			fprintf(file, "0");
		bits >>= 1;
	}
}

int DumpFloats(uint8_t * data, uint32_t size, uint32_t count, char * filename) {
	FILE * file;
	uint32_t v, o;
	float * f;
	if(fopen_s(&file, filename, "w") != 0) {
		printf("ERROR: Unable to open file %s\n", filename);
		return 1;
	}
	for(v=0; v < count; v++) {
		for(o = 0; o < size; o+=4) {
			f = (float*)(data + (v * size) + o);
			fprintf(file,"%10f ",*f);
		}
		fprintf(file,"\n");
	}

	fclose(file);
	return 0;
}

uint32_t EndianSwap(uint32_t x) {
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}
