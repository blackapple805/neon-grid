/* palette.c â€” implementation of the cyberpunk theme/shading logic. */
#include "palette.h"
#include <string.h>
#include <strings.h>  /* strcasecmp */

/* Same four themes as the C# version. `static` = file-private, like C#'s
 * `private static readonly`. */
static const Palette THEMES[] = {
    { "NEON",  { 235, 255, 255 }, {   0, 255, 170 } },  /* green-cyan (default) */
    { "ACID",  { 255, 255, 230 }, { 190, 255,   0 } },  /* toxic yellow-green   */
    { "BLOOD", { 255, 235, 235 }, { 255,  30,  80 } },  /* hot magenta-red      */
    { "ICE",   { 240, 250, 255 }, {  70, 160, 255 } },  /* electric blue        */
};

static const int THEME_COUNT = (int)(sizeof(THEMES) / sizeof(THEMES[0]));

/* Age (in frames) at which a cell has fully faded from head -> tail.
 * After this it just stays at the tail color. */
#define FADE_FRAMES 12

int palette_count(void) {
    return THEME_COUNT;
}

Palette palette_from_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {       /* skip argv[0] (program name) */
        for (int t = 0; t < THEME_COUNT; t++) {
            char flag[16] = "--";          /* build "--neon" etc. */
            /* THEMES are uppercase ("NEON"); strcasecmp ignores case so the
             * flag can be lowercase. */
            strncat(flag, THEMES[t].name, sizeof(flag) - 3);
            if (strcasecmp(argv[i], flag) == 0) {
                return THEMES[t];
            }
        }
    }
    return THEMES[0];
}

Palette palette_next(Palette current) {
    int i = 0;
    for (int t = 0; t < THEME_COUNT; t++) {
        if (strcmp(THEMES[t].name, current.name) == 0) {
            i = t;
            break;
        }
    }
    return THEMES[(i + 1) % THEME_COUNT];
}

Rgb palette_shade(Palette p, int age) {
    if (age < 0) age = 0;
    /* depth runs 0..1 as the cell ages toward FADE_FRAMES. */
    double depth = (double)age / (double)FADE_FRAMES;
    if (depth > 1.0) depth = 1.0;

    /* Mirrors C# Shade(): lerp head->tail, then a mild overall dim with depth so
     * older cells visually recede. fade stays >= 0.45 so settled cells are still
     * clearly visible rather than going black. */
    double fade = 1.0 - depth * 0.55;

    Rgb out;
    out.r = (int)((p.head.r + (p.tail.r - p.head.r) * depth) * fade);
    out.g = (int)((p.head.g + (p.tail.g - p.head.g) * depth) * fade);
    out.b = (int)((p.head.b + (p.tail.b - p.head.b) * depth) * fade);
    return out;
}

