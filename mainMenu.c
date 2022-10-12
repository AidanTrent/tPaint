// Aidan Trent

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <inttypes.h>

#include "tga.h"
#include "tPaint.h"
#include "menuStrings.h"

#define FILE_NAME_MAX 255
#define UNCOMP_RGB 2 // Uncompressed RGB image type
#define IMG_PIXEL_SIZE 24 // 24 bits for RGB. 32 bits for RGB+alpha (not implemented)

// Clears screen and prints title
void printTitle(){
	printf("\033[H\033[2J"); // Clear terminal
	char* title = TITLE_STR;

	int step = 0;
	int color = 0;
	for (int i = 0; title[i] != '\0'; i++){
		color++;
		if (step == 0){
			printf("\033[38;2;255;%d;0m", color);
		}
		else if (step == 1){
			printf("\033[38;2;0;255;%dm", color);
		}
		else{
			printf("\033[38;2;%d;0;255m", color);
		}
		if (color == 255){
			color = 0;
			step++;
		}

		printf("%c", title[i]);
	}
	// Reset foreground
	printf("\033[38;2;255;255;255m");
}

void menuFailure(char* errMsg){
	fprintf(stderr, "%s", errMsg);
	printf("Press any key to continue...");
	getchar();
	getchar();
}

TGAImg* menuNewCanvas(Screen* scr){
	printTitle();
	// Temp disable instant input and enable cursor
	struct termios termPaint;
	tcgetattr(STDIN_FILENO, &termPaint);
	tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
	printf("\033[?25h");

	// Get dimensions
	printf("Enter width : ");
	uint16_t width;
	scanf("%" SCNu16, &width);
	if (width < 0 || width > UINT16_MAX - 1){
		menuFailure("ERROR : Invalid width\n");
		return(NULL);
	}
	printf("Enter height : ");
	uint16_t height;
	scanf("%" SCNu16, &height);
	if (height < 0 || height > UINT16_MAX - 1){
		menuFailure("ERROR : Invalid height\n");
		return(NULL);
	}

	// Re-enable instant user input and disable cursor
	tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);
	printf("\033[?25l");

	// Attempt to make canvas
	TGAHeader header = {0, 0, UNCOMP_RGB, 0, 0, 0, 0, 0, width, height, IMG_PIXEL_SIZE, 0};
	TGAImg* img = makeImage(&header);
	if (img == NULL){
		menuFailure("");
		return(NULL);
	}

	return(img);
}
TGAImg* menuLoad(Screen* scr){
	printTitle();
	// Temp disable instant input and enable cursor
	struct termios termPaint;
	tcgetattr(STDIN_FILENO, &termPaint);
	tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
	printf("\033[?25h");

	// Get file name
	printf("Enter file name : ");
	char fileName[FILE_NAME_MAX];
	scanf("%s", fileName);

	// Re-enable instant user input and disable cursor
	tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);
	printf("\033[?25l");

	// Attempt to load image
	TGAImg* img = loadImage(fileName);
	if (img == NULL){
		menuFailure("");
	}

	return(img);
}

TGAImg* mainMenu(Screen* scr){
	TGAImg* img;
	int pickCanvas = 0;

	while(1){
		printTitle();
		printf(CTRL_STR);

		switch (getchar()){
			// Change increment
			case 'l':
				img = menuLoad(scr);
				if (img != NULL){
					return(img);
				}
				break;
			case 'n':
				img = menuNewCanvas(scr);
				if (img != NULL){
					return(img);
				}
				break;
			case 'q':
				return(NULL);
				break;
			default:
				break;
		}
	}
}
