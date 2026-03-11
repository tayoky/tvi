#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <tvi.h>

const char *types[] = {
	"char",
	"short",
	"int",
	"long",
	"signed",
	"unsigned",
	"float",
	"double",
	"static",
	"inline",
	"volatile",
	"extern",
	"void",
	"struct",
	"union",
	"enum",
	"const",
	"typedef",

// standard types
	"size_t",
	"ssize_t",
	"int8_t",
	"uint8_t",
	"int16_t",
	"uint16_t",
	"int32_t",
	"uint32_t",
	"int64_t",
	"uint64_t",
	"FILE",
	"DIR",
	"va_list",
};

const char *keywords[] = {
	"if",
	"else",
	"for",
	"while",
	"do",
	"return",
	"switch",
	"case",
	"default",
	"sizeof",
	"break",
	"continue",
};

const char *preprocs[] = {
	"#define",
	"#undef",
	"#ifdef",
	"#ifndef",
	"#if",
	"#else",
	"#elif",
	"#endif",
	"#include",
};

const char *consts[] = {
	"NULL",
	"EOF",
	"SEEK_SET",
	"SEEK_CUR",
	"SEEK_END",
	"stdout",
	"stdin",
	"stderr",
	"INT_MAX",
	"INT_MIN",
};

#define WORD_TYPES    0
#define WORD_KEYWORDS 1
#define WORD_PREPROCS 2
#define WORD_CONSTS   3
#define arraylen(a) sizeof(a)/sizeof(*a)

const char **words[] = {
	[WORD_TYPES] = types,
	[WORD_KEYWORDS] = keywords,
	[WORD_PREPROCS]  = preprocs,
	[WORD_CONSTS] = consts,
};

size_t words_len[] = {
	[WORD_TYPES] = arraylen(types),
	[WORD_KEYWORDS] = arraylen(keywords),
	[WORD_PREPROCS]  = arraylen(preprocs),
	[WORD_CONSTS] = arraylen(consts),
};

static int alpha_sort(const void *e1, const void *e2) {
	const char *const*str1 = e1;
	const char *const*str2 = e2;
	return strcmp(*str1, *str2);
}

int init() {
	for (size_t i=0; i<arraylen(words); i++) {
		qsort(words[i], words_len[i], sizeof(const char *), alpha_sort);
	}
	return 0;
}

int is_word_type(const char *word, size_t len, int type) {
	const char **list = words[type];
	size_t start = 0;
	size_t end  = words_len[type]-1;
	while (end >= start) {
		size_t middle = (start + end) / 2;
		int cmp = strncmp(word, list[middle], len);
		if (cmp == 0) {
			if (strlen(list[middle]) == len) return 1;
			else return 0;
		} if (cmp > 0) {
			start = middle + 1;
		} else {
			if (middle == 0) return 0;
			end = middle - 1;
		}
	}
	return 0;
}

const char *word_color(const char *word, size_t size) {
	if (is_word_type(word, size, WORD_TYPES)) return "\033[32m";
	if (is_word_type(word, size, WORD_KEYWORDS)) return "\033[33m";
	if (is_word_type(word, size, WORD_CONSTS) || isdigit(*word)) return "\033[35m";
	return NULL;
}

static int is_word_char(int c) {
	return isalnum(c) || c == '_';
}

static int reach_line_end(const char *line) {
	return !*line || (*line == '/' && line[1] == '/');
}

static const char *skip_backslash(const char *line) {
	if (*line != '\\') return line;
	if (line[1]) {
		return line + 2;
	}
	return line + 1;
}

void print_line(const char *line) {
	// print word by word
	int last_is_reset = 1;
	while (isblank(*line)) {
		putchar(*line);
		line++;
	}
	if (*line == '#') {
		// find lenght of word
		const char *word = line;
		size_t word_len = 1;
		while (is_word_char(word[word_len])) {
			word_len++;
		}
		if (is_word_type(word, word_len, WORD_PREPROCS)) {
			printf("\033[34m%.*s", (int)word_len, word);
			line += word_len;
			last_is_reset = 0;
		}
	}
	while (!reach_line_end(line)) {
		if (*line == '\'' && line[1]) {
			const char *end;
			if (line[1] == '\\') {
				end = skip_backslash(&line[1]);
			} else {
				end = &line[2];
			}
			if (*end == '\'') {
				last_is_reset = 0;
				printf("\033[35m%.*s", (int)(end - line + 1), line);
				line = end + 1;
				continue;
			} else {
				if (!last_is_reset) term_reset_color();
				last_is_reset = 1;
				putchar('\'');
				line++;
				continue;
			}
		}
		if (*line == '"') {
			last_is_reset = 0;
			printf("\033[35m");
			putchar(*(line++));
			while (*line != '"' && *line) {
				putchar(*(line++));
			}
			putchar(*(line++));
			continue;
		}
		if (!is_word_char(*line)) {
			const char *start = line;
			size_t len = 0;
			while (!is_word_char(*line) && !reach_line_end(line) && *line != '"' && *line != '\'') {
				line++;
				len++;
			}
			term_reset_color();
			last_is_reset = 1;
			printf("%.*s", (int)len, start);
			continue;
		}
		// find lenght of word
		const char *word = line;
		size_t word_len = 0;
		while (is_word_char(*line)) {
			word_len++;
			line++;
		}
		const char *color = word_color(word, word_len);
		if (color) {
			last_is_reset = 0;
			printf("%s", color);
		} else {
			if (!last_is_reset) term_reset_color();
			last_is_reset = 1;
		}
		printf("%.*s", (int)word_len, word);
		continue;
	}

	if (*line) {
		printf("\033[36m%s", line);
	}
}
