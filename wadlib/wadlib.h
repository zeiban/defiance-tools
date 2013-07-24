#ifndef _WADLIB_H_
#define _WADLIB_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <stdint.h>

#include "wadf.h"
#include "rmid.h"

uint32_t EndianSwap(uint32_t x);

/*
typedef struct {
	FILE * file;
	char name[256];
	unsigned int total_records;
	unsigned int total_records_read;
	long next_index_header_offset;
	unsigned int total_index_records;
	unsigned int index_records_read;
} wad_file;

*/
/*
typedef struct {
	int id;
	unsigned int type;
	long name_offset;
	char name[256];
	long offset;
	long size;
} wad_record;
*/
typedef struct {
	char * filename;
	uint32_t id;
	uint32_t type;
	uint64_t name_offset;
	char * name;
	uint64_t data_offset;
	uint64_t data_size;
	uint64_t modified_time;
} wad_record2;

typedef struct {
	char * filename;
	uint32_t total_records;
	wad_record2 * records;
} wad_file2;

typedef struct {
	uint32_t total_files;
	wad_file2 * files;	
} wad_dir;
/*
// Opens a wad file for reading
int WadOpen(wad_file * wf, const char * filename);

// Reads 1 record from the wad file 
int WadRecordRead(wad_file * wf, wad_record * wr, int read_name);

// Extracts data associated with record to a file.  
int WadRecordToFile(wad_file * wf, wad_record * wr, const char * filename);

// Closes the wad file should always be called after WadOpen()
void WadClose(wad_file * wf);
*/

// Loads all the index records of a WAD file
int WadFileLoad(wad_file2 * wf, const char * filename);

// Resolves the asset name of the specified WAD index record
int WadRecordResolveName(wad_record2 * wr);

// Must be called after WadFileLoad to free memory
void WadFileFree(wad_file2 * wf);

// Loads directory with WAD files and retives all the indexes
int WadDirLoad(wad_dir * wd, const char * filename);

// Finds a WAD record by ID
wad_record2 * WadDirFindByID(wad_dir * wd, uint32_t id);
wad_record2 * WadDirFindByName(wad_dir * wd, const char * name);

// Should always be called after WadDirLoad
void WadDirFree(wad_dir * wd);

int WadWriteRecordToRmid(wad_record2 * wr,  const char * dir, const char * name);

int WadWriteTexToPng(wad_record2 * wr, int y_invert, const char * dir, const char * name);
int WadWriteMesToObj(wad_dir * wd, wad_record2 * wr,  const char * dir);
int WadWriteSkiToObj(wad_dir * wd, wad_record2 * wr,  const char * dir);

typedef struct 
{
	unsigned int id; // Unique Resource Media ID 
	unsigned int version; // Seems to always be the same for each type 
	unsigned short num_references; // Number of rmid_reference structs at the end of the file
	unsigned short type;     
	unsigned int magic; // Always 'RMID' 
} rmid_header;

typedef struct 
{
	uint32_t id; // Asset ID
	uint32_t type; // Asset type
} rmid_reference;

typedef struct
{
	rmid_header * header;
	uint64_t size;
	void * data;
	rmid_reference * references; 
} rmid_file;

int RmidLoad(FILE * file, uint64_t size, rmid_file * rf);
int RmidWriteToFile(rmid_file * rf, FILE * file);
void RmidFree(rmid_file * rf);

int RmidLoadFromFile(const char * filename, uint64_t offset, uint64_t size, rmid_file * rf);
int RmidWriteTexToPng(rmid_file * rf,  uint32_t y_invert, const char * dir, const char * name);

int WadWriteSndToWav(wad_record2 * wr, const char * redist_dir, const char * out_dir, const char * name);

#endif // _WADLIB_H_