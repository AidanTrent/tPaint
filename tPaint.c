/* Aidan Trent
 * Paint inside your terminal with true (24 bit) color!
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "tga.h"
#include "tPaint.h"
#include "mainMenu.h"

#define FOREGROUND_SWAP 300 // Swap text color to white when r+g+b > than this

void refreshScreen(Screen* scr, TGAImg* img){
	// Handle term dimensions
	struct winsize win;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

	RGB color;
	char c;
	int32_t y = scr->viewY + win.ws_row / 2;
	printf("\033[H\033[2J"); // Clear terminal
	for (int row = 0; row < win.ws_row; y--, row++){
		// Print status bar
		if (row == win.ws_row - 1){
			for (int i = 0; i < SWATCHES_NUM; i++){
				printf("\033[48;2;%d;%d;%dm", scr->swatches[i].red, scr->swatches[i].green, scr->swatches[i].blue); // Background
				if (&scr->swatches[i] == scr->curSwatch){
					if (scr->curSwatch->red + scr->curSwatch->green + scr->curSwatch->blue > FOREGROUND_SWAP){
						printf("\033[38;2;0;0;0m"); // Foreground
					}
					printf("%d", i + 1);
				}
				else{
					printf(" ");
				}
			}
			printf("\033[38;2;255;255;255m"); // Foreground
			printf("\033[48;2;0;0;0m"); // Foreground
			printf(" R:%d G:%d B:%d", scr->curSwatch->red, scr->curSwatch->green, scr->curSwatch->blue);
			if (scr->setting == 1){
				printf(" D");
			}
			if (scr->inc != 1){
				printf(" F");
			}
		}
		// Print canvas
		else{
			int32_t x = scr->viewX - win.ws_col / 2;
			for (int col = 0; col < win.ws_col; x++, col++){

				c = ' ';
				if (x >= 0 && x < img->header.width && y >= 0 && y < img->header.height){
					color = getPixel(img, x, y);
				}
				else{
					color.red = 20;
					color.blue = 20;
					color.green = 20;
				}
				// Center character
				if (col == win.ws_col / 2 && row == win.ws_row / 2){
					if (scr->setting){
						setPixel(img, *scr->curSwatch, x, y);
						color = *scr->curSwatch;
					}
					c = '@';
				}

				printf("\x1b[48;2;%d;%d;%dm", color.red, color.green, color.blue); // Background
				printf("%c", c);
			}
		}
	}
}

// Processes user input. Returns 0 when user requests quit
int getInput(Screen* scr, TGAImg* img){
	char in = getchar();
	// Swatch switching
	if ((in - '0') >= 1 && (in - '0') <= 9){
		scr->curSwatch = &scr->swatches[(in - '0') - 1];
	}
	else{
		switch (in){
			// Change increment
			case 'f':
				if (scr->inc == 1){
					scr->inc = 5;
				}
				else{
					scr->inc = 1;
				}
				break;
			// Swatch color change
			case 'r':
				scr->curSwatch->red -= scr->inc;
				break;
			case 'R':
				scr->curSwatch->red += scr->inc;
				break;
			case 'g':
				scr->curSwatch->green -= scr->inc;
				break;
			case 'G':
				scr->curSwatch->green += scr->inc;
				break;
			case 'b':
				scr->curSwatch->blue -= scr->inc;
				break;
			case 'B':
				scr->curSwatch->blue += scr->inc;
				break;
			// Viewport movement
			case 'k':
				if (scr->viewY < img->header.height - 1){
					scr->viewY += 1;
				}
				break;
			case 'j':
				if (scr->viewY > 0){
					scr->viewY -= 1;
				}
				break;
			case 'l':
				if (scr->viewX < img->header.width - 1){
					scr->viewX += 1;
				}
				break;
			case 'h':
				if (scr->viewX > 0){
					scr->viewX -= 1;
				}
				break;
			// Set pixel
			case 'd':
				if (scr->setting == 1){
					scr->setting = 0;
				}
				else{
					scr->setting = 1;
				}
				break;
			case 's':
				saveImage("output.tga", img);
				break;
			// Quiting
			case 'q':
				return(0);
			default:
				break;
		}
	}
	return(1);
}

void endScreen(Screen* scr){
	tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
	// Re-enable cursor
	printf("\033[?25h");
	free(scr);
	scr = NULL;
}

Screen* initScreen(){
	Screen* scr = calloc(1, sizeof(Screen));

	// Instant user input with getchar
	struct termios termPaint;
	tcgetattr(STDIN_FILENO, &scr->termDefault);
	termPaint = scr->termDefault;
	termPaint.c_lflag &=(~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);

	// Hide cursor
	printf("\033[?25l");

	// Set increment
	scr->inc = 1;

	// Initialize swatches
	scr->curSwatch = scr->swatches;
	for (int i = 0; i < SWATCHES_NUM; i++){
		scr->swatches[i].red = 255;
		scr->swatches[i].green = 255;
		scr->swatches[i].blue = 255;
	}
	return scr;
}


int main(void){
	Screen* scr = initScreen();

	TGAImg* img = mainMenu(scr);
	if (img == NULL){
		endScreen(scr);
		exit(EXIT_FAILURE);
	}
	while (img != NULL){
		refreshScreen(scr, img);
		while (getInput(scr, img)){
			refreshScreen(scr, img);
		}
		scr->setting = 0;
		free(img);
		img = mainMenu(scr);
	}

	endScreen(scr);
	return EXIT_SUCCESS;
}
