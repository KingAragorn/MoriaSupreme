# Moria variant catalogue

This is the research list for Moria Supreme: races, classes, and the features worth keeping in a short classic game. Status is **included**, **planned**, or **not adopted**.

Nothing in the included rows was copied from Imoria, Omega, BOSS, or a GPL tree. Where a variant is GPL or the license is unclear, only public gameplay descriptions were used, and the code and text here are original. Umoria and Pmoria are the imported base and keep their own notices (`docs/CREDITS.md`).

"Included" for an Umoria or Pmoria race means the Pmoria 0.98 data is still in the game. "Included" for Gnome, Dryad, Druid, Bard, and Adventurer means a clean-room version is in this tree. The numbers for those five are not taken from another game's tables.

## Design fence

One town. The win is still the Balrog on dungeon level 50. Runs should stay a classic Moria length. Angband's extra towns, wilderness, and class list are out of scope. So is anything that turns the game into a long campaign.

## Umoria 5.5 (and the 5.x line)

Source notes: James E. Wilson and David J. Grabiner, Umoria 5.5, the same license as the imported code. The public manual is `historical/manual.md` in the Umoria history and the same text shipped as `doc/moria1.txt` here, minus Petit's pages. Umoria 5.7 later moved to the GPL. This project does not take code from that GPL line.

| Kind | Name | Status | Notes |
| --- | --- | --- | --- |
| Race | Human | included | Any class. Experience base 100. |
| Race | Half-Elf | included | Any class. |
| Race | Elf | included | No paladin in classic Moria. Pmoria also bars ranger, ninja, and wizard. Those bars are kept. |
| Race | Halfling | included | Classic bars: no priest, ranger, or paladin. |
| Race | Gnome | included | Pmoria removed this race and put the Spectre in its slot. Supreme puts the Gnome back as a new id, so old saves that say "race 4" are still Spectres. Stat line from the public Umoria manual: Str -1, Int +2, Wis 0, Dex +2, Con +1, Chr -2, hit die 7, +25% experience, 30 feet of infravision. Disarm 9, search 7, stealth 9, perception 9, fight 2, bows 8, save 9 on the manual's 1-10 scale. Ages, height, weight, and the raw skill fields are original numbers fitted to that scale and to "smaller than a dwarf, larger than a halfling." Classes: warrior, mage, priest, rogue (the classic four), plus sage, jester, healer, wizard, druid, bard, and adventurer. Not ranger, paladin, demigod, ninja, or monk. |
| Race | Dwarf | included | Classic limits kept. No new class. |
| Race | Half-Orc | included | Classic limits kept, plus bard and adventurer. |
| Race | Half-Troll | included | Classic limits kept, plus bard, because Pmoria already lets them be jesters. |
| Class | Warrior | included | |
| Class | Mage | included | Mage spellbook. |
| Class | Priest | included | Prayer book. |
| Class | Rogue | included | A few mage spells, late. |
| Class | Ranger | included | Warrior/mage. Best bows. No second ranger class was added. See "Ranger variations" under Imoria. |
| Class | Paladin | included | Warrior/priest. |

Notable Umoria features already in the base, and kept:

| Feature | Status | Notes |
| --- | --- | --- |
| One town, six shops, haggling | included | |
| Balrog at level 50 | included | `WIN_MON_APPEAR` is 50. |
| Hunger | included | |
| Day and night in town | included | Every 5000 turns the town is generated as night or day. At night the open street is dark and more people are placed (`MIN_MALLOC_TN` 8 versus `MIN_MALLOC_TD` 4). Shops stay open. |
| Real-world play hours | included | `files/hours`, optional. This is not the in-game clock. |
| Monster memory, options, spell study | included | Umoria 5.0 took a lot of this from the unreleased BRUCE Moria (Christopher J. Stuart). It is already in the inherited game. |

## Pmoria 0.98

Source notes: Christopher J. Petit, `pmoria.mod` and `doc/moria1.txt`. Imported from PmoriaPlus `main` (`03a6172`). These tables are the original Pmoria data, not a reimplementation.

| Kind | Name | Status | Notes |
| --- | --- | --- | --- |
| Race | Spectre | included | Half-ghost. Replaced the Gnome in Pmoria. Kept, and the Gnome is a separate race again. Excellent wizard, fair warrior, decent rogue. Very weak, high base armor, strong infravision. |
| Race | Avion | included | Half bird, half dragon. Strong hit points, poor wisdom, good mage or warrior. |
| Race | Sage Elf | included | Old elf. High wisdom, quiet, frail. Excellent priest. |
| Class | Demigod | included | Huge hit die, prayers, poor with bows. |
| Class | Sage | included | Knowledge spells (the sage books). Poor fighter, can wear heavy armor. |
| Class | Jester | included | Mage spells, average at everything. Mid-level titles already say "Bard". The Imoria bard below is a separate profession. |
| Class | Ninja | included | Fighter with sage spells, stealth, and bows. |
| Class | Healer | included | A subset of prayers, learned quickly, hardier than a priest. |
| Class | Wizard | included | Mage spells, no armor or weapons, fastest study, frail. |
| Class | Monk | included | No armor or weapons, bare-hand damage, high armor class from level, no spells. Class id 12 is unchanged so old saves stay monks. |

Pmoria's race/class mask for the old thirteen classes is unchanged. New professions were added only where they fit the race. Humans and half-elves can take every class.

Shop prices: the owner-versus-player table was 9 by 9 while ten races existed, so a Sage Elf's column was zero and their purchases priced at nothing. The Sage Elf column now follows the Elf column. That is a fix to Petit's table, not a new rule.

## Imoria and ICMoria

Source notes, public only: RogueBasin, "Imoria" (https://roguebasin.com/index.php/Imoria). Developers named there: Kenneth Case, Mary Conner, Robert DeLoura, Dan Flye, Todd Gardiner, Dave Jungck, Andy Walker, Dean Yasuda, University of Washington, 1989, VMS Pascal. Steve Kertes ported it to C as ICMoria (4.85.x, last release 20 August 2003). The RogueBasin page links the C sources. Those sources were not opened for this work, and no Pascal or C from Imoria was copied.

The article's race list, in its own words: human, elf, half-elf, half-orc, dwarf, "and some exotic ones such as dryad." Elves are the best wizards and get extra dexterity. Dryads are the best druids and get extra charisma. Humans are broadly useful. Races are biased toward the classes that suit them.

The article's class list, in its own words: a large AD&D-style set "including, among others, fighters, wizards, thieves, druids, bards, and monks," with different hit dice, melee, spells, and armor limits. Most classes had a guild. The thieves' guild door was hidden as a wall.

A public page that names Phraint, Adventurer, or a second ranger as Imoria roster entries was not found in this pass. Adventurer is included because it is the generalist those "among others" classes are remembered as, and because this game's design asks for it. Phraint is not included until a public roster confirms the name. No insect-person stats were taken from Arduin or from any other game.

| Kind | Name | Status | Notes |
| --- | --- | --- | --- |
| Race | Dryad | included | Clean-room. Str -2, Int +1, Wis +2, Dex +1, Con -1, Chr +3. Hit die 8, +20% experience, 20 feet of infravision, a little cold resistance and a little fire weakness. Quiet and perceptive, poor in a straight fight. Allowed: mage, priest, rogue, ranger, sage, jester, healer, wizard, druid, bard, adventurer. Refused: warrior, paladin, demigod, ninja, monk. |
| Class | Druid | included | Clean-room. Prayers stay on the priest book: healing, food, light, and sensing come early. Turn undead, dispel evil, and holy shield come late and fail more often. Hit-die bonus +3, experience +25%, first spells at level 1. Wisdom is the casting stat. Will not equip metal body armor (`TV_HARD_ARMOR`). Starts with a wooden club and a prayer book. A second, short grove list is optional: see Dual-realm druid below. |
| Class | Bard | included | Clean-room, mage spellbook used as songs. Sleep, confusion, light, detection, and identify come early and fail less. Bolts and balls come later. Word of Destruction is level 32. Charisma and dexterity are the high stats. Experience +25%. First spells at level 2. Armor heavier than 15 pounds is bulky. Starts with a flute. The best instrument in the pack lowers song failure; see Bard instruments below. |
| Class | Adventurer | included | Clean-room generalist. Fighting between rogue and warrior, some stealth, some device skill, a short mage list (missile, light, sleep, cures, a few bolts). Fireball, recharge, and the great spells are unlearnable (level 99 in the table). Experience +35%, first spells at level 4, can wear medium armor (bulky past 30 pounds). |
| Class | Monk | included | Already in Pmoria. Not a second monk. |
| Class | Ranger variations | not adopted | No public source found for a named second ranger. The Umoria ranger remains the bow-and-spell warrior. The druid covers the nature-caster side of that idea. |
| Feature | Guild halls | included | One adventurers' hall, door `7`, placed again each time the town is generated. Identify one item (20 gold plus 5 per level), one rumor, or buy the profession's starting book if it was lost. Warriors, monks, and ninjas are turned away from the book. Druids choose the prayer book or the grove book. |
| Feature | Secret thieves' guild | included | One extra tile, character `#`, not a second district. Rogues, ninjas, bards, and jesters see the guild and enter the same hall. Everyone else sees a stone wall and is told the stone is solid. |
| Feature | Water levels | not adopted | New terrain is a large generator change and slows the dive. |
| Feature | Expanded random town | not adopted | The classic town stays fixed. |
| Feature | Quests | included | Notice board, door `?`. Two jobs: kill 3 to 6 of a small monster that already exists, or bring back rations, torches, oil, or a club. Reward is gold and a little experience. A finished job is replaced. Nothing blocks the stairs to level 50. |
| Feature | Trading post | not adopted | It was a multi-player exchange. This is a single-player game. |
| Feature | Bank and four coin metals | included | Door `8`. The balance is saved. Deposit keeps a tenth as a fee (at least 1). Withdraw is free. Copper, silver, gold, and mithril trade bars are bought at a tenth over face and cashed at a tenth under (50, 200, 1000, 5000). Shops stay priced in gold coins. Coin piles on the floor still become gold. The bars are a new object type so they can be carried. |
| Feature | Bard instruments | included | Flute, lute, and harp. The general store can stock them. A bard starts with a flute. The best instrument in the pack lowers song failure by its own bonus plus the charisma adjustment. No instrument adds 12 to the failure chance. A song that works says which instrument was played. |
| Feature | Heavier food and speed costs | not adopted | Pmoria already has hunger. A quadratic food cost for speed rings would change every class. |
| Feature | Stat-minimum autoroller | not adopted | The existing reroll is enough, and an autoroller invites endless generation. |

Who may be a druid, bard, or adventurer, beyond the notes above:

| Race | Druid | Bard | Adventurer |
| --- | --- | --- | --- |
| Human, Half-Elf | yes | yes | yes |
| Elf, Sage Elf | yes | yes | yes |
| Halfling | yes | yes | yes |
| Gnome, Dryad | yes | yes | yes |
| Half-Orc | no | yes | yes |
| Half-Troll | no | yes | no |
| Avion | no | no | yes |
| Spectre, Dwarf | no | no | no |

## MacMoria and the druid patches

Source notes: MacMoria 5.5d readme (Ross Geck, Ben Schreiber, Maarten Hazewinkel), archived at Beej's Moria page. It is a color Macintosh Umoria 5.5 with an unofficial druid. Save files stayed compatible unless the character was a druid. RogueBasin also lists "Druid UMoria" (a patch, found on FUNet) and "Colour Druid UMoria." A public stat block for that druid was not found.

A player write-up of MacMoria 5.5c (These Old Games, 2019) says that build's druid could cast both clerical and magic-user spells, rather than a separate circle.

| Feature | Status | Notes |
| --- | --- | --- |
| Color map | included | Already in PmoriaPlus. `-m` and `MORIA_MONOCHROME` turn it off. |
| Druid class | included | Prayer book, as above, plus an optional grove book. |
| Dual-realm druid | included | Prayers stay on the original spell mask, so an old druid save still has the same prayers. Grove workings are a second mask in the patch 1 trailer, eight spells, learned with `G` only when the grove book is carried. `m` casts those workings. `p` still prays. Mages skip that book. A druid with no grove book plays as before. |
| Monster list browser | not adopted | A Mac application beside the game, not a dungeon rule. |

## Other Moria variants

This tree no longer contains the DOS, Atari ST, Amiga, Mac Classic, VMS, or IBM PC port sources. The table below is history: what those games added, not a build you can compile from here. Linux, macOS, and the browser build are the ones this repository builds.

| Variant | What it added | Status | Source note |
| --- | --- | --- | --- |
| VMS Moria 4.8 / 5.0 | The original game. Six classes by the Umoria era. | included | Koeneke. Already the ancestor of this tree. |
| PC-Moria, Amiga (CWM) Moria, XMoria, kMoria, Russian Moria | Ports. | not adopted | No new race or class turned up in the public notes used here. |
| BRUCE Moria | Recall, options, and other interface work, merged into Umoria 5.0. Never separately released. | included | Already in Umoria 5.5. Christopher J. Stuart. |
| JAMoria / Purple X | Outdoor map, rivers, several towns, several dungeons. Macintosh, John Magdaleno, 1993. | not adopted | That is a longer game than one town and one pit. |
| Morgul 0.5 | Chris Wilde, 1993, from Umoria 5.5. Targeting. Win by killing the nine Nazgul, in Minas Morgul. | not adopted | Replaces the Balrog goal. |
| Mines of Morgoth | 2002 Windows rewrite. Only a human wizard, stops at level 20, no haggling, tile graphics. | not adopted | Different game, and it drops most of character creation. |
| gmoria | Data files for monsters, items, and races. Up to 32 classes. Gendered titles. Hit points shown as current/maximum. Traps disappear once sprung. | not adopted | GNU project, GPL. No new default race. Would be a clean-room port of ideas only, and it is not needed for a quick dive. |
| BOSS (Beyond Moria) | Robert Gulledge and Jason Black, University of Washington, about 1990-91. Science fiction under New York. Races: Human, Jarwangian (friendly aliens), Daysian (hostile aliens). Classes: Wrestler, Scientist, Guru, Ninja, Soldier, Wanderer, Con-Artist, Thief, Journalist. Several cities, no classic magic, a softer time limit. RogueBasin lists the license as the Moria license. Later ports exist. | not adopted | The sci-fi roster does not belong in a Tolkien-flavored mine. Ninja is already a Pmoria class. Soldier, Thief, and Wanderer overlap warrior, rogue, and adventurer. Guru overlaps monk and sage. No BOSS text or tables were copied. |
| Angband and its children | Dozens of races and classes, many towns, a long dungeon. | not adopted | Out of the design fence on purpose. |

## Omega, borrowed lightly

Omega is not a Moria variant. It is a different roguelike (the Brothers, later maintained by others). Its code is not used here. The ideas to borrow, and only these, are guilds, gods, and quests, cut down so they fit one town and a short game.

| Idea | Status | How it should land |
| --- | --- | --- |
| Guilds | included | One adventurers' hall. Identify, a rumor, or a replacement book. The hidden door is one tile for rogues, ninjas, bards, and jesters. |
| Priesthoods | included | In the temple, `D` dedicates a priest, paladin, druid, or healer. The four patrons are the Forge, the Wild, the Merciful, and the Nameless Deep. Each is one small combat boon and one small ban, recomputed with the other bonuses so they do not stack. The Deep also makes shop prices worse. Demigods are not offered a patron. No Omega names, prayers, or ranks. The choice is permanent. |
| Quests | included | The notice board posts two jobs. Kill counts use monsters already in the list (mouse, kobold, yeek, centipede, orc, worm mass), never the Balrog. Fetch jobs use rations, torches, oil, or a club. |

These three are in the binary. They do not add a town, a dungeon, or a plot.

## Save files

The on-disk character is still version 0.9. `prace` and `pclass` are bytes. New races are indexes 10 (Gnome) and 11 (Dryad). New classes are 13 (Druid), 14 (Bard), and 15 (Adventurer). Indexes 0 through 9 and 0 through 12 are the Pmoria ids, including Monk at 12. A save from PmoriaPlus still loads and still shows the same race and class.

Patch level is 1. The bytes through `birth_date` are the patch 0 layout. After that, a patch 1 save adds a fixed trailer: patron, bank balance, the druid's second spell mask, and two notice-board jobs. A patch 0 save has no trailer and loads with those values empty. A patch 1 save will not load in the phase 1 binary. Score files still use the same record size; an older score file (patch 0) still loads, because only a newer patch than this binary is rejected.

Object indexes were not inserted. The hall, bank, board, hidden door, instruments, trade bars, and grove book are appended after the old object list. The six shops are still the only `store[]` entries, so that part of a save does not grow. The default filename for a new game is `moriasupreme.save` in the home directory, so it does not overwrite a `moria.save` from an older binary. Pass the old path on the command line to open it.
