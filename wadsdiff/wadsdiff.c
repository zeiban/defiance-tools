
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\wadlib\wadlib.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define RELEASE_VERSION 0

typedef struct {
	char name[256]; 
	int size; 
} record_info;

typedef struct {
	char name[256];
	int count;
	record_info * records;
} wad_info;

typedef struct {
	int count;
	wad_info ** wads;
} dir_info;

void PrintPercentStatus(float percent)
{
	static char line[256];
	static time_t next_update = 0;
	unsigned int i = 0;

	if(next_update < time(NULL)) {
		for(i = 0; i<strlen(line); i++) printf("\b");
		sprintf_s(line,sizeof(line), "[%.2f%%]", percent); 
		printf("%s", line);
		next_update = time(NULL) + 1;
	}
	
}
wad_info * ReadWadFile(const char * dir, const char * name) 
{
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

	if(WadOpen(&wf, filename) == 0)
	{
		wi = (wad_info *)malloc(sizeof(wad_info));
		strcpy_s(wi->name, sizeof(wi->name), name);
		wi->count = wf.total_records;
		wi->records = (record_info *)malloc(sizeof(record_info)*wi->count);
		printf("%s ", wi->name);

		next_update = time(NULL);

		while(WadRecordRead(&wf, &wr,1) == 1)
		{
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

dir_info * ReadWadDirectory(const char * dir)
{
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
	do 
	{
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
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
	do 
	{
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			wd->wads[count] = ReadWadFile(dir, ffd.cFileName);
			count++;
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);

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
void Usage() 
{
	printf("Usage wadsdiff.exe -f <from_wad_dir> -t <to_wad_dir> -o <out_csv_file> [-s <search>]\n");
	printf("Compares two Defiance wad directories and tells you what was added, changed, or deleted\n");
	printf("-f\t Directory to compare from\n");
	printf("-t\t Directory to compare to\n");
	printf("-o\t Name of the output CSV file to create.\n");
	printf("-s\t (Optional) Only report on assets with <search> in the name\n");
}

int main( int argc, const char* argv[])
{
	int count=0;
	const char * from_dir;
	const char * to_dir;
	const char * search = NULL;
	const char * out_filename;

	FILE * out_file;
	int i, w, r;
	int adds = 0, changes = 0, deletes = 0;

	dir_info * fdi;
	dir_info * tdi;
	record_info * fri;
	record_info * tri;
	int spin = 0;

	int assets_to_check = 0;
	int assets_checked = 0;

	char line[256];
	time_t next_update;

	memset(line, 0, sizeof(line));	
	
	printf("WAD Difference Report Generator by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

	for(i=0; i<argc; i++) 
	{
		if(strcmp(argv[i],"-f") == 0) {
			if(argc>i) 
			{
				from_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-t") == 0) 
		{
			if(argc>i) 
			{
				to_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-s") == 0) 
		{
			if(argc>i) 
			{
				search = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-o") == 0) 
		{
			if(argc>i) 
			{
				out_filename = argv[++i];
			}
		} 
	}

	fdi = ReadWadDirectory(from_dir);
	tdi = ReadWadDirectory(to_dir);
	

	if(fopen_s(&out_file, out_filename, "w") != 0)
	{
		printf("Unable to create file [%s]\n", out_filename);
		return 1;
	} 

	for(w = 0; w<fdi->count; w++) 
	{
		assets_to_check += fdi->wads[w]->count;
	}

	for(w = 0; w<tdi->count; w++) 
	{
		assets_to_check += tdi->wads[w]->count;
	}

	printf("Generating change report to %s ",out_filename);

	//From -> to for deletes and changes
	
	next_update = 0;

	for(w = 0; w<fdi->count; w++) 
	{
		for(r = 0; r < fdi->wads[w]->count; r++) 
		{
			fri = &fdi->wads[w]->records[r];
			tri = FindRecordInfo(fri->name, tdi);

			if(((search != NULL) && (strstr(fri->name, search) != NULL)) || search == NULL)
			{
				if(tri == NULL) 
				{
					fprintf(out_file, "\"D\",\"%s\",\"%s\",\"%d\"\n", fdi->wads[w]->name, fri->name);
					deletes++;
				}
				else if(fri->size != tri->size) {
					fprintf(out_file, "\"C\",\"%s\",\"%s\",\"\"\n", fdi->wads[w]->name, fri->name,tri->size - fri->size);
					changes++;
				}
			}
			assets_checked++;

			PrintPercentStatus(((float)assets_checked / assets_to_check)* 100.0f);
		}
	}
	//To -> From for adds
	for(w = 0; w<tdi->count; w++) 
	{
		for(r = 0; r<tdi->wads[w]->count; r++) 
		{
			tri = &tdi->wads[w]->records[r];

			if(((search != NULL) && (strstr(tri->name, search) != NULL)) || search == NULL)
			{
				fri = FindRecordInfo(tri->name, fdi);

				if(fri == NULL) 
				{
					fprintf(out_file,"\"A\",\"%s\",\"%s\",\"\"\n", tdi->wads[w]->name, tri->name);
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