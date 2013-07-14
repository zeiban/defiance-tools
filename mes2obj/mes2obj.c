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
			return s << 31; 
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
			return (s << 31) | 0x7f800000; 
		else // NaN 
			return (s << 31) | 0x7f800000 | (f << 13); 
	} 
	
	e = e + (127 - 15); 
	f = f << 13; 
	
	return ((s << 31) | (e << 23) | f); 
}

float HALFToFloat(unsigned short y) { 
	union { float f; unsigned int i; } v;
	v.i = halfToFloatI(y); 
	return v.f; 
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
	void * vertex_data;
	mes_vertex_64 * vertex64;
	
		printf("WORK IN PROGRESS DOES NOT WORK!!!!\n");
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
					for(m = 0;  m < mh->total_materials; m++) {
						material_header = (mes_material_header *)(data + material_records[m].offset);	
						swr = WadDirFindByID(&wd, material_header->shader_id);
						if(swr == NULL) {
							printf("Unable to find shader ID 0x%08X", EndianSwap(material_header->shader_id));
							continue;
						}
						WadRecordResolveName(swr);
						printf(" Shader %s\n", swr->name);

						material_params = (mes_material_param *)(data + material_records[m].offset + sizeof(mes_material_header));
						for(p = 0; p < material_header->total_material_params; p++) {
							printf("  [%d] T=0x%08X V=0x%08X ", p, EndianSwap(material_params[p].param_type), EndianSwap(material_params[p].texture_id));
							if((twr = WadDirFindByID(&wd, material_params[p].texture_id)) == NULL) {
								printf("\n");
							} else {
								WadRecordResolveName(twr);
								printf("%s\n", twr->name);
							}
						}
					}

					printf("Meshes\n");
					for(m = 0;  m < *total_meshes; m++) {
						mesh_header = (mes_mesh_header*)(data + mesh_records[m].offset);	

						printf(" [%d] O=%d", m, mesh_records[m].offset);
						printf(" S=%d", mesh_records[m].size);
						printf(" BPV=%d", mesh_header->bytes_per_vertex);
						printf(" V=%d", mesh_header->num_vertices1);
						printf(" I=%d\n", mesh_header->num_indices1);

						vertex_data = (void*)(data + mesh_records[m].offset + mesh_header->vertex_data_offset);
						vertex64 = (mes_vertex_64*)vertex_data;
						for(v = 0; v < mesh_header->num_vertices1; v++) {
							printf(" %d [%f %f %f] [%f %f %f] [%f %f]\n", 
								sizeof(mes_vertex_64),
								HALFToFloat(vertex64[v].position.x), HALFToFloat(vertex64[v].position.y), HALFToFloat(vertex64[v].position.z), 
								vertex64[v].normal.x, vertex64[v].normal.y, vertex64[v].normal.z, 
								vertex64[v].texcoord.x, vertex64[v].texcoord.y);
						}
						/*	
						sprintf_s(out_filename, sizeof(out_filename),"%s\\%s-%d.obj",wad_out_dir,basename, m);
						if(fopen_s(&out_file, out_filename, "wb") != 0) {
							printf("ERROR: Unable to open file %s\n", wr->filename);
							continue;
						}

						fclose(out_file);
						*/
					}
					RmidFree(&rf);
					fclose(in_file);
				}
			}
		}
	} 
	WadDirFree(&wd);
}
