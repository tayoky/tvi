#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tvi.h>

win_t *win_create(tvi_t *tvi) {
	win_t *win = malloc(sizeof(win_t));
	memset(win, 0, sizeof(win_t));
	win->next = tvi->first_window;
	if (tvi->first_window) {
		tvi->first_window->prev = win;
	}
	tvi->first_window = win;
	tvi->focus_window = win;
	win->text = malloc(sizeof(char*));
	win->text[0] = strdup("hello world");
	win->lines_count = 1;
	return win;
}

void win_free(tvi_t *tvi, win_t *win) {
}
