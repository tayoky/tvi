#include <stdlib.h>
#include <string.h>
#include <tvi.h>

// tools to manipulate text

void text_insert_lines(win_t *win, int addr, char *const*lines, size_t lines_count) {
	win->text = realloc(win->text, sizeof(char*) * (win->lines_count + lines_count));
	memmove(&win->text[addr+lines_count], &win->text[addr], (win->lines_count-addr)*sizeof(char*));
	win->lines_count += lines_count;
	for (size_t i=0; i<lines_count; i++) {
		win->text[addr+i] = strdup(lines[i]);
	}
}

void text_insert_newline(win_t *win, int x, int y) {
	char *line = win->text[y];
	char *new_line = &line[x];
	text_insert_lines(win, y+1, &new_line, 1);
	line[x] = '\0';
}

void text_insert_buf(win_t *win, int x, int y, const char *buf, size_t count) {
	char *line = win->text[y];
	line = realloc(line, strlen(line) + 1 + count);
	memmove(&line[x+count], &line[x], strlen(line) - x + 1);
	memcpy(&line[x], buf, count);
	win->text[y] = line;
}

void text_delete_buf(win_t *win, int x, int y, char *buf, size_t count) {
	char *line = win->text[y];
	if (buf) {
		memcpy(buf, &line[x], count);
	}
	memmove(&line[x], &line[x+count], strlen(line) - x - count + 1);
}

void text_delete(win_t *win, int x, int y, size_t count) {
	text_delete_buf(win, x, y, NULL, count);
}
