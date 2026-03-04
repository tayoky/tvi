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

typedef struct reg {
	char **text;
	size_t lines_count;
	int type;
} reg_t;

#define REG_CHAR 0
#define REG_LINE 1

typedef struct tvi {
	int mode;
	int flags;
	char prompt[256];
	size_t prompt_len;
	size_t prompt_cursor;
	win_t *focus_window;
	win_t *first_window;
	reg_t alpha_regs[26];
	reg_t digit_regs[10];
	reg_t unamed_reg;
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
void text_insert_newline(win_t *win, int x, int y);
void text_insert_buf(win_t *win, int x, int y, const char *buf, size_t count);
void text_delete(win_t *win, int x, int y, size_t count);
void text_delete_reg(tvi_t *tvi, win_t *win, int x, int y, size_t count, int reg);
void text_delete_lines(win_t *win, int addr, size_t count);
void text_delete_lines_reg(tvi_t *tvi, win_t *win, int addr, size_t count, int reg);
void text_yank_lines(tvi_t *tvi, win_t *win, int addr, size_t count, int reg);
void text_join(win_t *win, int first, int last, char sep);
void text_mark_dirty(win_t *win);
void reg_write(tvi_t *tvi, int name, char *const*lines, size_t lines_count, int type);
void reg_put(tvi_t *tvi, win_t *win, int name, int x, int y);
int ex_command(tvi_t *tvi, const char *command);
void open_files(win_t *win, char *const*files, size_t files_count);
void read_file(win_t *win, const char *path);
int write_file(tvi_t *tvi, win_t *win, const char *path, int first, int last);
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
#define FLAG_DIRTY    0x04

#endif
