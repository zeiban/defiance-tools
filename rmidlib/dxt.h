#ifndef _DXT_H_
#define _DXT_H_
/*
	DXT Decompression Module
*/
#include <stdint.h>

uint32_t PackRGBA(uint8_t  r, uint8_t  g, uint8_t  b, uint8_t  a);
void DecompressBlockDXT1Internal(const uint8_t * block, uint32_t  * output, uint32_t  output_stride, const uint8_t * alpha_values);
void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width, uint8_t * block_storage, uint32_t * image);
void DecompressBlockDXT3(uint32_t  x, uint32_t  y, uint32_t  width, const uint8_t * blocks, uint32_t * image);
void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width, uint8_t * block_storage, uint32_t * image);
void DecompressDXT1(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image);
void DecompressDXT3(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image);
void DecompressDXT5(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image);

#endif // _DXT_H_