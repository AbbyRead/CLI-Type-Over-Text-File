# typetracker

A terminal-based typing tracker implemented with ncurses.

## Features

- Loads a text file and displays it in a scrollable ncurses interface.
- Correctly typed characters are shown in a greyish-blue color.
- Supports cursor movement with arrow keys.
- Skip around as needed/desired. Aiming for zero-stress typing practice.

## Requirements

- C compiler (gcc or clang)
- ncurses library and development headers installed
- `pkg-config` (recommended for build)

## Building

```sh
make
