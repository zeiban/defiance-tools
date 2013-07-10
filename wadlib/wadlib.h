#ifndef _WADLIB_H_
#define _WADLIB_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <stdint.h>

#include "wadf.h"

typedef struct 
{
	FILE * file;
	char name[256];
	unsigned int total_records;
	unsigned int total_records_read;
	long next_index_header_offset;
	unsigned int total_index_records;
	unsigned int index_records_read;
} wad_file;

typedef struct
{
	int id;
	unsigned int type;
	long name_offset;
	char name[256];
	long offset;
	long size;
} wad_record;

typedef struct
{
	struct wad_file2 * file;
	uint32_t id;
	uint32_t type;
	uint64_t name_offset;
	uint8_t * name;

} wad_record2;

typedef struct
{
	uint8_t * filename;
	uint32_t total_records;
	wad_record2 * records;
} wad_file2;
/*
typedef struct
{
	uint32_t total_files;
	wad_file2 * files;
} wad_dir;
*/
// Opens a wad file for reading
int WadOpen(wad_file * wf, const char * filename);

// Reads 1 record from the wad file 
int WadRecordRead(wad_file * wf, wad_record * wr, int read_name);

// Extracts data associated with record to a file.  
int WadRecordToFile(wad_file * wf, wad_record * wr, const char * filename);

// Closes the wad file should always be called after WadOpen()
void WadClose(wad_file * wf);

int WadLoad(wad_file2 * wf, const uint8_t* filename);
void WadFree(wad_file2 * wf);
//int WadLoadDirectory(wad_dir * wd, uint8_t *);
//void WadFreeDirectory(wad_dir * wd);
#endif // _WADLIB_H_