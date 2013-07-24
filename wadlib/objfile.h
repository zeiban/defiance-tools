#ifndef _OBJFILE_H_
#define _OBJFILE_H_

#include "rmid.h"

/*
	OBJ/MTL Helper Module
*/

void ObjWritePosition(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data);
void ObjWriteNormal(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data);
void ObjWriteTexCoord(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data);
void ObjWriteFace16(FILE * file, uint32_t offset, void * data, uint32_t index);
void ObjWriteFace32(FILE * file, uint32_t offset, void * data, uint32_t index);


#endif // _OBJFILE_H_