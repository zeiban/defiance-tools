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
	printf("Usage mes2obj.exe  [-i <wad_file>] [-o <output_dir>] [-s <search_name>] [-c]\n");
	printf("Extracts Defiance meshes and converts them to OBJ files\n");
	printf("-i\t Input WAD filename\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t pi;
	uint32_t mi;
	wad_file wf;
	wad_record wr;
	wad_file2 wf2;
	rmid_file rf;
	FILE * in_file;
	void * out_data = NULL;
	unsigned int out_size = 0;
	char basename[256];
	char wad_out_dir[256];

	const char * wad_file = NULL;
	const char * out_dir = NULL;
	const char * search_name = NULL;
	const char * mss_redist_dir = NULL;
	int create_dir = 0;
	uint8_t * data;
	mes_header * mh;
	mes_mesh_record * mesh_records;
	mes_material_record * material_records;
	uint32_t * mesh_material_records;
	uint32_t * total_meshes;
	uint32_t i2;
	mes_mesh_header * mesh_header;
	mes_material_header * material_header;
	mes_material_param * material_params;

	printf("Defiance Mesh Extraction Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

	for(i=0; i<argc; i++) {
		if(strcmp(argv[i],"-i") == 0) {
			if(argc>i) {
				wad_file = argv[++i];
			}
		} else if(strcmp(argv[i],"-o") == 0) {
			if(argc>i) {
				out_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-s") == 0) {
			if(argc>i) {
				search_name = argv[++i];
			}
		} else if(strcmp(argv[i],"-c") == 0) {
			create_dir = 1;
		} 
	}
	
	if(wad_file == NULL) {
		printf("-i required\n");
		Usage();
		return 1;
	}


	if(create_dir) {
		_splitpath_s(wad_file,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
		sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
	} else {
		strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
	}
	
	WadFileLoad(&wf2, wad_file);
	for(mi = 0; mi < wf2.total_records; mi++) {
		printf("%d\n", wf2.records[mi].type);
	}
	WadFileFree(&wf2);

	if(WadOpen(&wf, wad_file) != 0) {
		printf("Failed to open WAD file %s", wad_file);
		return;
	}

	printf("Input: %s\n", wad_file);
	if(search_name != NULL) {
		printf("Search String: \"%s\"\n", search_name);
	}

	printf("Output directory: %s\n", wad_out_dir);
	printf("Processing %d records\n", wf.total_records);

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
	printf("\n");
}
