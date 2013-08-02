#ifndef _WADLIB_H_
#define _WADLIB_H_

#ifdef WIN32
#include <windows.h>
#endif

#ifdef _DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 4
#define VERSION_PATCH 0
#define VERSION_SUFFIX ""

#include <stdint.h>

#include "wadf.h"
#include "rmid.h"
#include "util.h"

uint32_t EndianSwap(uint32_t x);

typedef struct {
	char * filename;
	uint32_t id;
	uint32_t type;
	uint64_t name_offset;
	char * name;
	uint64_t data_offset;
	uint64_t data_size;
	uint64_t modified_time;
} wad_record;

typedef struct {
	char * filename;
	uint32_t total_records;
	wad_record * records;
} wad_file;

typedef struct {
	uint32_t total_files;
	wad_file * files;	
} wad_dir;

// Loads all the index records of a WAD file
int WadFileLoad(wad_file * wf, const char * filename);

// Resolves the asset name of the specified WAD index record
int WadRecordResolveName(wad_record * wr);

// Must be called after WadFileLoad to free memory
void WadFileFree(wad_file * wf);

// Loads directory with WAD files and retives all the indexes
int WadDirLoad(wad_dir * wd, const char * filename);

// Finds a WAD record by ID
wad_record * WadDirFindByID(wad_dir * wd, uint32_t id);
wad_record * WadDirFindByName(wad_dir * wd, const char * name);

// Should always be called after WadDirLoad
void WadDirFree(wad_dir * wd);

int WadWriteRecordToRmid(wad_record * wr,  const char * dir, const char * name);

int WadWriteTexToPng(wad_record * wr, int y_invert, uint32_t opaque_alpha, const char * dir, const char * name);
int WadWriteTexToDds(wad_record * wr, int y_invert, uint32_t opaque_alpha, const char * dir, const char * name);

int WadWriteMesToObj(wad_dir * wd, wad_record * wr,  uint32_t opaque_alpha, const char * dir);
int WadWriteSkiToObj(wad_dir * wd, wad_record * wr,  uint32_t opaque_alpha, uint32_t level_of_detail, const char * dir);

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
int RmidWriteTexToPng(rmid_file * rf,  uint32_t y_invert, uint32_t no_alpha, const char * dir, const char * name);

int WadWriteSndToWav(wad_record * wr, const char * redist_dir, const char * out_dir, const char * name);

#endif // _WADLIB_H_