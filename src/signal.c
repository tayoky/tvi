#include <tvi.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>

void sigint_handler(int signum) {
	(void)signum;
	print(&tvi, "do :q to exit tvi");
	render_flush(&tvi);
}

void sigwinch_handler(int signum) {
	(void)signum;
	term_fetch_size();
	if (tvi.mode == MODE_VISUAL) {
		// TODO : go trough each windows
		tvi.first_window->width  = term_width;
		tvi.first_window->height = term_height - 1;
		render_all_windows(&tvi);
		render_prompt(&tvi);
		render_flush(&tvi);
	}
}

void signal_install_handlers(void) {
	signal(SIGINT, sigint_handler);
	signal(SIGWINCH, sigwinch_handler);
}
#else
void signal_install_handlers(void) {
}
#endif
