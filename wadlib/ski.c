#include <stdio.h>

#include "wadlib.h"
#include "objfile.h"

typedef struct {
	uint32_t total_meshes;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;

	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;
} ski_mesh_group_record;

int WadWriteSkiToObj(wad_dir * wd,  wad_record * wr, const char * dir) {
	uint8_t * data;
	mes_ski_header * header;
	uint32_t * mesh_material_ids;
	mes_ski_mesh_record * mesh_records;
	mes_ski_material_record * material_records;
	mes_ski_material_header * material_header;
	mes_ski_mesh_header * mesh_header;
	mes_ski_material_param * material_params;
	ski_mesh_group_record * mesh_group_records;

	FILE * in_file;
	FILE * out_file;
	rmid_file rf;
	wad_record * swr;
	wad_record * twr;
	char filename[512];
	uint32_t m, p, v, f;
	rmid_file trf;
	uint32_t vertex_counter;
	uint32_t index_data_size;
	void * vertex_data;	
	uint8_t * vertex_byte_data;
	void * face_data;
	uint32_t total_meshes;

	if(wr->type != RMID_TYPE_SKI) {
		return 1;
	}

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

	header				= (mes_ski_header*)(data + sizeof(rmid_header));
	mesh_material_ids	= (uint32_t*)(data + header->mesh_material_ids_offset); 
	mesh_records		= (mes_ski_mesh_record*)(data + header->mesh_table_offset);
	material_records	= (mes_ski_material_record*)(data + header->material_table_offset);
	mesh_group_records	= (ski_mesh_group_record *)(data + header->mesh_group_table_offset);

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
					RmidWriteTexToPng(&trf, 0, dir, twr->name); 
					fprintf(out_file, "map_Ka %s.png\n", twr->name);
					fprintf(out_file, "map_Kd %s.png\n", twr->name);
					RmidFree(&trf);
				}
				DEBUG_PRINT(("%s\n", twr->name));
			}
		}
	}
	fclose(out_file);

	total_meshes = 0;

	for(m = 0; m < header->total_meshes; m++) {
		total_meshes += mesh_group_records[m].total_meshes;		
	}

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
