/* ansi.h â€” minimal VT100/ANSI escape sequence helpers (24-bit color, cursor,
 * screen control). C sibling of the repo's C# Ansi.cs.
 *
 * These are static inline so the whole module is header-only: no ansi.c, no
 * extra object file to link. `static inline` means each translation unit that
 * includes this gets its own copy the compiler can inline away entirely.
 */
#ifndef ANSI_H
#define ANSI_H

#include <stdio.h>

#define ANSI_ESC "\x1b["

/* Literal sequences â€” usable directly in fputs/printf. */
#define ANSI_CLEAR            ANSI_ESC "2J" ANSI_ESC "H"
#define ANSI_HOME             ANSI_ESC "H"
#define ANSI_RESET            ANSI_ESC "0m"
#define ANSI_ENTER_ALT_SCREEN ANSI_ESC "?1049h"
#define ANSI_LEAVE_ALT_SCREEN ANSI_ESC "?1049l"
#define ANSI_HIDE_CURSOR      ANSI_ESC "?25l"
#define ANSI_SHOW_CURSOR      ANSI_ESC "?25h"

/* Move cursor to a 0-indexed row/col, written to the given stream.
 * Terminals are 1-indexed, so we add 1 â€” same translation as the C# MoveTo. */
static inline void ansi_move_to(FILE *out, int row, int col) {
    fprintf(out, ANSI_ESC "%d;%dH", row + 1, col + 1);
}

/* 24-bit (true color) foreground. */
static inline void ansi_fg(FILE *out, int r, int g, int b) {
    fprintf(out, ANSI_ESC "38;2;%d;%d;%dm", r, g, b);
}

#endif /* ANSI_H */

