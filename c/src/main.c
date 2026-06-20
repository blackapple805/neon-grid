/* main.c â€” entry point, terminal setup, render loop, input.  [Windows native]
 *
 * This is the Windows port of the render loop. The simulation core (life.c) and
 * the palette/ansi helpers are 100% portable and unchanged â€” only the
 * platform-specific terminal handling lives here.
 *
 * Windows-specific pieces vs. the POSIX version:
 *   - ENABLE_VIRTUAL_TERMINAL_PROCESSING: modern Windows 10+ consoles support
 *     ANSI escape codes, but you must opt in by flipping this console mode bit.
 *     Without it, our color/cursor escapes print as literal garbage.
 *   - _kbhit() / _getch() (from <conio.h>): non-blocking keyboard input, the
 *     Windows equivalent of raw-mode read().
 *   - GetConsoleScreenBufferInfo(): read the window size (replaces ioctl).
 *   - Sleep(): millisecond sleep (replaces nanosleep).
 */
#include "ansi.h"
#include "life.h"
#include "palette.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <conio.h>   /* _kbhit, _getch */

/* Saved console mode so we can restore it on exit. */
static DWORD g_orig_out_mode = 0;
static HANDLE g_out = NULL;
static int g_mode_saved = 0;

/* Flag set by the Ctrl+C handler. */
static volatile int g_should_quit = 0;

/* Restore the console: show cursor, leave alt screen, reapply original mode. */
static void restore_terminal(void) {
    fputs(ANSI_SHOW_CURSOR, stdout);
    fputs(ANSI_LEAVE_ALT_SCREEN, stdout);
    fflush(stdout);
    if (g_mode_saved && g_out) {
        SetConsoleMode(g_out, g_orig_out_mode);
    }
}

/* Ctrl+C / close handler â€” just flip the flag and let the loop exit cleanly. */
static BOOL WINAPI ctrl_handler(DWORD type) {
    (void)type;
    g_should_quit = 1;
    return TRUE;   /* we handled it; don't let Windows kill us immediately */
}

/* Turn on ANSI escape processing for the output handle, switch to the alt
 * screen, and hide the cursor. Returns 1 on success. */
static int setup_terminal(void) {
    g_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_out == INVALID_HANDLE_VALUE || g_out == NULL) {
        return 0;
    }
    if (!GetConsoleMode(g_out, &g_orig_out_mode)) {
        return 0;   /* output is redirected, not a real console */
    }
    g_mode_saved = 1;

    /* The crucial Windows step: enable VT/ANSI sequence interpretation. */
    DWORD mode = g_orig_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(g_out, mode)) {
        return 0;   /* too old a Windows build (pre-1511) */
    }

    /* UTF-8 output so the '@'/'o' and box glyphs render predictably. */
    SetConsoleOutputCP(CP_UTF8);

    fputs(ANSI_ENTER_ALT_SCREEN, stdout);
    fputs(ANSI_HIDE_CURSOR, stdout);
    fputs(ANSI_CLEAR, stdout);
    fflush(stdout);
    return 1;
}

/* Query console size; fall back to 80x24. */
static void get_terminal_size(int *cols, int *rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (g_out && GetConsoleScreenBufferInfo(g_out, &csbi)) {
        *cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    } else {
        *cols = 80;
        *rows = 24;
    }
}

/* Draw the whole grid. Full repaint each frame â€” small grids at 15 FPS are
 * smooth. (Porting the C# diff renderer is a good next exercise.) */
static void render(const Life *life, Palette pal) {
    fputs(ANSI_HOME, stdout);
    for (int y = 0; y < life->height; y++) {
        for (int x = 0; x < life->width; x++) {
            size_t idx = (size_t)y * life->width + x;
            if (life->cells[idx]) {
                Rgb c = palette_shade(pal, life->age[idx]);
                ansi_fg(stdout, c.r, c.g, c.b);
                fputc(life->age[idx] <= 1 ? '@' : 'o', stdout);
            } else {
                fputc(' ', stdout);
            }
        }
        if (y < life->height - 1) fputc('\n', stdout);
    }
    fputs(ANSI_RESET, stdout);
    fflush(stdout);
}

int main(int argc, char **argv) {
    Palette pal = palette_from_args(argc, argv);

    if (!setup_terminal()) {
        fprintf(stderr, "neon-life: could not enable a VT console "
                        "(need Windows 10 1511+ in a real terminal).\n");
        return 1;
    }
    atexit(restore_terminal);
    SetConsoleCtrlHandler(ctrl_handler, TRUE);   /* Ctrl+C -> graceful quit */

    int cols, rows;
    get_terminal_size(&cols, &rows);
    int grid_h = rows - 1;
    if (grid_h < 1) grid_h = 1;

    Life life;
    if (!life_init(&life, cols, grid_h)) {
        fprintf(stderr, "neon-life: out of memory allocating %dx%d grid.\n",
                cols, grid_h);
        return 1;
    }
    life_randomize(&life, 22, (unsigned int)time(NULL));

    int paused = 0;
    long generation = 0;

    while (!g_should_quit) {
        /* Non-blocking input: only read if a key is waiting. */
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
                case 'q': case 'Q': case 27:   /* 27 = Esc */
                    g_should_quit = 1;
                    break;
                case ' ':
                    pal = palette_next(pal);
                    break;
                case 'p': case 'P':
                    paused = !paused;
                    break;
                case 'r': case 'R':
                    life_randomize(&life, 22,
                                   (unsigned int)time(NULL) + generation);
                    break;
            }
        }

        if (!paused) {
            life_step(&life);
            generation++;
        }
        render(&life, pal);
        Sleep(66);   /* ~15 FPS */
    }

    life_free(&life);
    return 0;
}

