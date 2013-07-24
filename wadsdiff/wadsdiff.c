
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\wadlib\wadlib.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define RELEASE_VERSION 1

/*
typedef struct {
	char name[256]; 
	int size; 
} record_info;

typedef struct {
	char name[256];
	uint32_t count;
	record_info * records;
} wad_info;

typedef struct {
	uint32_t count;
	wad_info ** wads;
} dir_info;
*/

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
/*
wad_info * ReadWadFile(const char * dir, const char * name) {
	wad_file wf;
	wad_record wr;
	wad_info * wi;
	char filename[256];
	int count = 0;
	int update_count = 0;
	unsigned int i = 0;
	char line[256];
	time_t next_update;

	memset(line, 0, sizeof(line));	
	sprintf_s(filename, sizeof(filename),"%s\\%s", dir, name);

	if(WadOpen(&wf, filename) == 0) {
		wi = (wad_info *)malloc(sizeof(wad_info));
		strcpy_s(wi->name, sizeof(wi->name), name);
		wi->count = wf.total_records;
		wi->records = (record_info *)malloc(sizeof(record_info)*wi->count);
		printf("%s ", wi->name);

		next_update = time(NULL);

		while(WadRecordRead(&wf, &wr,1) == 1) {
			strcpy_s(wi->records[count].name, sizeof(filename), wr.name);
			wi->records[count].size = wr.size;
			
			
			count++;
			if((time(NULL) > next_update) || ((count) == wf.total_records)) {
				for(i = 0; i<strlen(line); i++)
					printf("\b");

				sprintf_s(line, sizeof(line),"[%d of %d]", count, wf.total_records);
				printf("%s", line);	
				next_update = time(NULL) + 1;
			}
		}
		printf("\n",count);
		WadClose(&wf);
	}
	return wi;
}

dir_info * ReadWadDirectory(const char * dir) {
	WIN32_FIND_DATA ffd;
	char search_dir[256];
	HANDLE hFind;
	int count=0;
	dir_info * wd;


	printf("Reading WAD directory %s\n", dir);

	sprintf_s(search_dir, sizeof(search_dir), "%s\\*.wad",dir);
	hFind = FindFirstFile(search_dir, &ffd);

	if(hFind == INVALID_HANDLE_VALUE) {
		printf("Error [%d] directory doesn't exist or is not accessable.\n", dir);
		return 0;
	}

	// Count the number of WAD files
	do {
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			count++;
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
	
	printf("%d WAD files found\n", count);


	wd = (dir_info*)malloc(sizeof(dir_info));
	wd->wads = (wad_info**)malloc(sizeof(wad_info*)*count);
	wd->count = count;

	count = 0;
	hFind = FindFirstFile(search_dir, &ffd);
	do {
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			wd->wads[count] = ReadWadFile(dir, ffd.cFileName);
			count++;
		}
	} while(FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	printf("\n", dir);
	return wd;
}

record_info * FindRecordInfo(char * name, dir_info* di)
{
	int w, r;
	record_info * ri;

	for(w = 0; w<di->count; w++) {
		for(r = 0; r<di->wads[w]->count; r++) {
			ri = &di->wads[w]->records[r];
			if(strcmp(ri->name, name) == 0)
			{
				return ri;
			}
		}
	}
	return NULL;
}
*/

void Usage() {
	printf("Usage wadsdiff.exe -f <from_wad_dir> -t <to_wad_dir> -o <out_csv_file> [-s <search>]\n");
	printf("Compares two Defiance wad directories and tells you what was added, changed, or deleted\n");
	printf("-f\t Directory to compare from\n");
	printf("-t\t Directory to compare to\n");
	printf("-o\t Name of the output CSV file to create.\n");
	printf("-s\t (Optional) Only report on assets with <search> in the name\n");
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
	wad_record2 * from_wr;
	wad_dir to_wd;
	wad_record2 * to_wr;
	int spin = 0;

	int assets_to_check = 0;
	int assets_checked = 0;

	char line[256];
	time_t next_update;

	memset(line, 0, sizeof(line));	
	
	printf("WAD Difference Report Generator by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

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