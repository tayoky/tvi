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
