#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tvi.h>

void read_file(win_t *win, const char *path) {
	free_list(win->text, win->lines_count);
	FILE *file = fopen(path, "r");
	if (!file) {
empty:
		win->lines_count = 1;
		win->text = malloc(sizeof(char*));
		win->text[0] = strdup("");
		return;
	}
	char line[LINE_MAX];
	size_t lines_count = 0;
	win->text = NULL;
	while (fgets(line, sizeof(line), file)) {
		lines_count++;
		win->text = realloc(win->text, sizeof(char*)*lines_count);
		win->text[lines_count-1] = strdup(line);
	}
	if (lines_count == 0) goto empty;
	win->lines_count = lines_count;
}

void open_files(win_t *win, char *const*files, size_t files_count) {;
	free_list(win->files, win->files_count);
	win->files_count = files_count;
	win->files = malloc(sizeof(char*) * files_count);
	for (size_t i=0; i<files_count; i++) {
		win->files[i] = strdup(files[i]);
	}
	read_file(win, files[0]);
}
