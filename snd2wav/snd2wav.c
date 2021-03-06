#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "wadlib.h"

void Usage(void)
{
	printf("Usage snd2png.exe  [-w <wad_dir>] [-o <output_dir>] [-s <search_name>] [-f]\n");
	printf("Extracts Defiance audio and converts them to WAV files\n");
	printf("-w\t Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	printf("-m\t Miles Sound System Redist directory. Usually in <game install dir>\\live\\MilesRedist \n");
	printf("-o\t (Optional) Directory to output WAV files otherwise the current directory is used\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-f\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
	printf("-h\t Displays this information\n");
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
	const char * redist_dir = NULL;
	uint32_t create_wad_dir = 0;
	uint32_t create_name_dir = 0;
	char wad_out_dir[256];
	char basename[256];
	char full_out_dir[512];

	printf("Defiance Tools Audio Extraction Utility by Zeiban v%d.%d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_SUFFIX);

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
		} else if(strcmp(argv[i],"-m") == 0) {
			if(argc>i) {
				redist_dir = argv[++i];
			}
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

	if(redist_dir == NULL) {
		printf("-m required\n");
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

	if(WadMilesStartup(redist_dir) != 0) {
		printf("Failed Startup MSS");
		return;
	}

	for(f = 0; f < wd.total_files; f++) {
		for(r = 0; r < wd.files[f].total_records; r++) {
			wr = &wd.files[f].records[r];
			if(wr->type == RMID_TYPE_SND) {
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
						strcpy_s(full_out_dir, sizeof(wad_out_dir), out_dir); 
					}
	
					printf("0x%08X %s ", EndianSwap(wr->id), wr->name);
					
					if(WadWriteSndToWav(wr, full_out_dir, wr->name) != 0) {
						printf("Failed to write WAV file\n");
					} else {
						printf("\n");
					}
					
				}
			}
		}
	} 
	WadMilesShutdown();
	WadDirFree(&wd);
}
