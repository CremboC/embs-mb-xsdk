#include "vga.h"

#define FRAME_BUFFER XPAR_DDR_SDRAM_MPMC_BASEADDR
#define BLOCK_SIZE 10
#define PATH_BLOCK_SIZE 4

static u32 buffer;

void init_vga() {
	buffer = XPAR_DDR_SDRAM_MPMC_BASEADDR;

	// set base address
	*((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR) = buffer;

	// enable output graphics
	*((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR + 1) = 1;

	// reset background
	reset_screen();
}

void reset_screen() {
	draw_rect(0, 0, WIDTH, HEIGHT, WHITE);
}

inline void draw_rect(int x_loc, int y_loc, int width, int height, u8 color) {
    int x, y;

    for (y = y_loc; y < y_loc + height; y++) {
        for (x = x_loc; x < x_loc + width; x++) {
            *((volatile u8 *) FRAME_BUFFER + x + (WIDTH * y)) = color;
        }
    }
}

inline void draw_horizontal_line(int y, int end) {
    int x;
	for (x = 0; x < end; x++) {
		*((volatile u8 *) FRAME_BUFFER + x + (WIDTH * y)) = BLACK;
	}
}

inline void draw_vertical_line(int x, int end) {
	draw_rect(x, 0, 1, end, BLACK);
}

inline void fill_square(u8 x, u8 y, int color) {
	draw_rect(x * BLOCK_SIZE + 1, y * BLOCK_SIZE + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

inline void draw_path_square(u8 x, u8 y) {
	draw_rect(x * BLOCK_SIZE + 4, y * BLOCK_SIZE + 4, PATH_BLOCK_SIZE - 1, PATH_BLOCK_SIZE - 1, RED);
}

void draw_wall(u8 x, u8 y, u8 direction, u8 length, u8 world_size) {
	int i;
	switch (direction) {
	case 0: // horizontal
		for (i = 0; i < length; i++) {
			if (x + i < world_size) {
				fill_square(x + i, y, BLACK);
			}
		}
		break;
	case 1: // vertical
		for (i = 0; i < length; i++) {
			if (y + i < world_size) {
				fill_square(x, y + i, BLACK);
			}
		}
		break;
	}
}

void draw_grid(int size) {
	int x;
	int x_end = size * BLOCK_SIZE + 1;
	int y_end = size * BLOCK_SIZE + 1;

	for (x = 0; x < size + 1; x++) {
		draw_vertical_line(x * BLOCK_SIZE, y_end);
		draw_horizontal_line(x * BLOCK_SIZE, x_end);
	}
}
