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
	win->text[0] = strdup("");
	win->lines_count = 1;
	win->files = malloc(sizeof(char*));
	win->files[0] = NULL;
	win->files_count = 1;
	return win;
}

void free_list(char **list, size_t count) {
	for (size_t i=0; i<count; i++) {
		free(list[i]);
	}
	free(list);
}

void win_free(tvi_t *tvi, win_t *win) {
	(void)tvi;
	free_list(win->text, win->lines_count);
	free_list(win->files, win->files_count);
	syntax_unload(win->syntax);
}
