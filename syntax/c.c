#include <string.h>
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

// standard types
	"size_t",
	"ssize_t",
	"FILE",
	"DIR",
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

const char *word_color(const char *word, size_t size) {
	if (is_type(word, size)) return "\033[32m";
	if (is_keyword(word, size)) return "\033[33m";
	return NULL;
}
