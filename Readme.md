# typetracker

A terminal-based typing tracker implemented with ncurses.

## Features

- Loads a text file and displays it in a scrollable ncurses interface.
- Shows a cursor to track your typing progress.
- Correctly typed characters are shown in a greyish-blue color.
- Requires typing each character including newline to advance.
- Supports cursor movement with arrow keys.
- Exits cleanly on `ESC` or `Ctrl+C`.

## Requirements

- C compiler (gcc or clang)
- ncurses library and development headers installed
- `pkg-config` (recommended for build)

## Building

```sh
make
