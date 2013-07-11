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
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t f;
	uint32_t r;
	uint32_t m;
	uint32_t p;
	wad_dir wd;

	wad_file2 * wf;
	wad_record2 * wr;
	wad_record2 * twr;
	wad_record2 * swr;

	rmid_file rf;
	FILE * in_file;
	void * out_data = NULL;
	unsigned int out_size = 0;

	const char * wad_dir = NULL;
	const char * out_dir = NULL;
	const char * search_name = NULL;

	uint8_t * data;
	mes_header * mh;
	mes_mesh_record * mesh_records;
	mes_material_record * material_records;
	uint32_t * mesh_material_records;
	uint32_t * total_meshes;
	mes_mesh_header * mesh_header;
	mes_material_header * material_header;
	mes_material_param * material_params;

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
		} 
	}
	
	if(wad_dir == NULL) {
		printf("-i required\n");
		Usage();
		return 1;
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

					fseek(in_file, wr->data_offset, SEEK_SET);

					if(RmidLoad(in_file, wr->data_size, &rf) != 0) {
						printf("ERROR: Failed to load RMID data\n");
						fclose(in_file);
						continue;
					}
	//				_mkdir(wad_out_dir);

					data = (uint8_t *)rf.data;

					mh = (mes_header*)(data + sizeof(rmid_header));
				
					mesh_records = (mes_mesh_record*)(data + mh->mesh_table_offset);
					mesh_material_records = (uint32_t*)(data + mh->mesh_material_table_offset);
					material_records = (mes_material_record*)(data + mh->material_table_offset);
					total_meshes = (uint32_t *)&material_records[mh->total_materials];

					printf("0x%08X %s\n", EndianSwap(wr->id), wr->name);

					printf("Materials\n");
					for(m = 0;  m < mh->total_materials; m++) {
						material_header = (mes_material_header *)(data + material_records[m].offset);	
//						printf(" [%d] %d", m, material_records[m].offset);
//						printf(" %d", material_records[m].size);
						swr = WadDirFindByID(&wd, material_header->shader_id);
						if(swr == NULL) {
							printf("Unable to find shader ID 0x%08X", EndianSwap(material_header->shader_id));
							continue;
						}
						WadRecordResolveName(swr);
						printf(" Shader %s\n", swr->name);
//						printf("\n", material_header->total_material_params, material_header->shader_id);

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
						
					}
					RmidFree(&rf);
					fclose(in_file);
				}
			}
		}
	} 
	WadDirFree(&wd);

	/*
	while(WadRecordRead(&wf, &wr, 1) > 0) {
		if(((search_name != NULL) && (strstr(wr.name,search_name) != NULL)) || search_name == NULL) {
			if(wr.type == RMID_TYPE_MES) {

				if(fopen_s(&in_file, wad_file, "rb") != 0) {
					printf("ERROR: Unable to open file %s\n", wad_file);
					continue;
				}
				fseek(in_file, wr.offset, SEEK_SET);

				if(RmidLoad(in_file, wr.size, &rf) != 0) {
					printf("ERROR: Failed to load RMID file\n");
					continue;
				}
//				_mkdir(wad_out_dir);

				data = (uint8_t *)rf.data;

				mh = (mes_header*)(data + sizeof(rmid_header));
				
				mesh_records = (mes_mesh_record*)(data + mh->mesh_table_offset);
				mesh_material_records = (uint32_t*)(data + mh->mesh_material_table_offset);
				material_records = (mes_material_record*)(data + mh->material_table_offset);
				total_meshes = (uint32_t *)&material_records[mh->total_materials];

				printf("0x%08X %s\n", EndianSwap(wr.id), wr.name);

				printf("Materials\n");
				for(i2 = 0;  i2 < mh->total_materials; i2++) {
					material_header = (mes_material_header *)(data + material_records[i2].offset);	
					printf(" [%d] %d", i2, material_records[i2].offset);
					printf(" %d", material_records[i2].size);
					printf(" P=%d SID=0x%08X", material_header->total_material_params, EndianSwap(material_header->shader_id));
					printf("\n", material_header->total_material_params, material_header->shader_id);

					material_params = (mes_material_param *)(data + material_records[i2].offset + sizeof(mes_material_header));
					for(pi = 0; pi < material_header->total_material_params; pi++) {
						printf("\t[%d] T=0x%08X V=0x%08X\n", pi, EndianSwap(material_params[pi].param_type), EndianSwap(material_params[pi].texture_id));
					}
				}

				printf("Meshes\n");
				for(mi = 0;  mi < *total_meshes; mi++) {
					mesh_header = (mes_mesh_header*)(data + mesh_records[mi].offset);	
					printf(" [%d] O=%d", mi, mesh_records[mi].offset);
					printf(" S=%d", mesh_records[mi].size);
					printf(" BPV=%d", mesh_header->bytes_per_vertex);
					printf(" V=%d", mesh_header->num_vertices1);
					printf(" I=%d\n", mesh_header->num_indices1);

				}
				RmidFree(&rf);
				fclose(in_file);
			}
		} 
	}
	WadClose(&wf);
	printf("\n");*/
}
