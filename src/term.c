#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#include <unistd.h>
#include <stdio.h>
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#include <tvi.h>

#ifdef HAVE_TERMIOS_H
static struct termios old;
static struct winsize winsz;
#endif
int term_width;
int term_height;

void term_fetch_size(void) {
#if defined(HAVE_SYS_IOCTL_H) && defined(TIOCGWINSZ)
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
	term_width = winsz.ws_col;
	term_height = winsz.ws_row;
#else
	// just blindly guess terminal size
	term_width = 80;
	term_height = 25;
#endif
}

int term_enable_raw_mode(void) {
#ifdef HAVE_TERMIOS_H // without termios it will be probably broken but we can try
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
#endif

	// disable buffering
	setvbuf(stdin, NULL, _IONBF, 0);;
	return 0;
}

void term_quit_raw_mode(void) {
#ifdef HAVE_TERMIOS_H
	// restore old termios
	if(tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0){
		perror("tcsetattr");
	}
#endif
}

int term_have_input(void) {
#ifdef HAVE_POLL_H
	struct pollfd fd = {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	};
	if (poll(&fd, 1, 0) < 0) return 0;
	return fd.revents & POLLIN;
#else
	return 0;
#endif
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
#ifdef HAVE_TERMIOS_H
	return c == old.c_cc[VERASE];
#else
	return c == '\b' || c == 0x7f;
#endif
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
