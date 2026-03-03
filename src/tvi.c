#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <tvi.h>

int prompt(tvi_t *tvi, const char *initial, int newline) {
	strcpy(tvi->prompt, initial);
	tvi->prompt_len = strlen(initial);
	tvi->prompt_cursor = tvi->prompt_len;
	tvi->flags |= FLAG_PROMPT;
	render_prompt(tvi);
	render_flush(tvi);

	for (;;) {
		int c = getchar();
		if (c == '\033') {
exit_prompt:
			tvi->prompt_len = 0;
			tvi->flags &= ~FLAG_PROMPT;
			render_prompt(tvi);
			render_flush(tvi);
			return -1;
		}

		if (term_is_delete(c)) {
			if (tvi->prompt_len <= strlen(initial)) {
				// exit prompt when it become empty
				goto exit_prompt;
			}
			if (tvi->prompt_cursor <= 0) {
				term_bell();
				continue;
			}
			tvi->prompt_cursor--;
			memmove(&tvi->prompt[tvi->prompt_cursor], &tvi->prompt[tvi->prompt_cursor+1], tvi->prompt_len - tvi->prompt_cursor);
			tvi->prompt_len--;
			render_prompt(tvi);
			render_flush(tvi);
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
		if (c == '\n' && !newline) break;
		render_prompt(tvi);
		if (c == '\n') break;
		render_flush(tvi);
	}
	tvi->prompt[tvi->prompt_len] = '\0';
	tvi->flags &= ~FLAG_PROMPT;
	render_flush(tvi);
	return 0;
}

// fix cursor position
static void fix_cursor(tvi_t *tvi) {
	win_t *win = tvi->focus_window;
	int x = win->cursor_x;
	int y = win->cursor_y;
	size_t line_len = strlen(win->text[y]);
	if ((size_t)x > line_len) {
		win->cursor_x = line_len;
	}
}

// set cursor to first non blank char on the line
static void cursor_to_non_blank(tvi_t *tvi) {
	win_t *win = tvi->focus_window;
	int x = 0;
	const char *line = win->text[win->cursor_y];
	while (isblank(line[x])) {
		x++;
	}
	win->cursor_x = x;
}

// return 1 if interpreted
static int move_command(tvi_t *tvi, int c, int count) {
	win_t *win = tvi->focus_window;

	int have_count = count ? 1 : 0;
	if (!have_count) count = 1;

	// we cannot know backward char at compile time
	if (term_is_delete(c)) {
		goto backward;
	}
	int line_len = strlen(win->text[win->cursor_y]);

	switch (c) {
	case '^':
		cursor_to_non_blank(tvi);
		return 1;
	case 'h':
	case CRTL('H'):
backward:
		fix_cursor(tvi);
		if (win->cursor_x <= 0) {
			term_bell();
		} else if (win->cursor_x < count) {
			win->cursor_x = 0;
		} else {
			win->cursor_x -= count;
		}
		return 1;
	case '\n':
	case CRTL('N'):
	case 'j':
	case '\r':
	case '+':
		if (win->lines_count - 1 <= win->cursor_y) {
			term_bell();
		} else if (win->lines_count - win->cursor_y - 1 < count) {
			win->cursor_y = win->lines_count-1;
		} else {
			win->cursor_y += count;
		}
		// TODO : change cursor x
		return 1;
	case CRTL('P'):
	case 'k':
	case '-':
		if (win->cursor_y <= 0) {
			term_bell();
		} else if (win->cursor_y < count) {
			win->cursor_y = 0;
		} else {
			win->cursor_y -= count;
		}
		// TODO : change cursor x
		return 1;
	case ' ':
	case 'l':
		if (line_len <= win->cursor_x) {
			term_bell();
		} else if (line_len - win->cursor_x < count) {
			win->cursor_x = line_len;
		} else {
			win->cursor_x += count;
		}
		return 1;
	case '$':
		if (count) {
			if (win->lines_count >= win->cursor_y) {
				term_bell();
			} else if (win->lines_count - win->cursor_y < count-1) {
				win->cursor_y = win->lines_count;
			} else {
				win->cursor_y += count-1;
			}
		}
		win->cursor_x = INT_MAX;
		return 1;
	case '0':
		win->cursor_x = 0;
		return 1;
	case 'G':
		if (have_count) {
			if (count >= win->lines_count) {
				term_bell();
				return 1;
			}
		} else {
			count = win->lines_count-1;
		}
		win->cursor_y = count;
		cursor_to_non_blank(tvi);
		return 1;
	default:
		return 0;
	}
}

// the main loop
int tvi_main(tvi_t *tvi) {
	if (term_enable_raw_mode() < 0) {
		return -1;
	}
	win_t *win = tvi->focus_window;
	term_fetch_size();
	win->width  = term_width;
	win->height = term_height-1;
	render_window(tvi, win);
	render_flush(tvi);
	while (!(tvi->flags & FLAG_QUIT)) {
		int c = getchar();
		int count = 0;

		if (isdigit(c) && c != '0') {
			// we have a count
			while (isdigit(c)) {
				count = c - '0';
				c *= 10;
				c = getchar();
			}
		}
		win_t *win = tvi->focus_window;
		if (move_command(tvi, c, count)) {
			render_status(tvi, win);
			render_flush(tvi);
			continue;
		}
		switch (c) {
		case ':':
			if (prompt(tvi, ":", 0) < 0) break;
			if (tvi->prompt_len < 2) break;
			ex_command(tvi, tvi->prompt);
			break;
		case CRTL('E'):
			if (win->scroll >= win->lines_count - 1) {
				term_bell();
				break;
			}
			win->scroll++;
			render_window(tvi, win);
			render_flush(tvi);
			break;
		case CRTL('Y'):
			if (win->scroll <= 0) {
				term_bell();
				break;
			}
			win->scroll--;
			render_window(tvi, win);
			render_flush(tvi);
			break;
		}
	}
	term_quit_raw_mode();
	return 0;
}
