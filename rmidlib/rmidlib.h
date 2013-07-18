#ifndef _RMIDLIB_H_
#define _RMIDLIB_H_

#include "rmid.h"

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
int RmidWriteTexToPng(rmid_file * rf,  const char * dir, const char * name);
#endif //_RMIDLIB_H_