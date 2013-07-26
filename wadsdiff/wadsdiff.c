
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\wadlib\wadlib.h"

void PrintPercentStatus(float percent) {
	static char line[256];
	static time_t next_update = 0;
	uint32_t i = 0;

	if(next_update < time(NULL)) {
		for(i = 0; i<strlen(line); i++) printf("\b");
		sprintf_s(line,sizeof(line), "[%.2f%%]", percent); 
		printf("%s", line);
		next_update = time(NULL) + 1;
	}
}

void Usage() {
	printf("Usage wadsdiff.exe -f <from_wad_dir> -t <to_wad_dir> -o <out_csv_file> [-s <search>]\n");
	printf("Compares two Defiance wad directories and tells you what was added, changed, or deleted\n");
	printf("-f\t Directory to compare from\n");
	printf("-t\t Directory to compare to\n");
	printf("-o\t Name of the output CSV file to create.\n");
	printf("-s\t (Optional) Only report on assets with <search> in the name\n");
	printf("-h\t Displays this information\n");
}

int main( int argc, const char* argv[]) {
	int count=0;
	const char * from_dir;
	const char * to_dir;
	const char * search = NULL;
	const char * out_filename;

	FILE * out_file;
	int i; 
	uint32_t w, r;
	int adds = 0, changes = 0, deletes = 0;

	wad_dir from_wd;
	wad_record * from_wr;
	wad_dir to_wd;
	wad_record * to_wr;
	int spin = 0;

	int assets_to_check = 0;
	int assets_checked = 0;

	char line[256];
	time_t next_update;

	memset(line, 0, sizeof(line));	
	
	printf("Defiance Tools WAD Difference Report Generator by Zeiban v%d.%d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_SUFFIX);

	for(i=0; i<argc; i++) {
		if(strcmp(argv[i],"-f") == 0) {
			if(argc>i) 
			{
				from_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-t") == 0) {
			if(argc>i) {
				to_dir = argv[++i];
			}
		} else if(strcmp(argv[i],"-s") == 0) {
			if(argc>i) {
				search = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-o") == 0) {
			if(argc>i) {
				out_filename = argv[++i];
			}
		}  else if(strcmp(argv[i],"-h") == 0) {
			Usage();
			return 1;
		} 
	}

	WadDirLoad(&from_wd, from_dir);
	WadDirLoad(&to_wd, to_dir);	

	if(fopen_s(&out_file, out_filename, "w") != 0) {
		printf("Unable to create file [%s]\n", out_filename);
		return 1;
	} 

	for(w = 0; w < from_wd.total_files; w++) {
		assets_to_check += from_wd.files[w].total_records;
	}

	for(w = 0; w < from_wd.total_files; w++) {
		assets_to_check += to_wd.files[w].total_records;
	}

	printf("Generating change report to %s ",out_filename);

	//From -> to for deletes and changes
	
	next_update = 0;

	for(w = 0; w < from_wd.total_files; w++) {
		for(r = 0; r < from_wd.files[w].total_records; r++) {
			from_wr = &from_wd.files[w].records[r];
			WadRecordResolveName(from_wr);
			to_wr = WadDirFindByName(&to_wd, from_wr->name);

			if(((search != NULL) && (strstr(from_wr->name, search) != NULL)) || search == NULL) {
				if(to_wr == NULL) {
					fprintf(out_file, "\"D\",\"%s\",\"%s\",\"%d\"\n", from_wd.files[w].filename, from_wr->name);
					deletes++;
				}
				else if(from_wr->data_size != to_wr->data_size) {
					fprintf(out_file, "\"C\",\"%s\",\"%s\",\"\"\n", from_wd.files[w].filename, from_wr->name, to_wr->data_size - from_wr->data_size);
					changes++;
				}
			}
			assets_checked++;

			PrintPercentStatus(((float)assets_checked / assets_to_check)* 100.0f);
		}
	}
	//To -> From for adds
	for(w = 0; w < to_wd.total_files; w++) {
		for(r = 0; r < to_wd.files[w].total_records; r++) {
			to_wr = &to_wd.files[w].records[r];
			WadRecordResolveName(to_wr);
			from_wr = WadDirFindByName(&from_wd, to_wr->name);

			if(((search != NULL) && (strstr(to_wr->name, search) != NULL)) || search == NULL) {
				if(from_wr == NULL) {
					fprintf(out_file,"\"A\",\"%s\",\"%s\",\"\"\n", to_wd.files[w].filename, to_wr->name);
					adds++;
				}
			}
			assets_checked++;

			PrintPercentStatus(((float)assets_checked / assets_to_check)* 100.0f);
		}
	}

	fclose(out_file);
	printf("\n\nAsset Stats\n%d Added\n%d Changed\n%d Deleted\n", adds, changes, deletes);
}