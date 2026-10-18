/* source/hobbit.c: a few encounters and items in the existing dungeon

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies.

   The creatures, riddles, and item text in this file are original to
   Moria Supreme. No passage from The Hobbit was copied. */

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include <stdlib.h>
#include <string.h>

int8u riddler_state = 0;

static int blade_glow = 0;
static int hoard_noted = 0;

static int in_light();

static int env_set(name)
char *name;
{
  char *v;

  v = getenv(name);
  return (v != 0 && v[0] != '\0' && v[0] != '0');
}

static int open_floor(y, x)
int y, x;
{
  if (y <= 0 || x <= 0 || y >= cur_height - 1 || x >= cur_width - 1)
    return FALSE;
  if (cave[y][x].fval > MAX_OPEN_SPACE)
    return FALSE;
  if (cave[y][x].cptr != 0 || cave[y][x].tptr != 0)
    return FALSE;
  return TRUE;
}

static int place_beside(kind)
int kind;
{
  int dy, dx, y, x;

  for (dy = -1; dy <= 1; dy++)
    for (dx = -1; dx <= 1; dx++)
      {
	if (dy == 0 && dx == 0)
	  continue;
	y = char_row + dy;
	x = char_col + dx;
	if (open_floor(y, x))
	  return place_monster(y, x, kind, FALSE);
      }
  return FALSE;
}

static int find_open(y, x)
int *y, *x;
{
  int i, yy, xx;

  for (i = 0; i < 60; i++)
    {
      yy = randint(cur_height - 2);
      xx = randint(cur_width - 2);
      if (open_floor(yy, xx)
	  && distance(yy, xx, char_row, char_col) > 8)
	{
	  *y = yy;
	  *x = xx;
	  return TRUE;
	}
    }
  return FALSE;
}

static void place_near(cy, cx, kind, slp)
int cy, cx, kind, slp;
{
  int dy, dx, y, x, i;

  for (i = 0; i < 12; i++)
    {
      dy = randint(3) - 2;
      dx = randint(3) - 2;
      y = cy + dy;
      x = cx + dx;
      if (open_floor(y, x))
	{
	  (void) place_monster(y, x, kind, slp);
	  return;
	}
    }
}

static void place_pack(kind, extra, boss)
int kind, extra, boss;
{
  int y, x, n, i;

  if (!find_open(&y, &x))
    return;
  if (boss >= 0)
    (void) place_monster(y, x, boss, TRUE);
  else
    (void) place_monster(y, x, kind, TRUE);
  n = extra;
  for (i = 0; i < n; i++)
    place_near(y, x, kind, TRUE);
}

static void heap_gold(y, x, always)
int y, x, always;
{
  int dy, dx, yy, xx;

  for (dy = -1; dy <= 1; dy++)
    for (dx = -1; dx <= 1; dx++)
      {
	if (dy == 0 && dx == 0)
	  continue;
	yy = y + dy;
	xx = x + dx;
	if (!open_floor(yy, xx))
	  continue;
	if (always || randint(2) == 1)
	  place_gold(yy, xx);
      }
}

static int dragon_index(level)
int level;
{
  int i, best, best_gap, gap;

  best = -1;
  best_gap = 999;
  for (i = 0; i < MON_TABLE_END; i++)
    {
      if (c_list[i].cchar != 'd' && c_list[i].cchar != 'D')
	continue;
      if (c_list[i].cmove & CM_WIN)
	continue;
      if (c_list[i].level > MAX_MONS_LEVEL)
	continue;
      gap = c_list[i].level - level;
      if (gap < 0)
	gap = -gap;
      if (gap < best_gap)
	{
	  best_gap = gap;
	  best = i;
	}
    }
  return best;
}

static void dress_hoard()
{
  int i, y, x, which;
  monster_type *m_ptr;

  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->hp < 0)
	continue;
      if (c_list[m_ptr->mptr].cchar != 'd'
	  && c_list[m_ptr->mptr].cchar != 'D')
	continue;
      heap_gold((int)m_ptr->fy, (int)m_ptr->fx, FALSE);
      return;
    }
  which = dragon_index(dun_level);
  if (which < 0 || !find_open(&y, &x))
    return;
  if (place_monster(y, x, which, TRUE))
    heap_gold(y, x, FALSE);
}

static void force_hoard()
{
  int y, x, which;

  heap_gold(char_row, char_col, TRUE);
  which = dragon_index(30);
  if (which < 0)
    return;
  if (find_open(&y, &x))
    (void) place_monster(y, x, which, TRUE);
}

/* A new character carries a weapon but has not wielded it yet. */
static void brand_weapon()
{
  int i;
  inven_type obj;
  inven_type *w;

  w = &inventory[INVEN_WIELD];
  if (w->tval < TV_HAFTED || w->tval > TV_SWORD)
    {
      for (i = 0; i < inven_ctr; i++)
	if (inventory[i].tval >= TV_HAFTED && inventory[i].tval <= TV_SWORD)
	  {
	    obj = inventory[i];
	    obj.number = 1;
	    inven_destroy(i);
	    *w = obj;
	    equip_ctr++;
	    break;
	  }
    }
  if (w->tval < TV_HAFTED || w->tval > TV_SWORD)
    return;
  w->name2 = SN_ORC_WARNING;
  w->tohit += 2;
  w->todam += 2;
  known2(w);
  py_bonuses(w, 1);
  calc_bonuses();
}

/* Shallow packs, one riddler, and a deep hoard. The ordinary monster
   mix is untouched. */
void hobbit_dress_level()
{
  blade_glow = 0;
  hoard_noted = 0;

  if (env_set("MORIA_FORCE_RIDDLE") && riddler_state == 0)
    (void) place_beside(MON_PALE_RIDDLER);
  if (env_set("MORIA_FORCE_ORCS"))
    (void) place_beside(MON_GOBLIN);
  if (env_set("MORIA_FORCE_TROLL"))
    (void) place_beside(MON_STONE_TROLL);
  if (env_set("MORIA_FORCE_BLADE"))
    brand_weapon();
  if (env_set("MORIA_FORCE_HOARD"))
    force_hoard();

  if (dun_level <= 0)
    return;

  if (dun_level <= 8 && randint(4) == 1)
    place_pack(MON_GOBLIN, 2 + randint(3), MON_GOBLIN_CAPTAIN);
  if (dun_level >= 5 && dun_level <= 14 && randint(5) == 1)
    place_pack(MON_WARG, 1 + randint(2), -1);
  if (dun_level >= 8 && dun_level <= 18 && randint(4) == 1)
    place_pack(MON_WOOD_SPIDER, randint(2), -1);
  if (dun_level >= 16 && dun_level <= 30 && randint(6) == 1)
    {
      int y, x;
      if (find_open(&y, &x))
	(void) place_monster(y, x, MON_STONE_TROLL, TRUE);
    }
  if (riddler_state == 0 && dun_level >= 10 && dun_level <= 22
      && randint(3) == 1)
    {
      int y, x;
      if (find_open(&y, &x))
	(void) place_monster(y, x, MON_PALE_RIDDLER, FALSE);
    }
  if (dun_level >= 30 && randint(5) == 1)
    dress_hoard();
}

static int in_light(m_ptr)
monster_type *m_ptr;
{
  /* Day on the surface. turn/5000 odd is night, matching generate.c. */
  if (dun_level == 0)
    return ((turn / 5000) & 1) == 0;
  if (!player_light)
    return FALSE;
  return cave[m_ptr->fy][m_ptr->fx].tl != 0;
}

/* 0 still moves, 1 stiffened, 2 gone. Called only when the creature is awake. */
static int stone_tick(monptr)
int monptr;
{
  monster_type *m_ptr;
  creature_type *c_ptr;
  int day, stiff, dam, seen;
  vtype msg;

  m_ptr = &m_list[monptr];
  c_ptr = &c_list[m_ptr->mptr];
  if ((c_ptr->cdefense & CD_LIGHTBANE) == 0)
    return 0;
  if (!in_light(m_ptr))
    return 0;
  day = (dun_level == 0);
  stiff = day || (randint(2) == 1);
  if (!stiff)
    return 0;
  dam = day ? randint(6) + 2 : randint(3);
  seen = m_ptr->ml;
  c_recall[m_ptr->mptr].r_cdefense |= CD_LIGHTBANE;
  if (seen && m_ptr->hp > dam)
    {
      (void) sprintf(msg, "Light crawls over the %s, and its hide cracks.",
		     c_ptr->name);
      msg_print(msg);
    }
  hack_monptr = monptr;
  if (mon_take_hit(monptr, dam) >= 0)
    {
      hack_monptr = -1;
      if (seen)
	{
	  (void) sprintf(msg, "The %s freezes into a cracked statue.",
			 c_ptr->name);
	  msg_print(msg);
	}
      return 2;
    }
  hack_monptr = -1;
  m_ptr->csleep = 2;
  return 1;
}

static void fold_word(dst, src)
char *dst;
char *src;
{
  while (*src == ' ')
    src++;
  if (strncmp(src, "a ", 2) == 0)
    src += 2;
  else if (strncmp(src, "an ", 3) == 0)
    src += 3;
  else if (strncmp(src, "the ", 4) == 0)
    src += 4;
  while (*src && *src != ' ' && *src != '\n' && *src != '\r')
    {
      char c;
      c = *src++;
      if (c >= 'A' && c <= 'Z')
	c = (char)(c - 'A' + 'a');
      *dst++ = c;
    }
  *dst = '\0';
}

static void give_ring()
{
  inven_type ring;

  invcopy(&ring, OBJ_RING_INVIS);
  identify(&ring);
  known2(&ring);
  if (inven_ctr >= INVEN_WIELD)
    {
      msg_print("Your pack is full. The ring falls at your feet.");
      if (cave[char_row][char_col].tptr == 0)
	{
	  int pos;
	  pos = popt();
	  cave[char_row][char_col].tptr = pos;
	  t_list[pos] = ring;
	}
    }
  else
    (void) inven_carry(&ring);
}

/* Ask once. Returns TRUE if the creature should still attack. */
static int ask_riddle(monptr)
int monptr;
{
  static char *text[3] = {
    "The pale thing croaks: A neck, no head, and a cap. What am I?",
    "The pale thing croaks: Take from me and I grow. What am I?",
    "The pale thing croaks: I run downhill but never walk. What am I?"
  };
  static char *need[3] = { "bottle", "hole", "river" };
  char buf[80], got[80];
  int which;

  which = randint(3) - 1;
  msg_print(text[which]);
  /* Hold the riddle on the message line until it has been read. */
  msg_print(CNIL);
  prt("Answer? ", MSG_LINE, 0);
  if (!get_string(buf, MSG_LINE, 8, 24))
    {
      erase_line(MSG_LINE, 0);
      msg_print("The pale thing waits, grinning.");
      return TRUE;
    }
  erase_line(MSG_LINE, 0);
  fold_word(got, buf);
  if (strcmp(got, need[which]) == 0)
    {
      msg_print("It drops a plain gold ring of invisibility and slips away.");
      give_ring();
      riddler_state = 2;
      hack_monptr = monptr;
      fix1_delete_monster(monptr);
      hack_monptr = -1;
      return FALSE;
    }
  riddler_state = 1;
  msg_print("The pale thing hisses and leaps.");
  return TRUE;
}

int hobbit_before_move(monptr)
int monptr;
{
  monster_type *m_ptr;
  int stone;

  m_ptr = &m_list[monptr];
  stone = stone_tick(monptr);
  if (stone == 2)
    return FALSE;
  if (stone == 1)
    return FALSE;
  if (m_ptr->hp < 0)
    return FALSE;
  if (m_ptr->mptr == MON_PALE_RIDDLER && riddler_state == 0
      && m_ptr->cdis <= 1)
    {
      if (!ask_riddle(monptr))
	return FALSE;
    }
  return TRUE;
}

static int orc_near()
{
  int i;
  monster_type *m_ptr;

  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->hp < 0)
	continue;
      if (m_ptr->cdis > 12)
	continue;
      if ((c_list[m_ptr->mptr].cdefense & CD_ORC) == 0)
	continue;
      if (los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	return TRUE;
    }
  return FALSE;
}

static int hoard_seen()
{
  int i, y, x, dy, dx;
  monster_type *m_ptr;

  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->hp < 0 || !m_ptr->ml)
	continue;
      if (c_list[m_ptr->mptr].cchar != 'd'
	  && c_list[m_ptr->mptr].cchar != 'D')
	continue;
      for (dy = -2; dy <= 2; dy++)
	for (dx = -2; dx <= 2; dx++)
	  {
	    y = (int)m_ptr->fy + dy;
	    x = (int)m_ptr->fx + dx;
	    if (y < 0 || x < 0 || y >= cur_height || x >= cur_width)
	      continue;
	    if (cave[y][x].tptr == 0)
	      continue;
	    if (t_list[cave[y][x].tptr].tval == TV_GOLD)
	      return TRUE;
	  }
    }
  return FALSE;
}

void hobbit_after_turn()
{
  inven_type *w;
  int warned, glow;

  if (death)
    return;
  w = &inventory[INVEN_WIELD];
  warned = (w->tval != TV_NOTHING && w->name2 == SN_ORC_WARNING);
  glow = warned && orc_near();
  if (glow && !blade_glow)
    msg_print("Your blade glows with a cold blue light.");
  else if (!glow && blade_glow)
    msg_print("The blue light in your blade fades.");
  blade_glow = glow;

  if (!hoard_noted && hoard_seen())
    {
      hoard_noted = 1;
      msg_print("Gold is heaped here, scorched and piled deep.");
    }
}
