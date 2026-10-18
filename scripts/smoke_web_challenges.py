#!/usr/bin/env python3
"""Headless Chrome: birth challenges on the sheet, and a patch 2 save."""

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


def install_save_route(page):
    """Serve index.html with a hook that drops a patch 2 save in before main.

    The hook runs only when the URL asks for it, so a normal visit is the
    page as shipped.
    """
    html_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "web", "dist", "index.html"
    )

    def handle(route):
        url = route.request.url.split("?")[0]
        if not (url.endswith("/") or url.endswith("index.html")):
            route.continue_()
            return
        html = open(os.path.abspath(html_path), encoding="utf-8").read()
        needle = "Module.callMain([]);"
        insert = """
          if (location.search.indexOf("patch2=1") >= 0) {
            try {
              var xhr = new XMLHttpRequest();
              xhr.open("GET", "patch2.save", false);
              xhr.overrideMimeType("text/plain; charset=x-user-defined");
              xhr.send(null);
              var raw = xhr.responseText || "";
              var bytes = new Uint8Array(raw.length);
              for (var bi = 0; bi < raw.length; bi++)
                bytes[bi] = raw.charCodeAt(bi) & 0xff;
              FS.writeFile("/home/web_user/moriasupreme.save", bytes);
            } catch (e) { console.error(e); }
          }
          Module.callMain([]);
        """
        if needle not in html:
            route.fulfill(status=500, body="callMain hook missing")
            return
        route.fulfill(
            status=200,
            content_type="text/html",
            body=html.replace(needle, insert, 1),
        )

    page.route("**/*", handle)


def open_page(browser, url):
    page = browser.new_page(viewport={"width": 1100, "height": 820})
    install_save_route(page)
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
        page.close()

        page = open_page(browser, BASE + "?patch2=1")
        push(page, " ")
        text = wait_text(page, "Patchtwo", 20)
        if not text:
            raise SystemExit("patch 2 name missing\n" + screen(page))
        text = screen(page)
        if "none" not in text:
            raise SystemExit("patch 2 web challenges not off\n" + text)
        if "Ordinary" not in text:
            raise SystemExit("patch 2 web temperament missing\n" + text)
        page.screenshot(path=os.path.join(SHOTS, "web-patch2.png"))
        page.close()
        browser.close()
    print("web challenges ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
