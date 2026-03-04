#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tvi.h>

static void dup_lines(char **dest, char *const*src, size_t count) {
	for (size_t i=0; i<count; i++) {
		dest[i] = strdup(src[i]);
	}
}

static void reg_set(reg_t *reg, char *const*lines, size_t count, int type) {
	free_list(reg->text, reg->lines_count);
	if (!count) {
		reg->text = NULL;
		reg->lines_count = 0;
		return;
	}
	if (type == REG_CHAR) {
		reg->text = malloc(sizeof(char*));
		reg->text[0] = strndup(*lines, count);
		reg->lines_count = 1;
	} else {
		reg->text = malloc(count * sizeof(char*));
		dup_lines(reg->text, lines, count);
		reg->lines_count = count;
	}
	reg->type = type;
}

static void reg_append(reg_t *reg, char *const*lines, size_t count, int type) {
	size_t index = reg->lines_count;
	if (type == REG_CHAR) {
		reg->lines_count++;
	} else {
		reg->lines_count += count;
	}
	reg->text = realloc(reg->text, reg->lines_count*sizeof(char*));
	if (type == REG_CHAR) {
		reg->text[index] = strndup(*lines, count);
	} else {
		dup_lines(&reg->text[index], lines, count);
	}

	// line mode over char mode
	if (reg->type == REG_CHAR) reg->type = type;
}

void reg_write(tvi_t *tvi, int name, char *const*lines, size_t count, int type) {
	if (isalpha(name)) {
		reg_t *reg = &tvi->alpha_regs[tolower(name)-'a'];
		if (islower(name)) {
			reg_set(reg, lines, count, type);
			return;
		} else {
			reg_append(reg, lines, count, type);
			return;
		}
	}
	if (name == '0') {
		reg_set(&tvi->digit_regs[0], lines, count, type);
		return;
	}
	if (name == '"') {
		reg_set(&tvi->unamed_reg, lines, count, type);
		return;
	}
}

void reg_put(tvi_t *tvi, win_t *win, int name, int x, int y) {
	if (name == '_') return;
	reg_t *reg = NULL;
	if (isalpha(name)) {
		reg = &tvi->alpha_regs[tolower(name)-'a'];
	} else if (isdigit(name)) {
		reg = &tvi->digit_regs[name-'0'];
	} else if (name == '"') {
		reg = &tvi->unamed_reg;
	}
	if (!reg) return;
	if (!reg->text) {
		error(tvi, "nothing in register %c", name);
		return;
	}

	switch (reg->type) {
	case REG_CHAR:
		text_insert_buf(win, x, y, reg->text[0], strlen(reg->text[0]));
		break;
	case REG_LINE:
		text_insert_lines(win, y, reg->text, reg->lines_count);
		break;
	}
}
