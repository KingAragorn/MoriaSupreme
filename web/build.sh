#!/bin/sh
# Build a static browser folder: index.html, the wasm module, and xterm.js.
# Host web/dist on GitHub Pages or any other static server. No server code.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$ROOT"

if ! command -v emcc >/dev/null 2>&1; then
  if [ -f /tmp/emsdk/emsdk_env.sh ]; then
    # shellcheck disable=SC1091
    . /tmp/emsdk/emsdk_env.sh
  elif [ -n "${HOME:-}" ] && [ -f "$HOME/emsdk/emsdk_env.sh" ]; then
    # shellcheck disable=SC1091
    . "$HOME/emsdk/emsdk_env.sh"
  else
    echo "emcc not found. Install emsdk and source emsdk_env.sh." >&2
    exit 1
  fi
fi

emcmake cmake -S "$ROOT" -B "$ROOT/web/build"
cmake --build "$ROOT/web/build"

rm -rf "$ROOT/web/dist"
mkdir -p "$ROOT/web/dist/vendor" "$ROOT/web/.cache"
XTERM_JS="$ROOT/web/.cache/xterm.min.js"
XTERM_CSS="$ROOT/web/.cache/xterm.min.css"
if [ ! -s "$XTERM_JS" ] || [ ! -s "$XTERM_CSS" ]; then
  curl -fsSL -o "$XTERM_JS" https://cdn.jsdelivr.net/npm/xterm@5.3.0/lib/xterm.min.js
  curl -fsSL -o "$XTERM_CSS" https://cdn.jsdelivr.net/npm/xterm@5.3.0/css/xterm.min.css
fi
cp "$XTERM_JS" "$ROOT/web/dist/vendor/xterm.min.js"
cp "$XTERM_CSS" "$ROOT/web/dist/vendor/xterm.min.css"
cp "$ROOT/web/index.html" "$ROOT/web/dist/index.html"
cp "$ROOT/web/build/moriasupreme.js" "$ROOT/web/dist/moriasupreme.js"
cp "$ROOT/web/build/moriasupreme.wasm" "$ROOT/web/dist/moriasupreme.wasm"
cp "$ROOT/web/build/moriasupreme.data" "$ROOT/web/dist/moriasupreme.data"
echo "static site is in $ROOT/web/dist"
