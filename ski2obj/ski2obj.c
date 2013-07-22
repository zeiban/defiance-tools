#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "..\wadlib\wadlib.h"
#include "..\rmidlib\rmidlib.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define RELEASE_VERSION 1

typedef void (* WriteFaceData)(FILE * file, uint32_t offset, void * data, uint32_t index);

void PrintBits8(FILE * file, uint8_t bits) {
	int i;
	for(i=0; i<8; i++) {
		if (bits & 1)
			fprintf(file, "1");
		else
			fprintf(file, "0");
		bits >>= 1;
	}
}

void PrintBits16(FILE * file, uint16_t bits) {
	int i;
	for(i=0; i<16; i++) {
		if (bits & 1)
			fprintf(file, "1");
		else
			fprintf(file, "0");
		bits >>= 1;
	}
}

int DumpFloats(uint8_t * data, uint32_t size, uint32_t count, char * filename) {
	FILE * file;
	uint32_t v, o;
	float * f;
	if(fopen_s(&file, filename, "w") != 0) {
		printf("ERROR: Unable to open file %s\n", filename);
		return 1;
	}
	for(v=0; v < count; v++) {
		for(o = 0; o < size; o+=4) {
			f = (float*)(data + (v * size) + o);
			fprintf(file,"%10f ",*f);
		}
		fprintf(file,"\n");
	}

	fclose(file);
	return 0;
}

/*
void ObjWritePosition(FILE * file, uint16_t format, uint8_t * vertex_data) {
	float x, y, z;
	if(format & UNCOMPRESSED) {
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
void WriteFace16(FILE * file, uint32_t offset, void * data, uint32_t index) {
	mes_face_16 * face = (mes_face_16 * ) data;
	fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
		offset + face[index].v3+1, offset + face[index].v3+1, offset + face[index].v3+1,
		offset + face[index].v2+1, offset + face[index].v2+1, offset + face[index].v2+1,
		offset + face[index].v1+1, offset + face[index].v1+1, offset + face[index].v1+1);
}


void WriteFace32(FILE * file, uint32_t offset, void * data, uint32_t index) {
	mes_face_32 * face = (mes_face_32 * ) data;
	fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
		offset + face[index].v3+1, offset + face[index].v3+1, offset + face[index].v3+1,
		offset + face[index].v2+1, offset + face[index].v2+1, offset + face[index].v2+1,
		offset + face[index].v1+1, offset + face[index].v1+1, offset + face[index].v1+1);
}
*/

void Usage(void)
{
	printf("Usage ski2obj.exe  [-w <wad_dir>] [-o <output_dir>] [-s <search_name>] [-c] [-f] [-v]\n");
	printf("Extracts Defiance skinned meshes and converts them to OBJ files\n");
	printf("-w\t Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-f\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
	printf("-n\t (Optional) Creates a sub directory under the <output_dir> with the name mesh\n");
	printf("-v\t (Optional) Verbose output. Outputs more information than you proably want to know.\n");
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t f, r;

	wad_dir wd;
	wad_record2 * wr;

	void * out_data = NULL;
	uint32_t out_size = 0;
	const char * wad_dir = NULL;
	const char * out_dir = NULL;
	const char * search_name = NULL;
	uint32_t create_wad_dir = 0;
	uint32_t create_name_dir = 0;
	char wad_out_dir[256];
	char basename[256];
	char full_out_dir[512];
	
	printf("Defiance Skinned Mesh Extraction Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

	for(i=0; i<argc; i++) {
		if(strcmp(argv[i],"-w") == 0) {
			if(argc>i) {
				wad_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-o") == 0) {
			if(argc>i) {
				out_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-s") == 0) {
			if(argc>i) {
				search_name = argv[++i];
			}
		}  else if(strcmp(argv[i],"-f") == 0) {
			create_wad_dir = 1;
		}  else if(strcmp(argv[i],"-n") == 0) {
			create_name_dir = 1;
		} 
	}
	
	if(wad_dir == NULL) {
		printf("-w required\n");
		Usage();
		return 1;
	}

	if(out_dir == NULL) {
		out_dir = ".";
	}

	printf("Input: %s\n", wad_dir);
	if(search_name != NULL) {
		printf("Search String: \"%s\"\n", search_name);
	}

	printf("Output directory: %s\n", out_dir);

	printf("Loading WAD files");
	if(WadDirLoad(&wd, wad_dir) != 0) {
		printf(" Failed to open WAD files in %s\n", wad_dir);
		return;
	}

	printf(" %d files loaded\n",wd.total_files);

	for(f = 0; f < wd.total_files; f++) {
		for(r = 0; r < wd.files[f].total_records; r++) {
			wr = &wd.files[f].records[r];
			if(wr->type == RMID_TYPE_SKI) {
				if(WadRecordResolveName(wr) != 0) {
					printf("Failed to resolve name for ID 0x%08X\n", EndianSwap(wr->id));
					continue;
				}

				if(((search_name != NULL) && (strstr(wr->name,search_name) != NULL)) || search_name == NULL) {

					if(create_wad_dir) {
						_splitpath_s(wr->filename,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
						sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
						_mkdir(wad_out_dir);
					} else {
						strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
					}

					if(create_name_dir) {
						if(create_wad_dir) {
							sprintf_s(full_out_dir, sizeof(full_out_dir),"%s\\%s",wad_out_dir,wr->name);
						} else {
							sprintf_s(full_out_dir, sizeof(full_out_dir),"%s\\%s",out_dir,wr->name);
						}
						_mkdir(full_out_dir);
					} else {
						strcpy_s(full_out_dir, sizeof(full_out_dir), wad_out_dir); 
					}

					printf("0x%08X %s ", EndianSwap(wr->id), wr->name);

					if(WadWriteSkiToObj(&wd, wr, full_out_dir) != 0) {
						printf("Failed to write OBJ/MTL file\n");
					} else {
						printf("\n");
					}

				}
			}
		}
	} 
	WadDirFree(&wd);
}
