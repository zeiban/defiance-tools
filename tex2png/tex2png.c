#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <assert.h>
#include "..\wadlib\wadlib.h"
#include "..\rmidlib\rmidlib.h"
#include "..\libpng\png.h"
#include "dxt.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define RELEASE_VERSION 0

int PngWriteToFile(FILE * file, uint32_t bits_per_pixel, uint32_t width, uint32_t height, uint8_t * image) {
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

	for(y=0; y<height; y++) {
		row_pointers[y] = image + (y * width * bytes_per_pixel);
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

static unsigned int EndianSwap(unsigned int x)
{
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

void Usage(void)
{
	printf("Usage tex2png.exe (-i <wad_file> [-o <output_dir>] [-n <search_name>] [-c]\n");
	printf("Extracts Defiance textures and converts them to PNG files\n");
	printf("-i\t Input WAD filename\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-c\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t m;
	wad_file wf;
	wad_record wr;
	rmid_file rf;
	FILE * in_file;
	FILE * out_file;

	const char * wad_file = NULL;
	const char * out_dir = NULL;
	const char * search_string = NULL;
	int create_dir = 0;
	char basename[256];
	char full_out_dir[256];
	char out_filename[256];
	uint32_t * image;
	uint8_t * bytes;
	rmid_tex_header * rmidth;
	uint8_t * blocks; 
	uint8_t * data; 

	printf("Defiance Texture Extraction Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);
	
	for(i=0; i<argc; i++) 
	{
		if(strcmp(argv[i],"-i") == 0) {
			if(argc>i) 
			{
				wad_file = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-o") == 0) 
		{
			if(argc>i) 
			{
				out_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-s") == 0) 
		{
			if(argc>i) 
			{
				search_string = argv[++i];
			}
		} else if(strcmp(argv[i],"-c") == 0) {
			create_dir = 1;
		} 
	}
	
	if(wad_file == NULL)
	{
		printf("-i required\n");
		Usage();
		return 1;
	}

	if(create_dir) 
	{
		_splitpath_s(wad_file,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
		sprintf_s(full_out_dir, sizeof(full_out_dir),"%s\\%s",out_dir,basename);
	} 
	else 
	{
		strcpy_s(full_out_dir, sizeof(full_out_dir), out_dir); 
	}

	if(WadOpen(&wf, wad_file) != 0)
		{
		printf("Failed to open WAD file %s", wad_file);
		return;
	}

	printf("Input: %s\n", wad_file);
	if(search_string != NULL)
	{
		printf("Search String: \"%s\"\n", search_string);
	}
	printf("Output directory: %s\n", full_out_dir);
	printf("Processing %d records\n", wf.total_records);

	while(WadRecordRead(&wf, &wr, 1) > 0)
	{
		if(((search_string != NULL) && (strstr(wr.name,search_string) != NULL)) || search_string == NULL)
		{
			if(wr.type == RMID_TYPE_TEX){
				
				if(fopen_s(&in_file, wad_file, "rb") != 0)
				{
					printf("ERROR: Unable to open file %s\n", wad_file);
					continue;
				}
				
				fseek(in_file, wr.offset, SEEK_SET);

				if(RmidLoad(in_file, wr.size, &rf) != 0)
				{
					printf("0x%08X %s Failed to load\n", EndianSwap(wr.id), wr.name);
					fclose(in_file);
					continue;
				}

				rmidth = (rmid_tex_header*)((uint8_t *)rf.data + sizeof(rmid_header));

				bytes = ((uint8_t*)rf.data) + 96;
				/*
				printf("0x%08X", EndianSwap(wr.id)); 
				for(i = 0; i<16; i++) {
					printf(" %02X", *(bytes + i));
				}
				*/
				printf("0x%08X %s ", EndianSwap(wr.id),  wr.name);
				fflush(stdout);

				_mkdir(full_out_dir);

				data = ((uint8_t *)rf.data) + sizeof(rmid_header) + sizeof(rmid_tex_header);

				if(rmidth->format == 1 || rmidth->format == 3  || rmidth->format == 8)
				{
					image = (uint32_t *) malloc(rmidth->mmh1.width * rmidth->mmh1.height * 4); 
					blocks = data;

					if(rmidth->format == 1) {
						DecompressDXT1(rmidth->mmh1.width, rmidth->mmh1.height, blocks, image); 
					} else if(rmidth->format == 3 || rmidth->format == 8){
						DecompressDXT5(rmidth->mmh1.width, rmidth->mmh1.height, blocks, image); 
					}

					sprintf_s(out_filename,sizeof(out_filename), "%s\\%s.png", full_out_dir, wr.name);
					if(fopen_s(&out_file, out_filename, "wb") == 0) {
						if(PngWriteToFile(out_file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
							printf("Failed to write PNG file\n");
						}
					} else {
						printf("ERROR: Failed to open output PNG file\n");
					}

					free(image);
					if(out_file != NULL) {
						fclose(out_file);
					}
				} else if(rmidth->format == 6) {
					
					sprintf_s(out_filename,sizeof(out_filename), "%s\\%s.png", full_out_dir, wr.name, i+1);

					if(fopen_s(&out_file, out_filename, "wb") == 0) {
						if(PngWriteToFile(out_file, 64,rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)data) != 0){
							printf("Failed to write PNG file\n");
						}
					} else {
						printf("ERROR: Failed to open output DAT file\n");
					}

					if(out_file != NULL) {
						fclose(out_file);
					}
				} else if(rmidth->format == 0) {
					if(rmidth->unk2 == 1) {
						image = (uint32_t *)data;
						for(i = 0; i < 6; i++) {
							for(m = 0; m < rmidth->mmh1.mipmap_count; m++) {
								// Only save the 1st mipmap 
								if(m == 0) {
									sprintf_s(out_filename,sizeof(out_filename), "%s\\%s-%d.png", full_out_dir, wr.name, i+1);
									if(fopen_s(&out_file, out_filename, "wb") == 0) {
										if(PngWriteToFile(out_file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
											printf("Failed to write PNG file\n");
										}
									} else {
										printf("ERROR: Failed to open output PNG file\n");
									}

									if(out_file != NULL) {
										fclose(out_file);
									}
								}
								image += (rmidth->mmr1[m].width * rmidth->mmr1[m].height);
							}

						}
					} else if (rmidth->unk2 == 0) {
						image = (uint32_t *)data;

						sprintf_s(out_filename,sizeof(out_filename), "%s\\%s.png", full_out_dir, wr.name);
						if(fopen_s(&out_file, out_filename, "wb") == 0) {
							if(PngWriteToFile(out_file, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
								printf("Failed to write PNG file\n");
							}
						} else {
							printf("ERROR: Failed to open output PNG file\n");
						}
						
						if(out_file != NULL) {
							fclose(out_file);
						}
					} else {
						printf("unknown type 0 format\n");
					}
				} else {
					printf("Unsupported format\n");
				}

				fclose(in_file);
				RmidFree(&rf);
				printf("\n");
			}
		} 
	}
	WadClose(&wf);

}
