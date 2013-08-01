#include <stdlib.h>
#include <stdio.h>

#include "wadlib.h"
#include "dds.h"

int WadWriteTexToPng(wad_record * wr,  int y_invert, uint32_t no_alpha, const char * dir, const char * name) {
	rmid_file rf;

	if(wr->type != RMID_TYPE_TEX) {
		return 1;
	}

	if(RmidLoadFromFile(wr->filename, wr->data_offset, wr->data_size, &rf) != 0) {
		return 1;
	}
	
	if(RmidWriteTexToPng(&rf, y_invert, no_alpha, dir, name == NULL ? name : wr->name) != 0) {
		RmidFree(&rf);
		return 1;
	}

	RmidFree(&rf);
	return 0;
}

int WadWriteTexToDds(wad_record * wr,  int y_invert, const char * dir, const char * name) {
	char filename[512];
	FILE * file;
	rmid_file rf;
	rmid_tex_header * th;
	uint8_t * image;
	dds_header ddsh;
	uint32_t size;

	memset(&ddsh, 0, sizeof(dds_header));

	if(wr->type != RMID_TYPE_TEX) {
		return 1;
	}

	if(RmidLoadFromFile(wr->filename, wr->data_offset, wr->data_size, &rf) != 0) {
		return 1;
	}
	
	th		= (rmid_tex_header *)(((uint8_t*)rf.data) + sizeof(rmid_header));
	image	= (uint8_t *)(((uint8_t*)rf.data) + sizeof(rmid_header) + sizeof(rmid_tex_header));

	ddsh.dwMagic = DDS_MAGIC;
	ddsh.dwMipMapCount = th->mmh1.mipmap_count;
	ddsh.dwWidth = th->mmh1.width;
	ddsh.dwHeight = th->mmh1.height;
	ddsh.dwSize = 124;
	ddsh.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;

	if(th->format == 0) {
		// RGBA
		ddsh.dwDepth = 32;
		ddsh.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;
		ddsh.sPixelFormat.dwFlags = DDPF_RGB;
		ddsh.sPixelFormat.dwRGBBitCount = 32;
		ddsh.sPixelFormat.dwRBitMask	= 0xff;;
		ddsh.sPixelFormat.dwGBitMask	= 0xff00;
		ddsh.sPixelFormat.dwBBitMask	 = 0xff0000;
		ddsh.sPixelFormat.dwAlphaBitMask = 0xff000000;
		ddsh.sPixelFormat.dwRGBBitCount = 32;
		size = (ddsh.dwWidth * 32  + 7) / 8;
	}
	else if(th->format == 1) {
		// DXT1
		ddsh.sPixelFormat.dwFlags = DDPF_FOURCC;
		ddsh.sPixelFormat.dwFourCC = D3DFMT_DXT1;
		size = max( 4, ddsh.dwWidth )/4 * max( 4, ddsh.dwHeight )/4 * 8;
	}
	else if (th->format == 3 || th->format == 8) { 
		// DXT5
		ddsh.sPixelFormat.dwFlags = DDPF_FOURCC;
		ddsh.sPixelFormat.dwFourCC = D3DFMT_DXT5;
		size = max( 4, ddsh.dwWidth )/4 * max( 4, ddsh.dwHeight )/4 * 16;
	} 
	else {
		RmidFree(&rf);
		printf("Unsupported format %d for DDS export\n", th->format);
		return 1;
	}

	ddsh.dwPitchOrLinearSize = size;
	ddsh.sCaps.dwCaps1 = DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_TEXTURE;
	ddsh.sPixelFormat.dwSize = 32;

	sprintf_s(filename, sizeof(filename),"%s\\%s.dds", dir, name == NULL ? wr->name : name);
	if(fopen_s(&file, filename, "wb") != 0) {
		printf("ERROR: Unable to open file %s\n", filename);
		RmidFree(&rf);
		return 1;
	}

	fwrite(&ddsh, sizeof(ddsh), 1, file);
	fwrite(image, (size_t)(rf.size - (sizeof(rmid_header) + sizeof(rmid_tex_header))), 1, file);

	fclose(file);
	

	RmidFree(&rf);
	return 0;

}