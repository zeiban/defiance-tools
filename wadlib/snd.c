#include <stdio.h>

#include "wadlib.h"

typedef int (CALLBACK* _AIL_decompress_ASI)(void *, unsigned int, char *, void **, unsigned int *, void* );
typedef char* (CALLBACK* _AIL_last_error)();
typedef int* (CALLBACK* _AIL_set_redist_directory)(const char *);
typedef void (CALLBACK* _AIL_mem_free_lock)(void *);
typedef int (CALLBACK* _AIL_startup)();
typedef int (CALLBACK* _AIL_shutdown)();

int WadWriteSndToWav(wad_record2 * wr, const char * redist_dir, const char * out_dir, const char * name) {
	char dll_filename[512];
	char out_filename[512];
	HMODULE module;
	FILE * in_file;
	FILE * out_file;
	rmid_file rf;
	uint8_t * data;

	int result;
	void * out_data = NULL;
	unsigned int out_size = 0;

	_AIL_decompress_ASI fn_AIL_decompress_ASI;
	_AIL_last_error fn_AIL_last_error;
    _AIL_set_redist_directory fn_AIL_set_redist_directory;
	_AIL_mem_free_lock fn_AIL_mem_free_lock;
	_AIL_startup fn_AIL_startup;
	_AIL_shutdown fn_AIL_shutdown;		

	if(wr->type != RMID_TYPE_SND) {
		return 1;
	}

	sprintf_s(dll_filename, sizeof(dll_filename), "%s\\mss32.dll", redist_dir);
	module = LoadLibrary(dll_filename);
	if(module == NULL) {
		printf("Unable to load mss32.dll from %s. Is the path correct?\n", dll_filename);
		return 1;
	}

	fn_AIL_decompress_ASI = (_AIL_decompress_ASI)GetProcAddress(module, "_AIL_decompress_ASI@24");
	fn_AIL_last_error = (_AIL_last_error)GetProcAddress(module, "_AIL_last_error@0");
	fn_AIL_set_redist_directory = (_AIL_set_redist_directory)GetProcAddress(module, "_AIL_set_redist_directory@4");
	fn_AIL_mem_free_lock = (_AIL_mem_free_lock)GetProcAddress(module, "_AIL_mem_free_lock@4");
	fn_AIL_startup = (_AIL_startup)GetProcAddress(module, "_AIL_startup@0");
	fn_AIL_shutdown = (_AIL_shutdown)GetProcAddress(module, "_AIL_shutdown@0");

	if(fopen_s(&in_file, wr->filename, "rb") != 0) {
		printf("ERROR: Unable to open file %s\n", wr->filename);
		return 1;
	}

	fseek(in_file, (long)wr->data_offset, SEEK_SET);

	if(fn_AIL_startup() == 0) {
		printf("_AIL_startup FAILED\n");
		return 1;
	}

	fn_AIL_set_redist_directory(redist_dir);
	if(RmidLoad(in_file, wr->data_size, &rf) != 0) {
		return 1;
	}
	fclose(in_file);

	data = (uint8_t*)rf.data;
	data += sizeof(rmid_header);
	data += sizeof(rmid_snd_header);

	result = fn_AIL_decompress_ASI(data,(uint32_t)rf.size - (sizeof(rmid_snd_header) + sizeof(rmid_snd_header)), ".binka", &out_data, &out_size, 0); 
	if(result == 0)	{
		printf("Unable to decompress audio. %s\n", fn_AIL_last_error());
		return 1;
	} else {
 		sprintf_s(out_filename,sizeof(out_filename), "%s\\%s.wav", out_dir, name == NULL ? wr->name : name);
		if(fopen_s(&out_file, out_filename, "wb") != 0) {
			printf("Failed to open output file %s", out_filename);
			fn_AIL_mem_free_lock(out_data);
			fn_AIL_shutdown();
			return 1;
		}
		fwrite(out_data, out_size, 1, out_file);
		fclose(out_file);
		fn_AIL_mem_free_lock(out_data);
		fn_AIL_shutdown();
	}
	FreeLibrary(module);
	return 0;
}