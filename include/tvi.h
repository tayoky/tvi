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
	char **files;
	int lines_count;
	int files_count;
	int file_index;
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

int term_enable_raw_mode(void);
void term_quit_raw_mode(void);
int term_enter_fullscreen(void);
void term_exit_fullscreen(void);
void term_clear_line(void);
void term_goto(int x, int y);
void term_bell(void);
int term_is_delete(int c);
void term_fetch_size(void);
void term_reset_color(void);
void term_inverse_color(void);
void term_error_color(void);
void render_text(tvi_t *tvi, win_t *win);
void render_line(tvi_t *tvi, win_t *win, int y);
void render_status(tvi_t *tvi, win_t *win);
void render_window(tvi_t *tvi, win_t *win);
void render_all_windows(tvi_t *tvi);
void render_prompt(tvi_t *tvi);
void render_cursor(tvi_t *tvi);
void render_flush(tvi_t *tvi);
win_t *win_create(tvi_t *tvi);
void win_free(tvi_t *tvi, win_t *win);
void text_insert_lines(win_t *win, int addr, char *const*lines, size_t lines_count);
int ex_command(tvi_t *tvi, const char *command);
void open_files(win_t *win, char *const*files, size_t files_count);
void read_file(win_t *win, const char *path);
void free_list(char **list, size_t count);
int tvi_main(tvi_t *tvi);
int ex_main(tvi_t *tvi);
void error(tvi_t *tvi, const char *fmt, ...);
void print(tvi_t *tvi, const char *fmt, ...);
int prompt(tvi_t *tvi, const char *initial, int newline);

extern int term_width;
extern int term_height;

#define ESC "\033"
#define CRTL(c) (c - 'A' + 1)
#define MODE_VISUAL   1
#define MODE_EX       2
#define FLAG_PROMPT   0x01 // when prompt is enabled
#define FLAG_QUIT     0x02

#endif
