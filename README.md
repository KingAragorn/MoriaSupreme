# Moria Supreme

Moria Supreme is a free, non-commercial hobby roguelike: the ultimate classic Moria. It is one town and a dive to the Balrog on dungeon level 50, with the races and classes gathered from the Moria family. It is not an Angband-length game.

The playable base is [PmoriaPlus](https://github.com/KingAragorn/PmoriaPlus) current `main`: Pmoria 0.98, Christopher J. Petit's modification of Umoria 5.5, with a modern CMake/ncurses build, terminal color, and the macOS AppleClang fixes. This game will not be sold.

Moria Supreme is not a DOS game. The DOS, Atari ST, Amiga, Mac Classic, VMS, and IBM PC port trees are gone. What remains builds with CMake and ncurses on Linux and macOS, and the same sources compile to WebAssembly for a browser. Wilson's original `readme` and the credits in `files/version.hlp` still name the people who wrote those older ports.

## License

The imported source keeps the Wilson/Koeneke notice:

> Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke
>
> This software may be copied and distributed for educational, research, and not for profit purposes provided that this copyright and statement are included in all such copies.

`source/main.c` also keeps Koeneke's dedication, including the condition that nobody may sell or market the software without his written consent. Moria Supreme is not relicensed. Do not put it under the GPL, and do not drop these notices.

Imoria, Omega, BOSS, and other variants were not copied. Races, classes, and rules added from those games are clean-room: original code and original text, written from public descriptions of how the games play. Provenance for each addition is in [docs/VARIANTS.md](docs/VARIANTS.md) and [docs/CREDITS.md](docs/CREDITS.md).

## Races and classes

Pmoria's races and classes are all still here, including Avion, Sage Elf, Spectre, Demigod, Sage, Jester, Ninja, Healer, Wizard, and Monk.

Umoria's Gnome is restored beside the Spectre. Pmoria had used the Spectre's slot for that race. Gnome keeps the stat line published in the Umoria manual. Ages, size, and the raw skill numbers are original values fitted to that table.

Clean-room additions:

| Kind | Name | Idea |
| --- | --- | --- |
| Race | Dryad | Wood spirit. Extra charisma and wisdom. Best as a druid. |
| Class | Druid | Prayers leaned toward healing and the wild. Will not wear metal armor. An optional grove book adds a few mage-like workings. |
| Class | Bard | Mage spells learned as songs. Charm and lore before fire. Light armor. Starts with a flute. |
| Class | Adventurer | A bit of fighting, stealth, and a few mage spells. Slow to level. |

Which older classes each older race may take is unchanged, except that Humans, Half-Elves, and the races that fit a new profession may also take Druid, Bard, or Adventurer. The full grid is in `docs/VARIANTS.md`.

Save files are still version 0.9. Race and class are stored as indexes, and the new ones are appended, so an old save still names the same race and class. Patch 1 adds a trailer after the old character record (patron, bank balance, the druid's second spell list, and two town jobs). A patch 0 save still loads. New games write `$HOME/moriasupreme.save`. Scores go to `$HOME/.moriasupreme/scores`.

## Town

The same single town now has an adventurers' hall (`7`), a bank (`8`), and a notice board (`?`). Rogues, ninjas, bards, and jesters also have a door that looks like a wall. The temple can take a dedication from a priest, paladin, druid, or healer: the Forge, the Wild, the Merciful, or the Nameless Deep. None of this adds a second town or a quest that blocks the stairs to the Balrog. Details and the license notes are in `docs/VARIANTS.md`.

Day and night in town is still the classic Pmoria cycle: at night the streets are darker and more people are out.

## Build

You need a C compiler, CMake 3.10 or newer, and ncurses development headers.

Debian or Ubuntu:

```sh
sudo apt install build-essential cmake libncurses-dev
```

macOS (Homebrew):

```sh
brew install cmake ncurses
```

Then:

```sh
cmake -S . -B build
cmake --build build
./build/moriasupreme
```

The binary looks for `files/hours`, the news file, and the help files in this order:

1. `$MORIA_LIB`, if that directory contains `hours`
2. the `files/` directory in this source tree
3. `../share/moriasupreme` next to the executable
4. the install prefix data directory (`share/moriasupreme`)

Install with:

```sh
cmake --install build
```

## Terminal color

The dungeon map is drawn in color when the terminal supports it. The character sheet and messages stay in the terminal's normal text.

Color is off when the terminal has no color, you pass `-m`, or `MORIA_MONOCHROME` is set to anything other than `0`.

```sh
./build/moriasupreme -m
MORIA_MONOCHROME=1 ./build/moriasupreme
```

Other startup flags are the original ones: `-n` ignores a save file, `-o` and `-r` choose the original or rogue-like key sets, `-s` and `-S` show scores, and `-w` requests wizard mode.

On macOS, if CMake does not find Homebrew's ncurses, configure with `CMAKE_PREFIX_PATH` set to the Homebrew prefix (`/opt/homebrew` or `/usr/local`). The AppleClang `lseek` and `flock` fixes in `source/death.c` are kept.

`source/makefile` is James Wilson's old UNIX makefile. The CMake build above is the one that works. `cmake --build build --target web` runs `web/build.sh` when Emscripten is installed.

## Play in a browser

The browser build is the same C sources, compiled with Emscripten. A small curses shim draws a 24x80 screen and an xterm.js terminal shows it. There is no server. Saves and the score file are stored in the browser with IndexedDB (IDBFS), under the page's origin.

Install the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then:

```sh
./web/build.sh
```

That writes a static site to `web/dist/` (`index.html`, `moriasupreme.js`, `moriasupreme.wasm`, `moriasupreme.data`, and xterm.js). Serve that folder with any static host. Opening the html file directly will not load the wasm.

```sh
python3 -m http.server -d web/dist 8080
```

Movement is `hjkl`. Ctrl-X saves and exits. Reload the page and the same character comes back. Arrow keys are mapped to `hjkl`.

GitHub Actions (`.github/workflows/pages.yml`) builds `web/dist` and deploys it to GitHub Pages when `main` changes. The repository owner has to turn Pages on once: Settings, Pages, Build and deployment, Source: GitHub Actions. The workflow cannot flip that switch. The site is then `https://<user>.github.io/<repo>/`.

## Smoke test

`scripts/smoke_characters.py` creates every legal race and class, walks a few steps in town, and saves:

```sh
python3 scripts/smoke_characters.py --binary build/moriasupreme
```

It needs the Python package `pyte`.

## Provenance of the import

PmoriaPlus `main` at `03a6172709909a157dbce10cdfc6ad6596d002d7` is merged into this history. That tree's first commit is the unpacked `pmoria.tar.Z` from the Funet unofficial Moria source mirror (8 April 1993):

https://ftp.zx.net.nz/pub/mirror/ftp.funet.fi/pub/unix/games/moria/unofficial/source/pmoria.tar.Z

Petit's notes are in `pmoria.mod`.
