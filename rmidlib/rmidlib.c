#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dxt.h"
#include "rmidlib.h"
#include "..\zlib\zlib.h"
#include "..\libpng\png.h"
#define CHUNK 1024
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
/*
static int CopyFileBytes(FILE * in_file, FILE * out_file, long size)
{
	char buffer[2048];
	long total_bytes_read = 0;
	long bytes_read = 0;
	long bytes_to_copy;

	while(total_bytes_read < size)
	{
		bytes_to_copy = MIN(sizeof(buffer),size - total_bytes_read);
		bytes_read = fread(&buffer, 1,bytes_to_copy, in_file); 
		if(bytes_read != bytes_to_copy)
		{
			return 1;
		}
		total_bytes_read += bytes_read;
		fwrite(&buffer, 1, bytes_read, out_file);
	}
	return 0;
}

static int Inflate(FILE * in_file, FILE * out_file)
{
	z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
	int ret;
	unsigned have;

	strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

	do {
        strm.avail_in = fread(in, 1, CHUNK, in_file);
        if (ferror(in_file)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;

        strm.next_in = in;
		
		do {
			strm.avail_out = CHUNK;
            strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  // state not clobbered 
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     // and fall through 
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, out_file) != have || ferror(out_file)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);

	// clean up and return 
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
int RmidExtractContainer(FILE * in_file, FILE * out_file)
{
	rmid_header rh;
	con_header ch;
	long offset;
	fread(&ch, sizeof(ch), 1, in_file);
	offset = ftell(in_file);
	fread(&rh, sizeof(rh), 1, in_file);
	fseek(in_file, offset, SEEK_SET);

	if(rh.id == ch.id)
	{
		// Not compressed
		CopyFileBytes(in_file, out_file, ch.compressed_size);
	}
	else if(((unsigned short)rh.id) == 40065)
	{
		// zlib compressesed, inflate!
		if(Inflate(in_file, out_file) != Z_OK)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int RmidDecompress(FILE * in_file, FILE * out_file)
{
	if(Inflate(in_file, out_file) != Z_OK)
	{
		return 1;
	}
	return 0;
}

*/
static int InflateToMemory(FILE * file, void * data)
{
	z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
	int ret;
	unsigned have;
	char * cdata = (char *)data;

	strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

	do {
        strm.avail_in = fread(in, 1, CHUNK, file);
        if (ferror(file)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;

        strm.next_in = in;
		
		do {
			strm.avail_out = CHUNK;
            strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  // state not clobbered 
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     // and fall through 
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
            }
            have = CHUNK - strm.avail_out;
			memcpy(cdata,out, have);
			cdata+=have;
			/*
            if (fwrite(out, 1, have, out_file) != have || ferror(out_file)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
			*/
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);

	// clean up and return 
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int RmidLoadFromFile(const char * filename, uint64_t offset, uint64_t size, rmid_file * rf)
{
	FILE * file;
	if(fopen_s(&file, filename, "rb") != 0) {
		return 1;
	}
	fseek(file, (long)offset, SEEK_SET);
	if(RmidLoad(file, size, rf) != 0) {
		return 1;
		fclose(file);
	}
	fclose(file);
	return 0;
}
int RmidLoad(FILE * file, uint64_t size, rmid_file * rf)
{
	rmid_header rh;
	rmid_con_header ch;
	long offset;
	uint8_t * data;
	
	offset = ftell(file);
	fread(&rh, sizeof(rmid_header), 1, file);
	if(rh.magic != RMID_MAGIC)
	{
		return 1;
	}
	//TODO Add ref information
	rf->references = 0;

	if(rh.type == RMID_TYPE_CON)
	{
		fread(&ch, sizeof(rmid_con_header), 1, file);

		offset = ftell(file);
		fread(&rh, sizeof(rmid_header), 1, file);
//		fread(&id, sizeof(int), 1, file);
		fseek(file, offset, SEEK_SET);

		rf->size = ch.uncompressed_size;
		rf->data = malloc((size_t)rf->size);

		if(rh.id == ch.id)
		{
//			rf->size = ch.uncompressed_size - sizeof(rmid_header);
//			rf->data = malloc(rf->size);
//			fread(&rh, sizeof(rmid_header), 1, file);
			if(rh.type == RMID_TYPE_TEX) {
				//872
				data = (uint8_t*)rf->data;
				
				fread(data, 1, sizeof(rmid_header), file);
				rf->header = (rmid_header *)data; 
				data += sizeof(rmid_header);

				fread(data, 1, sizeof(rmid_tex_header), file);
				data += sizeof(rmid_tex_header);

				if(InflateToMemory(file, data) != Z_OK) {
					return 1;
				}
				//rf->header.num_references
			} else {
				return 1;
			}

		} else {
			// Compressed 
//			data = (char*)malloc(ch.uncompressed_size);
			if(InflateToMemory(file, rf->data) != Z_OK) 
			{
				return 1;
			}
			rf->header = (rmid_header *)rf->data;
//			memcpy(rf->data, data, rf->size);
//			memcpy(&rf->header, data, sizeof(rmid_header));
//			memcpy(rf->data, data+sizeof(rmid_header), rf->size);
//			free(data);
		}
	} else {
		fseek(file, offset, SEEK_SET);
		rf->size = size;
		rf->data = malloc((size_t)rf->size);
		fread(rf->data, (size_t)rf->size, 1, file);
		rf->header = (rmid_header *)rf->data;
		return 0;
	}
	return 0;
}

int RmidWriteToFile(rmid_file * rf, FILE * file)
{
	fwrite(rf->data, (size_t)rf->size, 1, file);
	return 0;
}

void RmidFree(rmid_file * rf)
{
	free(rf->data);
}
static int PngWriteToFile(FILE * file, uint32_t bits_per_pixel, uint32_t width, uint32_t height, uint8_t * image) {
	png_byte color_type = PNG_COLOR_TYPE_RGBA;
	png_byte bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;	
	png_bytep * row_pointers;
	png_byte bytes_per_pixel;
	
	uint32_t y;
	
	if(bits_per_pixel == 32) {
		bit_depth  = 8;
	} else if (bits_per_pixel == 64) {
		bit_depth  = 16;
	} else {
		return 1;
	}

	bytes_per_pixel = bit_depth / 2;	

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) {
		return 1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 1;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		return 1;
	}

	png_init_io(png_ptr, file);

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		return 1;
	}

	png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		return 1;
	}

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

	// Invert Y for textures
	for(y=0; y < height; y++) {
		row_pointers[y] = image + ((height-y-1) * width * bytes_per_pixel);
	}
	
	png_write_image(png_ptr, row_pointers);
	
	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		return 1;
	}
	png_write_end(png_ptr, NULL);

	free(row_pointers);

	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	

	return 0;
}

int RmidWriteTexToPng(rmid_file * rf,  const char * dir, const char * name) {
	rmid_tex_header * rmidth;
	uint8_t * bytes;
	uint8_t * data;
	uint32_t * image;
	uint8_t * blocks; 
	FILE * file;
	int i;
	char filename[256];
	uint32_t m;

	rmidth = (rmid_tex_header*)((uint8_t *)rf->data + sizeof(rmid_header));
	bytes = ((uint8_t*)rf->data) + 96;

	data = ((uint8_t *)rf->data) + sizeof(rmid_header) + sizeof(rmid_tex_header);

	if(rmidth->format == 1 || rmidth->format == 3  || rmidth->format == 8)
	{
		image = (uint32_t *) malloc(rmidth->mmh1.width * rmidth->mmh1.height * 4); 
		blocks = data;

		if(rmidth->format == 1) {
			DecompressDXT1(rmidth->mmh1.width, rmidth->mmh1.height, blocks, image); 
		} else if(rmidth->format == 3 || rmidth->format == 8){
			DecompressDXT5(rmidth->mmh1.width, rmidth->mmh1.height, blocks, image); 
		}

		sprintf_s(filename,sizeof(filename), "%s\\%s.png", dir, name);
		if(fopen_s(&file, filename, "wb") == 0) {
			if(PngWriteToFile(file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
				printf("Failed to write PNG file\n");
			}
		} else {
			printf("ERROR: Failed to open output PNG file\n");
		}

		free(image);
		if(file != NULL) {
			fclose(file);
		}
	} else if(rmidth->format == 6) {
		sprintf_s(filename,sizeof(filename), "%s\\%s.png", dir, name);
		if(fopen_s(&file, filename, "wb") == 0) {
			if(PngWriteToFile(file, 64,rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)data) != 0){
				printf("Failed to write PNG file\n");
			}
		} else {
			printf("ERROR: Failed to open output DAT file\n");
		}

		if(file != NULL) {
			fclose(file);
		}
	} else if(rmidth->format == 0) {
		if(rmidth->unk2 == 1) {
			image = (uint32_t *)data;
			for(i = 0; i < 6; i++) {
				for(m = 0; m < rmidth->mmh1.mipmap_count; m++) {
					// Only save the 1st mipmap 
					if(m == 0) {
						sprintf_s(filename,sizeof(filename), "%s\\%s-%d.png", dir, name, i+1);
						if(fopen_s(&file, filename, "wb") == 0) {
							if(PngWriteToFile(file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
								printf("Failed to write PNG file\n");
							}
						} else {
							printf("ERROR: Failed to open output PNG file\n");
						}

						if(file != NULL) {
							fclose(file);
						}
					}
					image += (rmidth->mmr1[m].width * rmidth->mmr1[m].height);
				}

			}
		} else if (rmidth->unk2 == 0) {
			image = (uint32_t *)data;

			sprintf_s(filename,sizeof(filename), "%s\\%s.png", dir, name);
			if(fopen_s(&file, filename, "wb") == 0) {
				if(PngWriteToFile(file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
					printf("Failed to write PNG file\n");
				}
			} else {
				printf("ERROR: Failed to open output PNG file\n");
			}
						
			if(file != NULL) {
				fclose(file);
			}
		} else {
			printf("unknown type 0 format\n");
			return 1;
		}
	} else {
		printf("Unsupported format\n");
		return 1;
	}

	return 0;
}
