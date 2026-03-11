#include <string.h>
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
	"#undef"
	"#ifdef",
	"#ifndef",
	"#if"
	"#else",
	"#elif",
	"#endif",
	"#include",
};

int init() {
	return 0;
}

int is_type(const char *word, size_t len) {
	for (size_t i=0; i<sizeof(types)/sizeof(*types); i++) {
		if (len != strlen(types[i])) continue;
		if (memcmp(word, types[i], len)) continue;
		return 1;
	}
	return 0;
}

int is_keyword(const char *word, size_t len) {
	for (size_t i=0; i<sizeof(keywords)/sizeof(*keywords); i++) {
		if (len != strlen(keywords[i])) continue;
		if (memcmp(word, keywords[i], len)) continue;
		return 1;
	}
	return 0;
}

int is_preproc(const char *word, size_t len) {
	for (size_t i=0; i<sizeof(preprocs)/sizeof(*preprocs); i++) {
		if (len != strlen(preprocs[i])) continue;
		if (memcmp(word, preprocs[i], len)) continue;
		return 1;
	}
	return 0;
}

const char *word_color(const char *word, size_t size) {
	if (is_type(word, size)) return "\033[32m";
	if (is_keyword(word, size)) return "\033[33m";
	return NULL;
}

static int is_word_char(int c) {
	return isalpha(c) || c == '_';
}

static int reach_line_end(const char *line) {
	return !*line || (*line == '/' && line[1] == '/');
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
		if (is_preproc(word, word_len)) {
			printf("\033[34m%.*s", (int)word_len, word);
			line += word_len;
		}
	}
	while (!reach_line_end(line)) {
		if (!is_word_char(*line)) {
			const char *start = line;
			size_t len = 0;
			while (!is_word_char(*line) && !reach_line_end(line)) {
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
