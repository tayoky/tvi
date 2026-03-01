#include <stdio.h>
#include <tvi.h>

void render_text(tvi_t *tvi, win_t *win) {
	for (int i=0; i<win->height; i++) {
		render_line(tvi, win, i);
	}
}

void render_line(tvi_t *tvi, win_t *win, int y) {
	(void)tvi;
	term_goto(win->x, win->y + y);
	term_reset_color();
	term_clear_line();
	if (y + win->scroll >= (int)win->lines_count) {
		printf("~");
	} else {
		printf("%s", win->text[y + win->scroll]);
	}
}

void render_status(tvi_t *tvi, win_t *win) {
	(void)tvi;
	term_goto(win->x, win->y + win->height - 1);
	term_inverse_color();
	term_clear_line();
	printf("[NO NAME]");
}

void render_window(tvi_t *tvi, win_t *win) {
	render_text(tvi, win);
	render_status(tvi, win);
}

void render_cursor(tvi_t *tvi) {
	if (tvi->flags & FLAG_PROMPT) {
		term_goto(tvi->prompt_cursor, term_height-1);
		return;
	}
	win_t *win = tvi->focus_window;
	int x = win->cursor_x;
	int y = win->cursor_y;
	size_t line_len = strlen(win->text[y]);
	if ((size_t)x > line_len) x = line_len;
	term_goto(win->x + x, win->y + y);
}

void render_prompt(tvi_t *tvi) {
	term_goto(0, term_height-1);
	term_reset_color();
	term_clear_line();
	printf("%.*s", (int)tvi->prompt_len, tvi->prompt);
}

void render_flush(tvi_t *tvi) {
	render_cursor(tvi);
	fflush(stdout);
}
