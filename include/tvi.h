#ifndef TVI_H
#define TVI_H

#include <stdint.h>

typedef struct win {
	struct win *next;
	struct win *prev;
	int x;
	int y;
	int width;
	int height;
	int cursor_x;
	int cursor_y;
	int scroll;
	int flags;
	char **text;
	size_t lines_count;
} win_t;

typedef struct tvi {
	int mode;
	int flags;
	char prompt[256];
	size_t prompt_len;
	size_t prompt_cursor;
	win_t *focus_window;
	win_t *first_window;
} tvi_t;

int term_enter_fullscreen(void);
void term_exit_fullscreen(void);
void term_clear_line(void);
void term_goto(int x, int y);
void term_bell(void);
int term_is_delete(int c);
void term_fetch_size(void);
void term_reset_color(void);
void term_inverse_color(void);
void render_text(tvi_t *tvi, win_t *win);
void render_line(tvi_t *tvi, win_t *win, int y);
void render_status(tvi_t *tvi, win_t *win);
void render_window(tvi_t *tvi, win_t *win);
void render_prompt(tvi_t *tvi);
void render_cursor(tvi_t *tvi);
void render_flush(tvi_t *tvi);
win_t *win_create(tvi_t *tvi);
void win_free(tvi_t *tvi, win_t *win);
int ex_command(tvi_t *tvi, const char *command);
int tvi_main(tvi_t *tvi);

extern int term_width;
extern int term_height;

#define ESC "\033"
#define MODE_COMMAND  2
#define MODE_INSERT   1
#define FLAG_PROMPT   0x01 // when prompt is enabled
#define FLAG_QUIT     0x02

#endif
