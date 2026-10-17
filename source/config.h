/* source/config.h: configuration definitions

   Copyright (c) 1989-92 James E. Wilson

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#define CONFIG_H_INCLUDED
#ifdef CONSTANT_H_INCLUDED
Constant.h should always be included after config.h, because it uses
some of the system defines set up here.
#endif

/* Modern POSIX hosts.  Game data and rules are unchanged.  This selects
   the System V shaped Unix code (signals, strings) and pulls in ANSI
   library prototypes so the K&R sources do not rely on implicit
   declarations.  SYS_V is intentionally not defined: that macro turns
   index into strchr and would rewrite the treasure field named index. */
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)      || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__EMSCRIPTEN__)
#define MORIA_POSIX 1
#endif

#ifdef MORIA_POSIX
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef USG
#define USG
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#ifndef STDIO_LOADED
#define STDIO_LOADED
#endif
/* K&R handlers do not match the POSIX signal prototype.  The cast keeps
   the original functions and satisfies current compilers. */
#define moria_signal(sig, handler) signal((sig), (void (*)(int))(handler))
#else
#define moria_signal(sig, handler) signal((sig), (handler))
#endif

/* Person to bother if something goes wrong.  */
/* Recompile files.c and misc2.c if this changes.  */
#define WIZARD	"David Grabiner"
/* The wizard password and wizard uid are no longer used.  */


/* Linux, macOS, the other BSDs listed above, and the Emscripten browser
   build set MORIA_POSIX themselves. DOS, Atari ST, Amiga, Mac Classic,
   VMS, and IBM PC sources are not part of this tree. */


/* Files used by moria, set these to valid pathnames for your system.  */




/* This must be unix.  */
#ifdef MORIA_POSIX
/* Filled in by moria_init_paths() from MORIA_LIB, the install data
   directory, or the source tree.  Scores go under $HOME/.moriasupreme. */
#define MORIA_SAV	"moriasupreme.save"
extern char moria_hou_path[];
extern char moria_mor_path[];
extern char moria_top_path[];
extern char moria_help_path[];
extern char moria_orig_help_path[];
extern char moria_wiz_help_path[];
extern char moria_owiz_help_path[];
extern char moria_welcome_path[];
extern char moria_ver_path[];
#define MORIA_HOU	moria_hou_path
#define MORIA_MOR	moria_mor_path
#define MORIA_TOP	moria_top_path
#define MORIA_HELP	moria_help_path
#define MORIA_ORIG_HELP	moria_orig_help_path
#define MORIA_WIZ_HELP	moria_wiz_help_path
#define MORIA_OWIZ_HELP	moria_owiz_help_path
#define MORIA_WELCOME	moria_welcome_path
#define MORIA_VER	moria_ver_path
void moria_init_paths();
#else
#define MORIA_SAV	"moria.save"
#define MORIA_HOU	"/home/17/petitc/public/Games/Moria/hours"
#define MORIA_MOR	"/home/17/petitc/public/Games/Moria/news"
#define MORIA_TOP	"/home/17/petitc/public/Games/Moria/scores"
#define MORIA_HELP	"/home/17/petitc/public/Games/Moria/roglcmds.hlp"
#define MORIA_ORIG_HELP	"/home/17/petitc/public/Games/Moria/origcmds.hlp"
#define MORIA_WIZ_HELP	"/home/17/petitc/public/Games/Moria/rwizcmds.hlp"
#define MORIA_OWIZ_HELP	"/home/17/petitc/public/Games/Moria/owizcmds.hlp"
#define MORIA_WELCOME	"/home/17/petitc/public/Games/Moria/welcome.hlp"
#define MORIA_VER	"/home/17/petitc/public/Games/Moria/version.hlp"
#endif



/* This sets the default user interface.  */
/* To use the original key bindings (keypad for movement) set ROGUE_LIKE
   to FALSE; to use the rogue-like key bindings (vi style movement)
   set ROGUE_LIKE to TRUE.  */
/* If you change this, you only need to recompile main.c.  */
#define ROGUE_LIKE TRUE


/* For the ANDREW distributed file system, define this to ensure that
   the program is secure with respect to the setuid code, this prohibits
   inferior shells.  It also does not relinquish setuid priviledges at the
   start, but instead calls the ANDREW library routines bePlayer(), beGames(),
   and Authenticate().  */
/* #define SECURE */



/* System dependent defines follow.  You should not need to change anything
   below.  */



/* Substitute strchr for index on USG versions of UNIX.  */


/* Define USG for many systems, this is basically to select SYS V style
   system calls (as opposed to BSD style).  */



/* Pyramid runs 4.2BSD-like UNIX version */







