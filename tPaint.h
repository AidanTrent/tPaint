#ifndef TPAINT_H
#define TPAINT_H

#include <termios.h>
#include <stdint.h>
#include "tga.h"

#define BUF_SIZE 255

#define SWATCHES_NUM 9
typedef struct{
	struct termios termDefault;
	uint16_t viewY;
	uint16_t viewX;
	uint8_t saving;
	uint8_t picking;
	uint8_t setting; // Flag to set a pixel
	uint8_t inc; // Value to change swatch color and view by on each input
	RGB swatches[SWATCHES_NUM];
	RGB* curSwatch;
} Screen;

#endif
