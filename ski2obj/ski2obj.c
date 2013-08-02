#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "wadlib.h"
#include "util.h"


void Usage(void)
{
	printf("Extracts Defiance skinned meshes and converts them to OBJ, MTL, and PNG files\n");
	printf("Usage: ski2obj.exe  [-w dir] [-o dir] [-s search] [-c] [-f] [-n depth] [-oa]\n");
	printf("					[-lod level] [-mml level]\n");

	printf("-w   (Required) Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	
	printf("-o   (Optional) Directory to output OBJ, MTL & PNG files otherwise the current\n"); 
	printf("     directory is used.\n");

	printf("-s   (Optional) Only extracts files that have \"search\" in the name.\n");

	printf("-f   (Optional) Creates a sub directory under the \"-o dir\" with the name of\n");
	printf("     the WAD file. Can be combined with -n.\n");

	printf("-n   (Optional) Creates a sub directory under the \"-o dir\" with the name mesh\n");
	printf("     If \"depth\" is specified it will create sub directories for strings between\n");
	printf("     the \"_\" in the mesh name.  Can be combined with -f\n");

	printf("-oa  (Optional) Alpha channel opaque in texture output. \n");

	printf("-lod (Optional) Specific level of detail mesh to extract. 1=High, 2=Medium, 2=Low\n");
	printf("	  Not all meshes have multiple LoDs. If not specified all LoDs are extracted\n");

	printf("-mml (Optional) Specific mipmap level to extract for textures. 0 is the largest\n");
	printf("	  decreasing by a factor of 2 as the level get higher. Defaults to 0 \n");

	printf("-h Displays this information\n");
}

int main( int argc, const char* argv[])
{
	int i;
	uint32_t f, r;

	wad_dir wd;
	wad_record * wr;

	void * out_data = NULL;
	uint32_t out_size = 0;
	const char * wad_dir = NULL;
	const char * out_dir = NULL;
	const char * search_name = NULL;
	uint32_t create_wad_dir = 0;
	uint32_t opaque_alpha = 0;
	uint32_t level_of_detail = 0;
	uint32_t mipmap_level = 0;
	char wad_out_dir[256];
	char basename[256];
	char name[512];
	char temp_out_dir[512];
	char full_out_dir[512];
	char * name_end = NULL;
	int32_t name_tok_level = -1;
	char * token = NULL, * next_token;
	
	printf("Defiance Tools Skinned Mesh Extraction Utility by Zeiban v%d.%d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_SUFFIX);

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
			if(argc>i) { 
				name_tok_level = strtol(argv[i+1], NULL, 10);
				if(name_tok_level != 0) {
					i++;
				}
			}
		}  else if(strcmp(argv[i],"-lod") == 0) {
			if(argc>i) { 
				level_of_detail = strtol(argv[i+1], NULL, 10);
				if(level_of_detail != 0) {
					i++;
				}
			}
		}  else if(strcmp(argv[i],"-mml") == 0) {
			if(argc>i) { 
				mipmap_level = strtol(argv[i+1], NULL, 10);
				if(mipmap_level != 0) {
					i++;
				}
			}
		} else if(strcmp(argv[i],"-oa") == 0) {
			opaque_alpha = 1;
		}  else if(strcmp(argv[i],"-h") == 0) {
			Usage();
			return 1;
		} else {
			printf("Warning: Unknown switch %s\n", argv[i]);
		}
	}
	
	if(wad_dir == NULL) {
		printf("-w required\n");
		Usage();
		return 1;
	}

	if(out_dir == NULL) {
		out_dir = ".";
	} else {
		if(!DirectoryExists(out_dir)) {
			printf("The output directoy %s doesn't exist\n", out_dir);
			return 1;
		}
	}

	
	printf("Input WAD Directory: %s\n", wad_dir);
	if(search_name != NULL) {
		printf("Search String: \"%s\"\n", search_name);
	}

	printf("Create WAD output directories: ");
	if(create_wad_dir != 0) {
		printf("Yes\n");
	} else {
		printf("No\n");
	}

	printf("Create asset name directories: ");
	if(name_tok_level != -1) {
		if(name_tok_level == 0) {
			printf("Yes\n");
		} else if (name_tok_level > 0) {
			printf("Yes (depth %d)\n", name_tok_level);
		}
	} else {
		printf("No\n");
	}

	printf("Level of Detail: ");
	if(level_of_detail == 0) {
		printf("All\n");
	} else if (level_of_detail > 0) {
		printf("%d\n", level_of_detail);
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
						sprintf_s(full_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
						_mkdir(full_out_dir);
					} else {
						strcpy_s(full_out_dir, sizeof(full_out_dir), out_dir); 
					}
					
					if(name_tok_level > 0) {
						i = name_tok_level;
						strcpy_s(name, sizeof(name), wr->name);
						do {
							token = strtok_s(i == name_tok_level ? name : NULL, "_", &next_token);
							if(token != NULL) {
								name_end = next_token;
								sprintf_s(temp_out_dir, sizeof(temp_out_dir),"%s\\%s",full_out_dir,token);
								strcpy_s(full_out_dir, sizeof(full_out_dir), temp_out_dir); 
								_mkdir(full_out_dir);
							}
							i--;
						} while(i > 0 && token != NULL);

						if(token != NULL) {
							sprintf_s(temp_out_dir, sizeof(temp_out_dir),"%s\\%s",full_out_dir,name_end);
							strcpy_s(full_out_dir, sizeof(full_out_dir), temp_out_dir); 
							_mkdir(full_out_dir);
						}
					} else if (name_tok_level == 0){
						sprintf_s(temp_out_dir, sizeof(temp_out_dir),"%s\\%s",full_out_dir,wr->name);
						strcpy_s(full_out_dir, sizeof(full_out_dir), temp_out_dir); 
						_mkdir(full_out_dir);
					}

					printf("0x%08X %s ", EndianSwap(wr->id), wr->name);

					if(WadWriteSkiToObj(&wd, wr, opaque_alpha, level_of_detail, mipmap_level, full_out_dir) != 0) {
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
