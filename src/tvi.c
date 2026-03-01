#include <stdio.h>
#include <tvi.h>

static int prompt(tvi_t *tvi, const char *initial, int multiline) {
	strcpy(tvi->prompt, initial);
	tvi->prompt_len = strlen(initial);
	tvi->prompt_cursor = tvi->prompt_len;
	tvi->flags |= FLAG_PROMPT;
	render_prompt(tvi);
	render_flush(tvi);

	for (;;) {
		int c = getchar();
		if (c == '\033') {
			tvi->prompt_len = 0;
			tvi->flags &= ~FLAG_PROMPT;
			render_prompt(tvi);
			render_flush(tvi);
			return -1;
		}

		if (term_is_delete(c)) {
			if (tvi->prompt_cursor <= 0) {
				term_bell();
				continue;
			}
			tvi->prompt_cursor--;
			memmove(&tvi->prompt[tvi->prompt_cursor], &tvi->prompt[tvi->prompt_cursor+1], tvi->prompt_len - tvi->prompt_cursor);
			tvi->prompt_len--;
			render_prompt(tvi);
			render_flush(tvi);
			if (!multiline && tvi->prompt_len == 0) {
				tvi->flags &= ~FLAG_PROMPT;
				render_flush(tvi);
				return -1;
			}
			continue;
		}

		// do we have place ?
		if (tvi->prompt_len >= sizeof(tvi->prompt) - 1) {
			term_bell();
			continue;
		}
		
		// insert the char
		memmove(&tvi->prompt[tvi->prompt_cursor+1], &tvi->prompt[tvi->prompt_cursor], tvi->prompt_len - tvi->prompt_cursor);
		tvi->prompt[tvi->prompt_cursor++] = c;
		tvi->prompt_len++;
		if (c == '\n') break;
		render_prompt(tvi);
		render_flush(tvi);
	}
	tvi->prompt[tvi->prompt_len] = '\0';
	tvi->flags &= ~FLAG_PROMPT;
	render_flush(tvi);
	return 0;
}

// the main loop
int tvi_main(tvi_t *tvi) {
	win_t *win = win_create(tvi);
	term_fetch_size();
	win->width  = term_width;
	win->height = term_height-1;
	render_window(tvi, win);
	render_flush(tvi);
	while (!(tvi->flags & FLAG_QUIT)) {
		int c = getchar();
		switch (c) {
		case ':':
			if (prompt(tvi, ":", 0) < 0) break;
			if (tvi->prompt_len < 2) break;
			ex_command(tvi, tvi->prompt);
			break;
		}
	}
	return 0;
}
