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
	printf("Usage snd2wav.exe  [-i <wad_file>] [-o <output_dir>] [-m <miles_redist>] [-s <search_name>] [-c]\n");
	printf("Extracts Defiance textures and converts them to PNG files\n");
	printf("-i\t Input WAD filename\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-m\t Miles Sound System Redist directory. Usianlly in <game install dir>\\live\\MilesRedist \n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
}

//typedef int (CALLBACK* _AIL_decompress_ASI)(void *, int, char *, char *, unsigned int *, unsigned int);
typedef int (CALLBACK* _AIL_decompress_ASI)(void *, unsigned int, char *, void **, unsigned int *, void* );
typedef char* (CALLBACK* _AIL_last_error)();
typedef int* (CALLBACK* _AIL_set_redist_directory)(const char *);
typedef void (CALLBACK* _AIL_mem_free_lock)(void *);
typedef int (CALLBACK* _AIL_startup)();
typedef int (CALLBACK* _AIL_shutdown)();

int main( int argc, const char* argv[])
{
	int i;
	wad_file wf;
	wad_record wr;
	rmid_file rf;
	FILE * file;
	void * out_data = NULL;
	unsigned int out_size = 0;
	char * error;
	char * data;
	FILE * out_file;
	char out_filename[256];
	char dll_filename[256];
	char basename[256];
	char wad_out_dir[256];

	const char * wad_file = NULL;
	const char * out_dir = NULL;
	const char * search_string = NULL;
	const char * mss_redist_dir = NULL;
	int create_dir = 0;

	HMODULE module = NULL;
	int result;

	_AIL_decompress_ASI fn_AIL_decompress_ASI;
	_AIL_last_error fn_AIL_last_error;
    _AIL_set_redist_directory fn_AIL_set_redist_directory;
	_AIL_mem_free_lock fn_AIL_mem_free_lock;
	_AIL_startup fn_AIL_startup;
	_AIL_shutdown fn_AIL_shutdown;		

	printf("Defiance Audio Extraction Utility by Zeiban v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE_VERSION);

	for(i=0; i<argc; i++) 
	{
		if(strcmp(argv[i],"-i") == 0) {
			if(argc>i) 
			{
				wad_file = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-o") == 0) 
		{
			if(argc>i) 
			{
				out_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-s") == 0) 
		{
			if(argc>i) 
			{
				search_string = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-m") == 0) 
		{
			if(argc>i) 
			{
				mss_redist_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-c") == 0) 
		{
			create_dir = 1;
		} 
	}
	
	if(wad_file == NULL)
	{
		printf("-i required\n");
		Usage();
		return 1;
	}
	if(mss_redist_dir == NULL)
	{
		printf("-m required\n");
		Usage();
		return 1;
	}

	if(create_dir) 
	{
		_splitpath_s(wad_file,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
		sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
	} 
	else 
	{
		strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
	}

	sprintf_s(dll_filename, sizeof(dll_filename), "%s\\mss32.dll", mss_redist_dir);
	module = LoadLibrary(dll_filename);
	if(module == NULL)
	{
		printf("Unable to load mss32.dll from %s\n", dll_filename);
		return 1;
	}

	fn_AIL_decompress_ASI = (_AIL_decompress_ASI)GetProcAddress(module, "_AIL_decompress_ASI@24");
	fn_AIL_last_error = (_AIL_last_error)GetProcAddress(module, "_AIL_last_error@0");
	fn_AIL_set_redist_directory = (_AIL_set_redist_directory)GetProcAddress(module, "_AIL_set_redist_directory@4");
	fn_AIL_mem_free_lock = (_AIL_mem_free_lock)GetProcAddress(module, "_AIL_mem_free_lock@4");
	fn_AIL_startup = (_AIL_startup)GetProcAddress(module, "_AIL_startup@0");
	fn_AIL_shutdown = (_AIL_shutdown)GetProcAddress(module, "_AIL_shutdown@0");

	if(WadOpen(&wf, wad_file) != 0)
	{
		printf("Failed to open WAD file %s", wad_file);
		return;
	}

	if(fn_AIL_startup() == 0)
	{
		printf("_AIL_startup FAILED\n");
		return 1;
	}

	printf("Input: %s\n", wad_file);
	if(search_string != NULL)
	{
		printf("Search String: \"%s\"\n", search_string);
	}
	printf("Output directory: %s\n", wad_out_dir);
	printf("Processing %d records\n", wf.total_records);

	while(WadRecordRead(&wf, &wr, 1) > 0)
	{
		if(((search_string != NULL) && (strstr(wr.name,search_string) != NULL)) || search_string == NULL)
		{
			if(wr.type == RMID_TYPE_SND){
				_mkdir(wad_out_dir);

				printf("0x%08X %s ", EndianSwap(wr.id), wr.name);

				if(fopen_s(&file, wad_file, "rb") != 0)
				{
					printf("ERROR: Unable to open file %s\n", wad_file);
					continue;
				}
				fseek(file, wr.offset, SEEK_SET);

				if(RmidLoad(file, wr.size, &rf) != 0)
				{
					printf("ERROR: Failed to load RMID file\n");
					continue;
				}
				data = (char*)rf.data;
				data += sizeof(rmid_header);
				data += sizeof(rmid_snd_header);
				fn_AIL_set_redist_directory(mss_redist_dir);

				result = fn_AIL_decompress_ASI(data,rf.size - (sizeof(rmid_snd_header)+sizeof(rmid_snd_header)), ".binka", &out_data, &out_size, 0); 
				if(result == 0)
				{
					error = fn_AIL_last_error();
					printf("Unable to decompress audio. %s\n", error);
					continue;
				}
				else
				{
					sprintf_s(out_filename,sizeof(out_filename), "%s\\%s.wav", wad_out_dir, wr.name);
					fopen_s(&out_file, out_filename, "wb");
					fwrite(out_data, out_size, 1, out_file);
					fclose(out_file);
					fn_AIL_mem_free_lock(out_data);
				}

				RmidFree(&rf);
				fclose(file);
				printf("\n");
			}
		} 
	}
	fn_AIL_shutdown();
	WadClose(&wf);
	FreeLibrary(module);
	printf("\n");
}
