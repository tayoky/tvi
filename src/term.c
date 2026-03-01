#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <tvi.h>

static struct termios old;
static struct winsize winsz;
int term_width;
int term_height;

void term_fetch_size(void) {
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
	term_width = winsz.ws_col;
	term_height = winsz.ws_row;
}

int term_enter_fullscreen(void) {
	printf(ESC"[?1049h");
	printf(ESC"[2J");
	printf(ESC"[H");
	fflush(stdout);

	// save old termios
	if(tcgetattr(STDIN_FILENO, &old) < 0){
		perror("tcgetattr");
		return -1;
	}
	struct termios new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	if(tcsetattr(STDIN_FILENO, TCSANOW, &new) < 0){
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

void term_exit_fullscreen(void) {
	// restore old termios
	if(tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0){
		perror("tcsetattr");
	}
	printf(ESC"[?1049l");
}

void term_clear_line(void) {
	printf(ESC"[2K");
}

void term_goto(int x, int y) {
	printf(ESC"[%d;%df", y+1, x+1);
}

void term_bell(void) {
	putchar('\a');
	fflush(stdout);
}

int term_is_delete(int c) {
	return c == old.c_cc[VERASE];
}

void term_reset_color(void) {
	printf(ESC"[0m");
}

void term_inverse_color(void) {
	printf(ESC"[0;7m");
}
