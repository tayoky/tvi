#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <tvi.h>

typedef struct ex_args {
	int addr1;
	int addr2;
} ex_args_t;

typedef struct ex_command {
	const char *name;
	int (*func)(tvi_t *tvi, ex_args_t *args);
	int min_args;
	int max_args;
} ex_command_t;

#define COMMAND(_name, _func, _min, _max) {.name = _name, .func = _func, .min_args = _min, .max_args = _max}

static int ex_quit(tvi_t *tvi, ex_args_t *args) {
	(void)args;
	tvi->flags |= FLAG_QUIT;
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

static ex_command_t commands[] = {
	COMMAND("quit", ex_quit, 0, 0),
	COMMAND("print", ex_print, 0, 0),
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
		break;
	case '$':
		addr = win->lines_count - 1;
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

	// skip leading blank
	while (is_blank(*command)) command++;
	
	if (!command) return 0;	

	const char *name = command;
	size_t name_len = 0;
	while (!is_blank(name[name_len]) && name[name_len]) name_len++;
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
		return commands[i].func(tvi, &args);
	}
	error(tvi, "not an editor command : '%.*s'", (int)name_len, name);
	return -1;
}
