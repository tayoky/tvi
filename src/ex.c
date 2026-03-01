#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <tvi.h>

typedef struct ex_command {
	const char *name;
	int (*func)(tvi_t *tvi, int argc, char **argv);
	int min_args;
	int max_args;
} ex_command_t;

#define COMMAND(_name, _func, _min, _max) {.name = _name, .func = _func, .min_args = _min, .max_args = _max}

static int quit(tvi_t *tvi, int argc, char **argv) {
	(void)argc;
	(void)argv;
	tvi->flags |= FLAG_QUIT;
	return 0;
}

static ex_command_t commands[] = {
	COMMAND("quit", quit, 0, 0),
	COMMAND(NULL, NULL, 0, 0),
};

static int is_blank(int c) {
	return isblank(c) || c == '\n';
}

// ex shell
int ex_command(tvi_t *tvi, const char *command) {
	if (*command == ':') command++;
	if (!command) return 0;

	const char *name = command;
	size_t name_len = 0;
	while (!is_blank(name[name_len]) && name[name_len]) name_len++;
	if (!name_len) return 0;
	for (size_t i=0; commands[i].name; i++) {
		if (strncmp(name, commands[i].name, name_len)) continue;
		// it's a match
		return commands[i].func(tvi, 0, NULL);
	}
	term_goto(0, term_height-1);
	printf("%.*s : invalid command", (int)name_len, name);
	fflush(stdout);
	return -1;
}
