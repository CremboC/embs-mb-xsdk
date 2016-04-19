#include "vga.h"

#define FRAME_BUFFER XPAR_DDR_SDRAM_MPMC_BASEADDR
#define BLOCK_SIZE 10
#define PATH_BLOCK_SIZE 4

static u32 buffer;

/**
 * Init VGA: set the buffer, set base address, enable ouput and reset to white background.
 */
void init_vga() {
	buffer = XPAR_DDR_SDRAM_MPMC_BASEADDR;

	// set base address
	*((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR) = buffer;

	// enable output graphics
	*((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR + 1) = 1;

	// reset background
	reset_screen();
}

/**
 * Helper to reset the screen to white.
 */
void reset_screen() {
	draw_rect(0, 0, WIDTH, HEIGHT, WHITE);
}

/**
 * Draw a rectangle.
 */
inline void draw_rect(int x_loc, int y_loc, int width, int height, u8 color) {
    int x, y;

    for (y = y_loc; y < y_loc + height; y++) {
        for (x = x_loc; x < x_loc + width; x++) {
            *((volatile u8 *) FRAME_BUFFER + x + (WIDTH * y)) = color;
        }
    }
}

/**
 * Draw horizontal line at given y, which ends at given end.
 */
inline void draw_horizontal_line(int y, int end) {
    int x;
	for (x = 0; x < end; x++) {
		*((volatile u8 *) FRAME_BUFFER + x + (WIDTH * y)) = BLACK;
	}
}

/**
 * Draw vertical line at given x, which ends at given end.
 */
inline void draw_vertical_line(int x, int end) {
	draw_rect(x, 0, 1, end, BLACK);
}

/**
 * Fill a grid square at the provided coordinates (grid coordinates, so max is 59x59) with a given colour.
 */
inline void fill_square(u8 x, u8 y, int color) {
	draw_rect(x * BLOCK_SIZE + 1, y * BLOCK_SIZE + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

/**
 * Draw a rectangle which represents the route at given grid coordinates (max 59x59).
 */
inline void draw_path_square(u8 x, u8 y) {
	draw_rect(x * BLOCK_SIZE + 4, y * BLOCK_SIZE + 4, PATH_BLOCK_SIZE - 1, PATH_BLOCK_SIZE - 1, RED);
}

/**
 * Draw a wall. Start at x/y, with a given direction of horizontal/vertical and given length.
 * World size is used to stop drawing at the correct time so there are no walls outside the grid.
 */
void draw_wall(u8 x, u8 y, u8 direction, u8 length, u8 world_size) {
	int i;
	switch (direction) {
	case 0: // horizontal
		for (i = 0; i < length; i++) {
			if (x + i == world_size) break;
			fill_square(x + i, y, BLACK);
		}
		break;
	case 1: // vertical
		for (i = 0; i < length; i++) {
			if (y + i == world_size) break;
			fill_square(x, y + i, BLACK);
		}
		break;
	}
}

/**
 * Draw a simple grid with the given world size.
 */
void draw_grid(int size) {
	int x;
	int x_end = size * BLOCK_SIZE + 1;
	int y_end = size * BLOCK_SIZE + 1;

	for (x = 0; x < size + 1; x++) {
		draw_vertical_line(x * BLOCK_SIZE, y_end);
		draw_horizontal_line(x * BLOCK_SIZE, x_end);
	}
}

/**
 * Simple function to show whether the answer is correct, too short or too long on the display.
 * GREEN: correct
 * YELLOW: too short
 * RED: too long
 */
void draw_answer(u8 answer) {
	switch (answer) {
	case 0:
		draw_rect(WIDTH - 50, 50, 50, 50, GREEN);
		break;
	case 1:
		draw_rect(WIDTH - 50, 50, 50, 50, YELLOW);
		break;
	case 2:
		draw_rect(WIDTH - 50, 50, 50, 50, RED);
		break;
	}
}
