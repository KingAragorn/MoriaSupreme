/* web/persist.c: flush the in-memory filesystem to IndexedDB.

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies.

   Saves and the score file live under /home/web, which index.html mounts
   as IDBFS. syncfs is asynchronous; this waits so a reload sees the write. */

#include <emscripten.h>
#include <stdio.h>

/* The in-game "Rogue like commands" flag. The character save stores it
   too. This file is the browser's own copy, so the page toggle survives
   a reload the same way a save does. */
extern int rogue_like_commands;

#define MORIA_KEYSET_PATH "/home/web_user/.moriasupreme/keyset"

void moria_store_keyset(void)
{
  FILE *fp;

  fp = fopen(MORIA_KEYSET_PATH, "w");
  if (fp == NULL)
    return;
  fputc(rogue_like_commands ? '1' : '0', fp);
  fclose(fp);
}

void moria_load_keyset(void)
{
  FILE *fp;
  int c;

  fp = fopen(MORIA_KEYSET_PATH, "r");
  if (fp == NULL)
    return;
  c = fgetc(fp);
  fclose(fp);
  if (c == '0')
    rogue_like_commands = 0;
  else if (c == '1')
    rogue_like_commands = 1;
}

int moria_rogue_like(void)
{
  return rogue_like_commands ? 1 : 0;
}

void moria_set_rogue_like(int on)
{
  rogue_like_commands = on ? 1 : 0;
  moria_store_keyset();
}

void moria_persist_sync(void)
{
  EM_ASM({
    var sync = (typeof FS !== "undefined" && FS.syncfs) ? FS.syncfs
      : (Module.FS && Module.FS.syncfs ? Module.FS.syncfs.bind(Module.FS) : null);
    Module.moriaSyncDone = 0;
    if (!sync) {
      Module.moriaSyncDone = 1;
      return;
    }
    sync(false, function(err) {
      if (err)
        console.error(err);
      Module.moriaSyncDone = 1;
    });
  });
  while (EM_ASM_INT({ return Module.moriaSyncDone ? 1 : 0; }) == 0)
    emscripten_sleep(20);
}
