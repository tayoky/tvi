#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
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

int term_enable_raw_mode(void) {
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

	// disable buffering
	setvbuf(stdin, NULL, _IONBF, 0);;
	return 0;
}

void term_quit_raw_mode(void) {
	// restore old termios
	if(tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0){
		perror("tcsetattr");
	}
}

int term_have_input(void) {
	struct pollfd fd = {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	};
	if (poll(&fd, 1, 0) < 0) return 0;
	return fd.revents & POLLIN;
}

int term_get_key(void) {
	int c = getchar();
	if (c != '\033') return c;
	if (!term_have_input()) return c;
	int c2 = getchar();
	if (c2 != '[') {
		ungetc(c2, stdin);
		return c;
	}
	int c3 = getchar();
	switch (c3) {
	case 'A':
		return KEY_UP;
	case 'B':
		return KEY_DOWN;
	case 'C':
		return KEY_RIGHT;
	case 'D':
		return KEY_LEFT;
	case 'H':
		return KEY_START;
	case 'F':
		return KEY_END;
	default:
		return '\033';
	}
}

int term_enter_fullscreen(void) {
	if (!isatty(STDOUT_FILENO)) return -1;
	printf(ESC"[?1049h");
	printf(ESC"[2J");
	printf(ESC"[H");
	fflush(stdout);
	return 0;
}

void term_exit_fullscreen(void) {
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

void term_error_color(void) {
	printf(ESC"[0;41;37m");
}
