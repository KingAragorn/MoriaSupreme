#!/usr/bin/env python3
"""Drive the hobbit-tale encounters and check that a patch 2 save still loads.

MORIA_FORCE_* places the encounter beside the character in town so the
test does not have to hunt the dungeon. Those variables are not a player
feature.
"""

import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_characters import (  # noqa: E402
    COLS,
    ROWS,
    Session,
    clone_screen,
    render_png,
)


class Game(Session):
    def __init__(self, binary, home, new_game=True, extra_env=None):
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
        env["HOME"] = home
        env["COLUMNS"] = str(COLS)
        env["LINES"] = str(ROWS)
        env.pop("MORIA_MONOCHROME", None)
        if extra_env:
            env.update(extra_env)
        argv = [binary, "-n"] if new_game else [binary]
        self.proc = subprocess.Popen(
            argv, stdin=slave, stdout=slave, stderr=slave, env=env, close_fds=True
        )
        os.close(slave)

    def clear_prompts(self, rounds=6):
        for _ in range(rounds):
            text = self.text()
            if "-more-" in text or "Press any key" in text:
                self.send(" ")
                self.pump(0.15)
            else:
                return


def birth_halfling(game, shots=None):
    if not game.wait_for("Press any key to continue"):
        return False, "news screen did not appear"
    game.send(" ")
    if not game.wait_for("Choose a race"):
        return False, "race prompt missing"
    game.send("d")  # Halfling
    if not game.wait_for("Choose a sex"):
        return False, "sex prompt missing"
    game.send("m")
    if not game.wait_for("reroll"):
        return False, "stat prompt missing"
    game.send("\033")
    if not game.wait_for("Choose a class"):
        return False, "class prompt missing"
    game.send("a")  # Warrior
    if not game.wait_for("temperament"):
        return False, "temperament prompt missing"
    game.send("a")
    if not game.wait_for("player's name"):
        return False, "name prompt missing"
    game.send("\r")
    if not game.wait_for("or Q to exit", timeout=12):
        return False, "character sheet did not finish"
    if shots is not None:
        shots["sheet"] = clone_screen(game.screen)
    game.send(" ")
    end = time.time() + 8
    while time.time() < end:
        game.pump(0.12)
        text = game.text()
        if "-more-" in text or "or Q to exit" in text:
            game.send(" ")
            continue
        if "@" in text:
            return True, "in town"
    return False, "never reached the map\n" + game.text()


def answer_for(text):
    low = text.lower()
    if "neck" in low:
        return "bottle"
    if "grow" in low:
        return "hole"
    if "downhill" in low or "never walk" in low:
        return "river"
    return None


def play_riddle(binary, home, shots):
    game = Game(binary, home, extra_env={"MORIA_FORCE_RIDDLE": "1"})
    try:
        ok, why = birth_halfling(game, shots)
        if not ok:
            return False, why
        # One rest. Rogue-like keys: '.' stays a turn. The riddler is beside us.
        game.send(".")
        end = time.time() + 8
        answer = None
        while time.time() < end and answer is None:
            game.pump(0.15)
            text = game.text()
            answer = answer_for(text)
            if answer is None and "-more-" in text:
                game.send(" ")
        if answer is None:
            return False, "riddle did not appear\n" + game.text()
        shots["riddle"] = clone_screen(game.screen)
        if "-more-" in game.text():
            game.send(" ")
            game.pump(0.3)
        if "Answer?" not in game.text():
            game.wait_for("Answer?", timeout=4)
        game.send(answer + "\r")
        if not game.wait_for("invisibility", timeout=6):
            return False, "ring was not given\n" + game.text()
        shots["ring"] = clone_screen(game.screen)
        return True, "riddle answered with " + answer
    finally:
        game.close()


def play_glow(binary, home, shots):
    game = Game(
        binary,
        home,
        extra_env={"MORIA_FORCE_ORCS": "1", "MORIA_FORCE_BLADE": "1"},
    )
    try:
        ok, why = birth_halfling(game)
        if not ok:
            return False, why
        game.send(".")
        end = time.time() + 8
        seen = False
        while time.time() < end:
            game.pump(0.15)
            text = game.text()
            if "cold blue light" in text:
                seen = True
                break
            if "-more-" in text:
                game.send(" ")
        if not seen:
            return False, "blade did not glow\n" + game.text()
        shots["glow"] = clone_screen(game.screen)
        return True, "blade glowed"
    finally:
        game.close()


def play_troll(binary, home, shots):
    game = Game(binary, home, extra_env={"MORIA_FORCE_TROLL": "1"})
    try:
        ok, why = birth_halfling(game)
        if not ok:
            return False, why
        game.send(".")
        end = time.time() + 8
        seen = False
        while time.time() < end:
            game.pump(0.15)
            text = game.text()
            if "hide cracks" in text or "cracked statue" in text:
                seen = True
                break
            if "-more-" in text:
                game.send(" ")
        if not seen:
            return False, "stone troll did not stiffen\n" + game.text()
        shots["troll"] = clone_screen(game.screen)
        return True, "stone troll stiffened in daylight"
    finally:
        game.close()


def play_hoard(binary, home, shots):
    game = Game(binary, home, extra_env={"MORIA_FORCE_HOARD": "1"})
    try:
        ok, why = birth_halfling(game)
        if not ok:
            return False, why
        game.pump(0.4)
        text = game.text()
        if "$" not in text:
            return False, "no gold pile in view\n" + text
        shots["hoard"] = clone_screen(game.screen)
        return True, "gold heaped beside the character"
    finally:
        game.close()


def save_with(binary, home):
    """New game on `binary`, then ^X so HOME holds moriasupreme.save."""
    game = Game(binary, home, new_game=True)
    try:
        ok, why = birth_halfling(game)
        if not ok:
            return False, why
        game.send("\033\x18")
        end = time.time() + 20
        while time.time() < end and game.proc.poll() is None:
            game.pump(0.15)
            text = game.text()
            if "Delete old savefile" in text:
                game.send("y")
            elif "-more-" in text or "Press any key" in text:
                game.send(" ")
        if game.proc.poll() is None:
            return False, "did not save\n" + game.text()
        path = os.path.join(home, "moriasupreme.save")
        if not os.path.exists(path):
            return False, "save file missing"
        return True, path
    finally:
        game.close()


def load_save(binary, home, shots, extra_env=None):
    game = Game(binary, home, new_game=False, extra_env=extra_env)
    try:
        end = time.time() + 12
        while time.time() < end:
            game.pump(0.15)
            text = game.text()
            if "will not load it" in text or "older game" in text:
                return False, text
            if "change character name" in text or "hange character" in text:
                game.send(" ")
                continue
            if "@" in text or "Press any key" in text or "-more-" in text:
                if "-more-" in text or "Press any key" in text:
                    game.send(" ")
                    continue
                if "@" in text and "Town level" in text:
                    shots["loaded"] = clone_screen(game.screen)
                    return True, "new save reloaded"
        return False, "load did not reach the map\n" + game.text()
    finally:
        game.close()


def expect_refused(binary, home, shots):
    game = Game(binary, home, new_game=False)
    try:
        if not game.wait_for("Press any key to continue", timeout=12):
            return False, "news missing\n" + game.text()
        game.send(" ")
        if not game.wait_for("will not load it", timeout=8):
            return False, "older save was not refused\n" + game.text()
        shots["refused"] = clone_screen(game.screen)
        return True, "older save refused"
    finally:
        game.close()


def main():
    binary = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "build/moriasupreme")
    old = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else ""
    shot_dir = sys.argv[3] if len(sys.argv) > 3 else ""
    if shot_dir:
        os.makedirs(shot_dir, exist_ok=True)
    failures = []
    shots = {}

    def run(name, fn, home):
        os.makedirs(home, exist_ok=True)
        ok, detail = fn()
        print(("ok  " if ok else "FAIL") + " " + name + ": " + detail.split("\n")[0])
        if not ok:
            failures.append(name + ": " + detail)

    run(
        "riddle",
        lambda: play_riddle(binary, "/tmp/hobbit-riddle", shots),
        "/tmp/hobbit-riddle",
    )
    run(
        "glow",
        lambda: play_glow(binary, "/tmp/hobbit-glow", shots),
        "/tmp/hobbit-glow",
    )
    run(
        "troll",
        lambda: play_troll(binary, "/tmp/hobbit-troll", shots),
        "/tmp/hobbit-troll",
    )
    run(
        "hoard",
        lambda: play_hoard(binary, "/tmp/hobbit-hoard", shots),
        "/tmp/hobbit-hoard",
    )
    def do_reload():
        home = "/tmp/hobbit-reload"
        os.makedirs(home, exist_ok=True)
        leftover = os.path.join(home, "moriasupreme.save")
        if os.path.exists(leftover):
            os.remove(leftover)
        ok, detail = save_with(binary, home)
        if not ok:
            return False, detail
        return load_save(binary, home, shots)

    run("reload", do_reload, "/tmp/hobbit-reload")

    if old:
        home = "/tmp/hobbit-oldsave"
        os.makedirs(home, exist_ok=True)
        leftover = os.path.join(home, "moriasupreme.save")
        if os.path.exists(leftover):
            os.remove(leftover)

        def do_refuse():
            ok, detail = save_with(old, home)
            if not ok:
                return False, "older binary: " + detail
            return expect_refused(binary, home, shots)

        run("old-save", do_refuse, home)

    if shot_dir:
        for name, screen in shots.items():
            render_png(screen, os.path.join(shot_dir, name + ".png"))
            print("shot", name)
    if failures:
        print("\n".join(failures))
        return 1
    print("hobbit smoke passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
