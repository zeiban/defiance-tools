#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "png.h"

#include "dxt.h"
#include "wadf.h"
#include "wadlib.h"
#include "objfile.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define CHUNK 1024

static int WadFileLoad(wad_file * wf, const char * filename)  {
	FILE * file;
	wadf_header wh;
	wadf_index_header wih;
	wadf_index_record wir;
	
	uint32_t records_read;
	uint32_t total_records_read = 0;

	if(fopen_s(&file, filename, "rb") != 0) {
		return 1;
	}

	fread(&wh, sizeof(wadf_header), 1, file);

	if(wh.magic != WADF_MAGIC) {
		fclose(file);
		return 1;
	}
	wf->filename = (int8_t *)malloc(strlen(filename)+1);
	strcpy_s(wf->filename, strlen(filename)+1, filename);
	wf->total_records = wh.total_records;
	wf->records = (wad_record * )malloc(sizeof(wad_record) * wh.total_records);

	while(total_records_read < wh.total_records) {
		fread(&wih,sizeof(wadf_index_header),1, file);
		records_read = 0;
		while(records_read < wih.num_records) {
			fread(&wir, sizeof(wadf_index_record), 1, file);
			wf->records[total_records_read].filename = wf->filename;
			wf->records[total_records_read].id = wir.id;
			wf->records[total_records_read].type = wir.type;
			wf->records[total_records_read].name_offset = wir.name_offset;
			wf->records[total_records_read].data_offset = wir.data_offset;
			wf->records[total_records_read].name = NULL;
			wf->records[total_records_read].data_size = wir.data_size;
			wf->records[total_records_read].modified_time = wir.modified_time;
			records_read++;
			total_records_read++;
		}
		if(wih.next_header_offset != 0) {
			fseek(file, wih.next_header_offset, SEEK_SET);
		}
	}

	fclose(file);
	return 0;
}

static void WadFileFree(wad_file * wf) {
	uint32_t i;
	free(wf->filename);
	for(i=0; i<wf->total_records; i++) {
		if(wf->records[i].name != NULL) {
			free(wf->records[i].name);
		}
	}
	free(wf->records);
}

int WadRecordResolveName(wad_record * wr) {
	FILE * file;
	wadf_header wh;
	char name[256];

	if(wr->name != NULL) {
		return 0;
	}

	if(fopen_s(&file, wr->filename, "rb") != 0) {
		return 1;
	}

	fread(&wh, sizeof(wh), 1, file);
	if(wh.magic != WADF_MAGIC) {
		return 1;
	}

	fseek(file, (long)wr->name_offset, SEEK_SET);
	fread(&name, sizeof(name), 1, file);
	wr->name = (char * )malloc(strlen(name)+1);
	strcpy_s(wr->name, strlen(name)+1, name);

	fclose(file);
	return 0;
}


int WadDirLoad(wad_dir * wd, const char * dir) {
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	char search_dir[1024];
	char wad_filename[1024];
	uint32_t count = 0;

	sprintf_s(search_dir, sizeof(search_dir), "%s\\*.wad",dir);
	
	hFind = FindFirstFile(search_dir, &ffd);
	if(hFind == INVALID_HANDLE_VALUE) {
		return 1;
	}

	do {
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			count++;
		}
	} while(FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
	wd->total_files = count;
	wd->files = (wad_file*)malloc(sizeof(wad_file) * wd->total_files);

	count=0;
	hFind = FindFirstFile(search_dir, &ffd);
	do 
	{
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			sprintf_s(wad_filename, sizeof(wad_filename), "%s\\%s", dir, ffd.cFileName);
			if(WadFileLoad(&wd->files[count], wad_filename) != 0) {
				break;
			}
			count++;
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return 0;
}

wad_record * WadDirFindByID(wad_dir * wd, uint32_t id) {
	uint32_t f;
	uint32_t r;

	for(f = 0; f < wd->total_files; f++) {
		for(r = 0; r < wd->files[f].total_records; r++) {
			if(wd->files[f].records[r].id == id) {
				return &wd->files[f].records[r];
			}
		}
	}
	return NULL;
}
wad_record * WadDirFindByName(wad_dir * wd, const char * name) {
	uint32_t f;
	uint32_t r;

	for(f = 0; f < wd->total_files; f++) {
		for(r = 0; r < wd->files[f].total_records; r++) {
			if(wd->files[f].records[r].name == NULL) {
				WadRecordResolveName(&wd->files[f].records[r]);
			}
			if(strcmp(wd->files[f].records[r].name, name) == 0) {
				return &wd->files[f].records[r];
			}
		}
	}
	return NULL;
}

void WadDirFree(wad_dir * wd) {
	uint32_t i;

	for(i=0; i<wd->total_files; i++) {
		WadFileFree(&wd->files[i]);
	}
	free(wd->files);
}

int WadWriteRecordToRmid(wad_record * wr,  const char * dir, const char * name) {
	FILE * in_file;
	FILE * out_file;
	char filename[512];
	rmid_file rf;

	if(fopen_s(&in_file, wr->filename, "rb") != 0) {
		return 1;
	}

	fseek(in_file, (uint32_t)wr->data_offset, SEEK_CUR);
	if(RmidLoad(in_file,wr->data_size, &rf) != 0) {
		return 1;
	}
	fclose(in_file);

	if(wr->name == NULL) {
		WadRecordResolveName(wr);
	}

	if(name == NULL) {
		sprintf_s(filename, sizeof(filename), "%s\\%s.rmid", dir, wr->name);
	} else {
		sprintf_s(filename, sizeof(filename), "%s\\%s", dir, name);
	}

	if(fopen_s(&out_file, filename, "wb") != 0) {
		RmidFree(&rf);
		return 1;
	}
	
	RmidWriteToFile(&rf, out_file);

	RmidFree(&rf);

	fclose(out_file);
	return 0;
}


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
static int PngWriteToFile(FILE * file, uint32_t y_invert, uint32_t bits_per_pixel, uint32_t width, uint32_t height, uint8_t * image) {
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

	for(y=0; y < height; y++) {
		if(y_invert) {
			row_pointers[y] = image + ((height-y-1) * width * bytes_per_pixel);
		} else {
			row_pointers[y] = image + (y * width * bytes_per_pixel);
		}
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

int RmidWriteTexToPng(rmid_file * rf,  uint32_t y_invert, uint32_t opaque_alpha, uint32_t mipmap_level, const char * dir, const char * name) {
	rmid_tex_header * rmidth;
	uint8_t * bytes;
	uint8_t * data;
	uint32_t * image;
	uint8_t * blocks; 
	FILE * file;
	uint32_t i;
	char filename[256];
	uint32_t m;
	uint32_t width, height;

	rmidth = (rmid_tex_header*)((uint8_t *)rf->data + sizeof(rmid_header));
	bytes = ((uint8_t*)rf->data) + 96;

	data = ((uint8_t *)rf->data) + sizeof(rmid_header) + sizeof(rmid_tex_header);

	if(rmidth->format == 1 || rmidth->format == 3  || rmidth->format == 8) {
		if(mipmap_level > rmidth->mmh1.mipmap_count) {
			mipmap_level = rmidth->mmh1.mipmap_count;
		}

		width = rmidth->mmh1.width;
		height = rmidth->mmh1.height;
		for(i=0; i<mipmap_level; i++) {
			if(i>0) {
				data += (rmidth->mmr1[i-1].size);
			}
			width = rmidth->mmr1[i].width;
			height = rmidth->mmr1[i].height;
		}
	
		width = MAX(width,4);
		height = MAX(height,4);

		image = (uint32_t *) malloc(width * height * 4); 
		
		blocks = data;

		if(rmidth->format == 1) {
			DecompressDXT1(width, height, blocks, image); 
		} else if(rmidth->format == 8 || rmidth->format == 3){
			DecompressDXT5(width, height, blocks, image); 
		}

		if(opaque_alpha) {
			// Set the alpha to 255;
			for(i=0; i < (width * height * 4); i += 4) {
				*(((uint8_t * )image) + i + 3) = 255;
			}
		}

		sprintf_s(filename,sizeof(filename), "%s\\%s.png", dir, name);
		if(fopen_s(&file, filename, "wb") == 0) {
			if(PngWriteToFile(file, y_invert, 32, width, height, (uint8_t *)image) != 0){
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
			if(opaque_alpha) {
				// Set the alpha to 255;
				for(i=0; i < (rmidth->mmh1.width * rmidth->mmh1.height * 4); i += 4) {
					*(((uint8_t * )data) + i + 3) = 255;
				}
			}

			if(PngWriteToFile(file, y_invert, 64,rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)data) != 0){
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
							if(opaque_alpha) {
								// Set the alpha to 255;
								for(i=0; i < (rmidth->mmh1.width * rmidth->mmh1.height * 4); i += 4) {
									*(((uint8_t * )image) + i + 3) = 255;
								}
							}
							if(PngWriteToFile(file, y_invert, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
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
				if(opaque_alpha) {
					// Set the alpha to 255;
					for(i=0; i < (rmidth->mmh1.width * rmidth->mmh1.height * 4); i += 4) {
						*(((uint8_t * )image) + i + 3) = 255;
					}
				}
				if(PngWriteToFile(file, y_invert, 32, rmidth->mmh1.width, rmidth->mmh1.height, (uint8_t *)image) != 0){
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
