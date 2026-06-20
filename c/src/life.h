/* life.h â€” Conway's Game of Life grid.
 *
 * This is the C-learning centerpiece. Key ideas on display:
 *   - a dynamically allocated grid you own and must free
 *   - DOUBLE BUFFERING: you cannot update Life in place. Every cell's next
 *     state depends on its CURRENT neighbors, so if you overwrite cell (0,0)
 *     before computing (0,1) you've corrupted the input. We compute into a
 *     second buffer, then swap.
 *   - an `age` array tracked alongside the cells so the renderer can color
 *     cells by how long they've been alive (ties into palette.h).
 */
#ifndef LIFE_H
#define LIFE_H

typedef struct {
    int width;
    int height;

    /* Cells stored as a flat width*height array, indexed [y*width + x].
     * 0 = dead, 1 = alive. This is how C handles dynamic 2D data â€” one
     * contiguous block instead of an array-of-pointers, which means one
     * malloc, one free, and better cache behavior. */
    unsigned char *cells;   /* current generation               */
    unsigned char *next;    /* scratch buffer for next gen       */
    int           *age;     /* frames each live cell has existed */
} Life;

/* Allocate a grid. Returns 1 on success, 0 on allocation failure.
 * On success the caller MUST eventually call life_free(). */
int life_init(Life *life, int width, int height);

/* Release everything life_init allocated. Safe to call on a zeroed Life. */
void life_free(Life *life);

/* Fill with random live cells. `density` is 0..100 (percent alive). */
void life_randomize(Life *life, int density, unsigned int seed);

/* Advance one generation (applies Conway's B3/S23 rules) and update ages. */
void life_step(Life *life);

/* Read a cell with toroidal (wrap-around) edges, so gliders loop the screen
 * instead of dying at the border. */
unsigned char life_get(const Life *life, int x, int y);

#endif /* LIFE_H */

