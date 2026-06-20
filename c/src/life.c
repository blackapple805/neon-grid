/* life.c â€” Game of Life simulation core. */
#include "life.h"
#include <stdlib.h>
#include <string.h>

int life_init(Life *life, int width, int height) {
    life->width  = width;
    life->height = height;
    size_t n = (size_t)width * (size_t)height;

    /* calloc zero-initializes, so the grid starts empty (all dead, age 0).
     * Three separate allocations because the buffers have different element
     * types (unsigned char vs int). */
    life->cells = calloc(n, sizeof(unsigned char));
    life->next  = calloc(n, sizeof(unsigned char));
    life->age   = calloc(n, sizeof(int));

    /* If ANY allocation failed, clean up the ones that succeeded and report
     * failure. This is the classic C "partial init" trap â€” life_free handles
     * NULLs safely, so we can just delegate. */
    if (!life->cells || !life->next || !life->age) {
        life_free(life);
        return 0;
    }
    return 1;
}

void life_free(Life *life) {
    if (!life) return;
    free(life->cells);
    free(life->next);
    free(life->age);
    /* Null them so a double-free or use-after-free is a visible NULL deref
     * rather than silent corruption. */
    life->cells = NULL;
    life->next  = NULL;
    life->age   = NULL;
}

void life_randomize(Life *life, int density, unsigned int seed) {
    srand(seed);
    size_t n = (size_t)life->width * (size_t)life->height;
    for (size_t i = 0; i < n; i++) {
        int alive = (rand() % 100) < density;
        life->cells[i] = (unsigned char)alive;
        life->age[i]   = alive ? 1 : 0;
    }
}

unsigned char life_get(const Life *life, int x, int y) {
    /* Toroidal wrap: -1 becomes the far edge, width becomes 0. The extra
     * "+ dim" before the modulo keeps the result non-negative for small
     * negatives (C's % can return negative for negative operands). */
    x = (x + life->width)  % life->width;
    y = (y + life->height) % life->height;
    return life->cells[(size_t)y * life->width + x];
}

void life_step(Life *life) {
    int w = life->width;
    int h = life->height;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            /* Count the 8 neighbors (Moore neighborhood), wrapping at edges. */
            int n = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;   /* skip self */
                    n += life_get(life, x + dx, y + dy);
                }
            }

            size_t idx = (size_t)y * w + x;
            unsigned char alive = life->cells[idx];

            /* Conway B3/S23: a dead cell with exactly 3 neighbors is born;
             * a live cell with 2 or 3 survives; everything else dies. We write
             * the result into `next`, never into `cells`. */
            unsigned char born = (!alive && n == 3);
            unsigned char survives = (alive && (n == 2 || n == 3));
            life->next[idx] = (born || survives) ? 1 : 0;

            /* Age bookkeeping: survivors get older, newborns reset to 1,
             * dead cells reset to 0. */
            if (life->next[idx]) {
                life->age[idx] = alive ? life->age[idx] + 1 : 1;
            } else {
                life->age[idx] = 0;
            }
        }
    }

    /* THE SWAP. Exchange the two buffer pointers so `next` becomes the live
     * grid for the next call. This is O(1) â€” we move pointers, not data â€”
     * which is the whole reason double buffering is cheap. */
    unsigned char *tmp = life->cells;
    life->cells = life->next;
    life->next  = tmp;
}

