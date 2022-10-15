/* Aidan Trent
 * An implementation of 24 bit PPM images
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

PPMImg* makeImage(PPMHeader* hd){
	// Allocate memory for header + image pixels (using info from header params)
	uint_fast32_t dataFieldBytes = hd->width * hd->height * 3;
	PPMImg* img = calloc(1, sizeof(PPMHeader) + dataFieldBytes);
	if (!img){
		fprintf(stderr, "ERROR: calloc fail for img @ makeImage\n");
		return(NULL);
	}
	img->header = *hd;

	return img;
}

void saveImage(char imgName[], PPMImg* img){
	FILE* imageFile = fopen(imgName, "wb");

	// Write header
    fprintf(imageFile, "%s\n", img->header.id);
    fprintf(imageFile, "%d %d\n", img->header.width, img->header.height);
    fprintf(imageFile, "%d\n", img->header.colorDepth);

	// Write imageDataField
	uint_fast32_t dataFieldBytes = img->header.width * img->header.height * 3;
    if (fwrite(&img->imageDataField, dataFieldBytes, 1, imageFile) != 1){
		fprintf(stderr, "ERROR: imageDataField not written properly @ saveImage\n");
		fclose(imageFile);
	}

    fclose(imageFile);
}

PPMImg* loadImage(char imgName[]){
	PPMHeader hd;
	FILE* imageFile = fopen(imgName, "rb");
	if (imageFile == NULL){
		fprintf(stderr, "ERROR: failed to open file %s @ loadImage\n", imgName);
		return(NULL);
	}

	// Write header
	fscanf(imageFile, "%s", hd.id);
	fscanf(imageFile, "%d %d", &hd.width, &hd.height);
	fscanf(imageFile, "%d", &hd.colorDepth);
	fscanf(imageFile, " ", NULL);

	// Check header
	if (strcmp(hd.id, "P6") || hd.colorDepth != 255){
		fprintf(stderr, "ERROR: PPM identifier or color depth incompatible");
		return(NULL);
	}

	// Read imageDataField
	PPMImg* img = makeImage(&hd);
	uint_fast32_t dataFieldBytes = img->header.width * img->header.height * 3;
	if (fread(img->imageDataField, dataFieldBytes, 1, imageFile) != 1){
		fprintf(stderr, "ERROR: imageDataField not read properly @ loadImage\n");
		fclose(imageFile);
		return(NULL);
	}

	fclose(imageFile);
	return(img);
}

void setPixel(PPMImg* img, RGB color, uint_fast16_t x, uint16_t y){
	uint_fast32_t index = ((y * img->header.width) + x) * 3; // Convert x and y to index for array
	// Apply pixel change to all colors
	memcpy(&img->imageDataField[index], &color, 3);
}

RGB getPixel(PPMImg* img, uint16_t x, uint16_t y){
	uint_fast32_t index = ((y * img->header.width) + x) * 3; // Convert x and y to index for array

	RGB color;
	memcpy(&color, &img->imageDataField[index], 3);
	return color;
}
