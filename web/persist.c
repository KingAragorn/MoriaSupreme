/* web/persist.c: flush the in-memory filesystem to IndexedDB.

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies.

   Saves and the score file live under /home/web, which index.html mounts
   as IDBFS. syncfs is asynchronous; this waits so a reload sees the write. */

#include <emscripten.h>

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
