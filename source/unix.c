/* unix/unix.c: UNIX dependent code.					-CJS-

   Copyright (c) 1989-92 James E. Wilson, Christopher J. Stuart

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */


/* defines NULL */
#include <stdio.h>
/* defines CTRL */
#include <sys/ioctl.h>
/* defines TRUE and FALSE */
#include <curses.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#ifdef unix


#include <signal.h>


#ifndef USG
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>
#endif

#ifdef USG
#include <string.h>
#include <fcntl.h>
#else
#include <strings.h>
#include <sys/wait.h>
#endif

/* This must be included after fcntl.h, which has a prototype for `open'
   on some systems.  Otherwise, the `open' prototype conflicts with the
   `topen' declaration.  */
#include "externs.h"

#include <pwd.h>
#include <sys/errno.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
int moria_kbhit();
#endif
#ifdef MORIA_POSIX
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <stdint.h>
#endif
#endif

#ifndef MORIA_POSIX
#ifdef USG
struct passwd *getpwuid();
struct passwd *getpwnam();
#endif
#endif


/* Fooling lint. Unfortunately, c defines all the TIO constants to be long,
   and lint expects them to be int. Also, ioctl is sometimes called with just
   two arguments. The following definition keeps lint happy. It may need to be
   reset for different systems. */

/* Provides for a timeout on input. Does a non-blocking read, consuming the
   data if any, and then returns 1 if data was read, zero otherwise.

   Porting:

   In systems without the select call, but with a sleep for
   fractional numbers of seconds, one could sleep for the time
   and then check for input.

   In systems which can only sleep for whole number of seconds,
   you might sleep by writing a lot of nulls to the terminal, and
   waiting for them to drain, or you might hack a static
   accumulation of times to wait. When the accumulation reaches a
   certain point, sleep for a second. There would need to be a
   way of resetting the count, with a call made for commands like
   run or rest. */
int check_input(microsec)
int microsec;
{
#ifdef __EMSCRIPTEN__
  int ch;

  /* Keys arrive from the browser, not from stdin. Yield so a key typed
     during rest or run can land in the queue. flush() passes 0 and stops
     on the first 0, so a 1ms yield there is one sleep. */
  if (!moria_kbhit())
    {
      emscripten_sleep(microsec > 1000 ? microsec / 1000 : 1);
      if (!moria_kbhit())
	return 0;
    }
  ch = getch();
  if (ch == ERR)
    {
      eof_flag++;
      return 0;
    }
  return 1;
#else
#ifdef MORIA_POSIX
  struct timeval tbuf;
  fd_set smask;
  int ch;

  /* Same contract as the original: consume one ready character, if any,
     so callers can poll for an interrupt. */
  tbuf.tv_sec = 0;
  tbuf.tv_usec = microsec;
  FD_ZERO(&smask);
  FD_SET(fileno(stdin), &smask);
  if (select(fileno(stdin) + 1, &smask, (fd_set *)0, (fd_set *)0, &tbuf) == 1)
    {
      ch = getch();
      if (ch == ERR)
	{
	  eof_flag++;
	  return 0;
	}
      return 1;
    }
  return 0;
#else
#if defined(USG) && !defined(M_XENIX)
  int arg, result;
#else
  struct timeval tbuf;
  int ch;
  int smask;
#endif

  /* Return true if a read on descriptor 1 will not block. */
#if !defined(USG) || defined(M_XENIX)
  tbuf.tv_sec = 0;
  tbuf.tv_usec = microsec;
  smask = 1;  /* i.e. (1 << 0) */
  if (select(1, &smask, (int *)0, (int *)0, &tbuf) == 1)
    {
      ch = getch();
      /* check for EOF errors here, select sometimes works even when EOF */
      if (ch == -1)
	{
	  eof_flag++;
	  return 0;
	}
      return 1;
    }
  else
    return 0;
#else /* SYS V code follows */
  if (microsec != 0 && (turn & 0x7F) == 0)
    (void) sleep (1); /* mod 128, sleep one sec every 128 turns */
  /* Can't check for input, but can do non-blocking read, so... */
  /* Ugh! */
  arg = 0;
  arg = fcntl(0, F_GETFL, arg);
  arg |= O_NDELAY;
  (void) fcntl(0, F_SETFL, arg);

  result = getch();

  arg = 0;
  arg = fcntl(0, F_GETFL, arg);
  arg &= ~O_NDELAY;
  (void) fcntl(0, F_SETFL, arg);
  if (result == -1)
    return 0;
  else
    return 1;
#endif
#endif
#endif
}


#ifndef MORIA_POSIX
#ifdef USG
unsigned short getuid();
#else
int getuid();
#endif
#endif

/* Find a default user name from the system. */
void user_name(buf)
char *buf;
{
  char *p;
  struct passwd *pw;

  p = getlogin();
  if (p && p[0])
    (void) strcpy(buf, p);
  else if ((pw = getpwuid(getuid())) != NULL && pw->pw_name != NULL
	   && pw->pw_name[0] != '\0')
    (void) strcpy(buf, pw->pw_name);
  else
    (void) strcpy(buf, "X");	/* Gotta have some name */
}

/* expands a tilde at the beginning of a file name to a users home
   directory */
int tilde(file, exp)
char *file, *exp;
{
  *exp = '\0';
  if (file)
    {
      if (*file == '~')
	{
	  char user[128];
	  struct passwd *pw = NULL;
	  int i = 0;

	  user[0] = '\0';
	  file++;
	  while (*file != '/' && i < sizeof(user))
	    user[i++] = *file++;
	  user[i] = '\0';
	  if (i == 0)
	    {
	      char *login = (char *) getlogin();

	      if (login != NULL)
		(void) strcpy (user, login);
	      else if ((pw = getpwuid(getuid())) == NULL)
		return 0;
	    }
	  if (pw == NULL && (pw = getpwnam(user)) == NULL)
	    return 0;
	  (void) strcpy (exp, pw->pw_dir);
	}
      (void) strcat(exp, file);
      return 1;
    }
  return 0;
}

/* undefine these so that tfopen and topen will work */
#undef fopen
#undef open

/* open a file just as does fopen, but allow a leading ~ to specify a home
   directory */
FILE *tfopen(file, mode)
char *file;
char *mode;
{
  char buf[1024];
#ifndef MORIA_POSIX
  extern int errno;
#endif

  if (tilde(file, buf))
    return (fopen(buf, mode));
  errno = ENOENT;
  return NULL;
}

/* open a file just as does open, but expand a leading ~ into a home directory
   name */
int topen(file, flags, mode)
char *file;
int flags, mode;
{
  char buf[1024];
#ifndef MORIA_POSIX
  extern int errno;
#endif

  if (tilde(file, buf))
    return (open(buf, flags, mode));
  errno = ENOENT;
  return -1;
}

#ifdef MORIA_POSIX
char moria_hou_path[1024];
char moria_mor_path[1024];
char moria_top_path[1024];
char moria_help_path[1024];
char moria_orig_help_path[1024];
char moria_wiz_help_path[1024];
char moria_owiz_help_path[1024];
char moria_welcome_path[1024];
char moria_ver_path[1024];

static int moria_dir_has_hours(dir)
const char *dir;
{
  char buf[1200];
  struct stat st;

  if (dir == 0 || dir[0] == '\0' || strlen(dir) > 1000)
    return 0;
  (void) sprintf(buf, "%s/hours", dir);
  return stat(buf, &st) == 0;
}

static void moria_join(dst, dstsz, dir, name)
char *dst;
int dstsz;
const char *dir;
const char *name;
{
  (void) snprintf(dst, (size_t)dstsz, "%s/%s", dir, name);
}

/* Locate help/news/hours, and a writable score file. */
void moria_init_paths()
{
  const char *lib = 0;
  const char *envlib;
  const char *home;
  const char *scores;
  char exe[1024];
  char sibling[1024];
  char dirbuf[1024];

  envlib = getenv("MORIA_LIB");
  if (moria_dir_has_hours(envlib))
    lib = envlib;
#ifdef MORIA_LIB_SOURCE
  if (lib == 0 && moria_dir_has_hours(MORIA_LIB_SOURCE))
    lib = MORIA_LIB_SOURCE;
#endif

  exe[0] = '\0';
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
  {
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0)
      exe[n] = '\0';
    else
      exe[0] = '\0';
  }
#elif defined(__APPLE__)
  {
    uint32_t sz = (uint32_t)sizeof(exe);
    if (_NSGetExecutablePath(exe, &sz) != 0)
      exe[0] = '\0';
  }
#endif
  if (exe[0] != '\0')
    {
      char *slash = strrchr(exe, '/');
      if (slash != 0)
	{
	  *slash = '\0';
	  (void) snprintf(sibling, sizeof(sibling), "%s/../share/moriasupreme", exe);
	  if (lib == 0 && moria_dir_has_hours(sibling))
	    lib = sibling;
	}
    }
#ifdef MORIA_LIB_DEFAULT
  if (lib == 0 && moria_dir_has_hours(MORIA_LIB_DEFAULT))
    lib = MORIA_LIB_DEFAULT;
#endif
  if (lib == 0)
    lib = "files";

  moria_join(moria_hou_path, (int)sizeof(moria_hou_path), lib, "hours");
  moria_join(moria_mor_path, (int)sizeof(moria_mor_path), lib, "news");
  moria_join(moria_help_path, (int)sizeof(moria_help_path), lib, "roglcmds.hlp");
  moria_join(moria_orig_help_path, (int)sizeof(moria_orig_help_path), lib,
	     "origcmds.hlp");
  moria_join(moria_wiz_help_path, (int)sizeof(moria_wiz_help_path), lib,
	     "rwizcmds.hlp");
  moria_join(moria_owiz_help_path, (int)sizeof(moria_owiz_help_path), lib,
	     "owizcmds.hlp");
  moria_join(moria_welcome_path, (int)sizeof(moria_welcome_path), lib,
	     "welcome.hlp");
  moria_join(moria_ver_path, (int)sizeof(moria_ver_path), lib, "version.hlp");

  scores = getenv("MORIA_SCORES");
  if (scores != 0 && scores[0] != '\0')
    (void) snprintf(moria_top_path, sizeof(moria_top_path), "%s", scores);
  else
    {
      home = getenv("HOME");
      if (home != 0 && home[0] != '\0')
	{
	  (void) snprintf(dirbuf, sizeof(dirbuf), "%s/.moriasupreme", home);
	  (void) mkdir(dirbuf, 0755);
	  (void) snprintf(moria_top_path, sizeof(moria_top_path),
			  "%s/scores", dirbuf);
	}
      else
	(void) snprintf(moria_top_path, sizeof(moria_top_path), "scores");
    }
}
#endif
#endif
