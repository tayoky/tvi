#include <stdlib.h>
#include <limits.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <tvi.h>

syntax_t *syntax_load(const char *name) {
#if defined(HAVE_DLOPEN) && defined(HAVE_DLSYM)
	char path[PATH_MAX];
	snprintf(path, sizeof(path), PREFIX"/lib/tvi/%s.so", name);
	void *handle = dlopen(path, RTLD_NOW);
	if (!handle) return NULL;
	int (*init)(void) = dlsym(handle, "init");
	if (init) init();
	syntax_t *syntax = malloc(sizeof(syntax_t));
	syntax->handle = handle;
	syntax->word_color = dlsym(handle, "word_color");
	return syntax;
#else
	return NULL;
#endif
}

void syntax_unload(syntax_t *syntax) {
	if (!syntax) return;
#ifdef HAVE_DLCLOSE
	dlclose(syntax->handle);
#endif
	free(syntax->handle);
}

const char *syntax_word_color(syntax_t *syntax, const char *word, size_t size) {
	if (syntax && syntax->word_color) return syntax->word_color(word, size);
	return NULL;
}
