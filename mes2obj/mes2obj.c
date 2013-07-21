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

typedef void (* WriteVertexData)(FILE * file, void * data, uint32_t index);
typedef void (* WriteFaceData)(FILE * file, uint32_t offset, void * data, uint32_t index);


#define UNCOMPRESSED		0x01 // Vertex position is uncompressed 12 bytes otherwise compressed 8 bytes
#define UNK_2				0x02
#define UNK 3				0x04
#define NORMAL				0x08 // Vertex has normals
#define TANGENT				0x10 // Vertex has tangent
#define BITANGENT			0x20 // Vertex has Bitangent
#define UNKNOWN				0x40 // Unknown 4 bytes in vertex
#define BIT_8				0x80

#define UNCOMPRESSED_TEXCOORD_1	0x01 // Vertex texcoord1 is uncompressed
#define UNCOMPRESSED_TEXCOORD_2	0x04 // Vertex texcoord2 is uncompressed
#define COMPRESSED_TEXCOORD_1	0x01 // Vertex texcoord1 is compressed
#define COMPRESSED_TEXCOORD_2	0x02 // Vertex texcoord2 is compressed

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

void ObjWriteNormal(FILE * file, mes_mesh_header * mh, uint8_t * vertex_data) {
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

void ObjWriteTexCoord(FILE * file, mes_mesh_header * mh, uint8_t * vertex_data) {
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
		/* OBJ doesn't support more than 1 texcoord;
		offset += 8;
		if(mh->uncompressed_texcoord_flags & UNCOMPRESSED_TEXCOORD_2) {
			u = 	*(float*)(vertex_data + offset);
			v = 	*(float*)(vertex_data + offset + 4);
		} 
		*/
	} else if(mh->compressed_texcoord_flags & COMPRESSED_TEXCOORD_1) {
		u = 	HALFToFloat(*(uint16_t *) (vertex_data + offset));
		v = 	HALFToFloat(*(uint16_t *) (vertex_data + offset + 2));
		/* OBJ doesn't support more than 1 texcoord;
		offset += 4;
			
		if(mh->compressed_texcoord_flags & UNCOMPRESSED_TEXCOORD_2) {
			u = 	HALFToFloat(*(uint16_t *) (vertex_data + offset));
			v = 	HALFToFloat(*(uint16_t *) (vertex_data + offset + 2));
		} 
		*/
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

unsigned int EndianSwap(unsigned int x)
{
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}



void Usage(void)
{
	printf("Usage mes2obj.exe  [-w <wad_dir>] [-o <output_dir>] [-s <search_name>] [-c] [-f] [-v]\n");
	printf("Extracts Defiance meshes and converts them to OBJ files\n");
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

	FILE * log;

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
	uint32_t create_wad_dir = 0;
	uint32_t create_name_dir = 0;
	uint32_t verbose_output = 0;
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
	uint8_t * vertex_byte_data;
	void * face_data;
	rmid_file trf;
	uint32_t offset = 0;
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
		}  else if(strcmp(argv[i],"-f") == 0) {
			create_wad_dir = 1;
		}  else if(strcmp(argv[i],"-n") == 0) {
			create_name_dir = 1;
		}  else if(strcmp(argv[i],"-v") == 0) {
			verbose_output = 1;
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
		printf(" Failed to open WAD files in %s", wad_dir);
		return;
	}

	printf(" %d files loaded\n",wd.total_files);

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
						mesh_header = (mes_mesh_header*)(data + mesh_records[m].offset);
						swr = WadDirFindByID(&wd, material_header->shader_id);
						if(swr == NULL) {
							printf("Unable to find shader ID 0x%08X", EndianSwap(material_header->shader_id));
							continue;
						}
						WadRecordResolveName(swr);
						printf(" Shader %s\n", swr->name);

						fprintf(out_file, "newmtl %s_%d_%d\n", swr->name, mesh_header->bytes_per_vertex, m);
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
						
						remove("c:\\temp\\mes2obj3.log");
						fopen_s(&log, "c:\\temp\\mes2obj3.log", "a");
						for(i=0; i<16; i++) {
							fprintf(log,"%02X ",*((uint8_t*)(&mesh_header->vertex_format) + i));
						}
						
						PrintBits16(log, mesh_header->vertex_format);
						fprintf(log, " ");
						PrintBits8(log, mesh_header->uncompressed_texcoord_flags);
						fprintf(log, " ");
						PrintBits8(log, mesh_header->compressed_texcoord_flags);
						fprintf(log, " ");
						fprintf(log,"%2d ",mesh_header->bytes_per_vertex);
						i = 0;
						/*
						if(mesh_header->vertex_format & UNCOMPRESSED) {
							i += 12; 
						} else {
							i += 8; 
						}
						if(mesh_header->vertex_format & NORMAL) {
							i += 12; 
						}
						if(mesh_header->vertex_format & TANGENT) {
							i += 12; 
						}
						if(mesh_header->vertex_format & BITANGENT) {
							i += 12; 
						}
						if(mesh_header->vertex_format & UNKNOWN) {
//							i += 4; 
						}
						*/
						for(;i < mesh_header->bytes_per_vertex; i+=4) {
							fprintf(log,"%15f ",*(float*)(data + mesh_records[m].offset + mesh_header->vertex_data_offset + i));
						}
						
						fprintf(log, "%s %s\n",swr->name, wr->name);
						fclose(log);
						
						printf(" [%d] O=%d", m, mesh_records[m].offset);
						printf(" S=%d", mesh_records[m].size);
						printf(" BPV=%d", mesh_header->bytes_per_vertex);
						printf(" V=%d", mesh_header->num_vertices1);
						printf(" I=%d\n", mesh_header->num_indices1);

						material_header = (mes_material_header *)(data + material_records[m].offset);
						swr = WadDirFindByID(&wd, material_header->shader_id);
							
						i = 0;

						
						sprintf_s(out_filename, sizeof(out_filename),"%s\\%d-%s-%s-%d.verts",wad_out_dir, m, wr->name, swr->name, mesh_header->bytes_per_vertex);
						DumpFloats(
							data + mesh_records[m].offset + mesh_header->vertex_data_offset, 
							mesh_header->bytes_per_vertex, 
							mesh_header->num_vertices1,
							out_filename);
						
						index_data_size = mesh_records[m].size  - (mesh_header->index_data_offset - *(data + mesh_records[m].offset));
						
						if((index_data_size / 2) == mesh_header->num_indices1) {
							wfd = WriteFace16;
						} else {
							wfd = WriteFace32;
						}

						fprintf(out_file, "o %s_%d\n", wr->name, m);

						// Vertices
						vertex_data = (void*)(data + mesh_records[m].offset + mesh_header->vertex_data_offset);
						vertex_byte_data = (uint8_t *)vertex_data;
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							ObjWritePosition(out_file, mesh_header->vertex_format, vertex_byte_data); 
							vertex_byte_data += mesh_header->bytes_per_vertex;
						}

						// Texture Coords
						vertex_byte_data = (uint8_t *)vertex_data;
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							ObjWriteTexCoord(out_file, mesh_header, vertex_byte_data); 
							vertex_byte_data += mesh_header->bytes_per_vertex;
						}

						// Normals
						vertex_byte_data = (uint8_t *)vertex_data;
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							ObjWriteNormal(out_file, mesh_header, vertex_byte_data);
							vertex_byte_data += mesh_header->bytes_per_vertex;
						}

						// Faces
						material_header = (mes_material_header *)(data + material_records[m].offset);	
						swr = WadDirFindByID(&wd, material_header->shader_id);

						fprintf(out_file, "usemtl %s_%d_%d\n", swr->name, mesh_header->bytes_per_vertex, m);
						fprintf(out_file, "s off\n");
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
