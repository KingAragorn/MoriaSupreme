#!/usr/bin/env python3
"""Birth challenges, the history log, an older save, and the score line.

Drives build/moriasupreme. Each challenge is checked on the character sheet.
No selling is also refused in a shop. The history command is checked before
and after the first stair. Quitting writes a score whose killed-by column
carries the challenge letters.
"""

import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_characters import clone_screen, render_png  # noqa: E402
from smoke_town import (  # noqa: E402
    Town,
    approach,
    create,
    find_char,
    go_to,
    save_and_quit,
)

BINARY = os.environ.get("MORIA_BIN", "/workspace/build/moriasupreme")
PATCH2 = os.environ.get(
    "MORIA_PATCH2_SAVE", "/tmp/patch2-home/moriasupreme.save"
)
SHOTS = os.environ.get(
    "MORIA_SHOTS", "/opt/cursor/artifacts/screenshots"
)


def fresh(home):
    if os.path.isdir(home):
        shutil.rmtree(home)
    os.makedirs(home)


def shot(session, name):
    os.makedirs(SHOTS, exist_ok=True)
    path = os.path.join(SHOTS, name + ".png")
    render_png(clone_screen(session.screen), path)
    return path


def load_sheet(session):
    if not session.wait_for("Press any key to continue", 12):
        return "news missing\n" + session.text()
    session.send(" ")
    if not session.wait_for("Challenges", 8):
        return "sheet missing\n" + session.text()
    return None


def test_old_save(home):
    fresh(home)
    shutil.copy(PATCH2, os.path.join(home, "moriasupreme.save"))
    session = Town(BINARY, home, new_game=False)
    try:
        if not session.wait_for("Press any key to continue", 12):
            return "news missing\n" + session.text()
        session.send(" ")
        if not session.wait_for("will not load it", 8):
            return "older save was not refused\n" + session.text()
        shot(session, "old-save-refused")
    finally:
        session.close()
    return None


def test_nosell(home):
    fresh(home)
    session = Town(BINARY, home, new_game=True)
    try:
        err = create(session, "a", "a", "Nosell", challenges="nnyn")
        if err:
            return err
        entered = approach(session, "1")
        if entered != "store":
            return "general store entry was %s\n%s" % (entered, session.text())
        session.send("s")
        if not session.wait_for("does not buy", 4):
            return "shop still buys\n" + session.text()
        shot(session, "no-selling")
    finally:
        session.close()
    return None


def test_history_and_score(home):
    fresh(home)
    session = Town(BINARY, home, new_game=True)
    try:
        err = create(
            session, "a", "a", "Allfour", challenges="yyyy",
            shots={}, shot_name="sheet",
        )
        if err:
            return err
        # create() stored the sheet only when shots was a dict it wrote.
        # Grab the live map, then the history after the stair.
        session.send("A")
        session.pump(0.4)
        if "No history has been written yet." not in session.text():
            return "town history should be empty\n" + session.text()
        stairs = find_char(session, ">")
        if not stairs:
            return "no down stair in town\n" + session.text()
        result = go_to(session, stairs[0])
        if result == "dead":
            return "died on the town stair\n" + session.text()
        # Standing on the stair. '>' is the down command.
        session.send(">")
        session.pump(0.5)
        session.clear_more()
        if "Town level" in session.text():
            return "did not leave town\n" + session.text()
        session.send("A")
        if not session.wait_for("Reached dungeon level", 4):
            return "depth was not written\n" + session.text()
        shot(session, "history-log")
        session.send(" ")
        session.pump(0.3)
        session.send("Q")
        if not session.wait_for("really want to quit", 4):
            return "quit prompt missing\n" + session.text()
        session.send("y")
        if not session.wait_for("character history", 6):
            return "death history prompt missing\n" + session.text()
        shot(session, "death-tomb")
        session.send(" ")
        if not session.wait_for("Reached dungeon level", 4):
            return "death history missing the depth note\n" + session.text()
        shot(session, "death-history")
        session.send(" ")
        if not session.wait_for("Character record", 4):
            return "tomb record prompt missing\n" + session.text()
        session.send("\033")
        if not session.wait_for("Rank", 6):
            return "score list missing\n" + session.text()
        text = session.text()
        if "INCE" not in text and "I" not in text:
            return "score line has no challenge letters\n" + text
        if "INCE" not in text:
            return "score line missing INCE\n" + text
        shot(session, "score-challenges")
        session.send(" ")
    finally:
        session.close()
    return None


def test_reload(home):
    fresh(home)
    session = Town(BINARY, home, new_game=True)
    try:
        err = create(session, "a", "a", "Reload", challenges="yyyy")
        if err:
            return err
        err = save_and_quit(session)
        if err:
            return err
    finally:
        session.close()
    session = Town(BINARY, home, new_game=False)
    try:
        err = load_sheet(session)
        if err:
            return err
        text = session.text()
        for word in ("ironman stairs", "no selling", "connected stairs",
                     "randomized egos", "Reload"):
            if word not in text:
                return "reload missing %s\n%s" % (word, text)
        shot(session, "reload-challenges")
    finally:
        session.close()
    return None


def main():
    if not os.path.isfile(BINARY):
        print("missing binary", BINARY)
        return 1
    if not os.path.isfile(PATCH2):
        print("missing patch 2 save", PATCH2)
        return 1
    home = "/tmp/moria-challenges"
    tests = (
        ("old-save", test_old_save),
        ("nosell", test_nosell),
        ("history", test_history_and_score),
        ("reload", test_reload),
    )
    failed = 0
    for name, fn in tests:
        err = fn(os.path.join(home, name))
        if err:
            failed += 1
            print("FAIL", name)
            print(err)
        else:
            print("ok", name)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
