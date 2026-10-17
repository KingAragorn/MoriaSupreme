/* web/curses.h: curses subset for the Emscripten build.

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies.

   The browser build draws into this buffer and ships ANSI to xterm.js.
   It is not a substitute for ncurses on Linux or macOS. */

#ifndef MORIA_WEB_CURSES_H
#define MORIA_WEB_CURSES_H

#define ERR (-1)
#define OK 0

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

#define A_NORMAL 0u
#define A_BOLD 0x100000u
#define A_REVERSE 0x200000u
#define COLOR_PAIR(n) ((unsigned long)(n) << 8)

typedef unsigned long chtype;

typedef struct moria_window {
  int _cury;
  int _curx;
  int _maxy;
  int _maxx;
  chtype cells[24][80];
} WINDOW;

extern int LINES;
extern int COLS;
extern WINDOW *stdscr;
extern WINDOW *curscr;

#define getyx(win, y, x) ((y) = (win)->_cury, (x) = (win)->_curx)

WINDOW *initscr(void);
int endwin(void);
WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int refresh(void);
int wrefresh(WINDOW *win);
int clear(void);
int wclear(WINDOW *win);
int move(int y, int x);
int wmove(WINDOW *win, int y, int x);
int addch(chtype ch);
int mvaddch(int y, int x, chtype ch);
int addstr(const char *s);
int mvaddstr(int y, int x, const char *s);
int clrtoeol(void);
int clrtobot(void);
int overwrite(WINDOW *src, WINDOW *dst);
int touchwin(WINDOW *win);
int getch(void);
int crmode(void);
int nocrmode(void);
int echo(void);
int noecho(void);
int cbreak(void);
int nocbreak(void);
int raw(void);
int noraw(void);
int mvcur(int oldrow, int oldcol, int newrow, int newcol);
int has_colors(void);
int start_color(void);
int use_default_colors(void);
int init_pair(short pair, short fg, short bg);
int init_color(short color, short r, short g, short b);

/* Browser key queue. Not part of curses. unix.c polls this while resting. */
int moria_kbhit(void);
void moria_push_key(int key);

#endif
