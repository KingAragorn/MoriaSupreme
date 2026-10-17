# Ideas from Hengband

Hengband is the long Japanese branch of Zangband: many races and classes, a personality on top of race and class, pets, town buildings, and a wilderness of towns and side dungeons that ends on the Serpent of Chaos. That is an Angband-length game. Moria Supreme is still one town and a dive to the Balrog on level 50.

This note is a shopping list, not a plan to build. Nothing here is implemented. No Hengband source was opened. No tables, spell lists, personality modifiers, monster names, or item text were copied. If any of these ideas is built later, the numbers and the wording are written for this game.

## What the public descriptions say

Used only as descriptions of how the game plays:

- RogueBasin, "Hengband" (https://www.roguebasin.com/index.php/Hengband). Names it as the variant of Japanese Zangband, maintained for years, later rewritten in C++.
- The project's English getting-started note (`readme-eng.md` on the public project site). Character creation asks for sex, race, class, and a personality, then birth options. The suggested first character is an ordinary dwarf warrior.
- A public English play guide for the 3.0.2 series (https://ruzzoli.de/roguelikes/hengband/help.html). It lists personalities, the wilderness and themed dungeons, quests, an arena, a casino, a hunter's office with bounties, a museum, the Mogaminator auto-picker, and a command for pets. Classes it names beyond the classic set include Samurai, Mirror-Master, Beastmaster, Cavalry, Smith, Sniper, Blue-Mage, Force-Trainer, Berserker, and Elementalist.
- The public score board's personality ranking (Hengband score pages). Names people actually play include Mighty, Ordinary, Combat, Fearless, Shrewd, Nimble, Lucky, Pious, Patient, and Lazy, plus a few joke entries. That list is a roster, not a stat table. This game would not reuse those modifiers.
- The game's own version notes, read as a changelog of player-facing changes, not as source. They mention personalities at birth, wielding two weapons, a town service that explains your armor, a home that stores items, auto-destroy after an item is identified, and a cure-mutation service.

## How to read the estimates

Effort is the size of the change, not a schedule.

- **Small.** A few functions in files that already exist. No new screen and no new save section.
- **Medium.** A new choice at birth, or a new counter in the town, plus a short field on the save.
- **Large.** The equipment layout or a follower that has to survive saves. Easy to get wrong.

Saves today are version 0.9. Patch 0 has no town trailer and still loads. Patch 1 appends the patron, the bank balance, the druid's second spell list, and two notice-board jobs. Anything new that must be remembered goes in a patch 2 trailer after that. Older saves load with the new field empty. Race and class numbers do not move.

## Worth doing

### Personality at birth

Hengband's distinctive extra choice. After race and class you pick a temperament. It nudges two stats and one habit. It is not a new class and it does not add a spell book.

A short original set is enough: Ordinary (no change), Mighty (stronger, clumsier), Shrewd (better devices and search, frailer), Nimble (dexterity and stealth, lighter hits), Pious (easier prayers, worse with weapons), Patient (rest and food last, slower to anger). Lucky-as-a-flat-bonus and the joke personalities (a character who starts overpowered, or one who is lazy on purpose) do not belong in a short scored game.

**Effort.** Medium. The birth screen, the character sheet, and one or two checks (device use, rest, or to-hit).

**Saves.** One byte after the patch 1 trailer, written only at patch 2. Patch 0 and patch 1 load as Ordinary. New games write the byte. Race and class ids stay where they are.

### Wanted notices

The hunter's office pays for a named kind of monster. This town already has a notice board and two jobs (`QUEST_KILL` and `QUEST_FETCH`). A bounty is the kill job aimed at a creature that is already in the monster list, paid in gold at the board, and never required before the stairs.

**Effort.** Small if a bounty is one of the two jobs the board already stores. Medium if a bounty sits beside both jobs at once.

**Saves.** None, if it uses one of the two quest records already in the patch 1 trailer (kind, target, need, have, done). A third job at the same time is another quest record and needs patch 2. Old saves keep the two jobs they have.

### A feeling for unknown items

Warriors in this game cannot cast identify, and a short dive does not hand out many scrolls. Hengband (like other Angband-family games) lets you sense an unknown weapon or armor: average, good, or bad, without naming the enchantment. Full identify still comes from scrolls and spells.

**Effort.** Small. One message when the item is picked up or worn, from the to-hit, to-dam, and curse flags the item record already has.

**Saves.** None if the feeling is computed again whenever you look, and is not stored as a separate fact. If the game must remember "you already sensed this," that is one bit on the item. Old items that lack the bit simply have not been sensed. Do not add a trailer for it.

### A locker at the bank

Hengband's home holds a character's spare gear in town. This game has a bank and no cupboard, so every spare sword rides in the pack. A locker of about a dozen items, in the bank, with no haggling and no fee beyond the deposit fee the bank already charges for gold, is the whole feature. It is not a second shop and it does not identify what you leave there.

**Effort.** Medium. A small screen to put an item in and take it out, and a fixed array, not a growing pack.

**Saves.** The items have to be in the save or they disappear. Patch 2 appends a count and that many item records after the patch 1 trailer. Older saves have a count of zero. The worn-equipment array does not grow.

### An armor reading at the hall

One of Hengband's town services tells you what your armor is actually doing. The hall can do the same in one screen: base armor, what each worn piece adds, and the total. No new item and no new stat.

**Effort.** Small. One prompt and the numbers the character sheet already uses.

**Saves.** None.

## Small, and safe to add beside those

### Mark junk, then destroy it

The Mogaminator is a rules editor for what to pick up and what to throw away. The part that fits here is the mark. Inscriptions are already stored on each item. A single agreed word, or the existing inscription command, means "this is junk," and one command destroys every marked item in the pack. No rule language, no editor, no auto-destroy of things you have not marked.

**Effort.** Small.

**Saves.** None. The inscription is already part of the item record.

### The Spectre spends a charge

Hengband lets some undead races eat the charges in a wand or staff. This game already has a Spectre. One original use: spend a charge from a wand you are holding and gain a little food or a little mana. It does not learn the wand's spell and it does not work for any other race.

**Effort.** Small.

**Saves.** None. Charges are already stored on the wand.

### Ask before the stairs when you are nearly dead

Hengband added warnings around stairs and one-way quest levels. This dungeon has ordinary stairs. The useful scrap is a question when you step on `>` or `<` at very low hit points. Yes goes through. Anything else stays put.

**Effort.** Small.

**Saves.** None.

### One more loaf at the general store

The public notes add a travel food to the general store. Here that is one new ration, sold beside the food that is already there, filling more hunger than a ration and costing more. It is not a new shop.

**Effort.** Small. Append one object the way the grove book was appended.

**Saves.** Same rule as the other appended objects. Old saves do not contain it. New games can buy it. Object indexes only grow at the end.

## Looked at, and a poor fit

### A hired sword for one descent

Beastmaster and Cavalry command pets, and Cavalry rides them. A pet that follows you through the whole game, or a mount, is a second character. The only version that stays small: pay the hall, a weak ally walks with you on the current level, and it is gone when you take the stairs. It must also be gone when you save, unless the monster record already has a place for "this one is friendly," which it does not. Adding that flag to every monster is a save-format change for the whole level. Dismiss on save and on the stairs, or do not build it.

**Effort.** Medium, and it grows the moment the ally is allowed to live in the save.

**Saves.** None only if the ally is never written. Persisting it is Large.

### Two weapons

The version notes say a character can wield two weapons. That means a new equipment slot, a new line on the sheet, blow calculations, and curses on the off hand. An unused slot reused for this is the only way that stays compatible. Growing the equipment array is how old saves start reading the wrong item.

**Effort.** Large.

**Saves.** Dangerous if the equipment array changes size. Not a candidate for the next patch.

### Mutations

The public mutation notes are a long comic list: breath, horns, random teleport, eating rock, and worse. A cure service exists because the list is mostly trouble. Two or three serious boons would still need a bitfield, a way to gain one, and a way to lose one. The tone fights a classic Moria dive.

**Effort.** Medium for a short list.

**Saves.** A bitfield in the patch 2 trailer. Not recommended.

### A dice game at the inn

The casino is a town building. Gold is already saved, so a bet would not need a new field. A stake that can wipe a level-1 purse also shortens the game in the wrong way. Skip it.

## Out of scope

These are the parts that make Hengband a long game. They stay out.

- A wilderness, more than one town, or a side dungeon with its own depth and boss.
- A win condition other than the Balrog on level 50, including the Serpent of Chaos.
- A new class (Samurai, Blue-Mage, Smith, Mirror-Master, Sniper, Force-Trainer, Berserker, Elementalist, Cavalry).
- Crafting essences into weapons.
- Learning a monster's spell by being hit by it.
- Riding, or a pet that lasts more than one level.
- The full auto-picker, with a rule editor and destroy-on-identify.
- A museum, a second map, or auto-explore that walks the level for you.

## First choices, if one of these is built

1. **Personality.** The idea that is actually Hengband's, and it fits birth without a new class.
2. **Wanted notices** on the board that is already there.
3. **Item feelings**, so a warrior can tell a bad sword from a good one before the first identify scroll.
4. **The bank locker**, so the pack is for the dive and the town holds the spare.
5. **The armor reading**, because it is a screen of numbers the game already has.

The junk mark, the Spectre's charge, the stair question, and the extra loaf can ride along with any of those. They do not need a save bump.
