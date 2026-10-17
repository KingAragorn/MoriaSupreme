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
| Class | Druid | included | Clean-room, one prayer book. Healing, food, light, and sensing come early. Turn undead, dispel evil, and holy shield come late and fail more often. Hit-die bonus +3, experience +25%, first spells at level 1. Wisdom is the casting stat, as for every priest-realm class. Will not equip metal body armor (`TV_HARD_ARMOR`). Leather and robes are allowed. Soft armor heavier than 20 pounds is called bulky, same warning other classes get. Starts with a wooden club and a prayer book. |
| Class | Bard | included | Clean-room, mage spellbook used as songs. Sleep, confusion, light, detection, and identify come early and fail less. Bolts and balls come later. Word of Destruction is level 32. Charisma and dexterity are the high stats. Experience +25%. First spells at level 2. Armor heavier than 15 pounds is bulky, so they live in leather. Not a second jester: the jester stays the even, comic mage. |
| Class | Adventurer | included | Clean-room generalist. Fighting between rogue and warrior, some stealth, some device skill, a short mage list (missile, light, sleep, cures, a few bolts). Fireball, recharge, and the great spells are unlearnable (level 99 in the table). Experience +35%, first spells at level 4, can wear medium armor (bulky past 30 pounds). |
| Class | Monk | included | Already in Pmoria. Not a second monk. |
| Class | Ranger variations | not adopted | No public source found for a named second ranger. The Umoria ranger remains the bow-and-spell warrior. The druid covers the nature-caster side of that idea. |
| Feature | Guild halls | planned | See Omega, below. One hall in the existing town, not a second town. |
| Feature | Secret thieves' guild | planned | A door that looks like a wall, only for rogues and nearby sneaks (ninja, bard). Part of the guild pass. |
| Feature | Water levels | not adopted | New terrain is a large generator change and slows the dive. |
| Feature | Expanded random town | not adopted | The classic town stays fixed. |
| Feature | Quests | planned | See below. |
| Feature | Trading post | not adopted | It was a multi-player exchange. This is a single-player game. |
| Feature | Bank and four coin metals | planned | Copper, silver, gold, and mithril, with a bank that changes them for a fee. Gold in the pack is enough for a quick game, so this waits. Shops stay priced in gold. |
| Feature | Bard instruments | planned | The article says bards play instruments and that charisma matters to that. This bard uses the mage book and a charisma bonus on the character, not a new item class. |
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
| Druid class | included | Single prayer book, as above. Not both books at once. |
| Dual-realm druid | planned | Spell knowledge is one 32-bit mask in the save file. A second book needs a save change. Deferred so current saves stay valid. |
| Monster list browser | not adopted | A Mac application beside the game, not a dungeon rule. |

## Other Moria variants

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
| Guilds | planned | One adventurers' hall on the town map, with a wing or a counter for each profession. Services stay small: identify one item, hear a rumor, buy the class's starting book if you lost it. The wrong profession is turned away. Rogues and ninjas can have the hidden door Imoria used, as a single extra tile, not a new district. |
| Priesthoods | planned | At the temple, a priest, paladin, druid, or healer picks one patron. Three or four original patrons are enough (for example a god of the forge, a god of the wild, a god of mercy, a nameless deep). Each gives one small boon and one small ban. No Omega deity names, prayers, or rank titles. Demigods can be left godless so they stay their own thing. |
| Quests | planned | A notice board in town with two jobs at a time: kill a number of one monster kind that already exists, or bring back an object kind the shops already sell. Reward is gold and a little experience. No multi-stage plot, no second dungeon, no quest that blocks the stairs to level 50. |

These three are the next phase. They are not in the binary yet.

## Save files

The on-disk character is still Pmoria's version 0.9, patch 0. `prace` and `pclass` are bytes. New races are indexes 10 (Gnome) and 11 (Dryad). New classes are 13 (Druid), 14 (Bard), and 15 (Adventurer). Indexes 0 through 9 and 0 through 12 are the Pmoria ids, including Monk at 12. A save from PmoriaPlus still loads and still shows the same race and class. The default filename for a new game is `moriasupreme.save` in the home directory, so it does not overwrite a `moria.save` from an older binary. Pass the old path on the command line to open it.
