#include <stdio.h>
#include <stdint.h>

#include "rmid.h"

float halfToFloatI(unsigned short y) 
{ 
	int s = (y >> 15) & 0x00000001; // sign 
	int e = (y >> 10) & 0x0000001f; // exponent 
	int f = y & 0x000003ff; // fraction 
	
	// need to handle 7c00 INF and fc00 -INF? 
	if (e == 0) 
	{ 
		// need to handle +-0 case f==0 or f=0x8000? 
		if (f == 0) // Plus or minus zero 
			return (float)(s << 31); 
		else { // Denormalized number -- renormalize it 
			while (!(f & 0x00000400)) 
			{ 
				f <<= 1; e -= 1; 
			} 
			e += 1; 
			f &= ~0x00000400; 
		} 
	} 
	else if (e == 31) 
	{ 
		if (f == 0) // Inf 
			return (float)((s << 31) | 0x7f800000); 
		else // NaN 
			return (float)((s << 31) | 0x7f800000 | (f << 13)); 
	} 
	
	e = e + (127 - 15); 
	f = f << 13; 
	
	return (float)(((s << 31) | (e << 23) | f)); 
}

float HALFToFloat(unsigned short y) { 
	union { float f; unsigned int i; } v;
	v.i = (uint32_t)halfToFloatI(y); 
	return v.f; 
}

void ObjWritePosition(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data) {
	float x, y, z;
	if(mh->vertex_format & UNCOMPRESSED) {
		x = 	*(float*)(vertex_data);
		y = 	*(float*)(vertex_data + 4);
		z = 	*(float*)(vertex_data + 8);
	} else {
		x = 	HALFToFloat(*(uint16_t*)(vertex_data));
		y = 	HALFToFloat(*(uint16_t*)(vertex_data + 2));
		z = 	HALFToFloat(*(uint16_t*)(vertex_data + 4));
	}

	fprintf(file, "v %f %f %f\n", x, y, z);

}

void ObjWriteNormal(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data) {
	float x, y, z;
	if(mh->vertex_format & NORMAL) {
		if(mh->vertex_format & UNCOMPRESSED) {
			vertex_data += 12;
		} else {
			vertex_data += 8;
		}
		x = 	*(float*)(vertex_data);
		y = 	*(float*)(vertex_data + 4);
		z = 	*(float*)(vertex_data + 8);
	} else {
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	fprintf(file, "vn %f %f %f\n", x, y, z);

}

void ObjWriteTexCoord(FILE * file, mes_ski_mesh_header * mh, uint8_t * vertex_data) {
	uint32_t offset = 0;
	float u, v;

	if(mh->vertex_format & UNCOMPRESSED) {
		offset += 12; 
	} else {
		offset += 8; 
	}
	if(mh->vertex_format & NORMAL) {
		offset += 12; 
	}
	if(mh->vertex_format & TANGENT) {
		offset += 12; 
	}
	if(mh->vertex_format & BITANGENT) {
		offset += 12; 
	}
	if(mh->vertex_format & UNKNOWN) {
		offset += 4; 
	}

	if(mh->uncompressed_texcoord_flags & UNCOMPRESSED_TEXCOORD_1) {
		u = 	*(float*)(vertex_data + offset);
		v = 	*(float*)(vertex_data + offset + 4);
	} else if(mh->compressed_texcoord_flags & COMPRESSED_TEXCOORD_1) {
		u = 	HALFToFloat(*(uint16_t *) (vertex_data + offset));
		v = 	HALFToFloat(*(uint16_t *) (vertex_data + offset + 2));
	} else {
		u = 0.0;
		v = 0.0;
	}

	fprintf(file, "vt %f %f\n", u, v);

}

void ObjWriteFace16(FILE * file, uint32_t offset, void * data, uint32_t index) {
	mes_face_16 * face = (mes_face_16 * ) data;
	fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
		offset + face[index].v3+1, offset + face[index].v3+1, offset + face[index].v3+1,
		offset + face[index].v2+1, offset + face[index].v2+1, offset + face[index].v2+1,
		offset + face[index].v1+1, offset + face[index].v1+1, offset + face[index].v1+1);
}

void ObjWriteFace32(FILE * file, uint32_t offset, void * data, uint32_t index) {
	mes_face_32 * face = (mes_face_32 * ) data;
	fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
		offset + face[index].v3+1, offset + face[index].v3+1, offset + face[index].v3+1,
		offset + face[index].v2+1, offset + face[index].v2+1, offset + face[index].v2+1,
		offset + face[index].v1+1, offset + face[index].v1+1, offset + face[index].v1+1);
}
