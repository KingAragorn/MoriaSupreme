/* source/townlife.c: one-town hall, bank, patrons, and notice board

   Copyright (c) 1989-92 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies.

   The services in this file are original to Moria Supreme. No Omega or
   Imoria source was used. */

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

int8u town_patron = PATRON_NONE;
int32 bank_gold = 0;
int32u spell_learned2 = 0;
int32u spell_worked2 = 0;
town_quest_type town_quests[TOWN_QUESTS];
int8u personality = PERS_ORDINARY;
int8u locker_ctr = 0;
inven_type locker[LOCKER_SLOTS];
int8u birth_rand_ego = 0;
int8u birth_ironman = 0;
int8u birth_no_sell = 0;
int8u birth_connected = 0;
int32 ego_seed = 0;
int8u history_ego = 0;
int16 history_ctr = 0;
history_record history_log[HISTORY_MAX];
int8u stair_link = 0;
int8u stair_y = 0;
int8u stair_x = 0;

static spell_type druid_arcane[8] = {
  { 1, 1, 30,  1},
  { 2, 1, 25,  1},
  { 2, 1, 20,  1},
  { 3, 2, 25,  2},
  { 5, 3, 40,  5},
  { 8, 4, 45,  8},
  {10, 5, 50, 10},
  {14, 6, 55, 14}
};

/* Indexes into spell_names[], reused so the name table does not grow. */
static int grove_name_idx[8] = {0, 1, 3, 16, 20, 14, 8, 22};

static char *kill_names[6] = {
  "Giant White Mouse",
  "Kobold",
  "Blue Yeek",
  "Giant Yellow Centipede",
  "Orc",
  "White Worm mass"
};

static int fetch_obj[4] = {351, 372, 373, 54};
static int fetch_need[4] = {2, 2, 1, 1};

static char *rumors[10] = {
  "The stairs in this town still lead down to the same mine.",
  "A dedication to the Forge favors a hard swing and a loose guard.",
  "The Wild mends a hide better than it mends a blade.",
  "The Merciful makes a body harder to cut, and a hand slower to cut.",
  "The Nameless Deep does not bargain. Shops smell it and charge more.",
  "The board posts small jobs: mice, yeeks, rations, torches.",
  "A bard without an instrument is singing into the wind.",
  "The grove book is kept at the hall, not in the magic shop.",
  "Some professions see a door where others see only stone.",
  "The bank will hold coin a pack cannot, and it keeps a fee."
};

static char *pers_names[MAX_PERSONALITY] = {
  "Ordinary", "Mighty", "Shrewd", "Nimble", "Pious", "Patient"
};

static char *wanted_names[6] = {
  "Orc Warrior",
  "Orc Shaman",
  "Skeleton Human",
  "Large Grey Snake",
  "Giant Black Centipede",
  "Jackal"
};

static char *patron_name[5] = {
  "nobody",
  "the Forge",
  "the Wild",
  "the Merciful",
  "the Nameless Deep"
};

static int patron_ok()
{
  switch (py.misc.pclass)
    {
    case CLASS_PRIEST:
    case CLASS_PALADIN:
    case CLASS_DRUID:
    case CLASS_HEALER:
      return TRUE;
    default:
      return FALSE;
    }
}

int town_sneak_p()
{
  switch (py.misc.pclass)
    {
    case CLASS_ROGUE:
    case CLASS_NINJA:
    case CLASS_BARD:
    case CLASS_JESTER:
      return TRUE;
    default:
      return FALSE;
    }
}

void town_clear()
{
  int i;

  town_patron = PATRON_NONE;
  bank_gold = 0;
  spell_learned2 = 0;
  spell_worked2 = 0;
  personality = PERS_ORDINARY;
  riddler_state = 0;
  locker_ctr = 0;
  birth_rand_ego = 0;
  birth_ironman = 0;
  birth_no_sell = 0;
  birth_connected = 0;
  ego_seed = 0;
  history_ego = 0;
  history_ctr = 0;
  stair_link = 0;
  stair_y = 0;
  stair_x = 0;
  for (i = 0; i < HISTORY_MAX; i++)
    history_log[i].note[0] = '\0';
  for (i = 0; i < LOCKER_SLOTS; i++)
    invcopy(&locker[i], OBJ_NOTHING);
  for (i = 0; i < TOWN_QUESTS; i++)
    {
      town_quests[i].kind = QUEST_NONE;
      town_quests[i].target = 0;
      town_quests[i].need = 0;
      town_quests[i].have = 0;
      town_quests[i].done = 0;
    }
}

static int leaving(com)
char com;
{
  return (com == ESCAPE || com == 'e' || com == 'E' || com == CTRL('X'));
}

static int place_ok(y, x)
int y, x;
{
  register cave_type *c_ptr;

  if (y <= 0 || x <= 0 || y >= cur_height - 1 || x >= cur_width - 1)
    return FALSE;
  if (y == char_row && x == char_col)
    return FALSE;
  c_ptr = &cave[y][x];
  if (c_ptr->tptr != 0 || c_ptr->cptr != 0)
    return FALSE;
  if (c_ptr->fval != DARK_FLOOR && c_ptr->fval != CORR_FLOOR)
    return FALSE;
  return TRUE;
}

static int door_present(subval)
int subval;
{
  int y, x, t;

  for (y = 1; y < cur_height - 1; y++)
    for (x = 1; x < cur_width - 1; x++)
      {
	t = cave[y][x].tptr;
	if (t > 0 && t_list[t].tval == TV_STORE_DOOR
	    && t_list[t].subval == subval)
	  return TRUE;
      }
  return FALSE;
}

static void drop_door(y, x, obj)
int y, x, obj;
{
  int cur;
  register cave_type *c_ptr;

  cur = popt();
  c_ptr = &cave[y][x];
  c_ptr->tptr = cur;
  invcopy(&t_list[cur], obj);
  c_ptr->fval = CORR_FLOOR;
  c_ptr->pl = TRUE;
  c_ptr->fm = TRUE;
}

static void place_one(obj, subval, y0, x0, y1, x1)
int obj, subval, y0, x0, y1, x1;
{
  int y, x;

  if (door_present(subval))
    return;
  if (place_ok(y0, x0))
    {
      drop_door(y0, x0, obj);
      return;
    }
  if (place_ok(y1, x1))
    {
      drop_door(y1, x1, obj);
      return;
    }
  for (x = cur_width - 3; x >= cur_width - 12; x--)
    for (y = 1; y < cur_height - 1; y++)
      if (place_ok(y, x))
	{
	  drop_door(y, x, obj);
	  return;
	}
}

void place_town_services()
{
  /* Right-hand edge, off the shop blocks. Town visits rebuild the map,
     so these are placed again each time the town is generated. */
  place_one(OBJ_HALL_DOOR, 107, 1, 60, 1, 58);
  place_one(OBJ_BANK_DOOR, 108, 1, 63, 1, 62);
  place_one(OBJ_BOARD, 109, 20, 60, 20, 58);
  place_one(OBJ_HIDDEN_DOOR, 110, 20, 63, 20, 62);
}

static int give_paid(obj, price)
int obj;
int32 price;
{
  inven_type item;
  bigvtype desc;
  vtype out;
  int loc;

  if (py.misc.au < price)
    {
      msg_print("You do not have the gold.");
      return FALSE;
    }
  invcopy(&item, obj);
  item.number = 1;
  if (!inven_check_num(&item) || !inven_check_weight(&item))
    {
      msg_print("You cannot carry that.");
      return FALSE;
    }
  store_bought(&item);
  loc = inven_carry(&item);
  py.misc.au -= price;
  prt_gold();
  objdes(desc, &inventory[loc], TRUE);
  (void) sprintf(out, "You have %s", desc);
  msg_print(out);
  return TRUE;
}

static int class_book()
{
  switch (py.misc.pclass)
    {
    case CLASS_MAGE:
    case CLASS_ROGUE:
    case CLASS_RANGER:
    case CLASS_JESTER:
    case CLASS_WIZARD:
    case CLASS_BARD:
    case CLASS_ADVENTURER:
      return 318;
    case CLASS_PRIEST:
    case CLASS_PALADIN:
    case CLASS_HEALER:
    case CLASS_DEMIGOD:
    case CLASS_DRUID:
      return 322;
    case CLASS_SAGE:
      return 329;
    default:
      return -1;
    }
}

static void buy_book()
{
  int which;
  char com;
  int32 price;

  which = class_book();
  if (which < 0)
    {
      msg_print("The hall keeps no book for your profession.");
      return;
    }
  if (py.misc.pclass == CLASS_DRUID)
    {
      prt("p) prayer book, or g) grove book?", 0, 0);
      if (!get_com(CNIL, &com))
	return;
      if (com == 'g' || com == 'G')
	which = OBJ_GROVE_BOOK;
      else if (com != 'p' && com != 'P')
	{
	  bell();
	  return;
	}
    }
  price = object_list[which].cost;
  if (price < 1)
    price = 20;
  (void) give_paid(which, price);
}

char *personality_name(which)
int which;
{
  if (which < 0 || which >= MAX_PERSONALITY)
    return pers_names[PERS_ORDINARY];
  return pers_names[which];
}

static void hall_armor()
{
  int i, row;
  vtype line;
  bigvtype name;
  register inven_type *i_ptr;

  clear_screen();
  prt("An armor reading", 1, 28);
  (void) sprintf(line, "Your body, before any gear, is armor %d.",
		 (int)race[py.misc.prace].base_ac);
  prt(line, 3, 4);
  row = 5;
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++)
    {
      i_ptr = &inventory[i];
      if (i_ptr->tval == TV_NOTHING)
	continue;
      if (i_ptr->ac == 0 && i_ptr->toac == 0)
	continue;
      objdes(name, i_ptr, FALSE);
      if ((int)strlen(name) > 36)
	name[36] = '\0';
      if (known2_p(i_ptr))
	(void) sprintf(line, "%s   base %d, %c%d more",
		       name, (int)i_ptr->ac,
		       (i_ptr->toac < 0) ? '-' : '+', abs((int)i_ptr->toac));
      else
	(void) sprintf(line, "%s   base %d, enchantment unread",
		       name, (int)i_ptr->ac);
      prt(line, row, 4);
      row++;
      if (row > 18)
	break;
    }
  if (row == 5)
    prt("Nothing you are wearing adds armor.", 5, 4);
  (void) sprintf(line, "The sheet's total is %d.", (int)py.misc.dis_ac);
  prt(line, 20, 4);
  prt("[Press any key.]", 21, 4);
  (void) inkey();
}

static void hall_screen()
{
  vtype out;
  char *title;

  clear_screen();
  prt("The adventurers' hall", 2, 20);
  title = class[py.misc.pclass].title;
  (void) sprintf(out, "You are %s %s.",
		 is_a_vowel(title[0]) ? "an" : "a", title);
  prt(out, 4, 20);
  prt("i) identify one item     r) hear a rumor", 6, 20);
  prt("b) replace a lost book   a) read your armor", 7, 20);
  prt("e) leave", 8, 20);
}

void town_enter_hall()
{
  int exit_flag;
  char com;
  int fee;
  vtype out;

  exit_flag = FALSE;
  hall_screen();
  do
    {
      if (!get_com("What service?", &com) || leaving(com))
	exit_flag = TRUE;
      else if (com == 'i' || com == 'I')
	{
	  fee = 20 + 5 * py.misc.lev;
	  if (py.misc.au < fee)
	    msg_print("You do not have the gold for that.");
	  else if (ident_spell())
	    {
	      py.misc.au -= fee;
	      prt_gold();
	      (void) sprintf(out, "The hall takes %d gold.", fee);
	      msg_print(out);
	    }
	  hall_screen();
	}
      else if (com == 'r' || com == 'R')
	{
	  msg_print(rumors[randint(10) - 1]);
	}
      else if (com == 'b' || com == 'B')
	{
	  buy_book();
	  hall_screen();
	}
      else if (com == 'a' || com == 'A')
	{
	  hall_armor();
	  hall_screen();
	}
      else
	bell();
    }
  while (!exit_flag);
  draw_cave();
}

static void bank_screen()
{
  vtype out;

  clear_screen();
  prt("The bank", 2, 28);
  (void) sprintf(out, "The bank holds %ld gold in your name.", (long)bank_gold);
  prt(out, 4, 12);
  (void) sprintf(out, "You are carrying %ld gold.", (long)py.misc.au);
  prt(out, 5, 12);
  prt("d) deposit, with a tenth kept as a fee", 7, 12);
  prt("w) withdraw          c) buy a copper bar (50)", 8, 12);
  prt("s) buy a silver bar (200)   g) buy a gold bar (1000)", 9, 12);
  prt("m) buy a mithril bar (5000)", 10, 12);
  prt("C/S/G/M) cash one bar of that metal", 11, 12);
  prt("l) open the locker          e) leave", 12, 12);
  prt("Bars are bought at a tenth over face, cashed at a tenth under.", 14, 12);
}

static int read_amount()
{
  vtype buf;
  int n;

  if (!get_string(buf, 0, 28, 8))
    return 0;
  n = atoi(buf);
  if (n < 1)
    return 0;
  return n;
}

static int cash_one(obj)
int obj;
{
  int i;
  int32 face, pay;
  vtype out;

  for (i = 0; i < inven_ctr; i++)
    if (inventory[i].index == obj)
      {
	face = object_list[obj].cost;
	pay = face - face / 10;
	inven_destroy(i);
	py.misc.au += pay;
	prt_gold();
	(void) sprintf(out, "The bank pays %ld gold.", (long)pay);
	msg_print(out);
	return TRUE;
      }
  msg_print("You are not carrying that bar.");
  return FALSE;
}

static void locker_remove(slot)
int slot;
{
  int j;

  for (j = slot; j < (int)locker_ctr - 1; j++)
    locker[j] = locker[j + 1];
  locker_ctr--;
  invcopy(&locker[locker_ctr], OBJ_NOTHING);
}

static void locker_screen()
{
  int i, row;
  vtype line;
  bigvtype name;

  clear_screen();
  prt("The locker", 1, 32);
  (void) sprintf(line, "It holds %d of %d piles. No fee, and nothing is identified.",
		 (int)locker_ctr, LOCKER_SLOTS);
  prt(line, 3, 4);
  row = 5;
  for (i = 0; i < (int)locker_ctr; i++)
    {
      objdes(name, &locker[i], TRUE);
      if ((int)strlen(name) > 60)
	name[60] = '\0';
      (void) sprintf(line, "%c) %s", 'a' + i, name);
      prt(line, row, 4);
      row++;
    }
  if (locker_ctr == 0)
    prt("The locker is empty.", 5, 4);
  prt("d) leave one item from your pack    w) take one back    e) back", 20, 4);
}

static void locker_deposit()
{
  int item;
  inven_type copy;

  if (locker_ctr >= LOCKER_SLOTS)
    {
      msg_print("The locker has no empty hook.");
      return;
    }
  if (inven_ctr < 1)
    {
      msg_print("Your pack is empty.");
      return;
    }
  if (!get_item(&item, "Leave which item?", 0, inven_ctr - 1, CNIL, CNIL))
    return;
  copy = inventory[item];
  if (copy.number > 1 && copy.subval <= ITEM_SINGLE_STACK_MAX)
    copy.number = 1;
  locker[locker_ctr] = copy;
  locker_ctr++;
  inven_destroy(item);
  msg_print("The locker takes it. It is not identified.");
}

static void locker_withdraw()
{
  char com;
  int slot;

  if (locker_ctr < 1)
    {
      msg_print("The locker is empty.");
      return;
    }
  if (!get_com("Take which item?", &com))
    return;
  slot = com - 'a';
  if (slot < 0 || slot >= (int)locker_ctr)
    {
      bell();
      return;
    }
  if (!inven_check_num(&locker[slot]))
    {
      msg_print("Your pack has no room for that.");
      return;
    }
  (void) inven_carry(&locker[slot]);
  locker_remove(slot);
  py.flags.status |= PY_STR_WGT;
  msg_print("You take it back.");
}

static void locker_use()
{
  int exit_flag;
  char com;

  exit_flag = FALSE;
  locker_screen();
  do
    {
      if (!get_com("Locker?", &com) || leaving(com))
	exit_flag = TRUE;
      else if (com == 'd')
	{
	  locker_deposit();
	  locker_screen();
	}
      else if (com == 'w')
	{
	  locker_withdraw();
	  locker_screen();
	}
      else
	bell();
    }
  while (!exit_flag);
}

void town_enter_bank()
{
  int exit_flag, amount, fee, kept;
  char com;
  vtype out;

  exit_flag = FALSE;
  bank_screen();
  do
    {
      if (!get_com("Banking?", &com) || leaving(com))
	exit_flag = TRUE;
      else if (com == 'd')
	{
	  prt("Deposit how much?", 0, 0);
	  amount = read_amount();
	  fee = amount / 10;
	  if (fee < 1)
	    fee = 1;
	  kept = amount - fee;
	  if (amount < 2 || kept < 1)
	    msg_print("That deposit would not survive the fee.");
	  else if (py.misc.au < amount)
	    msg_print("You do not have that much gold.");
	  else if (bank_gold > 2000000000L - kept)
	    msg_print("The bank will not hold any more for you.");
	  else
	    {
	      py.misc.au -= amount;
	      bank_gold += kept;
	      prt_gold();
	      (void) sprintf(out,
			     "The bank keeps %d and takes a fee of %d.",
			     kept, fee);
	      msg_print(out);
	    }
	  bank_screen();
	}
      else if (com == 'w')
	{
	  prt("Withdraw how much?", 0, 0);
	  amount = read_amount();
	  if (amount < 1)
	    ;
	  else if (bank_gold < amount)
	    msg_print("The bank is not holding that much.");
	  else
	    {
	      bank_gold -= amount;
	      py.misc.au += amount;
	      prt_gold();
	      (void) sprintf(out, "You withdraw %d gold.", amount);
	      msg_print(out);
	    }
	  bank_screen();
	}
      else if (com == 'c')
	(void) give_paid(OBJ_COPPER_BAR, 55);
      else if (com == 's')
	(void) give_paid(OBJ_SILVER_BAR, 220);
      else if (com == 'g')
	(void) give_paid(OBJ_GOLD_BAR, 1100);
      else if (com == 'm')
	(void) give_paid(OBJ_MITHRIL_BAR, 5500);
      else if (com == 'C')
	(void) cash_one(OBJ_COPPER_BAR);
      else if (com == 'S')
	(void) cash_one(OBJ_SILVER_BAR);
      else if (com == 'G')
	(void) cash_one(OBJ_GOLD_BAR);
      else if (com == 'M')
	(void) cash_one(OBJ_MITHRIL_BAR);
      else if (com == 'l')
	locker_use();
      else
	bell();
      if (!exit_flag && (com == 'c' || com == 's' || com == 'g' || com == 'm'
			 || com == 'C' || com == 'S' || com == 'G' || com == 'M'
			 || com == 'l'))
	bank_screen();
    }
  while (!exit_flag);
  draw_cave();
}

static void plain_name(buf, name)
char *buf;
char *name;
{
  char *out, *start;

  out = buf;
  while (*name != '\0')
    {
      if (*name != '&' && *name != '~')
	*out++ = *name;
      name++;
    }
  *out = '\0';
  start = buf;
  while (*start == ' ')
    start++;
  if (start != buf)
    {
      out = buf;
      while (*start != '\0')
	*out++ = *start++;
      *out = '\0';
    }
}

static int creature_by_name(name)
char *name;
{
  int i;

  for (i = 0; i < MAX_CREATURES; i++)
    if (c_list[i].name != CNIL && strcmp(c_list[i].name, name) == 0)
      return i;
  return -1;
}

static int carried(index)
int index;
{
  int i, n;

  n = 0;
  for (i = 0; i < inven_ctr; i++)
    if (inventory[i].index == index)
      n += inventory[i].number;
  return n;
}

static void take_carried(index, need)
int index, need;
{
  int i;

  i = 0;
  while (need > 0 && i < inven_ctr)
    {
      if (inventory[i].index == index)
	{
	  inven_destroy(i);
	  need--;
	}
      else
	i++;
    }
}

static int quest_sane(q)
town_quest_type *q;
{
  if (q->kind == QUEST_KILL || q->kind == QUEST_WANTED)
    return (q->target >= 0 && q->target < MAX_CREATURES
	    && c_list[q->target].name != CNIL && q->need > 0);
  if (q->kind == QUEST_FETCH)
    return (q->target >= 0 && q->target < MAX_OBJECTS && q->need > 0);
  return FALSE;
}

static void post_kill(slot)
int slot;
{
  int guard, id, k;

  for (guard = 0; guard < 12; guard++)
    {
      k = randint(6) - 1;
      id = creature_by_name(kill_names[k]);
      if (id >= 0)
	{
	  town_quests[slot].kind = QUEST_KILL;
	  town_quests[slot].target = id;
	  town_quests[slot].need = (int8u)(2 + randint(4));
	  town_quests[slot].have = 0;
	  town_quests[slot].done = 0;
	  return;
	}
    }
}

static void post_wanted(slot)
int slot;
{
  int guard, id, k;

  for (guard = 0; guard < 12; guard++)
    {
      k = randint(6) - 1;
      id = creature_by_name(wanted_names[k]);
      if (id >= 0)
	{
	  town_quests[slot].kind = QUEST_WANTED;
	  town_quests[slot].target = id;
	  town_quests[slot].need = 1;
	  town_quests[slot].have = 0;
	  town_quests[slot].done = 0;
	  return;
	}
    }
  post_kill(slot);
}

static void post_fetch(slot)
int slot;
{
  int k;

  k = randint(4) - 1;
  town_quests[slot].kind = QUEST_FETCH;
  town_quests[slot].target = fetch_obj[k];
  town_quests[slot].need = (int8u)fetch_need[k];
  town_quests[slot].have = 0;
  town_quests[slot].done = 0;
}

static void fill_quests()
{
  int i;

  for (i = 0; i < TOWN_QUESTS; i++)
    if (!quest_sane(&town_quests[i]))
      town_quests[i].kind = QUEST_NONE;
  if (town_quests[0].kind == QUEST_NONE)
    {
      if (randint(2) == 1)
	post_wanted(0);
      else
	post_kill(0);
    }
  if (town_quests[1].kind == QUEST_NONE)
    post_fetch(1);
}

static void quest_line(buf, q, letter)
char *buf;
town_quest_type *q;
char letter;
{
  vtype name;

  if (q->kind == QUEST_KILL)
    (void) sprintf(buf, "%c) Kill %d %s. %d down.",
		   letter, (int)q->need, c_list[q->target].name, (int)q->have);
  else if (q->kind == QUEST_WANTED)
    (void) sprintf(buf, "%c) Wanted: one %s. %d down.",
		   letter, c_list[q->target].name, (int)q->have);
  else if (q->kind == QUEST_FETCH)
    {
      plain_name(name, object_list[q->target].name);
      (void) sprintf(buf, "%c) Bring %d %s. You have %d.",
		     letter, (int)q->need, name, (int)q->have);
    }
  else
    (void) sprintf(buf, "%c) The nail is empty.", letter);
}

static void refresh_fetch()
{
  int i, n;
  town_quest_type *q;

  for (i = 0; i < TOWN_QUESTS; i++)
    {
      q = &town_quests[i];
      if (q->kind != QUEST_FETCH)
	continue;
      n = carried(q->target);
      if (n > 255)
	n = 255;
      q->have = (int8u)n;
      q->done = (q->have >= q->need) ? 1 : 0;
    }
}

static void pay_quest(q)
town_quest_type *q;
{
  int32 gold;
  vtype out;

  if (q->kind == QUEST_WANTED)
    {
      gold = 250;
      py.misc.exp += 80;
    }
  else
    {
      gold = (int32)q->need * 40 + 50;
      py.misc.exp += (int32)q->need * 15;
    }
  py.misc.au += gold;
  prt_gold();
  (void) sprintf(out, "The board pays you %ld gold.", (long)gold);
  msg_print(out);
  prt_experience();
  if (q->kind == QUEST_WANTED)
    history_add("Finished a wanted notice.");
  else if (q->kind == QUEST_KILL)
    history_add("Finished a kill job.");
  else
    history_add("Finished a fetch job.");
  q->kind = QUEST_NONE;
  fill_quests();
}

static void turn_in(letter)
char letter;
{
  int slot;
  town_quest_type *q;

  slot = (letter == 'a' || letter == 'A') ? 0 : 1;
  q = &town_quests[slot];
  if (q->kind == QUEST_FETCH)
    refresh_fetch();
  if (q->kind == QUEST_NONE || q->have < q->need)
    {
      msg_print("That job is not finished.");
      return;
    }
  if (q->kind == QUEST_FETCH)
    take_carried(q->target, q->need);
  pay_quest(q);
}

static void board_screen()
{
  vtype line;

  refresh_fetch();
  clear_screen();
  prt("The notice board", 2, 24);
  prt("Two jobs are posted. A wanted notice pays more. Stairs stay open.", 4, 4);
  quest_line(line, &town_quests[0], 'a');
  prt(line, 6, 8);
  quest_line(line, &town_quests[1], 'b');
  prt(line, 7, 8);
  prt("a) or b) turn that job in, if it is done    e) leave", 9, 8);
}

void town_enter_board()
{
  int exit_flag;
  char com;

  fill_quests();
  exit_flag = FALSE;
  board_screen();
  do
    {
      if (!get_com("The board?", &com) || leaving(com))
	exit_flag = TRUE;
      else if (com == 'a' || com == 'A' || com == 'b' || com == 'B')
	{
	  turn_in(com);
	  board_screen();
	}
      else
	bell();
    }
  while (!exit_flag);
  draw_cave();
}

void town_try_hidden()
{
  if (town_sneak_p())
    town_enter_hall();
  else
    msg_print("The stone is solid.");
}

void town_note_kill(mon)
int mon;
{
  int i;
  town_quest_type *q;

  for (i = 0; i < TOWN_QUESTS; i++)
    {
      q = &town_quests[i];
      if ((q->kind == QUEST_KILL || q->kind == QUEST_WANTED)
	  && q->target == mon && q->have < q->need)
	{
	  q->have++;
	  if (q->have >= q->need)
	    {
	      q->done = 1;
	      msg_print("The notice board's job is finished.");
	    }
	}
    }
}

void town_temple_notice()
{
  vtype out;

  if (town_patron >= PATRON_FORGE && town_patron <= PATRON_DEEP)
    {
      (void) sprintf(out, "The temple remembers your dedication to %s.",
		     patron_name[town_patron]);
      msg_print(out);
    }
  else if (patron_ok())
    msg_print("The altar will hear a dedication. Press D.");
}

void town_dedicate()
{
  char com;
  int which;
  vtype out;

  if (!patron_ok())
    {
      msg_print("The altar is silent for you.");
      return;
    }
  if (town_patron != PATRON_NONE)
    {
      (void) sprintf(out, "You are already dedicated to %s.",
		     patron_name[town_patron]);
      msg_print(out);
      return;
    }
  save_screen();
  clear_screen();
  prt("Choose one patron. The choice is permanent.", 2, 12);
  prt("f) the Forge. Harder blows, and your guard opens a little.", 4, 8);
  prt("w) the Wild. A tougher hide, and a softer swing.", 5, 8);
  prt("m) the Merciful. Harder to hurt, and you pull your blows.", 6, 8);
  prt("d) the Nameless Deep. Keener aim. Shopkeepers charge more.", 7, 8);
  if (!get_com("Which patron?", &com))
    {
      restore_screen();
      return;
    }
  which = PATRON_NONE;
  if (com == 'f' || com == 'F')
    which = PATRON_FORGE;
  else if (com == 'w' || com == 'W')
    which = PATRON_WILD;
  else if (com == 'm' || com == 'M')
    which = PATRON_MERCY;
  else if (com == 'd' || com == 'D')
    which = PATRON_DEEP;
  restore_screen();
  if (which == PATRON_NONE)
    {
      bell();
      return;
    }
  town_patron = (int8u)which;
  calc_bonuses();
  (void) sprintf(out, "You are dedicated to %s.", patron_name[which]);
  msg_print(out);
  (void) sprintf(out, "Dedicated to %s.", patron_name[which]);
  history_add(out);
}

int town_bard_fail()
{
  int i, best;

  best = 0;
  for (i = 0; i < inven_ctr; i++)
    if (inventory[i].tval == TV_INSTRUMENT && inventory[i].p1 > best)
      best = inventory[i].p1;
  if (best <= 0)
    return 12;
  return -(best + stat_adj(A_CHR));
}

void town_bard_played()
{
  int i, best, which;
  vtype out;

  best = -1;
  which = -1;
  for (i = 0; i < inven_ctr; i++)
    if (inventory[i].tval == TV_INSTRUMENT && (int)inventory[i].p1 > best)
      {
	best = inventory[i].p1;
	which = i;
      }
  if (which >= 0)
    {
      (void) sprintf(out, "You play the %s.",
		     object_list[inventory[which].index].name);
      msg_print(out);
    }
}

static int carrying_grove()
{
  int i;

  for (i = 0; i < inven_ctr; i++)
    if (inventory[i].tval == TV_MAGIC_BOOK
	&& inventory[i].subval == GROVE_BOOK_SUBVAL)
      return TRUE;
  return FALSE;
}

static int grove_chance(spell)
int spell;
{
  register spell_type *s_ptr;
  int chance;

  s_ptr = &druid_arcane[spell];
  chance = s_ptr->sfail - 4 * (py.misc.lev - s_ptr->slevel);
  chance -= 2 * (stat_adj(A_WIS) - 1);
  if (s_ptr->smana > py.misc.cmana)
    chance += 5 * (s_ptr->smana - py.misc.cmana);
  if (too_bulky())
    chance += too_bulky();
  if (chance > 95)
    chance = 95;
  else if (chance < 5)
    chance = 5;
  return chance;
}

int town_grove_offer()
{
  int spells[8], n, i, j;
  char query;
  vtype out;

  if (py.misc.pclass != CLASS_DRUID || py.flags.new_spells == 0)
    return FALSE;
  if (!carrying_grove())
    return FALSE;
  n = 0;
  for (i = 0; i < 8; i++)
    if ((spell_learned2 & (1L << i)) == 0
	&& druid_arcane[i].slevel <= py.misc.lev)
      spells[n++] = i;
  if (n == 0)
    return FALSE;
  if (!get_check("Learn a grove working instead of a prayer?"))
    return FALSE;
  save_screen();
  for (i = 0; i < n; i++)
    {
      j = spells[i];
      (void) sprintf(out, "  %c) %-24s %2d %4d %3d%%",
		     'a' + i, spell_names[grove_name_idx[j]],
		     druid_arcane[j].slevel, druid_arcane[j].smana,
		     grove_chance(j));
      prt(out, i + 1, 15);
    }
  while (get_com("Learn which working?", &query))
    {
      j = query - 'a';
      if (j >= 0 && j < n)
	{
	  i = spells[j];
	  spell_learned2 |= 1L << i;
	  restore_screen();
	  (void) sprintf(out, "You have learned the working of %s.",
			 spell_names[grove_name_idx[i]]);
	  msg_print(out);
	  py.flags.new_spells--;
	  if (py.flags.new_spells == 0)
	    py.flags.status |= PY_STUDY;
	  if (py.misc.mana == 0)
	    calc_mana(A_WIS, 0);
	  return TRUE;
	}
      bell();
    }
  restore_screen();
  return TRUE;
}

static void cast_grove_effect(choice)
int choice;
{
  int dir;

  switch (choice)
    {
    case 0:
      if (get_dir(CNIL, &dir))
	fire_bolt(GF_MAGIC_MISSILE, dir, char_row, char_col,
		  damroll(2, 8), spell_names[0]);
      break;
    case 1:
      (void) detect_monsters();
      break;
    case 2:
      (void) light_area(char_row, char_col);
      break;
    case 3:
      create_food();
      break;
    case 4:
      (void) ident_spell();
      break;
    case 5:
      if (get_dir(CNIL, &dir))
	fire_bolt(GF_FROST, dir, char_row, char_col,
		  damroll(6, 8), spell_names[14]);
      break;
    case 6:
      if (get_dir(CNIL, &dir))
	fire_bolt(GF_LIGHTNING, dir, char_row, char_col,
		  damroll(4, 8), spell_names[8]);
      break;
    case 7:
      if (get_dir(CNIL, &dir))
	fire_bolt(GF_FIRE, dir, char_row, char_col,
		  damroll(7, 8), spell_names[22]);
      break;
    default:
      break;
    }
}

void town_cast_grove()
{
  int i, j, item_val, choice, chance, n, k;
  int list[8];
  char com;
  register spell_type *s_ptr;
  vtype out;

  free_turn_flag = TRUE;
  if (!find_range(TV_MAGIC_BOOK, TV_NEVER, &i, &j))
    {
      msg_print("You are not carrying the grove book.");
      return;
    }
  if (!get_item(&item_val, "Use which book?", i, j, CNIL, CNIL))
    return;
  if (inventory[item_val].subval != GROVE_BOOK_SUBVAL)
    {
      msg_print("That book is not from the grove.");
      return;
    }
  n = 0;
  for (i = 0; i < 8; i++)
    if ((spell_learned2 & (1L << i)) != 0
	&& druid_arcane[i].slevel <= py.misc.lev)
      list[n++] = i;
  if (n == 0)
    {
      msg_print("You don't know any workings in that book.");
      return;
    }
  save_screen();
  for (i = 0; i < n; i++)
    {
      k = list[i];
      (void) sprintf(out, "  %c) %-24s %2d %4d %3d%%",
		     'a' + i, spell_names[grove_name_idx[k]],
		     druid_arcane[k].slevel, druid_arcane[k].smana,
		     grove_chance(k));
      prt(out, i + 1, 15);
    }
  if (!get_com("Cast which working?", &com))
    {
      restore_screen();
      return;
    }
  i = com - 'a';
  restore_screen();
  if (i < 0 || i >= n)
    {
      bell();
      return;
    }
  choice = list[i];
  chance = grove_chance(choice);
  s_ptr = &druid_arcane[choice];
  if (s_ptr->smana > py.misc.cmana)
    {
      if (!get_check("The grove may find you presumptuous. Confirm?"))
	return;
    }
  free_turn_flag = FALSE;
  if (randint(100) < chance)
    msg_print("You failed to get the working off!");
  else
    cast_grove_effect(choice);
  if (!free_turn_flag)
    {
      if ((spell_worked2 & (1L << choice)) == 0)
	{
	  py.misc.exp += s_ptr->sexp << 2;
	  spell_worked2 |= 1L << choice;
	  prt_experience();
	}
      if (s_ptr->smana > py.misc.cmana)
	{
	  msg_print("You faint from the effort!");
	  py.flags.paralysis =
	    randint((int)(5 * (s_ptr->smana - py.misc.cmana)));
	  py.misc.cmana = 0;
	  py.misc.cmana_frac = 0;
	}
      else
      py.misc.cmana -= s_ptr->smana;
      prt_cmana();
    }
}

/* The history is a short log of big moments. Notes are written here, not
   copied from another game. A full log drops the oldest line. */
void history_add(text)
char *text;
{
  int i, n;
  history_record *h;

  if (text == CNIL || text[0] == '\0')
    return;
  if (history_ctr >= HISTORY_MAX)
    {
      for (i = 1; i < HISTORY_MAX; i++)
	history_log[i - 1] = history_log[i];
      history_ctr = HISTORY_MAX - 1;
    }
  h = &history_log[history_ctr];
  h->turn = turn;
  h->depth = (int16)dun_level;
  n = 0;
  while (text[n] != '\0' && n < HISTORY_NOTE - 1)
    {
      h->note[n] = text[n];
      n++;
    }
  h->note[n] = '\0';
  history_ctr++;
}

/* Classic Moria has no named artifact list. An ego item is a wearable
   with one of the special names (HA), (DF), of Slay X, and the rest. */
void note_first_ego(i_ptr)
inven_type *i_ptr;
{
  int n;

  if (history_ego)
    return;
  n = (int)i_ptr->name2;
  if (n <= SN_NULL || n >= SN_ARRAY_SIZE)
    return;
  if (n >= SN_EMPTY && n <= SN_UNLOCKED)
    return;
  history_ego = 1;
  history_add("Found a first ego item.");
}

int challenge_flags()
{
  int f;

  f = 0;
  if (birth_rand_ego)
    f |= CHAL_EGO;
  if (birth_ironman)
    f |= CHAL_IRON;
  if (birth_no_sell)
    f |= CHAL_SELL;
  if (birth_connected)
    f |= CHAL_STAIR;
  return f;
}

static void append_piece(buf, piece, any)
char *buf;
char *piece;
int *any;
{
  char *d;

  d = buf;
  while (*d != '\0')
    d++;
  if (*any)
    {
      if (d - buf < 76)
	{
	  *d++ = ',';
	  *d++ = ' ';
	}
    }
  while (*piece != '\0' && (d - buf) < 78)
    *d++ = *piece++;
  *d = '\0';
  *any = 1;
}

char *challenge_text()
{
  static char buf[80];
  int any;

  buf[0] = '\0';
  any = 0;
  if (birth_ironman)
    append_piece(buf, "ironman stairs", &any);
  if (birth_no_sell)
    append_piece(buf, "no selling", &any);
  if (birth_connected)
    append_piece(buf, "connected stairs", &any);
  if (birth_rand_ego)
    append_piece(buf, "randomized egos", &any);
  if (!any)
    {
      buf[0] = 'n';
      buf[1] = 'o';
      buf[2] = 'n';
      buf[3] = 'e';
      buf[4] = '\0';
    }
  return buf;
}

/* for_death draws over the screen and leaves it there. Otherwise the
   dungeon screen is put back when the player is done reading. */
void history_show(for_death)
int for_death;
{
  int i, line, top, d;
  char buf[82], where[8];

  if (history_ctr <= 0 && !for_death)
    {
      msg_print("No history has been written yet.");
      return;
    }
  if (!for_death)
    save_screen();
  top = 0;
  do
    {
      clear_screen();
      prt("Character history", 0, 30);
      if (history_ctr <= 0)
	{
	  prt("No history was written.", 2, 2);
	  prt("[Press any key]", 23, 30);
	  (void) inkey();
	  break;
	}
      line = 2;
      for (i = top; i < (int)history_ctr && line < 22; i++)
	{
	  d = (int)history_log[i].depth;
	  if (d <= 0)
	    {
	      where[0] = 't';
	      where[1] = 'o';
	      where[2] = 'w';
	      where[3] = 'n';
	      where[4] = '\0';
	    }
	  else
	    (void) sprintf(where, "%d", d);
	  (void) sprintf(buf, "%6ld  %-5s  %.60s",
			 (long)history_log[i].turn, where, history_log[i].note);
	  prt(buf, line, 1);
	  line++;
	}
      if (i < (int)history_ctr)
	prt("-- more --", 23, 32);
      else
	prt("[Press any key]", 23, 30);
      if (inkey() == ESCAPE)
	break;
      if (i < (int)history_ctr)
	top = i;
      else
	break;
    }
  while (TRUE);
  if (!for_death)
    restore_screen();
}
