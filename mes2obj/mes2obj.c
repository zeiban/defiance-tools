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
#define RELEASE_VERSION 0

typedef void (* WriteVertexData)(FILE * file, void * data, uint32_t index);
typedef void (* WriteFaceData)(FILE * file, uint32_t offset, void * data, uint32_t index);

typedef struct {
	uint32_t type;
	uint32_t size;
} vertex_element;

/*
#define FLOAT2 1
#define HALFFLOAT2 2
#define FLOAT3 3
#define HALFFLOAT3 4

typedef struct {
	char * name;
	vertex_element position;
	vertex_element normal;
	vertex_element texcoord;
 } vertex_format; 

vertex_format formats [] = {
	{"shd_p_DeferredDualDiffuseSpecularNormal", {0,FLOAT3},{12,FLOAT3},{24,FLOAT2}},
	{"shd_p_DeferredDualDiffuseSpecularDualNormal", {0,FLOAT3},{12,FLOAT3},{24,FLOAT2}}
};
*/

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

// mes_vertex_12
void WriteVertexPositon12(FILE * file, void * data, uint32_t index) {
	mes_vertex_12 * vertex = (mes_vertex_12 *)data;
	fprintf(file, "v %f %f %f\n", HALFToFloat(vertex[index].position.x), HALFToFloat(vertex[index].position.y), HALFToFloat(vertex[index].position.z));
}

void WriteVertexNormal12(FILE * file, void * data, uint32_t index) {
	mes_vertex_12 * vertex = (mes_vertex_12 *)data;
	fprintf(file, "vn %f %f %f\n", HALFToFloat(vertex[index].normal.x), HALFToFloat(vertex[index].normal.y), HALFToFloat(vertex[index].normal.z));
}

void WriteVertexTexCoord12(FILE * file, void * data, uint32_t index) {
	fprintf(file, "vt 0.0 0.0\n");
}

// mes_vertex_28
void WriteVertexPositon28(FILE * file, void * data, uint32_t index) {
	mes_vertex_28 * vertex = (mes_vertex_28 *)data;
	fprintf(file, "v %f %f %f\n", vertex[index].position.x, vertex[index].position.y, vertex[index].position.z);
}

void WriteVertexNormal28(FILE * file, void * data, uint32_t index) {
	fprintf(file, "vn 0.0 0.0 0.0\n");
}

void WriteVertexTexCoord28(FILE * file, void * data, uint32_t index) {
	mes_vertex_28 * vertex = (mes_vertex_28 *)data;
	fprintf(file, "vt %f %f\n", vertex[index].texcoord.x, vertex[index].texcoord.y);
}

// mes_vertex_32
void WriteVertexPositon32(FILE * file, void * data, uint32_t index) {
	mes_vertex_32 * vertex = (mes_vertex_32 *)data;
	fprintf(file, "v %f %f %f\n", vertex[index].position.x, vertex[index].position.y, vertex[index].position.z);
}

void WriteVertexNormal32(FILE * file, void * data, uint32_t index) {
	mes_vertex_32 * vertex = (mes_vertex_32 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].normal.x, vertex[index].normal.y, vertex[index].normal.z);
}

void WriteVertexTexCoord32(FILE * file, void * data, uint32_t index) {
	// No texcoord
	fprintf(file, "vt 0.0 0.0\n");
}

// mes_vertex_36
void WriteVertexPositon36(FILE * file, void * data, uint32_t index) {
	mes_vertex_36 * vertex = (mes_vertex_36 *)data;
	fprintf(file, "v %f %f %f\n", vertex[index].position.x, vertex[index].position.y, vertex[index].position.z);
}

void WriteVertexNormal36(FILE * file, void * data, uint32_t index) {
	// No normal
	fprintf(file, "vn 0.0 0.0 0.0\n");
}

void WriteVertexTexCoord36(FILE * file, void * data, uint32_t index) {
	mes_vertex_36 * vertex = (mes_vertex_36 *)data;
	fprintf(file, "vt %f %f\n", vertex[index].texcoord.x, vertex[index].texcoord.y);
}

// mes_vertex_52
void WriteVertexPositon52(FILE * file, void * data, uint32_t index) {
	mes_vertex_52 * vertex = (mes_vertex_52 *)data;
	fprintf(file, "v %f %f %f\n", HALFToFloat(vertex[index].position.x), HALFToFloat(vertex[index].position.y), HALFToFloat(vertex[index].position.z));
}

void WriteVertexNormal52(FILE * file, void * data, uint32_t index) {
	mes_vertex_52 * vertex = (mes_vertex_52 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].normal.x, vertex[index].normal.y, vertex[index].normal.z);
}

void WriteVertexTexCoord52(FILE * file, void * data, uint32_t index) {
	mes_vertex_52 * vertex = (mes_vertex_52 *)data;
	fprintf(file, "vt %f %f\n", HALFToFloat(vertex[index].texcoord.x), HALFToFloat(vertex[index].texcoord.y));
}

// mes_vertex_56
void WriteVertexPositon56(FILE * file, void * data, uint32_t index) {
	float x, y, z;
	int sz = sizeof(mes_vertex_56);
	mes_vertex_56 * vertex = (mes_vertex_56 *)data;
	x = HALFToFloat(vertex[index].position.x);
	y = HALFToFloat(vertex[index].position.y);
	z = HALFToFloat(vertex[index].position.z);
	fprintf(file, "v %f %f %f\n", HALFToFloat(vertex[index].position.x), HALFToFloat(vertex[index].position.y), HALFToFloat(vertex[index].position.z));
}

void WriteVertexNormal56(FILE * file, void * data, uint32_t index) {
	mes_vertex_56 * vertex = (mes_vertex_56 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].normal.x, vertex[index].normal.y, vertex[index].normal.z);
}

void WriteVertexTexCoord56(FILE * file, void * data, uint32_t index) {
	mes_vertex_56 * vertex = (mes_vertex_56 *)data;
	fprintf(file, "vt %f %f\n", HALFToFloat(vertex[index].texcoord2.x), HALFToFloat(vertex[index].texcoord2.y));
}

// mes_vertex_60
void WriteVertexPositon60(FILE * file, void * data, uint32_t index) {
	mes_vertex_60 * vertex = (mes_vertex_60 *)data;
	fprintf(file, "v %f %f %f\n", vertex[index].position.x, vertex[index].position.y, vertex[index].position.z);
}

void WriteVertexNormal60(FILE * file, void * data, uint32_t index) {
	mes_vertex_60 * vertex = (mes_vertex_60 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].normal.x, vertex[index].normal.y, vertex[index].normal.z);
}

void WriteVertexTexCoord60(FILE * file, void * data, uint32_t index) {
	mes_vertex_60 * vertex = (mes_vertex_60 *)data;
	fprintf(file, "vt %f %f\n", vertex[index].texcoord.x, vertex[index].texcoord.y);
}

// mes_vertex_64
void WriteVertexPositon64(FILE * file, void * data, uint32_t index) {
	mes_vertex_64 * vertex = (mes_vertex_64 *)data;
	fprintf(file, "v %f %f %f\n", HALFToFloat(vertex[index].position.x), HALFToFloat(vertex[index].position.y), HALFToFloat(vertex[index].position.z));
}

void WriteVertexNormal64(FILE * file, void * data, uint32_t index) {
	mes_vertex_64 * vertex = (mes_vertex_64 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].normal.x, vertex[index].normal.y, vertex[index].normal.z);
}

void WriteVertexTexCoord64(FILE * file, void * data, uint32_t index) {
	mes_vertex_64 * vertex = (mes_vertex_64 *)data;
	fprintf(file, "vt %f %f\n", vertex[index].texcoord1.x, vertex[index].texcoord1.y);
}

// mes_vertex_68
void WriteVertexPositon68(FILE * file, void * data, uint32_t index) {
	int sz = sizeof(mes_vertex_68);
	mes_vertex_68 * vertex = (mes_vertex_68 *)data;
	fprintf(file, "v %f %f %f\n", vertex[index].position.x, vertex[index].position.y, vertex[index].position.z);
}

void WriteVertexNormal68(FILE * file, void * data, uint32_t index) {
	mes_vertex_68 * vertex = (mes_vertex_68 *)data;
	fprintf(file, "vn %f %f %f\n", vertex[index].bitangent.x, vertex[index].bitangent.y, vertex[index].bitangent.z);
}

void WriteVertexTexCoord68(FILE * file, void * data, uint32_t index) {
	mes_vertex_68 * vertex = (mes_vertex_68 *)data;
	fprintf(file, "vt %f %f\n", vertex[index].texcoord1.x, vertex[index].texcoord1.y);
}

unsigned int EndianSwap(unsigned int x)
{
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}



void Usage(void)
{
	printf("Usage mes2obj.exe  [-w <wad_dir>] [-s <search_name>] [-o <output_dir>]\n");
	printf("Extracts Defiance meshes and converts them to OBJ files\n");
	printf("-w\t Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-c\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t f;
	uint32_t r;
	uint32_t m;
	uint32_t v;
	uint32_t p;
	uint32_t idx;
	wad_dir wd;

	wad_record2 * wr;
	wad_record2 * twr;
	wad_record2 * swr;

	rmid_file rf;
	FILE * in_file;
	FILE * out_file;
	char out_filename[256];
	void * out_data = NULL;
	uint32_t out_size = 0;

	const char * wad_dir = NULL;
	const char * out_dir = NULL;
	const char * search_name = NULL;
	char wad_out_dir[256];
	uint32_t create_dir = 0;
	char basename[256];

	uint8_t * data;
	mes_header * mh;
	mes_mesh_record * mesh_records;
	mes_material_record * material_records;
	uint32_t * mesh_material_records;
	uint32_t * total_meshes;
	mes_mesh_header * mesh_header;
	mes_material_header * material_header;
	mes_material_param * material_params;
	uint64_t index_data_size;
	void * vertex_data;
	void * face_data;
	rmid_file trf;
	uint32_t offset = 0;

	WriteVertexData wvp;
	WriteVertexData wvn;
	WriteVertexData wvtc;
	WriteFaceData wfd;

	printf("Defiance Mesh Extraction Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

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
		}  else if(strcmp(argv[i],"-c") == 0) {
			create_dir = 1;
		} 
	}
	
	if(wad_dir == NULL) {
		printf("-i required\n");
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

	printf("Loading WAD files\n");
	if(WadDirLoad(&wd, wad_dir) != 0) {
		printf("Failed to open WAD files in %s", wad_dir);
		return;
	}
	printf("%d files loaded\n",wd.total_files);
	for(f = 0; f < wd.total_files; f++) {
		for(r = 0; r < wd.files[f].total_records; r++) {
			wr = &wd.files[f].records[r];
			if(wr->type == RMID_TYPE_MES) {
				if(WadRecordResolveName(wr) != 0) {
					printf("Failed to resolve name for ID 0x%08X\n", EndianSwap(wr->id));
				}

				if(((search_name != NULL) && (strstr(wr->name,search_name) != NULL)) || search_name == NULL) {
					if(fopen_s(&in_file, wr->filename, "rb") != 0) {
						printf("ERROR: Unable to open file %s\n", wr->filename);
						continue;
					}

					if(create_dir) {
						_splitpath_s(wr->filename,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
						sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
					} else {
						strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
					}
					
					_mkdir(wad_out_dir);
	
					fseek(in_file, (long)wr->data_offset, SEEK_SET);

					if(RmidLoad(in_file, (long)wr->data_size, &rf) != 0) {
						printf("ERROR: Failed to load RMID data\n");
						fclose(in_file);
						continue;
					}

					data = (uint8_t *)rf.data;

					mh = (mes_header*)(data + sizeof(rmid_header));
				
					mesh_records = (mes_mesh_record*)(data + mh->mesh_table_offset);
					mesh_material_records = (uint32_t*)(data + mh->mesh_material_table_offset);
					material_records = (mes_material_record*)(data + mh->material_table_offset);
					total_meshes = (uint32_t *)&material_records[mh->total_materials];

					printf("0x%08X %s\n", EndianSwap(wr->id), wr->name);


					printf("Materials %d\n", mh->total_materials);

					sprintf_s(out_filename, sizeof(out_filename),"%s\\%s.mtl",wad_out_dir,wr->name);
					if(fopen_s(&out_file, out_filename, "w") != 0) {
						printf("ERROR: Unable to open file %s\n", out_filename);
						fclose(in_file);
						continue;
					}

					for(m = 0;  m < mh->total_materials; m++) {
						material_header = (mes_material_header *)(data + material_records[m].offset);	
						swr = WadDirFindByID(&wd, material_header->shader_id);
						if(swr == NULL) {
							printf("Unable to find shader ID 0x%08X", EndianSwap(material_header->shader_id));
							continue;
						}
						WadRecordResolveName(swr);
						printf(" Shader %s\n", swr->name);

						fprintf(out_file, "newmtl %s_%d\n", swr->name, m);
						fprintf(out_file, "Ka 1.000 1.000 1.000\n");
						fprintf(out_file, "Kd 1.000 1.000 1.000\n");
						fprintf(out_file, "Ks 0.000 0.000 0.000\n");
						fprintf(out_file, "d 1.0\n");

						material_params = (mes_material_param *)(data + material_records[m].offset + sizeof(mes_material_header));
						for(p = 0; p < material_header->total_material_params; p++) {
							printf("  [%d] T=0x%08X V=0x%08X ", p, EndianSwap(material_params[p].param_type), EndianSwap(material_params[p].texture_id));
							if((twr = WadDirFindByID(&wd, material_params[p].texture_id)) == NULL) {
								printf("\n");
							} else {
								WadRecordResolveName(twr);
								if(material_params[p].param_type == RMID_MAT_PARAM_COLOR1) {
									RmidLoadFromFile(twr->filename, twr->data_offset, twr->data_size, &trf);
									RmidWriteTexToPng(&trf, wad_out_dir, twr->name); 
									fprintf(out_file, "map_Ka %s.png\n", twr->name);
									fprintf(out_file, "map_Kd %s.png\n", twr->name);
									RmidFree(&trf);
								}
								printf("%s\n", twr->name);
							}
						}
					}
					fclose(out_file);

					printf("Meshes\n");

					sprintf_s(out_filename, sizeof(out_filename),"%s\\%s.obj",wad_out_dir,wr->name);
					if(fopen_s(&out_file, out_filename, "w") != 0) {
						printf("ERROR: Unable to open file %s\n", out_filename);
						fclose(in_file);
						continue;
					}
					fprintf(out_file, "# Generated with mes2obj\n");
					fprintf(out_file, "mtllib %s.mtl\n", wr->name);

					offset = 0;
					for(m = 0;  m < *total_meshes; m++) {
						mesh_header = (mes_mesh_header*)(data + mesh_records[m].offset);	


						printf(" [%d] O=%d", m, mesh_records[m].offset);
						printf(" S=%d", mesh_records[m].size);
						printf(" BPV=%d", mesh_header->bytes_per_vertex);
						printf(" V=%d", mesh_header->num_vertices1);
						printf(" I=%d\n", mesh_header->num_indices1);

						if(mesh_header->bytes_per_vertex == 12) {
							wvp = WriteVertexPositon12;
							wvn = WriteVertexNormal12;
							wvtc = WriteVertexTexCoord12;
						} else if(mesh_header->bytes_per_vertex == 28) {
							wvp = WriteVertexPositon28;
							wvn = WriteVertexNormal28;
							wvtc = WriteVertexTexCoord28;
						}  else if(mesh_header->bytes_per_vertex == 32) {
							wvp = WriteVertexPositon32;
							wvn = WriteVertexNormal32;
							wvtc = WriteVertexTexCoord32;
						}  else if(mesh_header->bytes_per_vertex == 36) {
							wvp = WriteVertexPositon36;
							wvn = WriteVertexNormal36;
							wvtc = WriteVertexTexCoord36;
						}  else if(mesh_header->bytes_per_vertex == 60) {
							wvp = WriteVertexPositon60;
							wvn = WriteVertexNormal60;
							wvtc = WriteVertexTexCoord60;
						} else if(mesh_header->bytes_per_vertex == 64) {
							wvp = WriteVertexPositon64;
							wvn = WriteVertexNormal64;
							wvtc = WriteVertexTexCoord64;
						} else if(mesh_header->bytes_per_vertex == 68) {
							wvp = WriteVertexPositon68;
							wvn = WriteVertexNormal68;
							wvtc = WriteVertexTexCoord68;
						} else if(mesh_header->bytes_per_vertex == 52) {
							wvp = WriteVertexPositon52;
							wvn = WriteVertexNormal52;
							wvtc = WriteVertexTexCoord52;
						} else if(mesh_header->bytes_per_vertex == 56) {
							wvp = WriteVertexPositon56;
							wvn = WriteVertexNormal56;
							wvtc = WriteVertexTexCoord56;
						} else {
							continue;
						}
							material_header = (mes_material_header *)(data + material_records[m].offset);
							swr = WadDirFindByID(&wd, material_header->shader_id);
							/*
							sprintf_s(out_filename, sizeof(out_filename),"%s\\%s-%s-%d.verts",wad_out_dir,wr->name, swr->name, m);
							DumpFloats(
								data + mesh_records[m].offset + mesh_header->vertex_data_offset, 
								mesh_header->bytes_per_vertex, 
								mesh_header->num_vertices1,
								out_filename);
								*/
						index_data_size = mesh_records[m].size  - (mesh_header->index_data_offset - *(data + mesh_records[m].offset));
						
						if((index_data_size / 2) == mesh_header->num_indices1) {
							wfd = WriteFace16;
						} else {
							wfd = WriteFace32;
						}

						fprintf(out_file, "o %s_%d\n", wr->name, m);
						vertex_data = (void*)(data + mesh_records[m].offset + mesh_header->vertex_data_offset);

						// Vertices
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							wvp(out_file, vertex_data, v);
						}

						// Texture Coords
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							wvtc(out_file, vertex_data, v);
						}

						// Normals
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							wvn(out_file, vertex_data, v);
						}

						material_header = (mes_material_header *)(data + material_records[m].offset);	
						swr = WadDirFindByID(&wd, material_header->shader_id);

						fprintf(out_file, "usemtl %s_%d\n", swr->name, m);
						fprintf(out_file, "s off\n");
						// Faces
						face_data = (void *)(data + mesh_records[m].offset + mesh_header->index_data_offset);
						fprintf(out_file, "# Faces %d\n",mesh_header->num_indices1/3);
						for(idx = 0; idx < mesh_header->num_indices1/3; idx++) {
							wfd(out_file, offset, face_data, idx);
						}
						offset += mesh_header->num_vertices1;
					}
					fclose(out_file);
					fclose(in_file);
				}
			}
		}
	} 
	WadDirFree(&wd);
}
