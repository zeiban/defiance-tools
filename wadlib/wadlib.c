#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wadf.h"
#include "wadlib.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

int WadOpen(wad_file * wf, const char * filename)
{
	wadf_header wadfh;
	wadf_index_header wadfih;

	if(wf == NULL) return 1;
	if(filename == NULL) return 1;

	if(fopen_s(&wf->file, filename,"rb") != 0)
	{
		return 1;
	}
	strcpy_s(wf->name, strlen(filename)+1, filename);
	fread(&wadfh, sizeof(wadf_header), 1, wf->file);

	if(wadfh.magic != WADF_MAGIC)
	{
		fclose(wf->file);
		return 1;
	}

	fread(&wadfih,sizeof(wadf_index_header),1, wf->file);

	wf->total_records = wadfh.total_records;
	wf->total_records_read = 0;
	wf->index_records_read = 0;
	wf->total_index_records = wadfih.num_records;
	wf->next_index_header_offset = wadfih.next_header_offset;
	return 0;
}

int WadRecordRead(wad_file * wf, wad_record * wr, int read_name)
{
	wadf_index_header wadfih;
	wadf_index_record wadfir;
	long offset;

	if(wf->total_records_read < wf->total_records)
	{
		if(wf->index_records_read == wf->total_index_records)
		{
			wf->index_records_read = 0;
			if(wf->next_index_header_offset != 0) 
			{
				fseek(wf->file, wf->next_index_header_offset,SEEK_SET);
				fread(&wadfih,sizeof(wadf_index_header),1, wf->file);
				wf->next_index_header_offset = wadfih.next_header_offset;
				wf->total_index_records = wadfih.num_records;
			}
			else
			{
				return 0;
			}

		}

		fread(&wadfir,sizeof(wadfir),1, wf->file);
		wr->offset = wadfir.data_offset;
		wr->size = wadfir.data_size;
		wr->type = wadfir.type;
		wr->id = wadfir.id;
		wr->name_offset = wadfir.name_offset;

		if(read_name == 1)
		{
			offset = ftell(wf->file);
			fseek(wf->file, wr->name_offset, SEEK_SET);
			fread(&wr->name, sizeof(wr->name),1, wf->file);
			fseek(wf->file, offset, SEEK_SET);
		}
		wf->index_records_read++;
		wf->total_records_read++;
		return 1;

	}
	return 0;
}

int WadRecordToFile(wad_file * wf, wad_record * wr, const char * filename) {
	FILE * file;
	long offset;
	long bytes_read = 0;
	int bytes_to_copy;
	char buffer[2048];
	if(fopen_s(&file, filename, "wb") != 0) {
		return 1;
	}

	offset = ftell(wf->file);
	fseek(wf->file, wr->offset, SEEK_SET);

	while(bytes_read < wr->size) {
		bytes_to_copy = MIN(sizeof(buffer),wr->size - bytes_read);
		bytes_read += fread(&buffer, 1,bytes_to_copy, wf->file); 
		fwrite(&buffer, 1, bytes_to_copy, file);
	}
	fseek(wf->file, offset, SEEK_SET);
	fclose(file);
	return 0;
}

void WadClose(wad_file * wf) {
	fclose(wf->file);
}


int WadFileLoad(wad_file2 * wf, const char * filename)  {
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
	wf->records = (wad_record2 * )malloc(sizeof(wad_record2) * wh.total_records);

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
			wf->records[total_records_read].data_size = wir.data_size;
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
int WadRecordResolveName(wad_record2 * wr) {
	FILE * file;
	if(fopen_s(&file, wr->filename, "rb") != 0) {
		return 1;
	}
	fclose(file);
	return 0;
}

void WadFileFree(wad_file2 * wf) {
	free(wf->filename);
	free(wf->records);
}

