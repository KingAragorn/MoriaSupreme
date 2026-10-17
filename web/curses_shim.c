/* web/curses_shim.c: 24x80 curses buffer that speaks ANSI to xterm.js.

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "curses.h"

#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 24
#define COLS_N 80
#define QSIZE 512

int LINES = ROWS;
int COLS = COLS_N;
WINDOW *stdscr;
WINDOW *curscr;

static WINDOW *spare;
static short pair_fg[64];
static short pair_bg[64];
static int key_q[QSIZE];
static int qh, qt;
static int started;

static WINDOW *make_win(void)
{
  WINDOW *win;
  int y, x;

  win = (WINDOW *)calloc(1, sizeof(WINDOW));
  if (win == 0)
    return 0;
  win->_maxy = ROWS;
  win->_maxx = COLS_N;
  for (y = 0; y < ROWS; y++)
    for (x = 0; x < COLS_N; x++)
      win->cells[y][x] = (chtype)' ';
  return win;
}

static void send_ansi(const char *buf, int n)
{
  EM_ASM({
    var ptr = $0;
    var len = $1;
    var heap = HEAPU8;
    var s = "";
    var i;
    for (i = 0; i < len; i++)
      s += String.fromCharCode(heap[ptr + i]);
    if (typeof window !== "undefined" && window.moriaWrite)
      window.moriaWrite(s);
  }, buf, n);
}

static int sgr(char *dst, chtype attr)
{
  int pair = (int)((attr >> 8) & 63);
  int fg = pair_fg[pair];
  int bg = pair_bg[pair];
  int rev = (attr & A_REVERSE) != 0;
  int bold = (attr & A_BOLD) != 0;
  int tmp;

  if (pair == 0)
    {
      fg = COLOR_WHITE;
      bg = COLOR_BLACK;
    }
  if (rev)
    {
      tmp = fg;
      fg = bg;
      bg = tmp;
    }
  if (bold && fg < 8)
    fg += 8;
  return sprintf(dst, "\033[0;%d;%dm",
		 30 + (fg & 7) + (fg >= 8 ? 60 : 0),
		 40 + (bg & 7));
}

static void emit(WINDOW *win)
{
  static char buf[96 * 1024];
  int n = 0;
  int y, x;
  chtype prev = (chtype)-1;
  chtype cell, attr;
  unsigned char ch;

  n += sprintf(buf + n, "\033[H");
  for (y = 0; y < ROWS; y++)
    {
      if (y > 0)
	n += sprintf(buf + n, "\033[%d;1H", y + 1);
      for (x = 0; x < COLS_N; x++)
	{
	  cell = win->cells[y][x];
	  attr = cell & ~0xFFul;
	  ch = (unsigned char)(cell & 0xFF);
	  if (ch < 32 || ch == 127)
	    ch = ' ';
	  if (attr != prev)
	    {
	      n += sgr(buf + n, attr);
	      prev = attr;
	    }
	  buf[n++] = (char)ch;
	}
    }
  n += sprintf(buf + n, "\033[0m\033[%d;%dH", win->_cury + 1, win->_curx + 1);
  send_ansi(buf, n);
}

WINDOW *initscr(void)
{
  int i;

  if (!started)
    {
      stdscr = make_win();
      curscr = make_win();
      for (i = 0; i < 64; i++)
	{
	  pair_fg[i] = COLOR_WHITE;
	  pair_bg[i] = COLOR_BLACK;
	}
      started = 1;
    }
  LINES = ROWS;
  COLS = COLS_N;
  return stdscr;
}

int endwin(void)
{
  return OK;
}

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x)
{
  (void)nlines;
  (void)ncols;
  (void)begin_y;
  (void)begin_x;
  if (spare == 0)
    spare = make_win();
  return spare;
}

int refresh(void)
{
  if (stdscr == 0)
    return ERR;
  memcpy(curscr->cells, stdscr->cells, sizeof(stdscr->cells));
  curscr->_cury = stdscr->_cury;
  curscr->_curx = stdscr->_curx;
  emit(curscr);
  return OK;
}

int wrefresh(WINDOW *win)
{
  if (win == 0)
    return ERR;
  if (win == stdscr)
    return refresh();
  emit(win);
  return OK;
}

int wclear(WINDOW *win)
{
  int y, x;

  if (win == 0)
    return ERR;
  for (y = 0; y < ROWS; y++)
    for (x = 0; x < COLS_N; x++)
      win->cells[y][x] = (chtype)' ';
  win->_cury = 0;
  win->_curx = 0;
  return OK;
}

int clear(void)
{
  return wclear(stdscr);
}

int wmove(WINDOW *win, int y, int x)
{
  if (win == 0 || y < 0 || x < 0 || y >= ROWS || x >= COLS_N)
    return ERR;
  win->_cury = y;
  win->_curx = x;
  return OK;
}

int move(int y, int x)
{
  return wmove(stdscr, y, x);
}

static int put_cell(WINDOW *win, chtype ch)
{
  if (win == 0)
    return ERR;
  if (win->_cury < 0 || win->_cury >= ROWS || win->_curx < 0 || win->_curx >= COLS_N)
    return ERR;
  win->cells[win->_cury][win->_curx] = ch;
  if (win->_curx < COLS_N - 1)
    win->_curx++;
  return OK;
}

int addch(chtype ch)
{
  return put_cell(stdscr, ch);
}

int mvaddch(int y, int x, chtype ch)
{
  if (move(y, x) == ERR)
    return ERR;
  return addch(ch);
}

static int put_str(WINDOW *win, const char *s)
{
  if (s == 0)
    return ERR;
  while (*s)
    {
      if (put_cell(win, (chtype)(unsigned char)*s) == ERR)
	return ERR;
      s++;
    }
  return OK;
}

int addstr(const char *s)
{
  return put_str(stdscr, s);
}

int mvaddstr(int y, int x, const char *s)
{
  if (move(y, x) == ERR)
    return ERR;
  return addstr(s);
}

int clrtoeol(void)
{
  int x;

  if (stdscr == 0 || stdscr->_cury < 0 || stdscr->_cury >= ROWS)
    return ERR;
  for (x = stdscr->_curx; x < COLS_N; x++)
    stdscr->cells[stdscr->_cury][x] = (chtype)' ';
  return OK;
}

int clrtobot(void)
{
  int y, x, sx;

  if (stdscr == 0)
    return ERR;
  sx = stdscr->_curx;
  for (y = stdscr->_cury; y < ROWS; y++)
    {
      for (x = sx; x < COLS_N; x++)
	stdscr->cells[y][x] = (chtype)' ';
      sx = 0;
    }
  return OK;
}

int overwrite(WINDOW *src, WINDOW *dst)
{
  if (src == 0 || dst == 0)
    return ERR;
  memcpy(dst->cells, src->cells, sizeof(src->cells));
  dst->_cury = src->_cury;
  dst->_curx = src->_curx;
  return OK;
}

int touchwin(WINDOW *win)
{
  (void)win;
  return OK;
}

int moria_kbhit(void)
{
  return qh != qt;
}

void moria_push_key(int key)
{
  int next = (qt + 1) % QSIZE;

  if (next == qh)
    return;
  key_q[qt] = key;
  qt = next;
}

int getch(void)
{
  int key;

  while (qh == qt)
    emscripten_sleep(10);
  key = key_q[qh];
  qh = (qh + 1) % QSIZE;
  return key;
}

int crmode(void) { return OK; }
int nocrmode(void) { return OK; }
int echo(void) { return OK; }
int noecho(void) { return OK; }
int cbreak(void) { return OK; }
int nocbreak(void) { return OK; }
int raw(void) { return OK; }
int noraw(void) { return OK; }

int mvcur(int oldrow, int oldcol, int newrow, int newcol)
{
  (void)oldrow;
  (void)oldcol;
  return move(newrow, newcol);
}

int has_colors(void) { return TRUE; }
int start_color(void) { return OK; }
int use_default_colors(void) { return OK; }

int init_pair(short pair, short fg, short bg)
{
  if (pair < 0 || pair >= 64)
    return ERR;
  pair_fg[pair] = fg;
  pair_bg[pair] = bg;
  return OK;
}

int init_color(short color, short r, short g, short b)
{
  (void)color;
  (void)r;
  (void)g;
  (void)b;
  return OK;
}
