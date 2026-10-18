#!/usr/bin/env python3
"""Headless Chrome: birth challenges, then save and reload that character."""

import os
import sys
import time

from playwright.sync_api import sync_playwright

BASE = os.environ.get("MORIA_WEB", "http://127.0.0.1:8765/")
SHOTS = os.environ.get("MORIA_SHOTS", "/opt/cursor/artifacts/screenshots")


def screen(page):
    return page.evaluate("window.moriaScreen ? window.moriaScreen() : ''")


def wait_text(page, needle, timeout=20):
    end = time.time() + timeout
    last = ""
    while time.time() < end:
        last = screen(page)
        if needle in last:
            return last
        page.wait_for_timeout(200)
    return None


def push(page, text):
    for ch in text:
        code = 27 if ch == "\x1b" else 13 if ch == "\r" else ord(ch)
        page.evaluate(
            """(code) => {
              if (window.Module && Module.ccall)
                Module.ccall('moria_push_key', null, ['number'], [code]);
            }""",
            code,
        )
        page.wait_for_timeout(90)


def open_page(browser, url):
    page = browser.new_page(viewport={"width": 1100, "height": 820})
    page.goto(url, wait_until="domcontentloaded")
    page.click("#screen")
    if not wait_text(page, "Press any key", 40):
        raise SystemExit("news screen missing\n" + screen(page))
    return page


def birth_sheet(page, answers):
    push(page, " ")
    if not wait_text(page, "Choose a race"):
        raise SystemExit("race prompt missing\n" + screen(page))
    push(page, "a")
    if not wait_text(page, "Choose a sex"):
        raise SystemExit("sex prompt missing\n" + screen(page))
    push(page, "m")
    if not wait_text(page, "reroll"):
        raise SystemExit("stat prompt missing\n" + screen(page))
    push(page, "\x1b")
    if not wait_text(page, "Choose a class"):
        raise SystemExit("class prompt missing\n" + screen(page))
    push(page, "a")
    if not wait_text(page, "temperament"):
        raise SystemExit("temperament prompt missing\n" + screen(page))
    push(page, "a")
    page.wait_for_timeout(200)
    if "-more-" in (screen(page) or ""):
        push(page, " ")
    for prompt, ans in zip(
        ("Randomized ego", "Ironman stairs", "No selling", "Connected stairs"),
        answers,
    ):
        if not wait_text(page, prompt):
            raise SystemExit(prompt + " prompt missing\n" + screen(page))
        push(page, ans)
    if not wait_text(page, "player's name"):
        raise SystemExit("name prompt missing\n" + screen(page))
    push(page, "Webfour\r")
    text = wait_text(page, "Challenges")
    if not text:
        raise SystemExit("sheet missing\n" + screen(page))
    return screen(page)


def main():
    os.makedirs(SHOTS, exist_ok=True)
    with sync_playwright() as p:
        browser = p.chromium.launch(channel="chrome", headless=True)
        page = open_page(browser, BASE)
        text = birth_sheet(page, "yyyy")
        for word in ("ironman stairs", "no selling", "connected stairs",
                     "randomized egos"):
            if word not in text:
                raise SystemExit("web sheet missing %s\n%s" % (word, text))
        page.screenshot(path=os.path.join(SHOTS, "web-challenges.png"))
        push(page, " ")
        if not wait_text(page, "Town level", 15):
            raise SystemExit("web town missing\n" + screen(page))
        push(page, "\x18")
        end = time.time() + 20
        while time.time() < end:
            text = screen(page)
            if "Delete old savefile" in text or "File exists" in text:
                push(page, "y")
            elif "-more-" in text or "Press any key" in text:
                push(page, " ")
            exited = page.evaluate("window.moriaExited")
            if exited is not None:
                break
            page.wait_for_timeout(200)
        else:
            raise SystemExit("web save did not finish\n" + screen(page))
        page.close()

        page = open_page(browser, BASE)
        push(page, " ")
        text = wait_text(page, "Webfour", 25)
        if not text:
            raise SystemExit("reloaded name missing\n" + screen(page))
        for word in ("ironman stairs", "no selling", "connected stairs",
                     "randomized egos"):
            if word not in text:
                raise SystemExit("reload missing %s\n%s" % (word, text))
        page.screenshot(path=os.path.join(SHOTS, "web-reload.png"))
        page.close()
        browser.close()
    print("web challenges ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
