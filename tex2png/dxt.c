#include <stdint.h>

#include "dxt.h"

uint32_t PackRGBA(uint8_t  r, uint8_t  g, uint8_t  b, uint8_t  a)
{
	return r | (g  << 8) | (b << 16) | (a << 24); 
}

void DecompressBlockDXT1Internal(const uint8_t * block, uint32_t  * output, uint32_t  output_stride, const uint8_t * alpha_values)
{
	uint32_t temp, code;

	uint16_t color0, color1;
	uint8_t r0, g0, b0, r1, b1, g1;

	int i, j;

	color0 = *(const uint16_t*)(block);
	color1 = *(const uint16_t*)(block + 2);

    temp = (color0 >> 11) * 255 + 16;
    r0 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    g0 = (uint8_t)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    b0 = (uint8_t)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    r1 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    g1 = (uint8_t)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    b1 = (uint8_t)((temp/32 + temp)/32);

	code = *(const uint32_t *)(block + 4);

    if (color0 > color1) {
		for (j = 0; j < 4; ++j) {
			for (i = 0; i < 4; ++i) {
				uint32_t final_color, position_code;
				uint8_t alpha;

				alpha = alpha_values [j*4+i];

				final_color = 0;
				position_code = (code >>  2*(4*j+i)) & 0x03;

				switch (position_code) {
				case 0:
						final_color = PackRGBA(r0, g0, b0, alpha);
						break;
				case 1:
						final_color = PackRGBA(r1, g1, b1, alpha);
						break;
				case 2:
						final_color = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, alpha);
						break;
				case 3:
						final_color = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, alpha);
						break;
				}

				output [j*output_stride + i] = final_color;
			}
		}
	} else {
		for (j = 0; j < 4; ++j) {
			for (i = 0; i < 4; ++i) {
				uint32_t final_color, position_code;
				uint8_t alpha;

				alpha = alpha_values [j*4+i];

				final_color = 0;
				position_code = (code >>  2*(4*j+i)) & 0x03;

				switch (position_code) {
				case 0:
						final_color = PackRGBA(r0, g0, b0, alpha);
						break;
				case 1:
						final_color = PackRGBA(r1, g1, b1, alpha);
						break;
				case 2:
						final_color = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, alpha);
						break;
				case 3:
						final_color = PackRGBA(0, 0, 0, alpha);
						break;
				}

				output [j*output_stride + i] = final_color;
			}
		}
    }
}

void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width, uint8_t * block_storage, uint32_t * image)
{
    static const uint8_t const_alpha [] = {
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255
    };

	DecompressBlockDXT1Internal(block_storage, image + x + (y * width), width,  const_alpha);
}

void DecompressBlockDXT3(uint32_t  x, uint32_t  y, uint32_t  width, const uint8_t * blocks, uint32_t * image)
{
	int i;

	uint8_t  alpha_values[16] = {0};
	
	for(i = 0; i < 4; ++i)
	{
		const uint16_t * alpha_data = (const uint16_t *)(blocks);
		alpha_values[i*4 + 0] = (((*alpha_data) >> 0)  & 0xF ) * 17;
		alpha_values[i*4 + 1] = (((*alpha_data) >> 4)  & 0xF ) * 17;
		alpha_values[i*4 + 2] = (((*alpha_data) >> 8)  & 0xF ) * 17;
		alpha_values[i*4 + 3] = (((*alpha_data) >> 12) & 0xF ) * 17;

		blocks += 2;
	}

	DecompressBlockDXT1Internal(blocks, image + x + (y * width), width, alpha_values);
}

void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width, uint8_t * block_storage, uint32_t * image)
{

	uint8_t alpha0, alpha1;
	const uint8_t * bits;
	uint32_t alpha_code1;
	uint16_t alpha_code2;

	uint16_t color0, color1;
	uint8_t r0, g0, b0, r1, b1, g1;

	int i, j;

	unsigned int temp, code;

	alpha0 = *(block_storage);
	alpha1 = *(block_storage + 1);

	bits = block_storage + 2;
	alpha_code1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
	alpha_code2 = bits[0] | (bits[1] << 8);

	color0 = *(const uint16_t *)(block_storage + 8);
	color1 = *(const uint16_t *)(block_storage + 10);

    temp = (color0 >> 11) * 255 + 16;
    r0 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    g0 = (uint8_t)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    b0 = (uint8_t)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    r1 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    g1 = (uint8_t)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    b1 = (uint8_t)((temp/32 + temp)/32);

    code = *(const uint32_t*)(block_storage + 12);

    for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
            uint8_t final_alpha;
            int alpha_code, alpha_code_index;
            uint8_t color_code;
            uint32_t final_color;

            alpha_code_index = 3*(4*j+i);
            if (alpha_code_index <= 12) {
                    alpha_code = (alpha_code2 >> alpha_code_index) & 0x07;
            } else if (alpha_code_index == 15) {
                    alpha_code = (alpha_code2 >> 15) | ((alpha_code1 << 1) & 0x06);
            } else /* alphaCodeIndex >= 18 && alphaCodeIndex <= 45 */ {
                    alpha_code = (alpha_code1 >> (alpha_code_index - 16)) & 0x07;
            }

            if (alpha_code == 0) {
                    final_alpha = alpha0;
            } else if (alpha_code == 1) {
                    final_alpha = alpha1;
            } else {
                    if (alpha0 > alpha1) {
                            final_alpha = (uint8_t)(((8-alpha_code)*alpha0 + (alpha_code-1)*alpha1)/7);
                    } else {
                            if (alpha_code == 6) {
                                    final_alpha = 0;
                            } else if (alpha_code == 7) {
                                    final_alpha = 255;
                            } else {
                                    final_alpha = (uint8_t)(((6-alpha_code)*alpha0 + (alpha_code-1)*alpha1)/5);
                            }
                    }
            }

            color_code = (code >> 2*(4*j+i)) & 0x03; 
            final_color = 0;

            switch (color_code) {
            case 0:
                    final_color = PackRGBA(r0, g0, b0, final_alpha);
                    break;
            case 1:
                    final_color = PackRGBA(r1, g1, b1, final_alpha);
                    break;
            case 2:
                    final_color = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, final_alpha);
                    break;
            case 3:
                    final_color = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, final_alpha);
                    break;
            }

            image [i + x + (width* (y+j))] = final_color; 
        }
    }
}

void DecompressDXT1(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image)
{
	uint32_t block_count_x = (width + 3) / 4;
	uint32_t block_count_y = (height + 3) / 4;
	uint32_t block_width = (width < 4) ? width : 4;
	uint32_t block_height = (height < 4) ? height : 4;
	uint32_t j, i;

	for(j = 0; j < block_count_y; j++)
	{
		for(i = 0; i < block_count_x; i++)
		{
			DecompressBlockDXT1(i*4, j*4, width, blocks + i * 8, image); 
		}
		blocks += block_count_x * 8;
	}
}

void DecompressDXT3(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image)
{
	uint32_t block_count_x = (width + 3) / 4;
	uint32_t block_count_y = (height + 3) / 4;
	uint32_t block_width = (width < 4) ? width : 4;
	uint32_t block_height = (height < 4) ? height : 4;
	uint32_t j, i;

	for(j = 0; j < block_count_y; j++)
	{
		for(i = 0; i < block_count_x; i++)
		{
			DecompressBlockDXT3(i*4, j*4, width, blocks + i * 16, image); 
		}
		blocks += block_count_x * 16;
	}
}

void DecompressDXT5(uint32_t width, uint32_t height, uint8_t * blocks, uint32_t * image)
{
	uint32_t block_count_x = (width + 3) / 4;
	uint32_t block_count_y = (height + 3) / 4;
	uint32_t block_width = (width < 4) ? width : 4;
	uint32_t block_height = (height < 4) ? height : 4;
	uint32_t j, i;

	for(j = 0; j < block_count_y; j++)
	{
		for(i = 0; i < block_count_x; i++)
		{
			DecompressBlockDXT5(i*4, j*4, width, blocks + i * 16, image); 
		}
		blocks += block_count_x * 16;
	}
}
