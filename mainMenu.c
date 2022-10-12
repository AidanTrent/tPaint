// Aidan Trent

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <inttypes.h>

#include "ppm.h"
#include "tPaint.h"
#include "menuStrings.h"

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

void menuFailure(char* errMsg, struct termios* term){
	// Re-enable instant user input and disable cursor
	tcsetattr(STDIN_FILENO, TCSANOW, term);
	printf("\033[?25l");

	fprintf(stderr, "%s", errMsg);
	printf("Press any key to continue...");
	getchar();
	getchar();
}

PPMImg* menuNewCanvas(Screen* scr){
	char *end;
	char buf[BUF_SIZE];

	printTitle();
	// Temp disable instant input and enable cursor
	struct termios termPaint;
	tcgetattr(STDIN_FILENO, &termPaint);
	tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
	printf("\033[?25h");

	// Get dimensions
	printf("Enter width : ");
	fgets(buf, BUF_SIZE, stdin);
	unsigned long width = strtoul(buf, &end, 10);
	if (width < 1 || width > UINT16_MAX - 1 || end == buf){
		menuFailure("ERROR : Invalid height\n", &termPaint);
		return(NULL);
	}
	printf("Enter height : ");
	fgets(buf, BUF_SIZE, stdin);
	unsigned long height = strtoul(buf, &end, 10);
	if (height < 1 || height > UINT16_MAX - 1 || end == buf){
		menuFailure("ERROR : Invalid height\n", &termPaint);
		return(NULL);
	}

	// Attempt to make canvas
	PPMHeader header = {"P6", width, height, 255};
	PPMImg* img = makeImage(&header);
	if (img == NULL){
		menuFailure("", &termPaint);
		return(NULL);
	}

	// Re-enable instant user input and disable cursor
	tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);
	printf("\033[?25l");
	return(img);
}
PPMImg* menuLoad(Screen* scr){
	printTitle();
	// Temp disable instant input and enable cursor
	struct termios termPaint;
	tcgetattr(STDIN_FILENO, &termPaint);
	tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
	printf("\033[?25h");

	// Get file name
	printf("Enter file name : ");
	char fileName[BUF_SIZE];
	scanf(" %s", fileName);

	// Attempt to load image
	PPMImg* img = loadImage(fileName);
	if (img == NULL){
		menuFailure("", &termPaint);
		return(NULL);
	}

	// Re-enable instant user input and disable cursor
	tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);
	printf("\033[?25l");
	return(img);
}

PPMImg* mainMenu(Screen* scr){
	PPMImg* img;
	int pickCanvas = 0;

	while(1){
		printf("\033[48;2;0;0;0m"); // Background to black
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
