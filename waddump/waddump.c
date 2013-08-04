#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "wadlib.h"

void Usage(void) {
	printf("Usage waddump.exe [-x] [-w wad_dir] [-o output_dir] [-s search_name] [-f]\n");
	printf("Lists or Extract RMID assets from WAD files\n");
	printf("-x\t Extracts assets to RMID files\n");
	printf("-w\t Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	printf("-o\t Directory to output RMID files. Current directory is used if not specified\n");
	printf("-s\t (Optional) Only extracts files that have search_name in the name\n");
	printf("-f\t (Optional) Extraction creates a sub directory under the output_dir with the name of the WAD file\n");
	printf("-d\t (Optional) Display internal asset datetime stamp\n");
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
	uint32_t create_wad_dir = 0;
	uint32_t create_name_dir = 0;
	uint32_t display_datetime = 0;
	char wad_out_dir[256];
	char basename[256];
	char full_out_dir[512];
	uint32_t extract_files = 0;
	struct tm time;
	char date_time[256];
	uint32_t total_records = 0;

	printf("Defiance Tools WAD Dump Utility by Zeiban v%d.%d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_SUFFIX);

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
		}  else if(strcmp(argv[i],"-x") == 0) {
			extract_files = 1;
		}  else if(strcmp(argv[i],"-d") == 0) {
			display_datetime = 1;
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

	printf("Input: %s\n", wad_dir);
	if(search_name != NULL) {
		printf("Search String: \"%s\"\n", search_name);
	}

	printf("Output directory: %s\n", out_dir);

	if(WadDirLoad(&wd, wad_dir) != 0) {
		printf(" Failed to open WAD files in %s", wad_dir);
		return;
	}
	total_records = 0;
	for(f = 0; f < wd.total_files; f++) {
		total_records += wd.files[f].total_records;
	}
	printf("Loaded %d records from %d WAD files\n\n",total_records, wd.total_files);

	for(f = 0; f < wd.total_files; f++) {
		for(r = 0; r < wd.files[f].total_records; r++) {
			wr = &wd.files[f].records[r];
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

				printf("0x%08X 0x%02X ",  EndianSwap(wr->id), (uint16_t)wr->type);
				if(display_datetime) {
					localtime_s(&time,&wr->modified_time);
					asctime_s(date_time, sizeof(date_time), &time);
					printf("%02d\\%02d\\%d %02d:%02d:%02d ", 
						time.tm_mon, time.tm_mday, 1900+time.tm_year,  
						time.tm_hour, time.tm_min, time.tm_sec);
				}

				printf("%s ", wr->name);

				if(extract_files) {
					if(WadWriteRecordToRmid(wr, full_out_dir, NULL) != 0) {
						printf("Failed to write RMID file\n");
						continue;
					}
				}
				printf("\n");
				free(wr->name);
				wr->name = NULL;
			}
		}
	} 
	WadDirFree(&wd);
}
