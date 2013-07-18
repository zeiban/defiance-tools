#ifndef _RMID_H_
#define _RMID_H_

#include <stdint.h>

#define RMID_MAGIC 'DIMR'

// shd_p_DeferredDiffuseSpecularNormalEmissive 52
// shd_p_DeferredDiffuseSpecularNormal 52
// shd_p_DeferredDiffuseSpecularEmissive 52
// shd_p_DeferredAlphaTestDiffuseSpecularNormalEmissive 60
// shd_p_DeferredDualDiffuseSpecularNormal 64
// shd_p_DeferredAlphaTestDualDiffuseSpecularNormal 68

#define RMID_TYPE_RAW	0x0001 // Raw data any format
#define RMID_TYPE_SHD	0x0002 // Shader
#define RMID_TYPE_TEX	0x0003 // Texture
#define RMID_TYPE_MES	0x0004 // Static Mesh
#define RMID_TYPE_SKI	0x0005 // Skeletal Mesh (skin)
#define RMID_TYPE_ACT	0x0006 // Actor
#define RMID_TYPE_SKE	0x0007 // Skeleton
#define RMID_TYPE_ANI	0x0008 // Animation
#define RMID_TYPE_SND	0x0009 // Sound
#define RMID_TYPE_MOV	0x0011 
#define RMID_TYPE_SPK	0x0012
#define RMID_TYPE_CON	0x0016 // Container
#define RMID_TYPE_LIP	0x0023 

#define RMID_MAT_PARAM_COLOR1	0x774C45CE // Color Texture 1
#define RMID_MAT_PARAM_UNK2		0xA9AA497A // V=0x00000000
#define RMID_MAT_PARAM_COLOR2	0x9CB8070C // Color Texture 2
#define RMID_MAT_PARAM_UNK4		0xAD54B293 // V=0x00000000
#define RMID_MAT_PARAM_SPECULAR	0x516B333C // Specular Texture
#define RMID_MAT_PARAM_NORMAL	0xD6140246 // Normal Texture
#define RMID_MAT_PARAM_UNK7		0x44453BE5 // V=0x00000000
#define RMID_MAT_PARAM_UNK8		0x1592FD62 // V=0x00000000
#define RMID_MAT_PARAM_UNK9		0x761532CF // V=0x00000000
#define RMID_MAT_PARAM_COLOR3	0x716D7442 // Color texture 3
#define RMID_MAT_PARAM_EMISSIVE	0xFF780D7C // Emissive Texture (glow)
#pragma pack(push, 1)
typedef struct {
	unsigned long long data_offset_offset; 
	unsigned long long id_offset; // Unique ID Offset
	unsigned long long type_offset; // offset to type 
	unsigned long long compressed_size_offset; 
	unsigned long long uncompressed_size_offset; 
	unsigned long long unk1; // Always 1; 
	unsigned long long data_offset; 
	uint32_t id; // Unique Asset ID
	unsigned char type;  
	uint32_t compressed_size; 
	uint32_t uncompressed_size; 
} rmid_con_header;
#pragma pack(pop)

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t unk1;
	uint32_t mipmap_count;
} rmid_tex_mipmap_header;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t unk1;
	uint32_t size;
} rmid_tex_mipmap_record;

typedef struct {
	unsigned long long offset;
	uint32_t unk1;
	uint32_t unk2;
} rmid_tex_unk1;

typedef struct {
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;
} rmid_tex_unk2;

#pragma pack(push, 1)
typedef struct {
	unsigned char unk1[96];
	unsigned char unk2; // 0  = normal, 1 = cubemap
	unsigned char unk3; // Always 01
	unsigned char format; // 0 = RGBA , 01 = DXT1, 03, 08 = DXT3, 6 = UNK (blendshape)
	unsigned char format_type; // 0 = Uncompressed format, 1 = compressed format
	unsigned char bits_per_pixel; // 0 for compressed formats, uncompressed have seen 32 and 64.
	unsigned char unk4[3]; // Always 00 00 00 00
	uint32_t unk5; // 00 00 00 01 or 00 00 00 00 
	uint32_t unk6; // Always 01 00 00 00
	rmid_tex_mipmap_header mmh1;
	rmid_tex_mipmap_record mmr1[13];
	rmid_tex_unk1 unk7;
	unsigned char unk8[16];
	rmid_tex_mipmap_header mmh2;
	rmid_tex_unk1 unk9;
	rmid_tex_mipmap_record mmr2[13];
	rmid_tex_mipmap_header mmh3;
	rmid_tex_unk1 unk10;
	rmid_tex_mipmap_record mmr3[13];
	unsigned char unk11[24];
} rmid_tex_header;
#pragma pack(pop)

/*
typedef struct {
	uint32_t width; 
	uint32_t height; 
	uint32_t unk1;  // 01 00 00 00 
	uint32_t size; 
} texture_mipmap_data;

typedef struct {
	texture_mipmap_data maps[13];
} texture_mipmap_group;

typedef struct {
	unsigned char	null1[96];
	unsigned char	null2;
	unsigned char	unk1;	// 01
	unsigned char	format; // 00 = RGBA, 03 = DXT3, 08 = DXT5, 01 = DXT1
	unsigned char	unk2;	// 01 // Compressed 1=true 0= false
	uint32_t	bits_per_pixel;	// 00 00 00 00
	uint32_t	unk3;	// 00 00 00 01
	uint32_t	unk4;	// 01 00 00 00 
	uint32_t	width; 
	uint32_t	height; 
	uint32_t	unk5;	// 01 00 00 00
	uint32_t	num_mipmaps; 
	texture_mipmap_group mipmaps;
} texture_header1;


typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	unk3;	
	uint32_t	unk4;	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	uint32_t	unk9;	
	uint32_t	unk10;	
	uint32_t	unk11;	
	uint32_t	num_mipmaps;	

	uint32_t	unk13;	
	uint32_t	unk14;	
	uint32_t	unk15;	
	uint32_t	unk16;	
	texture_mipmap_group mipmaps;
} texture_header2;

typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	unk3;	
	uint32_t	num_mipmaps;	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	texture_mipmap_group mipmaps;
} texture_header3;

typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	unk3;	
	uint32_t	unk4;	

	uint32_t	unk5;	
	uint32_t	unk6;	
} texture_header4;
*/

typedef struct {
	uint32_t	total_materials;	
	uint32_t	unk1;	
	uint32_t	header_size; // includes materials and tables	
	uint32_t	unk4; 	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	uint32_t	unk9;	
	uint32_t	unk10;	
	uint32_t	unk11;	
	uint32_t	unk12;	

	uint64_t mesh_table_offset;	
	uint64_t mesh_material_table_offset;	
	uint64_t material_table_offset;	
	uint64_t unk13; // Always 0	

	uint32_t	unk21;	// last mesh data offset?
	uint32_t	unk22;	// always 0x0000000 
	uint32_t	num_array_size;	
	uint32_t	unk24; // always 0x100000xx	
	// uint32_t * num_array_size
} mes_header;

typedef struct {
	uint64_t offset;
	uint64_t size;
} mes_material_record;

typedef struct {
	uint32_t unk1;
	uint32_t unk2;
	uint32_t shader_id;
	uint32_t unk4;

	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;

	uint32_t total_material_params;
	uint32_t unk9;
	uint32_t unk10;
	uint32_t unk11;

	uint32_t mlaterial_size; // Includes material_header & all material_params
	uint32_t unk13;
	uint32_t unk14;
	uint32_t unk15;

	uint32_t unk16; // 0
	uint32_t unk17; // 0
	uint32_t unk18; // 0
	uint32_t unk19; // 0

} mes_material_header;

typedef struct {
	uint32_t unk1; // 64
	uint32_t unk2; // 0
	uint32_t unk3; // 0
	uint32_t unk4; // 0

	uint32_t param_type; // Param type ID
	uint32_t texture_id; // 
	uint32_t unk7;
	uint32_t unk8;

	uint32_t unk9;
	uint32_t unk10;
	uint32_t unk11;
	uint32_t unk12;

	uint32_t unk13;
	uint32_t unk14;
	uint32_t unk15;
	uint32_t unk16;

	uint32_t unk17;
	uint32_t unk18;
	uint32_t unk19;
	uint32_t unk20;

	uint32_t unk21;
	uint32_t unk22;
	uint32_t unk23;
	uint32_t unk24;

	uint32_t unk25;
	uint32_t unk26;
	uint32_t unk27;
	uint32_t unk28;

	uint32_t unk29;
	uint32_t unk30;
	uint32_t unk31;
	uint32_t unk32;
} mes_material_param;

typedef struct {
	uint64_t offset;
	uint64_t size;
} mes_mesh_record;
typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	nuk3;	
	uint32_t	unk4;	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	uint32_t	unk9;	
	uint32_t	unk10;	
	uint32_t	unk11;	
	uint32_t	unk12;	

	uint32_t	unk13;	
	uint32_t	unk14;	
	uint32_t	unk15;	
	uint32_t	unk16;	

	uint32_t	unk17;	
	uint32_t	unk18;	
	uint32_t	unk19;	
	uint32_t	unk20;	

	uint32_t	unk21;	
	uint32_t	unk22;	
	uint32_t	nuk23;	
	uint32_t	unk24;	

	uint32_t	bytes_per_vertex;	
	uint32_t	unk26;	
	uint32_t	unk27;	
	uint32_t	unk28;	

	uint32_t	num_vertices1;	
	uint32_t	num_indices1;	
	uint32_t	num_vertices2;	
	uint32_t	num_indices2;	

	uint32_t	unk33;	
	uint32_t	unk34;	
	uint32_t	unk35;	
	uint32_t	unk36;	

	uint32_t	unk37;	
	uint32_t	unk38;	
	uint32_t	unk39;	
	uint32_t	unk30;	

	uint32_t	unk41;	
	uint32_t	unk42;	
	uint32_t	nuk43;	
	uint32_t	unk44;	

	uint32_t	unk45;	
	uint32_t	unk46;	
	uint64_t	mesh_info_offset;	

	uint64_t	vertex_data_offset; // from begining of mes_mesh_header	
	uint64_t	index_data_offset; // from begining of mes_mesh_header	

	uint32_t	unk53;	
	uint32_t	unk54;	
	uint32_t	num_vertices3;	
	uint32_t	num_indices3;	

	uint32_t	unk57;	
	uint32_t	unk58;	
	uint32_t	unk59;	
	uint32_t	unk60;	
} mes_mesh_header;

typedef struct {
	uint32_t	unk1; // Always 3	
	uint32_t	unk2; // Always 0	
	uint32_t	total_vertices;	
	uint32_t	total_indices;	

	uint32_t	unk6;	// Always 0
	uint32_t	unk58;	// Always 0
	uint32_t	unk59;	// Always 0
	uint32_t	unk60;	// Always total_vertices - 1
  } mes_mesh_info;

typedef struct {
	float x;
	float y;
	float z;
} float_3;

typedef struct {
	float x;
	float y;
} float_2;

typedef struct {
	uint16_t x;
	uint16_t y;
} half_float_2;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} half_float_3;

typedef struct {
	uint16_t v1;
	uint16_t v2;
	uint16_t v3;
} mes_face_16; 

typedef struct {
	uint32_t v1;
	uint32_t v2;
	uint32_t v3;
} mes_face_32; 

typedef struct {
	half_float_3 position; 
	half_float_3 normal; 
} mes_vertex_12;

typedef struct {
	float_3 position; 
	uint32_t color;
	float_2 texcoord; 
	uint32_t unk2; 

} mes_vertex_28;

typedef struct {
	float_3 position; 
	float_3 normal; 
	uint32_t unk1; // 0xFFFF 
	uint32_t unk2; 
} mes_vertex_32;

typedef struct {
	float_3 position; 
	uint32_t unk1;
	float_2 texcoord; 
	uint32_t unk2; // 0xFFFF 
	uint32_t unk3; 
	uint32_t unk4; 
} mes_vertex_36;

typedef struct {
	half_float_3 position; //6
 	uint16_t unk1; //2
	float_3 normal; // 12
	float_2 texcoord; // 8
	uint32_t unk2; // 4

	uint32_t unk3;
	uint32_t unk4;
	uint32_t unk5;
	uint32_t unk6;

	uint32_t unk7;
	uint32_t unk8;
	uint32_t unk9;
	uint32_t unk10;
} mes_vertex_64;

typedef struct {
	half_float_3 position; //6
 	uint16_t unk1; //2
	float_3 normal; // 12
	uint32_t unk2; // 4

	uint32_t unk3;
	uint32_t unk4;
	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;
	half_float_2 texcoord; // 8
} mes_vertex_52;

typedef struct {
	half_float_3 position;
 	uint16_t unk1; 
	float_3 normal;
	uint32_t unk2; 
	uint32_t unk3; 
	uint32_t unk4;
	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;

	uint32_t unk8; // 0xFFFF
	half_float_2 texcoord1;
	half_float_2 texcoord2;
} mes_vertex_56;

/*
	mes_vertex_60
	shd_p_DeferredAlphaTestDiffuseSpecularNormalEmissive
*/
typedef struct {
	float_3 position; //6
	float_3 normal; // 12
	float_3 tangent;
	float_3 bitangent;
	uint32_t unk1;
	float_2 texcoord; // 8
} mes_vertex_60;

/*
	mes_vertex_68
	shd_p_DeferredAlphaTestDualDiffuseSpecularNormal
	shd_p_DeferredDualDiffuseSpecularNormal
*/
typedef struct {
	float_3 position;
	float_3 normal; 
	float_3 tangent;
	float_3 bitangent;
	uint32_t unk12;
	float_2 texcoord1;
	float_2 texcoord2;
} mes_vertex_68;

typedef struct { //96 bytes
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;

	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;

	uint32_t unk9;
	uint32_t unk10;
	uint32_t unk11;
	uint32_t unk12;

	uint32_t num_records;
	uint32_t unk14;
	uint32_t unk15;
	uint32_t unk16;

	uint32_t unk17;
	uint32_t unk18;
	uint32_t unk19;
	uint32_t unk20;

	uint32_t unk21;
	uint32_t unk22;
	uint32_t unk23;
	uint32_t unk24;
} mes_ski_material_header;

typedef struct { //128 bytes
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;

	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;

	uint32_t unk9;
	uint32_t unk10;
	uint32_t unk11;
	uint32_t unk12;

	uint32_t unk13;
	uint32_t unk14;
	uint32_t unk15;
	uint32_t unk16;

	uint32_t unk17;
	uint32_t unk18;
	uint32_t unk19;
	uint32_t unk20;

	uint32_t unk21;
	uint32_t unk22;
	uint32_t unk23;
	uint32_t unk24;

	uint32_t unk25;
	uint32_t unk26;
	uint32_t unk27;
	uint32_t unk28;

	uint32_t unk29;
	uint32_t unk30;
	uint32_t unk31;
	uint32_t unk32;
} mes_ski_material_record;

typedef struct {
	uint32_t	offset;	
	uint32_t	unk1;	// always 0x0000000 
	uint32_t	size;	
	uint32_t	unk2;	// always 0x0000000 
} mes_ski_material_index;

typedef struct {
	uint32_t	offset;	
	uint32_t	unk1;	// always 0x0000000 
	uint32_t	size;	
	uint32_t	unk2;	// always 0x0000000 
} mes_ski_mesh_index;

typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	nuk3;	
	uint32_t	unk4;	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	uint32_t	unk9;	
	uint32_t	unk10;	
	uint32_t	unk11;	
	uint32_t	unk12;	

	uint32_t	unk13;	
	uint32_t	unk14;	
	uint32_t	unk15;	
	uint32_t	unk16;	

	uint32_t	unk17;	
	uint32_t	unk18;	
	uint32_t	unk19;	
	uint32_t	unk20;	
} mes_ski_info_header;

typedef struct {
	uint32_t	unk1;	
	uint32_t	unk2;	
	uint32_t	nuk3;	
	uint32_t	unk4;	

	uint32_t	unk5;	
	uint32_t	unk6;	
	uint32_t	unk7;	
	uint32_t	unk8;	

	uint32_t	unk9;	
	uint32_t	unk10;	
	uint32_t	unk11;	
	uint32_t	unk12;	

	uint32_t	unk13;	
	uint32_t	unk14;	
	uint32_t	unk15;	
	uint32_t	unk16;	

	uint32_t	unk17;	
	uint32_t	unk18;	
	uint32_t	unk19;	
	uint32_t	unk20;	

	uint32_t	unk21;	
	uint32_t	unk22;	
	uint32_t	nuk23;	
	uint32_t	unk24;	

	uint32_t	bytes_per_vertex;	
	uint32_t	unk26;	
	uint32_t	unk27;	
	uint32_t	unk28;	

	uint32_t	num_vertices1;	
	uint32_t	num_indices1;	
	uint32_t	num_vertices2;	
	uint32_t	num_indices2;	

	uint32_t	unk33;	
	uint32_t	unk34;	
	uint32_t	unk35;	
	uint32_t	unk36;	

	uint32_t	unk37;	
	uint32_t	unk38;	
	uint32_t	unk39;	
	uint32_t	unk30;	

	uint32_t	unk41;	
	uint32_t	unk42;	
	uint32_t	nuk43;	
	uint32_t	unk44;	

	uint32_t	unk45;	
	uint32_t	unk46;	
	uint32_t	unk47;	
	uint32_t	unk48;	

	uint32_t	unk49;	
	uint32_t	unk50;	
	uint32_t	unk51;	
	uint32_t	unk52;	

	uint32_t	unk53;	
	uint32_t	unk54;	
	uint32_t	num_vertices3;	
	uint32_t	num_indices3;	

	uint32_t	unk57;	
	uint32_t	unk58;	
	uint32_t	unk59;	
	uint32_t	unk60;	
} mes_ski_mesh_header;

// Compressed vertices use a half float (short) for position and normal
typedef struct {
	unsigned short	x;	
	unsigned short	y;	
	unsigned short	z;	
	unsigned short	nx;	
	unsigned short	ny;	
	unsigned short	nz;	
	float	u;	
	float	v;	
} mes_ski_compressed_vertex;
/*
typedef struct {
	float	x;	
	float	y;	
	float	z;	
	float	nx;	
	float	ny;	
	float	nz;	
	float	u;	
	float	v;	
} mes_ski_vertex;
*/
// 60
typedef struct {
	short	x;	
	short	y;	
	short	z;	
	short unk1;
	float	nx;	
	float	ny;	
	float	nz;	
	int unk2;
	int unk3;
	int unk4;
	int unk5;
	int unk6;
	int unk7;
	int unk8;
	short	u;	
	short	v;	
} mes_ski_vertex;


typedef struct {
	unsigned short	v1;	
	unsigned short	v2;	
	unsigned short	v3;	
} mes_ski_face;

typedef struct {
	uint32_t id;
	uint32_t type;
} mes_ski_id_type;

typedef struct {
	uint32_t unk1;
	uint32_t unk2;
	uint32_t ske_id;
	uint32_t num_animations;

	unsigned long long ani_records_offset;
	unsigned long long unk_offset1;

	unsigned long long unk_offset2;
	unsigned long long unk_offset3;

	unsigned long long unk_offset4;
	unsigned long long unk_offset5;

	unsigned long long unk_offset6;
	unsigned long long unk_offset7;
} act_header;

typedef struct {
	uint32_t unk1;
} shd_header;

typedef struct {
	unsigned long long data_offset;
	uint32_t data_size;
	uint32_t unk1;

	uint32_t sample_rate; 
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;

	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;
} rmid_snd_header;

#endif // _RMID_H_