#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rmidlib.h"
#include "..\zlib\zlib.h"
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

int RmidLoad(FILE * file, long size, rmid_file * rf)
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
		rf->data = malloc(rf->size);

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
		rf->data = malloc(rf->size);
		fread(rf->data, rf->size, 1, file);
		rf->header = (rmid_header *)rf->data;
		return 0;
	}
	return 0;
}

int RmidWriteToFile(rmid_file * rf, FILE * file)
{
	fwrite(rf->data, rf->size, 1, file);
	return 0;
}

void RmidFree(rmid_file * rf)
{
	free(rf->data);
}

