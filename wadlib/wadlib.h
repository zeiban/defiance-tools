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
	char * filename;
	uint32_t id;
	uint32_t type;
	uint64_t name_offset;
	char * name;
	uint64_t data_offset;
	uint64_t data_size;
} wad_record2;

typedef struct
{
	int8_t * filename;
	uint32_t total_records;
	wad_record2 * records;
} wad_file2;

typedef struct {
	uint32_t total_files;
	wad_file2 * files;	
} wad_dir;

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


// Loads all the index records of a WAD file
int WadFileLoad(wad_file2 * wf, const char * filename);

// Resolves the asset name of the specified WAD index record
int WadRecordResolveName(wad_record2 * wr);

// Must be called after WadFileLoad to free memory
void WadFileFree(wad_file2 * wf);

int WadDirLoad(wad_dir * wd, const char * filename);
wad_record2 * WadDirFindByID(wad_dir * wd, uint32_t id);
int WadDirFree(wad_dir * wd);


#endif // _WADLIB_H_