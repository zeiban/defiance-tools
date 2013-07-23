#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\wadlib\wadlib.h"
#include "..\rmidlib\rmidlib.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define RELEASE_VERSION 1

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
/*
unsigned int EndianSwap(unsigned int x) {
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}
*/
void CopyBytes(FILE * from, FILE * to, unsigned int total_bytes)
{
	unsigned int bytes_read = 0;
	int bytes_to_copy;
	char buffer[2048];

	while(bytes_read < total_bytes) {
		bytes_to_copy = MIN(sizeof(buffer),total_bytes - bytes_read);
		bytes_read += fread(&buffer, 1,bytes_to_copy, from); 
		fwrite(&buffer, 1, bytes_to_copy, to);
	}
}
int WriteRecord(const char * in_filename, wad_record * wr, char * out_filename) {
	FILE * in_file;
	FILE * out_file;
	unsigned int bytes_read = 0;
	char error[2048];

	rmid_file rf;

	if(fopen_s(&in_file, in_filename, "rb") != 0) {
		strerror_s(error, sizeof(error),errno);
		printf("Unable to open input file %s %s", in_filename, error);
		return 1;
	}


	fseek(in_file, wr->offset, SEEK_SET);

	if(RmidLoad(in_file, wr->size, &rf) != 0) {
		printf("ERROR: Failed to load RMID file", wr->name);
		fclose(in_file);
		return 1;
	}

	if(fopen_s(&out_file, out_filename, "wb") != 0) {
		printf("Unable to open output file %s", out_filename, error);
		fclose(in_file);
		return 1;
	}

	if(RmidWriteToFile(&rf, out_file) != 0) {
		printf("Failed to write RMID file %s", out_filename, error);
	}

	RmidFree(&rf);
	fclose(out_file);
	fclose(in_file);
	return 0;
}

void Usage(void) {
	printf("Usage waddump.exe [-x] [-w <wad_dir>] [-o <output_dir>] [-s <search_name>] [-f] [-n]\n");
	printf("Lists or Extract RMID assets from WAD files\n");
	printf("-x\t Extracts assets to RMID files\n");
	printf("-w\t Wad directory. eg. c:\\games\\defiance\\live\\wad\n");
	printf("-o\t Directory to output RMID files. Current directory is used if not specified\n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-f\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
	printf("-n\t (Optional) Creates a sub directory under the <output_dir> with the asset name\n");
	printf("-d\t (Optional) Display internal asset time stamp\n");
}
/*
int main( int argc, const char* argv[]) {
	const char * in_filename;
	const char * out_dir;
	const char * search_string = NULL;
	int create_dir = 0;
	int list_only = 0;
	char out_filename[1024];
	char basename[256];
	char wad_out_dir[256];
		
	wad_file wf;
	wad_record wr;
	int count = 0;
	int i = 0;

	printf("Defiance WAD Dump Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

	for(i=0; i<argc; i++) {
		if(strcmp(argv[i],"-i") == 0) {
			if(argc>i) {
				in_filename = argv[++i];
			}
		} else if(strcmp(argv[i],"-o") == 0) {
			if(argc>i) {
				out_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-s") == 0) {
			if(argc>i) {
				search_string = argv[++i];
			}
		} else if(strcmp(argv[i],"-c") == 0) {
			create_dir = 1;
		} else if(strcmp(argv[i],"-l") == 0) {
			list_only = 1;
		} 
	}
	if(in_filename == NULL) {
		printf("-i is required\n");
		Usage();
		return 1;
	}

	if(out_dir == NULL) {
		printf("-o is required\n");
		Usage();
		return 1;
	}

	if(WadOpen(&wf, in_filename) != 0) {
		printf("Unable to open file %s\n", in_filename);
		return 1;
	}
	
	if(create_dir) {
		_splitpath_s(in_filename,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
		sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
	} else {
		strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
	}

	printf("Input: %s\n", in_filename);
	if(search_string != NULL) {
		printf("Search String: \"%s\"\n", search_string);
	}
	printf("Output directory: %s\n", wad_out_dir);
	printf("\n");
	printf("Processing %d records\n", wf.total_records);

	while(WadRecordRead(&wf, &wr, 1) == 1) {
		if(((search_string != NULL) && (strstr(wr.name,search_string) != NULL)) || search_string == NULL) {
			sprintf_s(out_filename, sizeof(out_filename), "%s\\%s.rmid", wad_out_dir, wr.name);

			printf("0x%08X 0x%04X %s ", EndianSwap(wr.id), wr.type, wr.name);
			if(list_only == 0) {
				_mkdir(wad_out_dir);

				if(wr.type == 1) {
					if(WadRecordToFile(&wf, &wr, out_filename) != 0) {
						printf("WadRecordToFile FAILED\n");
						continue;
					}
				} else {
					if(WriteRecord(in_filename, &wr, out_filename) != 0) {
						printf("WriteRecord FAILED\n");
						continue;
					}

				}
			}
			count++;
			printf("\n");
		}
	}
	
	WadClose(&wf);
	printf("Extracted %d records", count);
	if(search_string != NULL) {
		printf(" containg with the string \"%s\"\n", search_string);
	} else {
		printf("\n");
	}

	return 0;
}
*/

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
	uint32_t display_datetime = 0;
	char wad_out_dir[256];
	char basename[256];
	char full_out_dir[512];
	uint32_t extract_files = 0;
	struct tm time;
	char date_time[256];
	uint32_t total_records = 0;

	printf("Defiance WAD Dump Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

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
		}  else if(strcmp(argv[i],"-dt") == 0) {
			display_datetime = 1;
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
