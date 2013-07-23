#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wadf.h"
#include "wadlib.h"
#include "rmidlib.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

#ifdef _DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

int WadOpen(wad_file * wf, const char * filename)
{
	wadf_header wadfh;
	wadf_index_header wadfih;

	if(wf == NULL) return 1;
	if(filename == NULL) return 1;

	if(fopen_s(&wf->file, filename,"rb") != 0)
	{
		return 1;
	}
	strcpy_s(wf->name, strlen(filename)+1, filename);
	fread(&wadfh, sizeof(wadf_header), 1, wf->file);

	if(wadfh.magic != WADF_MAGIC)
	{
		fclose(wf->file);
		return 1;
	}

	fread(&wadfih,sizeof(wadf_index_header),1, wf->file);

	wf->total_records = wadfh.total_records;
	wf->total_records_read = 0;
	wf->index_records_read = 0;
	wf->total_index_records = wadfih.num_records;
	wf->next_index_header_offset = wadfih.next_header_offset;
	return 0;
}

int WadRecordRead(wad_file * wf, wad_record * wr, int read_name)
{
	wadf_index_header wadfih;
	wadf_index_record wadfir;
	long offset;

	if(wf->total_records_read < wf->total_records)
	{
		if(wf->index_records_read == wf->total_index_records)
		{
			wf->index_records_read = 0;
			if(wf->next_index_header_offset != 0) 
			{
				fseek(wf->file, wf->next_index_header_offset,SEEK_SET);
				fread(&wadfih,sizeof(wadf_index_header),1, wf->file);
				wf->next_index_header_offset = wadfih.next_header_offset;
				wf->total_index_records = wadfih.num_records;
			}
			else
			{
				return 0;
			}

		}

		fread(&wadfir,sizeof(wadfir),1, wf->file);
		wr->offset = wadfir.data_offset;
		wr->size = wadfir.data_size;
		wr->type = wadfir.type;
		wr->id = wadfir.id;
		wr->name_offset = wadfir.name_offset;

		if(read_name == 1)
		{
			offset = ftell(wf->file);
			fseek(wf->file, wr->name_offset, SEEK_SET);
			fread(&wr->name, sizeof(wr->name),1, wf->file);
			fseek(wf->file, offset, SEEK_SET);
		}
		wf->index_records_read++;
		wf->total_records_read++;
		return 1;

	}
	return 0;
}

int WadRecordToFile(wad_file * wf, wad_record * wr, const char * filename) {
	FILE * file;
	long offset;
	long bytes_read = 0;
	int bytes_to_copy;
	char buffer[2048];
	if(fopen_s(&file, filename, "wb") != 0) {
		return 1;
	}

	offset = ftell(wf->file);
	fseek(wf->file, wr->offset, SEEK_SET);

	while(bytes_read < wr->size) {
		bytes_to_copy = MIN(sizeof(buffer),wr->size - bytes_read);
		bytes_read += fread(&buffer, 1,bytes_to_copy, wf->file); 
		fwrite(&buffer, 1, bytes_to_copy, file);
	}
	fseek(wf->file, offset, SEEK_SET);
	fclose(file);
	return 0;
}

void WadClose(wad_file * wf) {
	fclose(wf->file);
}


int WadFileLoad(wad_file2 * wf, const char * filename)  {
	FILE * file;
	wadf_header wh;
	wadf_index_header wih;
	wadf_index_record wir;
	
	uint32_t records_read;
	uint32_t total_records_read = 0;

	if(fopen_s(&file, filename, "rb") != 0) {
		return 1;
	}

	fread(&wh, sizeof(wadf_header), 1, file);

	if(wh.magic != WADF_MAGIC) {
		fclose(file);
		return 1;
	}
	wf->filename = (int8_t *)malloc(strlen(filename)+1);
	strcpy_s(wf->filename, strlen(filename)+1, filename);
	wf->total_records = wh.total_records;
	wf->records = (wad_record2 * )malloc(sizeof(wad_record2) * wh.total_records);

	while(total_records_read < wh.total_records) {
		fread(&wih,sizeof(wadf_index_header),1, file);
		records_read = 0;
		while(records_read < wih.num_records) {
			fread(&wir, sizeof(wadf_index_record), 1, file);
			wf->records[total_records_read].filename = wf->filename;
			wf->records[total_records_read].id = wir.id;
			wf->records[total_records_read].type = wir.type;
			wf->records[total_records_read].name_offset = wir.name_offset;
			wf->records[total_records_read].data_offset = wir.data_offset;
			wf->records[total_records_read].name = NULL;
			wf->records[total_records_read].data_size = wir.data_size;
			wf->records[total_records_read].modified_time = wir.modified_time;
			records_read++;
			total_records_read++;
		}
		if(wih.next_header_offset != 0) {
			fseek(file, wih.next_header_offset, SEEK_SET);
		}
	}

	fclose(file);
	return 0;
}
int WadRecordResolveName(wad_record2 * wr) {
	FILE * file;
	wadf_header wh;
	char name[256];

	if(wr->name != NULL) {
		return 0;
	}

	if(fopen_s(&file, wr->filename, "rb") != 0) {
		return 1;
	}

	fread(&wh, sizeof(wh), 1, file);
	if(wh.magic != WADF_MAGIC) {
		return 1;
	}

	fseek(file, (long)wr->name_offset, SEEK_SET);
	fread(&name, sizeof(name), 1, file);
	wr->name = (char * )malloc(strlen(name)+1);
	strcpy_s(wr->name, strlen(name)+1, name);

	fclose(file);
	return 0;
}

void WadFileFree(wad_file2 * wf) {
	uint32_t i;
	free(wf->filename);
	for(i=0; i<wf->total_records; i++) {
		if(wf->records[i].name != NULL) {
			free(wf->records[i].name);
		}
	}
	free(wf->records);
}

int WadDirLoad(wad_dir * wd, const char * dir) {
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	char search_dir[1024];
	char wad_filename[1024];
	uint32_t count = 0;

	sprintf_s(search_dir, sizeof(search_dir), "%s\\*.wad",dir);
	
	hFind = FindFirstFile(search_dir, &ffd);
	if(hFind == INVALID_HANDLE_VALUE) {
		return 1;
	}

	do {
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			count++;
		}
	} while(FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
	wd->total_files = count;
	wd->files = (wad_file2*)malloc(sizeof(wad_file2) * wd->total_files);

	count=0;
	hFind = FindFirstFile(search_dir, &ffd);
	do 
	{
		if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			sprintf_s(wad_filename, sizeof(wad_filename), "%s\\%s", dir, ffd.cFileName);
			if(WadFileLoad(&wd->files[count], wad_filename) != 0) {
				break;
			}
			count++;
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return 0;
}

wad_record2 * WadDirFindByID(wad_dir * wd, uint32_t id) {
	uint32_t f;
	uint32_t r;

	for(f = 0; f < wd->total_files; f++) {
		for(r = 0; r < wd->files[f].total_records; r++) {
			if(wd->files[f].records[r].id == id) {
				return &wd->files[f].records[r];
			}
		}
	}
	return NULL;
}

void WadDirFree(wad_dir * wd) {
	uint32_t i;

	for(i=0; i<wd->total_files; i++) {
		WadFileFree(&wd->files[i]);
	}
	free(wd->files);
}

static int WadWriteObj(wad_dir * wd,  wad_record2 * wr, const char * dir) {
	uint8_t * data;
	mes_ski_header * header;
	uint32_t * mesh_material_ids;
	mes_ski_mesh_record * mesh_records;
	mes_ski_material_record * material_records;
	mes_ski_material_header * material_header;
	mes_ski_mesh_header * mesh_header;
	mes_ski_material_param * material_params;
	FILE * in_file;
	FILE * out_file;
	rmid_file rf;
	wad_record2 * swr;
	wad_record2 * twr;
	char filename[512];
	uint32_t m, p, v, f;
	rmid_file trf;
	uint32_t vertex_counter;
	uint32_t index_data_size;
	void * vertex_data;	
	uint8_t * vertex_byte_data;
	void * face_data;
	uint32_t total_meshes;
//	mes_ski_mesh_record * mesh_records_check;

	if(fopen_s(&in_file, wr->filename, "rb") != 0) {
		printf("ERROR: Unable to open file %s\n", wr->filename);
		return 1;
	}
	fseek(in_file, (long)wr->data_offset, SEEK_SET);

	if(RmidLoad(in_file, (long)wr->data_size, &rf) != 0) {
		printf("ERROR: Failed to load RMID data\n");
		fclose(in_file);
		return 1;
	}
	data = (uint8_t *)rf.data;

	header =			(mes_ski_header*)(data + sizeof(rmid_header));
	mesh_material_ids = (uint32_t*)(data + header->mesh_material_ids_offset); 
	mesh_records =		(mes_ski_mesh_record*)(data + header->mesh_table_offset);
	material_records =	(mes_ski_material_record*)(data + header->material_table_offset);
	//header->total_meshes+=1;
	/*
	if(verbose_output) {
		printf("Materials %d\n", header->total_materials);
	}
	*/
	sprintf_s(filename, sizeof(filename),"%s\\%s.mtl", dir, wr->name);
	if(fopen_s(&out_file, filename, "w") != 0) {
		printf("ERROR: Unable to open file %s\n", filename);
		RmidFree(&rf);
		fclose(in_file);
		return 1;
	}

	for(m = 0;  m < header->total_materials; m++) {
		material_header = (mes_ski_material_header *)(data + material_records[m].offset);	
		swr = WadDirFindByID(wd, material_header->shader_id);
		if(swr == NULL) {
			printf("Unable to find shader ID 0x%08X", EndianSwap(material_header->shader_id));
			RmidFree(&rf);
			fclose(in_file);
			fclose(out_file);
			return 1;
		}
		WadRecordResolveName(swr);
						
		DEBUG_PRINT((" Shader %s\n", swr->name));
						
		fprintf(out_file, "newmtl %s_%d\n", swr->name, m);
		fprintf(out_file, "Ka 1.000 1.000 1.000\n");
		fprintf(out_file, "Kd 1.000 1.000 1.000\n");
		fprintf(out_file, "Ks 0.000 0.000 0.000\n");
		fprintf(out_file, "d 1.0\n");

		material_params = (mes_ski_material_param *)(data + material_records[m].offset + sizeof(mes_ski_material_header));
		for(p = 0; p < material_header->total_material_params; p++) {
					DEBUG_PRINT(("  [%d] T=0x%08X V=0x%08X ", p, EndianSwap(material_params[p].param_type), EndianSwap(material_params[p].texture_id)));
			if((twr = WadDirFindByID(wd, material_params[p].texture_id)) == NULL) {
					DEBUG_PRINT(("\n"));
			} else {
				WadRecordResolveName(twr);
				if(material_params[p].param_type == RMID_MAT_PARAM_COLOR1) {
					RmidLoadFromFile(twr->filename, twr->data_offset, twr->data_size, &trf);
					RmidWriteTexToPng(&trf, dir, twr->name); 
					fprintf(out_file, "map_Ka %s.png\n", twr->name);
					fprintf(out_file, "map_Kd %s.png\n", twr->name);
					RmidFree(&trf);
				}
				DEBUG_PRINT(("%s\n", twr->name));
			}
		}
	}
	fclose(out_file);

	total_meshes = *((uint32_t *)&material_records[header->total_materials]);
	
	//total_meshes = header->total_meshes;
	/*
	mesh_records_check = mesh_records;
	total_meshes = 0;
	while((uint32_t*)mesh_records_check != (uint32_t*)material_records) {
		mesh_records_check++;
		total_meshes++;
	}
	*/
	DEBUG_PRINT(("Meshes %d\n", total_meshes));
	sprintf_s(filename, sizeof(filename),"%s\\%s.obj",dir, wr->name);
	if(fopen_s(&out_file, filename, "w") != 0) {
		printf("ERROR: Unable to open file %s\n", filename);
		RmidFree(&rf);
		fclose(in_file);
		return 1;
	}
	fprintf(out_file, "# Generated with mes2obj\n");
	fprintf(out_file, "mtllib %s.mtl\n", wr->name);

	vertex_counter = 0;


	for(m = 0;  m < total_meshes; m++) {
		mesh_header = (mes_ski_mesh_header*)(data + mesh_records[m].offset);	
		material_header = (mes_ski_material_header *)(data + material_records[mesh_material_ids[m]].offset);
						
		DEBUG_PRINT((" [%d] O=%d", m, mesh_records[m].offset));
		DEBUG_PRINT((" S=%d", mesh_records[m].size));
		DEBUG_PRINT((" BPV=%d", mesh_header->bytes_per_vertex));
		DEBUG_PRINT((" V=%d", mesh_header->num_vertices1));
		DEBUG_PRINT((" I=%d\n", mesh_header->num_indices1));

		//mesh_header = (mes_ski_mesh_header*)(data + mesh_records[m].offset);
	
		swr = WadDirFindByID(wd, material_header->shader_id);
							
//		i = 0;
						
		index_data_size = (uint32_t)(mesh_records[m].size  - (mesh_header->index_data_offset - *(data + mesh_records[m].offset)));
						

		fprintf(out_file, "o %s_%d\n", wr->name, m);

		// Vertices
		vertex_data = (void*)(data + mesh_records[m].offset + mesh_header->vertex_data_offset);
		vertex_byte_data = (uint8_t *)vertex_data;
		for(v = 0; v < mesh_header->num_vertices1; v++) {
			ObjWritePosition(out_file, mesh_header, vertex_byte_data); 
			vertex_byte_data += mesh_header->bytes_per_vertex;
		}

		// Texture Coords
		vertex_byte_data = (uint8_t *)vertex_data;
		for(v = 0; v < mesh_header->num_vertices1; v++) {
			ObjWriteTexCoord(out_file, mesh_header, vertex_byte_data); 
			vertex_byte_data += mesh_header->bytes_per_vertex;
		}

		// Normals
		vertex_byte_data = (uint8_t *)vertex_data;
		for(v = 0; v < mesh_header->num_vertices1; v++) {
			ObjWriteNormal(out_file, mesh_header, vertex_byte_data);
			vertex_byte_data += mesh_header->bytes_per_vertex;
		}

		// Faces
		fprintf(out_file, "usemtl %s_%d\n", swr->name,mesh_material_ids[m]);
		fprintf(out_file, "s off\n");
		face_data = (void *)(data + mesh_records[m].offset + mesh_header->index_data_offset);
		fprintf(out_file, "# Faces %d\n",mesh_header->num_indices1/3);
		for(f = 0; f < mesh_header->num_indices1/3; f++) {
			if((index_data_size / 2) == mesh_header->num_indices1) {
				ObjWriteFace16(out_file, vertex_counter, face_data, f); 
			} else {
				ObjWriteFace32(out_file, vertex_counter, face_data, f); 
			}
		}
		vertex_counter += mesh_header->num_vertices1;
	}
	fclose(out_file);
	RmidFree(&rf);
	fclose(in_file);
	return 0;
}
int WadWriteRecordToRmid(wad_record2 * wr,  const char * dir, const char * name) {
	FILE * in_file;
	FILE * out_file;
	char filename[512];
	rmid_file rf;

	if(fopen_s(&in_file, wr->filename, "rb") != 0) {
		return 1;
	}

	fseek(in_file, (uint32_t)wr->data_offset, SEEK_CUR);
	if(RmidLoad(in_file,wr->data_size, &rf) != 0) {
		return 1;
	}
	fclose(in_file);

	if(wr->name == NULL) {
		WadRecordResolveName(wr);
	}

	if(name == NULL) {
		sprintf_s(filename, sizeof(filename), "%s\\%s.rmid", dir, wr->name);
	} else {
		sprintf_s(filename, sizeof(filename), "%s\\%s", dir, name);
	}

	if(fopen_s(&out_file, filename, "wb") != 0) {
		RmidFree(&rf);
		return 1;
	}
	
	RmidWriteToFile(&rf, out_file);

	RmidFree(&rf);

	fclose(out_file);
	return 0;
}

int WadWriteMesToObj(wad_dir * wd,  wad_record2 * wr, const char * dir) {
	if(wr->type != RMID_TYPE_MES) {
		return 1;
	}
	return WadWriteObj(wd, wr, dir);
}

int WadWriteSkiToObj(wad_dir * wd,  wad_record2 * wr, const char * dir) {
	if(wr->type != RMID_TYPE_SKI) {
		return 1;
	}
	return WadWriteObj(wd, wr, dir);
}
