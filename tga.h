#ifndef TGA_H
#define TGA_H
#include <stdint.h>

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

TGAImg* makeImage(TGAHeader* hd);

// Writes contents of a TGAImg struct to a image of name imgName
void saveImage(char imageName[], TGAImg* img);

TGAImg* loadImage(char imgName[]);

void setPixel(TGAImg* img, RGB color, uint16_t x, uint16_t y);

RGB getPixel(TGAImg* img, uint16_t x, uint16_t y);
#endif
