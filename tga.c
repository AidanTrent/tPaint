/* Aidan Trent
 * An implementation of 24 bit TARGA image (create, manipulate pixels, and export)
 * Put together with a elementary understanding of C
 * Made with info from https://www.gamers.org/dengine/quake3/TGA.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct{
	uint8_t idLength;			// Length of image ID field (0-255)
	uint8_t colorMapType;		// If a color map is present (1) or not (0)
	uint8_t imageType;			// Compression and color types (0-3 and 9-11)
	uint16_t colorMapOrigin;	// Index of first color map entry
	uint16_t colorMapLength;	// Count of color map entries
	uint8_t colorMapEntrySize;	// Number of bits in each color map entry. 16 for Targa 16, 24 for Targa 24...
	uint16_t xOrigin;			// X coord of the lower left corner of the image
	uint16_t yOrigin;			// Y coord of the lower left corner of the image
	uint16_t width;				// Width of the image in pixels
	uint16_t height;			// Height of the image in pixels
	uint8_t imagePixelSize;		// Number of bits in a stored pixel index
	uint8_t imageDescriptorByte;// Document says to just keep this byte as 0
} TGAHeader;

typedef struct{
	TGAHeader header;
	uint8_t imageDataField[];	// Array of image pixels.
} TGAImg;

typedef struct{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} RGB;

TGAImg* makeImage(TGAHeader* hd){
	// Allocate memory for header + image pixels (using info from header params)
	uint_fast32_t dataFieldBytes = hd->width * hd->height * (hd->imagePixelSize / 8);
	TGAImg* img = calloc(1, sizeof(TGAHeader) + dataFieldBytes);
	if (!img){
		fprintf(stderr, "ERROR: calloc fail for img @ makeImage\n");
		return(NULL);
	}
	img->header = *hd;

	return img;
}

// Writes contents of a TGAImg struct to a image of name imgName
void saveImage(char imgName[], TGAImg* img){
	FILE* imageFile = fopen(imgName, "wb");

    fwrite(&img->header.idLength, sizeof(img->header.idLength), 1, imageFile);
    fwrite(&img->header.colorMapType, sizeof(img->header.colorMapType), 1, imageFile);
    fwrite(&img->header.imageType, sizeof(img->header.imageType), 1, imageFile);
    fwrite(&img->header.colorMapOrigin, sizeof(img->header.colorMapOrigin), 1, imageFile);
    fwrite(&img->header.colorMapLength, sizeof(img->header.colorMapLength), 1, imageFile);
    fwrite(&img->header.colorMapEntrySize, sizeof(img->header.colorMapEntrySize), 1, imageFile);
    fwrite(&img->header.xOrigin, sizeof(img->header.xOrigin), 1, imageFile);
    fwrite(&img->header.yOrigin, sizeof(img->header.yOrigin), 1, imageFile);
    fwrite(&img->header.width, sizeof(img->header.width), 1, imageFile);
    fwrite(&img->header.height, sizeof(img->header.height), 1, imageFile);
    fwrite(&img->header.imagePixelSize, sizeof(img->header.imagePixelSize), 1, imageFile);
    fwrite(&img->header.imageDescriptorByte, sizeof(img->header.imageDescriptorByte), 1, imageFile);

	uint32_t dataFieldBytes = img->header.width * img->header.height * (img->header.imagePixelSize / 8);
    if (fwrite(&img->imageDataField, dataFieldBytes, 1, imageFile) != 1){
		fprintf(stderr, "ERROR: imageDataField not written properly @ saveImage\n");
		fclose(imageFile);
	}

    fclose(imageFile);
}

TGAImg* loadImage(char imgName[]){
	TGAHeader hd;
	FILE* imageFile = fopen(imgName, "rb");
	if (imageFile == NULL){
		fprintf(stderr, "ERROR: failed to open file %s @ loadImage\n", imgName);
		return(NULL);
	}

    fread(&hd.idLength, sizeof(hd.idLength), 1, imageFile);
    fread(&hd.colorMapType, sizeof(hd.colorMapType), 1, imageFile);
    fread(&hd.imageType, sizeof(hd.imageType), 1, imageFile);
    fread(&hd.colorMapOrigin, sizeof(hd.colorMapOrigin), 1, imageFile);
    fread(&hd.colorMapLength, sizeof(hd.colorMapLength), 1, imageFile);
    fread(&hd.colorMapEntrySize, sizeof(hd.colorMapEntrySize), 1, imageFile);
    fread(&hd.xOrigin, sizeof(hd.xOrigin), 1, imageFile);
    fread(&hd.yOrigin, sizeof(hd.yOrigin), 1, imageFile);
    fread(&hd.width, sizeof(hd.width), 1, imageFile);
    fread(&hd.height, sizeof(hd.height), 1, imageFile);
    fread(&hd.imagePixelSize, sizeof(hd.imagePixelSize), 1, imageFile);
    fread(&hd.imageDescriptorByte, sizeof(hd.imageDescriptorByte), 1, imageFile);

	TGAImg* img = makeImage(&hd);
	uint32_t dataFieldBytes = img->header.width * img->header.height * (img->header.imagePixelSize / 8);
	if (fread(img->imageDataField, dataFieldBytes, 1, imageFile) != 1){
		fprintf(stderr, "ERROR: imageDataField not read properly @ loadImage\n");
		fclose(imageFile);
		return(NULL);
	}

	fclose(imageFile);
	return(img);
}

void setPixel(TGAImg* img, RGB color, uint16_t x, uint16_t y){
	uint_fast32_t index = ((y * img->header.width) + x) * 3; // Convert x and y to index for array
	// Apply pixel change to all colors
	memcpy(&img->imageDataField[index], &color, 3);
}

RGB getPixel(TGAImg* img, uint16_t x, uint16_t y){
	uint_fast32_t index = ((y * img->header.width) + x) * 3; // Convert x and y to index for array

	RGB color;
	memcpy(&color, &img->imageDataField[index], 3);
	return color;
}
