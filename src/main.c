#include <stdio.h>
#include <stdlib.h>
#include <tvi.h>

int main(int argc, char **argv) {
	if (term_enter_fullscreen() < 0) {
		return -1;
	}
	tvi_t tvi = {
		.mode = MODE_COMMAND,
	};
	tvi_main(&tvi);
	term_exit_fullscreen();
	return 0;
}
