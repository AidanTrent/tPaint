#ifndef PPM_H
#define PPM_H
#include <stdint.h>

typedef struct{
	char id[2]; // TODO : only supporting P6 for now
	int width;
	int height;
	int colorDepth; // TODO : only supporting 24 bit for now
} PPMHeader;

typedef struct{
	PPMHeader header;
	uint8_t imageDataField[];	// Array of image pixels.
} PPMImg;

typedef struct{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} RGB;

PPMImg* makeImage(PPMHeader* hd);

void saveImage(char imgName[], PPMImg* img);

PPMImg* loadImage(char imgName[]);

void setPixel(PPMImg* img, RGB color, uint_fast16_t x, uint16_t y);

RGB getPixel(PPMImg* img, uint16_t x, uint16_t y);
#endif
