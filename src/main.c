#include <stdio.h>
#include <stdlib.h>
#include <tvi.h>

int main(int argc, char **argv) {
	int i=1;
	tvi_t tvi = {
		.mode = MODE_VISUAL,
	};
	for (;i<argc;i++) {
		if (argv[i][0] != '-') break;
		if (argv[i][1] == '-') {
			if (!argv[i][2]) {
				// do not parse after "--"
				i++;
				break;
			}
		} else {
			switch (argv[i][1]) {
			case 'v':
				tvi.mode = MODE_VISUAL;
				break;
			case 'e':
				tvi.mode = MODE_EX;
				break;
			default:
				fprintf(stderr, "tvi : unknow option '%x'\n", argv[i][1]);
				return 1;
			}
		}
	}
	if (term_enter_fullscreen() < 0) {
		return 1;
	}
	win_t *win = win_create(&tvi);
	if (i < argc) {
		// we have files to edit
		// TODO : open them
	}
	if (tvi.mode == MODE_VISUAL) {
		tvi_main(&tvi);
	} else {
		ex_main(&tvi);
	}
	term_exit_fullscreen();
	return 0;
}
