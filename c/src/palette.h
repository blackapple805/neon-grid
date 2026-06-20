/* palette.h â€” cyberpunk color themes + age-based RGB interpolation.
 * C port of the repo's C# Palette.cs, adapted for Game of Life: instead of a
 * falling-tail depth, cells shade by how long they've been continuously alive.
 */
#ifndef PALETTE_H
#define PALETTE_H

typedef struct {
    int r, g, b;
} Rgb;

typedef struct {
    const char *name;
    Rgb head;   /* bright â€” freshly born cell */
    Rgb tail;   /* the steady "settled" color a long-lived cell fades toward */
} Palette;

/* How many themes exist (for cycling). */
int palette_count(void);

/* Look at argv for --neon / --acid / --blood / --ice; default to theme 0. */
Palette palette_from_args(int argc, char **argv);

/* The theme after `current`, wrapping around â€” for SPACE-to-cycle. */
Palette palette_next(Palette current);

/* Shade a cell by `age` (frames alive). age 0 = bright head; older cells
 * interpolate toward a dimmed tail. Returns the final RGB to print. */
Rgb palette_shade(Palette p, int age);

#endif /* PALETTE_H */

