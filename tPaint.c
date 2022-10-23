/* Aidan Trent
 * Paint inside your terminal with true (24 bit) color!
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "ppm.h"
#include "tPaint.h"
#include "mainMenu.h"

#define BG_SHADE 20

#define LUM_R 0.2126
#define LUM_G 0.7152
#define LUM_B 0.0722
#define LUM_THRESH 100

// Checks if RGB color is over the luminence threshold. Changes text to black if too bright
void lumCheck(RGB* color){
	if (LUM_R * color->red + LUM_G * color->green + LUM_B * color->blue > LUM_THRESH){
		printf("\033[38;2;0;0;0m"); // Text to black
	}
	else{
		printf("\033[38;2;255;255;255m"); // Text to white
	}
}

void refreshScreen(Screen* scr, PPMImg* img){
	// Handle term dimensions
	struct winsize win;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

	RGB color;
	char c;
	int statusPrntd = 0;
	int charsPrntd = 0;
	int32_t y = scr->viewY - win.ws_row / 2;
	printf("\033[H\033[2J"); // Clear terminal
	for (int row = 0; row < win.ws_row; y++, row++){
		int32_t x = scr->viewX - win.ws_col / 2;
		for (int col = 0; col < win.ws_col; x++, col++){
			// Print status bar
			if (row == win.ws_row - 1 && !statusPrntd){
				// Display swatches
				for (int i = 0; i < SWATCHES_NUM; i++){
					printf("\033[48;2;%d;%d;%dm", scr->swatches[i].red, scr->swatches[i].green, scr->swatches[i].blue); // Background to color
					if (&scr->swatches[i] == scr->curSwatch){
						lumCheck(scr->curSwatch);
						charsPrntd = printf("%d", i + 1);
						col += charsPrntd;
						x += charsPrntd;
					}
					else{
						charsPrntd = printf(" ");
						col += charsPrntd;
						x += charsPrntd;
					}
				}
				// Display RGB values of current swatch
				printf("\033[38;2;255;255;255m"); // Text to white
				printf("\033[48;2;%d;%d;%dm", BG_SHADE, BG_SHADE, BG_SHADE); // Background to black
				charsPrntd = printf(" R:%d G:%d B:%d ", scr->curSwatch->red, scr->curSwatch->green, scr->curSwatch->blue);
				col += charsPrntd;
				x += charsPrntd;
				// Display brush down/up status
				if (scr->setting == 1){
					charsPrntd = printf("D ");
					col += charsPrntd;
					x += charsPrntd;
				}
				// Display step value
				charsPrntd = printf("+%d ", scr->step);
				col += charsPrntd;
				x += charsPrntd;
				// Display saving (when saving)
				if (scr->saving){
					// Temp disable instant input and enable cursor
					struct termios termPaint;
					tcgetattr(STDIN_FILENO, &termPaint);
					tcsetattr(STDIN_FILENO, TCSANOW, &scr->termDefault);
					printf("\033[?25h");

					// Get file name
					printf(" Save as : ");
					char fileName[BUF_SIZE];
					scanf(" %s", fileName);

					saveImage(fileName, img);

					// Re-enable instant user input and disable cursor
					tcsetattr(STDIN_FILENO, TCSANOW, &termPaint);
					printf("\033[?25l");
					scr->saving = 0;
				}
				statusPrntd = 1;
				col--;
				x--;
			}
			// Print canvas
			else{
				c = ' ';
				// On canvas pixel
				if (x >= 0 && x < img->header.width && y >= 0 && y < img->header.height){
					color = getPixel(img, x, y);
				}
				else{
					color.red = BG_SHADE;
					color.blue = BG_SHADE;
					color.green = BG_SHADE;
				}
				// Center character
				if (col == win.ws_col / 2 && row == win.ws_row / 2){
					if (scr->picking){
						*scr->curSwatch = getPixel(img, x, y);
						scr->picking = 0;
					}
					lumCheck(&color);
					c = '@';
				}
				printf("\033[48;2;%d;%d;%dm", color.red, color.green, color.blue); // Background to color
				printf("%c", c);
			}
		}
	}
}

// Processes user input. Returns 0 when user requests quit
int getInput(Screen* scr, PPMImg* img){
	char in = getchar();
	// Swatch switching
	if ((in - '0') >= 1 && (in - '0') <= 9){
		scr->curSwatch = &scr->swatches[(in - '0') - 1];
	}
	else{
		uint16_t lastPos;

		switch (in){
			// Change increment
			case '+':
				scr->step += 1;
				break;
			case '-':
				scr->step -= 1;
				break;
			// Swatch color change
			case 'r':
				scr->curSwatch->red -= scr->step;
				break;
			case 'R':
				scr->curSwatch->red += scr->step;
				break;
			case 'g':
				scr->curSwatch->green -= scr->step;
				break;
			case 'G':
				scr->curSwatch->green += scr->step;
				break;
			case 'b':
				scr->curSwatch->blue -= scr->step;
				break;
			case 'B':
				scr->curSwatch->blue += scr->step;
				break;
			// Viewport movement
			case 'j':
				lastPos = scr->viewY;
				if (scr->viewY + scr->step < img->header.height){
					scr->viewY += scr->step;
				}
				else {
					scr->viewY = img->header.height - 1;
				}
				if (scr->setting){
					for (int i = lastPos; i <= scr->viewY; i++){
						setPixel(img, *scr->curSwatch, scr->viewX, i);
					}
				}
				break;
			case 'k':
				lastPos = scr->viewY;
				if (scr->viewY - scr->step > 0){
					scr->viewY -= scr->step;
				}
				else {
					scr->viewY = 0;
				}
				if (scr->setting){
					for (int i = lastPos; i >= scr->viewY; i--){
						setPixel(img, *scr->curSwatch, scr->viewX, i);
					}
				}
				break;
			case 'l':
				lastPos = scr->viewX;
				if (scr->viewX + scr->step < img->header.width){
					scr->viewX += scr->step;
				}
				else {
					scr->viewX = img->header.width - 1;
				}
				if (scr->setting){
					for (int i = lastPos; i <= scr->viewX; i++){
						setPixel(img, *scr->curSwatch, i, scr->viewY);
					}
				}
				break;
			case 'h':
				lastPos = scr->viewX;
				if (scr->viewX - scr->step > 0){
					scr->viewX -= scr->step;
				}
				else {
					scr->viewX = 0;
				}
				if (scr->setting){
					for (int i = lastPos; i >= scr->viewX; i--){
						setPixel(img, *scr->curSwatch, i, scr->viewY);
					}
				}
				break;
			// Painting tools
			case 'd':
				if (scr->setting == 1){
					scr->setting = 0;
				}
				else{
					scr->setting = 1;
					setPixel(img, *scr->curSwatch, scr->viewX, scr->viewY);
				}
				break;
			case 'p':
				if (scr->picking == 1){
					scr->picking = 0;
				}
				else{
					scr->picking = 1;
				}
				break;
			case 's':
				scr->saving = 1;
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

	// Set step
	scr->step = 1;

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

	PPMImg* img = mainMenu(scr);
	if (img == NULL){
		endScreen(scr);
		exit(EXIT_FAILURE);
	}
	while (img != NULL){
		refreshScreen(scr, img);
		while (getInput(scr, img)){
			refreshScreen(scr, img);
		}
		// Reset screen values
		scr->setting = 0;
		scr->viewX = 0;
		scr->viewY = 0;
		// Enter menu
		free(img);
		img = mainMenu(scr);
	}

	endScreen(scr);
	return EXIT_SUCCESS;
}
