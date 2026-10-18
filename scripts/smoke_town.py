#!/usr/bin/env python3
"""Walk the phase-2 town: hall, bank, patron, hidden door, and an old save.

Drives build/moriasupreme on a pseudoterminal. A phase-1 binary, if passed
with --phase1, is used only to write a patch-0 save for the new binary to load.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_characters import (  # noqa: E402
    COLS,
    ROWS,
    Session,
    clone_screen,
    render_png,
)

MAP_COLS = range(13, COLS)
MAP_ROWS = range(1, 23)
DIRS = ((-1, 0, "k"), (1, 0, "j"), (0, -1, "h"), (0, 1, "l"))


class Town(Session):
    def __init__(self, binary, home, new_game=True):
        self.home = home
        if new_game:
            # A retry must not inherit a save. A new game will not overwrite it.
            leftover = os.path.join(home, "moriasupreme.save")
            if os.path.exists(leftover):
                os.remove(leftover)
            # Session always passes -n.
            super().__init__(binary, home)
        else:
            self._spawn([binary])

    def _spawn(self, argv):
        import fcntl
        import pty
        import struct
        import subprocess
        import termios

        import pyte

        self.screen = pyte.Screen(COLS, ROWS)
        self.stream = pyte.Stream(self.screen)
        self.master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
        env = os.environ.copy()
        env["TERM"] = "xterm-256color"
        env["ESCDELAY"] = "10"
        env["HOME"] = self.home if hasattr(self, "home") else env.get("HOME", "/tmp")
        env["COLUMNS"] = str(COLS)
        env["LINES"] = str(ROWS)
        env.pop("MORIA_MONOCHROME", None)
        self.proc = subprocess.Popen(
            argv, stdin=slave, stdout=slave, stderr=slave, env=env, close_fds=True
        )
        os.close(slave)

    def clear_more(self):
        for _ in range(4):
            text = self.text()
            if "-more-" in text or "Press any key" in text:
                self.send(" ")
                self.pump(0.2)
            else:
                return


CHALLENGE_WORDS = (
    "randomized egos",
    "ironman stairs",
    "no selling",
    "connected stairs",
)


TEMPERAMENT_NAMES = {
    "a": "Ordinary",
    "b": "Mighty",
    "c": "Shrewd",
    "d": "Nimble",
    "e": "Pious",
    "f": "Patient",
}


def answer_challenges(session, answers="nnnn"):
    prompts = ("Randomized ego", "Ironman stairs", "No selling", "Connected stairs")
    for prompt, ans in zip(prompts, answers):
        if not session.wait_for(prompt):
            return prompt + " prompt missing\n" + session.text()
        session.send(ans)
    return None


def create(session, race_letter, class_letter, name, temperament="a", shots=None, shot_name=None, challenges="nnnn"):
    if not session.wait_for("Press any key to continue", 12):
        return "news missing\n" + session.text()
    session.send(" ")
    if not session.wait_for("Choose a race"):
        return "race prompt missing\n" + session.text()
    session.send(race_letter)
    if not session.wait_for("Choose a sex"):
        return "sex prompt missing\n" + session.text()
    session.send("m")
    if not session.wait_for("reroll"):
        return "stat prompt missing\n" + session.text()
    session.send("\033")
    if not session.wait_for("Choose a class"):
        return "class prompt missing\n" + session.text()
    session.send(class_letter)
    if not session.wait_for("temperament"):
        return "temperament prompt missing\n" + session.text()
    session.send(temperament)
    err = answer_challenges(session, challenges)
    if err:
        return err
    if not session.wait_for("player's name"):
        return "name prompt missing\n" + session.text()
    session.send(name + "\r")
    if not session.wait_for("or Q to exit"):
        return "sheet missing\n" + session.text()
    expected = TEMPERAMENT_NAMES.get(temperament, "Ordinary")
    lines = session.text().splitlines()
    if len(lines) < 9 or expected not in lines[6]:
        return "sheet temperament is not %s\n%s" % (expected, session.text())
    sheet_chal = lines[8]
    if challenges == "nnnn":
        if "none" not in sheet_chal:
            return "sheet challenges are not off\n" + session.text()
    else:
        for bit, word in zip(challenges, CHALLENGE_WORDS):
            if bit == "y" and word not in sheet_chal:
                return "sheet missing %s\n%s" % (word, session.text())
            if bit != "y" and word in sheet_chal:
                return "sheet unexpectedly has %s\n%s" % (word, session.text())
    if shots is not None and shot_name:
        shots[shot_name] = clone_screen(session.screen)
    session.send(" ")
    # A caster may be told they can learn a prayer before the map is drawn.
    # That -more- sits on the sheet, so the map's @ is not up yet.
    import time

    end = time.time() + 8
    while time.time() < end:
        session.pump(0.12)
        text = session.text()
        if "-more-" in text:
            session.send(" ")
            continue
        if player_at(session) is not None:
            session.clear_more()
            return None
        if session.proc.poll() is not None:
            break
    return "map missing\n" + session.text()


def save_and_quit(session):
    session.send("\033\x18")
    import time

    end = time.time() + 20
    while time.time() < end and session.proc.poll() is None:
        session.pump(0.15)
        text = session.text()
        if "Delete old savefile" in text or "File exists" in text:
            session.send("y")
        elif "-more-" in text or "Press any key" in text:
            session.send(" ")
    if session.proc.poll() is None:
        return "did not save\n" + session.text()
    return None


def cell(session, y, x):
    if y < 0 or x < 0 or y >= ROWS or x >= COLS:
        return " "
    data = session.screen.buffer[y][x].data or " "
    return data[0]


def find_char(session, ch):
    found = []
    for y in MAP_ROWS:
        for x in MAP_COLS:
            if cell(session, y, x) == ch:
                found.append((y, x))
    return found


def player_at(session):
    found = find_char(session, "@")
    return found[0] if found else None


def walkable(ch):
    # Stairs are legal floor, but stepping on '>' leaves town.
    return ch in ".@"


def is_dead(session):
    text = session.text()
    return "killed by" in text or "Character record?" in text


def step(session, key, timeout=2.5):
    import time

    session.clear_more()
    if is_dead(session):
        return "dead"
    start = player_at(session)
    if start is None:
        return "dead" if is_dead(session) else "stuck"
    session.send(key)
    end = time.time() + timeout
    while time.time() < end:
        session.pump(0.08)
        session.clear_more()
        if is_dead(session):
            return "dead"
        now = player_at(session)
        text = session.text()
        if "The bank" in text or "adventurers' hall" in text or "notice board" in text:
            return "menu"
        if "The stone is solid" in text:
            return "solid"
        if "Asking Price" in text or "Press D" in text or "dedication" in text:
            return "store"
        if now is not None and now != start:
            return "moved"
    return "stuck"


def path_to_adjacent(session, target):
    start = player_at(session)
    if start is None:
        return None
    ty, tx = target
    # Step onto the target from a neighboring floor tile.
    goals = []
    for dy, dx, _key in DIRS:
        ny, nx = ty + dy, tx + dx
        if 1 <= ny < ROWS and 13 <= nx < COLS and walkable(cell(session, ny, nx)):
            goals.append((ny, nx))
    if start in goals or start == target:
        return []
    if not goals:
        return None
    want = set(goals)
    prev = {start: None}
    queue = [start]
    found = None
    while queue:
        y, x = queue.pop(0)
        if (y, x) in want:
            found = (y, x)
            break
        for dy, dx, key in DIRS:
            ny, nx = y + dy, x + dx
            if (ny, nx) in prev:
                continue
            if not (1 <= ny < ROWS and 13 <= nx < COLS):
                continue
            if not walkable(cell(session, ny, nx)):
                continue
            prev[(ny, nx)] = (y, x, key)
            queue.append((ny, nx))
    if found is None:
        return None
    keys = []
    cur = found
    while prev[cur] is not None:
        y, x, key = prev[cur]
        keys.append(key)
        cur = (y, x)
    keys.reverse()
    return keys


def go_to(session, target):
    """Walk next to target, then step onto it. Replan when a townsperson blocks."""
    session.clear_more()
    for _ in range(6):
        if is_dead(session):
            return "dead"
        pos = player_at(session)
        if pos is None:
            return "dead" if is_dead(session) else "lost the player\n" + session.text()
        for dy, dx, key in DIRS:
            if (pos[0] + dy, pos[1] + dx) == target:
                return step(session, key)
        keys = path_to_adjacent(session, target)
        if not keys:
            return "no path to %s\n%s" % (target, session.text())
        blocked = False
        for key in keys:
            result = step(session, key)
            if result in ("menu", "store", "solid", "dead"):
                return result
            if result != "moved":
                blocked = True
                break
            pos = player_at(session)
            if pos is not None:
                for dy, dx, dirkey in DIRS:
                    if (pos[0] + dy, pos[1] + dx) == target:
                        return step(session, dirkey)
        if not blocked:
            continue
        # A beggar stepped into the square. Sidestep onto open floor and look again.
        sidestepped = False
        pos = player_at(session)
        for dy, dx, key in DIRS:
            if pos is None:
                break
            ny, nx = pos[0] + dy, pos[1] + dx
            if cell(session, ny, nx) not in ".@":
                continue
            result = step(session, key)
            if result in ("menu", "store", "solid", "dead"):
                return result
            if result == "moved":
                sidestepped = True
                break
        if not sidestepped:
            return "stuck walking toward %s\n%s" % (target, session.text())
    return "could not reach %s\n%s" % (target, session.text())


def uncover(session):
    """Step onto open floor so a door hidden under @ can be seen."""
    pos = player_at(session)
    if pos is None:
        return
    for dy, dx, key in DIRS:
        ny, nx = pos[0] + dy, pos[1] + dx
        if cell(session, ny, nx) == ".":
            step(session, key)
            return


def approach(session, ch):
    session.clear_more()
    uncover(session)
    spots = find_char(session, ch)
    if not spots:
        return "no %r on the map\n%s" % (ch, session.text())
    return go_to(session, spots[0])


def leave_menu(session):
    session.send("\033")
    session.pump(0.4)
    if player_at(session) is None and "@" not in session.text():
        session.send("e")
        session.pump(0.3)
        session.send("\033")
        session.pump(0.3)


def run_priest(binary, home, shots):
    session = Town(binary, home, new_game=True)
    session.home = home
    try:
        err = create(session, "a", "c", "Priestly")
        if err:
            return err
        for ch in ("7", "8", "?"):
            if not find_char(session, ch):
                return "town is missing %s\n%s" % (ch, session.text())
        if shots is not None:
            shots["town"] = clone_screen(session.screen)
        entered = approach(session, "8")
        if entered != "menu":
            return "bank entry was %s\n%s" % (entered, session.text())
        if "The bank holds" not in session.text():
            return "bank screen missing\n" + session.text()
        if shots is not None:
            shots["bank"] = clone_screen(session.screen)
        session.send("d")
        if not session.wait_for("Deposit", 4):
            return "deposit prompt missing\n" + session.text()
        session.send("20\r")
        session.pump(0.6)
        if "holds 18" not in session.text() and "keeps 18" not in session.text():
            return "deposit did not keep 18\n" + session.text()
        if shots is not None and "holds 18" in session.text():
            shots["bank"] = clone_screen(session.screen)
        leave_menu(session)
        if not session.wait_for("@", 4):
            return "did not return from the bank\n" + session.text()
        entered = approach(session, "4")
        if entered not in ("store", "menu"):
            return "temple entry was %s\n%s" % (entered, session.text())
        session.send("D")
        if not session.wait_for("Which patron", 4):
            return "patron menu missing\n" + session.text()
        if shots is not None:
            shots["patron"] = clone_screen(session.screen)
        session.send("f")
        if not session.wait_for("dedicated to the Forge", 4):
            return "dedication missing\n" + session.text()
        leave_menu(session)
        err = save_and_quit(session)
        if err:
            return err
    finally:
        session.close()

    # Reload the patch 1 save and confirm the trailer survived.
    session = Town(binary, home, new_game=False)
    session.home = home
    try:
        if not session.wait_for("Press any key to continue", 12):
            return "reload news missing\n" + session.text()
        session.send(" ")
        if not session.wait_for("Priest", 8):
            return "reload sheet missing priest\n" + session.text()
        session.send(" ")
        if not session.wait_for("@", 8):
            return "reload map missing\n" + session.text()
        entered = approach(session, "4")
        if entered not in ("store", "menu"):
            return "reload temple was %s\n%s" % (entered, session.text())
        if "dedication to the Forge" not in session.text():
            return "patron was not saved\n" + session.text()
        leave_menu(session)
        if not session.wait_for("@", 4):
            return "did not leave the temple\n" + session.text()
        entered = approach(session, "8")
        if entered != "menu":
            return "reload bank was %s\n%s" % (entered, session.text())
        if "holds 18" not in session.text():
            return "bank balance was not saved\n" + session.text()
        leave_menu(session)
        err = save_and_quit(session)
        if err:
            return err
    finally:
        session.close()
    return None


def run_bard(binary, home):
    session = Town(binary, home, new_game=True)
    session.home = home
    try:
        err = create(session, "a", "o", "Songbird")
        if err:
            return err
        session.send("i")
        if not session.wait_for("flute", 4):
            return "bard is not carrying a flute\n" + session.text()
        session.send("\033")
        session.pump(0.3)
        return save_and_quit(session)
    finally:
        session.close()


def run_druid(binary, home):
    session = Town(binary, home, new_game=True)
    session.home = home
    try:
        # Dryad is race index 11, letter 'l'. Druid is class index 13.
        # Dryad's legal-class letter for druid is not 'n' if some classes
        # are refused. smoke_characters.class_letter handles that.
        from smoke_characters import RACES, class_letter

        letter = class_letter(RACES[11][1], 13)
        err = create(session, "l", letter, "Grove")
        if err:
            return err
        session.send("i")
        if not session.wait_for("Prayer", 4) and "Handbook" not in session.text():
            return "druid prayer book missing\n" + session.text()
        session.send("\033")
        session.pump(0.4)
        if not session.wait_for("@", 4):
            return "inventory did not close\n" + session.text()
        entered = approach(session, "7")
        if entered != "menu":
            return "hall entry was %s\n%s" % (entered, session.text())
        session.send("b")
        if not session.wait_for("grove book", 4):
            return "druid book choice missing\n" + session.text()
        session.send("g")
        session.pump(0.6)
        leave_menu(session)
        if not session.wait_for("@", 4):
            return "did not leave the hall\n" + session.text()
        session.send("i")
        if not session.wait_for("Grove", 4):
            return "grove book was not bought\n" + session.text()
        session.send("\033")
        session.pump(0.3)
        return save_and_quit(session)
    finally:
        session.close()


def run_hidden(binary, home, class_letter, name, expect_hall):
    session = Town(binary, home, new_game=True)
    session.home = home
    try:
        err = create(session, "a", class_letter, name)
        if err:
            return err
        # The hidden door is drawn as stone a few columns right of the
        # notice board, on the same town row. Cave (20, 63) is screen (21, 76).
        target = None
        for by, bx in find_char(session, "?"):
            for pos in ((by, bx + 3), (by, bx + 2), (by, bx + 4), (by, bx + 1)):
                if cell(session, pos[0], pos[1]) == "#":
                    target = pos
                    break
            if target is not None:
                break
        if target is None:
            for pos in ((21, 76), (21, 75), (21, 74)):
                if cell(session, pos[0], pos[1]) == "#":
                    target = pos
                    break
        if target is None:
            return "hidden door tile is not stone\n" + session.text()
        stepped = go_to(session, target)
        if stepped == "dead":
            return "died before the hidden door\n" + session.text()
        if expect_hall:
            if stepped != "menu" and "adventurers' hall" not in session.text():
                return "sneak was not admitted (%s)\n%s" % (stepped, session.text())
        else:
            if "The stone is solid" not in session.text():
                return "outsider was not refused (%s)\n%s" % (stepped, session.text())
        if "adventurers' hall" in session.text() or "What service" in session.text():
            leave_menu(session)
        return save_and_quit(session)
    finally:
        session.close()


def run_hengband(binary, home, shots):
    """Temperament, junk, spectre refusal, armor reading, locker, wanted board."""
    session = Town(binary, home, new_game=True)
    session.home = home
    try:
        err = create(session, "a", "a", "Mighty", temperament="b", shots=shots, shot_name="temperament")
        if err:
            return err
        session.send("}")
        if not session.wait_for("marked junk", 4):
            return "junk command missing\n" + session.text()
        session.send("]")
        if not session.wait_for("Only a spectre", 4):
            return "spectre command missing\n" + session.text()
        entered = approach(session, "7")
        if entered != "menu":
            return "hall entry was %s\n%s" % (entered, session.text())
        session.send("a")
        if not session.wait_for("armor reading", 4):
            return "armor reading missing\n" + session.text()
        if shots is not None:
            shots["armor"] = clone_screen(session.screen)
        session.send(" ")
        session.pump(0.4)
        if "What service" not in session.text() and "adventurers' hall" not in session.text():
            return "armor reading did not return\n" + session.text()
        leave_menu(session)
        if not session.wait_for("@", 4):
            return "did not leave the hall\n" + session.text()
        entered = approach(session, "8")
        if entered != "menu":
            return "bank entry was %s\n%s" % (entered, session.text())
        session.send("l")
        if not session.wait_for("The locker", 4):
            return "locker missing\n" + session.text()
        if "empty" not in session.text():
            return "new locker was not empty\n" + session.text()
        if shots is not None:
            shots["locker"] = clone_screen(session.screen)
        session.send("e")
        session.pump(0.4)
        leave_menu(session)
        if not session.wait_for("@", 4):
            return "did not leave the bank\n" + session.text()
        entered = approach(session, "?")
        if entered != "menu":
            return "board entry was %s\n%s" % (entered, session.text())
        text = session.text()
        if "Wanted:" not in text and "Kill " not in text:
            return "board has neither a wanted notice nor a kill job\n" + text
        if "Bring " not in text:
            return "board is missing the fetch job\n" + text
        if shots is not None:
            shots["board"] = clone_screen(session.screen)
        leave_menu(session)
        return save_and_quit(session)
    finally:
        session.close()


def run_patch1_save(binary, home, save_src):
    import shutil

    os.makedirs(home, exist_ok=True)
    shutil.copy(save_src, os.path.join(home, "moriasupreme.save"))
    os.chmod(os.path.join(home, "moriasupreme.save"), 0o600)
    session = Town(binary, home, new_game=False)
    session.home = home
    try:
        if not session.wait_for("Press any key to continue", 12):
            return "patch1 news missing\n" + session.text()
        session.send(" ")
        if not session.wait_for("Warrior", 8):
            return "patch1 sheet missing the warrior\n" + session.text()
        text = session.text()
        if "Choose a race" in text:
            return "patch1 save started a new character\n" + text
        lines = text.splitlines()
        if len(lines) < 7 or "Ordinary" not in lines[6]:
            return "patch1 save is not Ordinary\n" + text
        session.send(" ")
        if not session.wait_for("@", 8):
            return "patch1 map missing\n" + session.text()
        entered = approach(session, "8")
        if entered != "menu":
            return "patch1 bank was %s\n%s" % (entered, session.text())
        session.send("l")
        if not session.wait_for("The locker", 4):
            return "patch1 locker missing\n" + session.text()
        if "empty" not in session.text():
            return "patch1 locker was not empty\n" + session.text()
        session.send("e")
        session.pump(0.3)
        leave_menu(session)
        return save_and_quit(session)
    finally:
        session.close()


def run_phase1_save(phase1, phase2, home):
    session = Town(phase1, home, new_game=True)
    session.home = home
    try:
        err = create(session, "a", "a", "Oldsave")
        if err:
            return "phase1 create: " + err
        err = save_and_quit(session)
        if err:
            return "phase1 save: " + err
    finally:
        session.close()

    session = Town(phase2, home, new_game=False)
    session.home = home
    try:
        if not session.wait_for("Press any key to continue", 12):
            return "phase1 reload news missing\n" + session.text()
        session.send(" ")
        if not session.wait_for("Warrior", 8):
            return "phase1 save did not restore the warrior\n" + session.text()
        session.send(" ")
        if not session.wait_for("@", 8):
            return "phase1 reload map missing\n" + session.text()
        if not find_char(session, "7"):
            return "old town save did not gain a hall\n" + session.text()
        entered = approach(session, "8")
        if entered != "menu" or "holds 0" not in session.text():
            return "old save bank was %s\n%s" % (entered, session.text())
        leave_menu(session)
        return save_and_quit(session)
    finally:
        session.close()


def main():
    binary = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "build/moriasupreme")
    phase1 = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 and sys.argv[2] else ""
    shot_dir = sys.argv[3] if len(sys.argv) > 3 else ""
    home_root = "/tmp/moriasupreme-town-%d" % os.getpid()
    os.makedirs(home_root, exist_ok=True)
    shots = {} if shot_dir else None
    checks = [
        ("hengband", lambda: run_hengband(binary, os.path.join(home_root, "heng"), shots)),
        ("priest", lambda: run_priest(binary, os.path.join(home_root, "priest"), shots)),
        ("bard", lambda: run_bard(binary, os.path.join(home_root, "bard"))),
        ("druid", lambda: run_druid(binary, os.path.join(home_root, "druid"))),
        ("rogue-door", lambda: run_hidden(binary, os.path.join(home_root, "rogue"), "d", "Sneak", True)),
        ("warrior-door", lambda: run_hidden(binary, os.path.join(home_root, "warrior"), "a", "Solid", False)),
    ]
    if phase1:
        checks.append(
            ("phase1-save", lambda: run_phase1_save(phase1, binary, os.path.join(home_root, "old")))
        )
    failed = False
    for sub in ("heng", "priest", "bard", "druid", "rogue", "warrior", "old"):
        os.makedirs(os.path.join(home_root, sub), exist_ok=True)
    for name, fn in checks:
        print("run %s" % name, flush=True)
        err = None
        tries = 3 if name != "bard" else 1
        for attempt in range(tries):
            err = fn()
            if not err:
                break
            if "killed" not in err and "died" not in err and "dead" not in err and "stuck" not in err:
                break
            print("retry %s (%d)" % (name, attempt + 2), flush=True)
        if err:
            failed = True
            print("FAIL %s" % name)
            print(err)
        else:
            print("ok   %s" % name, flush=True)
    if shots and shot_dir:
        os.makedirs(shot_dir, exist_ok=True)
        for key, screen in shots.items():
            render_png(screen, os.path.join(shot_dir, "town-%s.png" % key))
            print("shot town-%s.png" % key)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
