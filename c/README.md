# neon-life â€” C Game of Life

A neon cyberpunk **Conway's Game of Life** for the terminal, written in C as a
sibling to this repo's C# `neon-grid` visualizer. Living cells glow and fade
through the same four cyberpunk palettes (`NEON`, `ACID`, `BLOOD`, `ICE`),
shaded by how long each cell has survived.

This project exists as a **C learning exercise** â€” it deliberately uses the
language features that matter most when leveling up in C, with heavy comments
explaining the *why* behind each one.

## Build & run

POSIX terminal (Linux / macOS / WSL). Needs `make` and a C compiler.

```bash
cd c
make          # build ./neon-life
make run      # build and run
make debug    # build with AddressSanitizer + UBSan for memory-bug hunting
make clean    # remove build artifacts
```

Pick a palette up front:

```bash
./neon-life --neon    # green-cyan (default)
./neon-life --acid    # toxic yellow-green
./neon-life --blood   # magenta-red
./neon-life --ice     # electric blue
```

## Controls

| Key       | Action            |
|-----------|-------------------|
| `SPACE`   | Cycle palette     |
| `P`       | Pause / resume    |
| `R`       | Reseed grid       |
| `Q`/`Esc` | Quit              |
| `Ctrl+C`  | Quit              |

## What this teaches (the point of the project)

| File         | C concepts on display |
|--------------|-----------------------|
| `life.c/.h`  | `malloc`/`calloc`/`free`, ownership, **double buffering** with an O(1) pointer swap, 2D data as a flat `y*width+x` array, toroidal index math |
| `main.c`     | raw-mode `termios`, non-blocking `read`, signal handling with `volatile sig_atomic_t`, `atexit` cleanup, `_POSIX_C_SOURCE` feature-test macro |
| `palette.c/.h` | file-private `static` data, RGB interpolation, string/arg parsing |
| `ansi.h`     | header-only `static inline` helpers, ANSI/VT100 true-color escapes |
| `Makefile`   | object-file build, `debug` target with sanitizers, strict warning flags |

### The single most important idea: double buffering

In Game of Life every cell's next state depends on its **current** neighbors.
If you update cells in place, you corrupt the input for cells you haven't
visited yet. So `life_step` computes the whole next generation into a second
buffer (`next`), then swaps the two pointers:

```c
unsigned char *tmp = life->cells;
life->cells = life->next;   /* next generation becomes current */
life->next  = tmp;          /* old buffer reused as next scratch */
```

Moving two pointers is instant regardless of grid size â€” that's why this
pattern is the standard approach.

## Layout

```
c/
â”œâ”€â”€ Makefile
â””â”€â”€ src/
    â”œâ”€â”€ ansi.h        # ANSI/VT100 escape helpers (header-only)
    â”œâ”€â”€ palette.h/.c  # cyberpunk themes + age-based RGB shading
    â”œâ”€â”€ life.h/.c     # Game of Life core (the C-learning centerpiece)
    â””â”€â”€ main.c        # terminal setup, render loop, input
```

## Ideas for next steps

- Port the C# **diff renderer** (only repaint changed cells) instead of full
  repaints â€” a great pointer/optimization exercise.
- Add live terminal-resize handling (`SIGWINCH`).
- Let `R` cycle through classic seed patterns (glider gun, pulsar) instead of
  random noise.
- Write a proper test file under `tests/` driving the `life` core headlessly.

