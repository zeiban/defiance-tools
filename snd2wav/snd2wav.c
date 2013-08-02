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

					if(WadWriteSndToWav(wr, redist_dir, full_out_dir, wr->name) != 0) {
						printf("Failed to write WAV file\n");
					} else {
						printf("\n");
					}
				}
			}
		}
	} 
	WadDirFree(&wd);
}

/*
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
#define RELEASE_VERSION 1

void Usage(void)
{
	printf("Usage snd2wav.exe  [-i <wad_file>] [-o <output_dir>] [-m <miles_redist>] [-s <search_name>] [-c]\n");
	printf("Extracts Defiance textures and converts them to PNG files\n");
	printf("-i\t Input WAD filename\n");
	printf("-o\t (Optional) Directory to output PNG file otherwise the current directory is used\n");
	printf("-m\t Miles Sound System Redist directory. Usually in <game install dir>\\live\\MilesRedist \n");
	printf("-s\t (Optional) Only extracts files that have <search_name> in the name\n");
	printf("-c\t (Optional) Creates a sub directory under the <output_dir> with the name of the WAD file\n");
}

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

	for(i=0; i<argc; i++) {
		if(strcmp(argv[i],"-i") == 0) {
			if(argc>i) {
				wad_file = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-o") == 0) {
			if(argc>i) {
				out_dir = argv[++i];
			}
		} 
		else if(strcmp(argv[i],"-s") == 0) 
		{
			if(argc>i) {
				search_string = argv[++i];
			}
		} else if(strcmp(argv[i],"-m") == 0) {
			if(argc>i) {
				mss_redist_dir = argv[++i];
			}
		}  else if(strcmp(argv[i],"-c") == 0) {
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

	if(create_dir) {
		_splitpath_s(wad_file,NULL,0,NULL,0,basename,sizeof(basename),NULL,0);
		sprintf_s(wad_out_dir, sizeof(wad_out_dir),"%s\\%s",out_dir,basename);
	} else {
		strcpy_s(wad_out_dir, sizeof(wad_out_dir), out_dir); 
	}

	sprintf_s(dll_filename, sizeof(dll_filename), "%s\\mss32.dll", mss_redist_dir);
	module = LoadLibrary(dll_filename);
	if(module == NULL) {
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

				result = fn_AIL_decompress_ASI(data,(uint32_t)rf.size - (sizeof(rmid_snd_header)+sizeof(rmid_snd_header)), ".binka", &out_data, &out_size, 0); 
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
*/