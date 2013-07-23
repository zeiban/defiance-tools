#ifndef _WADF_H_
#define _WADF_H_

#include <stdint.h>

#define WADF_REC_TYPE_CON	0x0001 

#define WADF_MAGIC 'FDAW'

typedef struct {
	unsigned int magic; 
	unsigned int unk1; 
	unsigned int total_records; 
	unsigned int unk2; 
	unsigned int unk3; 
	unsigned int unk4; 
	unsigned int unk5; 
	unsigned int unk6; 
} wadf_header;

typedef struct {
	unsigned int num_records; 
	unsigned int next_header_offset; 
	unsigned int unk1; 
	unsigned int unk2; 
} wadf_index_header;

typedef struct {
	unsigned int id; // Unique Asset ID
	unsigned int data_offset; 
	unsigned int data_size; 
	unsigned int name_offset; 
	uint64_t modified_time;
	unsigned int type; 
	unsigned int null1; // Always 0  
} wadf_index_record;

#endif // _WADF_H_