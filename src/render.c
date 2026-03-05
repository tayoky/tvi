#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <tvi.h>

void render_text(tvi_t *tvi, win_t *win) {
	for (int i=0; i<win->height-1; i++) {
		if (render_line(tvi, win, win->scroll + i) < 0) break;
	}
}

static size_t render_len(const char *str) {
	size_t len = 0;
	while (*str) {
		if (*str == '\t') {
			len += 8;
		} else {
			len++;
		}
		str++;
	}
	return len;
}

static size_t get_line_height(win_t *win, const char *line) {
	size_t line_len = render_len(line);
	size_t lines_count = (line_len + win->width - 1) / win->width;
	if (lines_count == 0) lines_count = 1;
	return lines_count;
}

static int get_line_y(win_t *win, int index) {
	int y = 0;
	for (int i=win->scroll; i<index ;i++) {
		if (i >= win->lines_count) {
			y++;
			continue;
		}
		char *line = win->text[i];
		y += get_line_height(win, line);
	}
	return y;
}

int render_line(tvi_t *tvi, win_t *win, size_t index) {
	(void)tvi;
	if (index < (size_t)win->scroll) return -1;
	int y = get_line_y(win, index);
	if (y >= win->height - 1) {
		return -1;
	}
	term_goto(win->x, win->y + y);
	term_reset_color();
	term_clear_line();
	if (index >= (size_t)win->lines_count) {
		printf("~");
		return 0;
	}
	char *line = win->text[index];
	size_t line_height = get_line_height(win, line);
	if (y + line_height > (size_t)win->height - 1) {
		printf("@@@");
	} else {
		// clear multiple lines if needed
		if (line_height > 1) {
			for (size_t i=1; i<line_height; i++) {
				putchar('\n');
				term_clear_line();
			}
			term_goto(win->x, win->y + y);
		}
		printf("%s", line);
	}
	return 0;
}

void render_status(tvi_t *tvi, win_t *win) {
	(void)tvi;
	term_goto(win->x, win->y + win->height - 1);
	term_inverse_color();
	term_clear_line();
	int y = win->cursor_y;
	int x = win->cursor_x;
	size_t line_len = strlen(win->text[y]);
	if ((size_t)x > line_len) x = line_len;

	const char *file = win->files[win->file_index];
	if (!file) file = "[NO NAME]";

	// see if we have enought place
	size_t max_len = win->width - 12;
	if (strlen(file) > max_len) {
		file += strlen(file) - max_len;
		printf("<%s %d,%d", file, y+1, x+1);
	} else {
		printf("%s %d,%d", file, y+1, x+1);
	}
	term_goto(win->x + win->width - 3, win->y + win->height - 1);
	printf("tvi");
}

void render_window(tvi_t *tvi, win_t *win) {
	if (tvi->mode != MODE_VISUAL) return;
	render_text(tvi, win);
	render_status(tvi, win);
}

void render_all_windows(tvi_t *tvi) {
	if (tvi->mode != MODE_VISUAL) return;
	for (win_t *win=tvi->first_window; win; win=win->next) {
		render_window(tvi, win);
	}
}

void render_cursor(tvi_t *tvi) {
	if (tvi->flags & FLAG_PROMPT) {
		term_goto(tvi->prompt_cursor, term_height-1);
		return;
	}
	win_t *win = tvi->focus_window;
	int x = win->cursor_x;
	int y = win->cursor_y;
	const char *line = win->text[y];
	size_t line_len = strlen(line);
	if ((size_t)x > line_len) x = line_len;
	int screen_x = 0;
	for (int i=0; i<x; i++) {
		if (line[i] == '\t') {
			screen_x += 8;
		} else {
			screen_x++;
		}
	}
	int screen_y = get_line_y(win, y);
	term_goto(win->x + screen_x, win->y + screen_y);
}

void render_prompt(tvi_t *tvi) {
	term_goto(0, term_height-1);
	term_reset_color();
	term_clear_line();
	if (tvi->flags & FLAG_PROMPT) {
		printf("%.*s", (int)tvi->prompt_len, tvi->prompt);
	}
}

void render_flush(tvi_t *tvi) {
	if (tvi->mode != MODE_VISUAL) return;
	render_cursor(tvi);
	fflush(stdout);
}

void error(tvi_t *tvi, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if (tvi->mode == MODE_VISUAL) term_goto(0, term_height-1);
	term_reset_color();
	term_clear_line();
	term_error_color();
	vprintf(fmt, args);
	va_end(args);
	term_reset_color();
	if (tvi->mode == MODE_EX) putchar('\n');
}

void print(tvi_t *tvi, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if (tvi->mode == MODE_VISUAL) term_goto(0, term_height-1);
	term_reset_color();
	term_clear_line();
	vprintf(fmt, args);
	va_end(args);
	term_reset_color();
	if (tvi->mode == MODE_EX && fmt[strlen(fmt)-1] != '\n') putchar('\n');
}
