#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <tvi.h>

typedef struct ex_args {
	int addr1;
	int addr2;
	int flags;
	int addrs_count;
	int reg;
} ex_args_t;

typedef struct ex_command {
	const char *name;
	int (*func)(tvi_t *tvi, ex_args_t *args);
	int flags;
	int max_addrs;
} ex_command_t;

#define COMMAND(_name, _func, _flags, _max_addrs) {.name = _name, .func = _func, .flags = _flags, .max_addrs = _max_addrs}

#define FLAG_BANG 0x01
#define FLAG_REG  0x02

static char **ex_input(tvi_t *tvi, size_t *_lines_count) {
	size_t lines_count = 0;
	char **lines = NULL;
	for (;;) {
		if (tvi->mode == MODE_VISUAL) {
			if (prompt(tvi, "", 1) < 0) break;
		} else {
			if (!fgets(tvi->prompt, sizeof(tvi->prompt), stdin)) break;
		}
		// stop input on line with only "."
		if (!strcmp(".\n", tvi->prompt)) break;

		// strip the newline
		if (tvi->prompt[strlen(tvi->prompt)-1] == '\n') {
			tvi->prompt[strlen(tvi->prompt)-1] = '\0';
		}

		lines_count++;
		lines = realloc(lines, sizeof(char*) * lines_count);
		lines[lines_count-1] = strdup(tvi->prompt);
	}
	if (!lines_count) return NULL;
	render_all_windows(tvi);
	*_lines_count = lines_count;
	return lines;
}

static void free_input(char **lines, size_t lines_count) {
	for (size_t i=0; i<lines_count; i++) {
		free(lines[i]);
	}
	free(lines);
}

static int check_dirty(tvi_t *tvi, ex_args_t *args) {
	if (args->flags & FLAG_BANG) return 0;
	win_t *win = tvi->focus_window;
	if (win->flags & FLAG_DIRTY) {
		error(tvi, "no write since last change(add ! to override)");
		return -1;
	}
	return 0;
}

static int ex_append(tvi_t *tvi, ex_args_t *args) {
	size_t lines_count;
	char **lines = ex_input(tvi, &lines_count);
	if (!lines) return 0;
	text_insert_lines(tvi->focus_window, args->addr1+1, lines, lines_count);
	tvi->focus_window->cursor_y = args->addr1 + lines_count;
	render_window(tvi, tvi->focus_window);
	render_flush(tvi);
	free_input(lines, lines_count);
	return 0;
}

static int ex_delete(tvi_t *tvi, ex_args_t *args) {
	text_delete_lines_reg(tvi, tvi->focus_window, args->addr1, args->addr2 - args->addr1 + 1, args->reg);
	render_window(tvi, tvi->focus_window);
	render_flush(tvi);
	return 0;
}

static int ex_insert(tvi_t *tvi, ex_args_t *args) {
	size_t lines_count;
	char **lines = ex_input(tvi, &lines_count);
	if (!lines) return 0;
	text_insert_lines(tvi->focus_window, args->addr1, lines, lines_count);
	tvi->focus_window->cursor_y = args->addr1 + lines_count - 1;
	render_window(tvi, tvi->focus_window);
	render_flush(tvi);
	free_input(lines, lines_count);
	return 0;
}

static int ex_help(tvi_t *tvi, ex_args_t *args) {
	(void)args;
	static char *help_path = PREFIX"/share/tvi/help.txt";
	win_t *old_win = tvi->focus_window;
	win_t *win = win_create(tvi);
	win->width = old_win->width;
	win->height = old_win->height - 2;
	win->x = old_win->x;
	win->y = old_win->y;
	open_files(win, &help_path, 1);
	render_window(tvi, win);
	render_flush(tvi);
	return 0;
}

static int ex_print(tvi_t *tvi, ex_args_t *args) {
	win_t *win = tvi->focus_window;
	for (int line=args->addr1; line<=args->addr2; line++) {
		const char *content = win->text[line];
		if (line == args->addr2) {
			print(tvi, "%s", content);
		} else {
			print(tvi, "%s\n", content);
		}
	}
	return 0;
}

static int ex_put(tvi_t *tvi, ex_args_t *args) {
	if (reg_put_lines(tvi, tvi->focus_window, args->reg, args->addr1 + 1) < 0) return -1;
	render_window(tvi, tvi->focus_window);
	render_flush(tvi);
	return 0;
}

static int ex_quit(tvi_t *tvi, ex_args_t *args) {
	if (check_dirty(tvi, args) < 0) return -1;
	tvi->flags |= FLAG_QUIT;
	return 0;
}

static int ex_next(tvi_t *tvi, ex_args_t *args) {
	if (check_dirty(tvi, args) < 0) return -1;
	win_t *win = tvi->focus_window;
	if (win->files_count == 1) {
		error(tvi, "there is only one file to edit");
		return -1;
	} else if (win->file_index + 1 >= win->files_count) {
		error(tvi, "cannot go beyond last file");
		return -1;
	}

	win->file_index++;
	read_file(win, win->files[win->file_index]);
	render_window(tvi, win);
	render_flush(tvi);
	return 0;
}

static int ex_join(tvi_t *tvi, ex_args_t *args) {
	win_t *win = tvi->focus_window;
	// TODO : modify lines
	text_join(win, args->addr1, args->addr2, ' ');
	render_window(tvi, win);
	render_flush(tvi);
	return 0;
}

static int ex_write(tvi_t *tvi, ex_args_t *args) {
	if (args->addrs_count == 0) {
		// by default write whole file
		args->addr1 = 0;
		args->addr2 = tvi->focus_window->lines_count - 1;
	}
	if ((args->addr1 != 0 || args->addr2 != tvi->focus_window->lines_count - 1) && !(args->flags & FLAG_BANG)) {
		error(tvi, "use '!' to write partial buffer");
		return -1;
	}
	return write_file(tvi, tvi->focus_window, NULL, args->addr1, args->addr2);
}

static int ex_wq(tvi_t *tvi, ex_args_t *args) {
	if (ex_write(tvi, args) < 0) return -1;
	ex_args_t empty = {0};
	return ex_quit(tvi, &empty);
}

static int ex_xit(tvi_t *tvi, ex_args_t *args) {
	if (tvi->focus_window->flags & FLAG_DIRTY) {
		return ex_wq(tvi, args);
	} else {
		return ex_quit(tvi, args);
	}
}

static int ex_yank(tvi_t *tvi, ex_args_t *args) {
	text_yank_lines(tvi, tvi->focus_window, args->addr1, args->addr2 - args->addr1 + 1, args->reg);
	return 0;
}

static ex_command_t commands[] = {
	COMMAND("append", ex_append, FLAG_BANG, 1),
	COMMAND("delete", ex_delete, FLAG_REG, 2),
	COMMAND("help", ex_help, 0, 0),
	COMMAND("insert", ex_insert, FLAG_BANG, 1),
	COMMAND("join", ex_join, FLAG_BANG, 2),
	COMMAND("next", ex_next, FLAG_BANG, 0),
	COMMAND("print", ex_print, 0, 2),
	COMMAND("put", ex_put, FLAG_REG, 1),
	COMMAND("quit", ex_quit, FLAG_BANG, 0),
	COMMAND("write", ex_write, FLAG_BANG, 2),
	COMMAND("wq", ex_wq, FLAG_BANG, 2),
	COMMAND("xit", ex_xit, FLAG_BANG, 2),
	COMMAND("yank", ex_yank, FLAG_REG, 2),
	COMMAND(NULL, NULL, 0, 0),
};

static int is_blank(int c) {
	return isblank(c) || c == '\n';
}

static int parse_addr(tvi_t *tvi, const char *src, const char **end) {
	win_t *win = tvi->focus_window;
	int addr = 0;
	switch (*src) {
	case '.':
		addr = win->cursor_y;
		src++;
		break;
	case '$':
		addr = win->lines_count - 1;
		src++;
		break;
	case '+':
	case '-':
		// TODO
		break;
	default:
		if (!isdigit(*src)) {
			*end = NULL;
			return 0;
		}
		addr = strtol(src, (char**)&src, 10) - 1;
		break;
	}
	*end = src;
	// TODO : offsets
	return addr;
}

// ex shell
int ex_command(tvi_t *tvi, const char *command) {
	win_t *win = tvi->focus_window;

	// skip leading colons
	while (*command == ':') command++;

	ex_args_t args = {
		.addr1 = win->cursor_y,
		.addr2 = win->cursor_y,
		.reg = '"',
	};
	int addrs_count = 0;

	// parse addresses
	const char *end;
	for (;;) {
		// skip leading blank
		while (is_blank(*command)) command++;

		int addr = parse_addr(tvi, command, &end);
		if (!end && addrs_count == 0 && *command != ';' && *command != ',') break;
		if (end) {
			command = end;
		} else {
			// current line by default
			addr = win->cursor_y;
		}
		addrs_count++;
		switch (addrs_count) {
		case 1:
			args.addr1 = addr;
			break;
		case 2:
			args.addr2 = addr;
			break;
		default:
			// discard old addr
			args.addr1 = args.addr2;
			args.addr2 = addr;
			break;
		}

		// skip leading blank
		while (is_blank(*command)) command++;
		// see if we have a separator
		if (*command == ',' || *command == ';') {
			command++;
			continue;
		}
		break;
	}
	args.addrs_count = addrs_count;

	// skip leading blank
	while (is_blank(*command)) command++;
	
	if (!command) return 0;

	// TODO : range check

	const char *name = command;
	size_t name_len = 0;
	while (isalpha(name[name_len]) && name[name_len]) {
		name_len++;
		command++;
	}
	if (*command == '!') {
		command++;
		args.flags |= FLAG_BANG;
	}
	
	if (!name_len) {
		if (!addrs_count || tvi->mode != MODE_VISUAL) {
			return 0;
		}
		int y = 0;
		if (addrs_count > 1) {
			y = args.addr2;
		} else {
			y = args.addr1;
		}
		if (y < 0) {
			error(tvi, "invalid range");
			return -1;
		} else if (y >= win->lines_count) {
			error(tvi, "invalid range");
			return -1;
		}
		win->cursor_y = y;
		return 0;
	}
	for (size_t i=0; commands[i].name; i++) {
		if (strncmp(name, commands[i].name, name_len)) continue;
		// it's a match
		if (commands[i].max_addrs == 0 && addrs_count > 0) {
			error(tvi, "no range allowed");
			return -1;
		}
		if ((args.flags & FLAG_BANG) && !(commands[i].flags & FLAG_BANG)) {
			error(tvi, "no '!' allowed");
			return -1;
		}
		// parse reg
		if (commands[i].flags & FLAG_REG) {
			// skip leading blank
			while (is_blank(*command)) command++;
			if (*command) {
				args.reg = *command;
			}
		}
		return commands[i].func(tvi, &args);
	}
	error(tvi, "not an editor command : '%.*s'", (int)name_len, name);
	return -1;
}

int ex_main(tvi_t *tvi) {
	print(tvi, "entering ex mode, type visual to get to visual mode");
	while (!(tvi->flags & FLAG_QUIT)) {
		char command[256];
		printf(":");
		fflush(stdout);
		while (!fgets(command, sizeof(command), stdin));
		ex_command(tvi, command);
	}
	return 0;
}
