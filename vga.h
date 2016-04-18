/*
 * vga.h
 *
 *  Created on: 12 Apr 2016
 *      Author: pi514
 */

#ifndef VGA_H_
#define VGA_H_

#include "xparameters.h"
#include "xil_types.h"

#define WIDTH          800
#define HEIGHT         600
#define BITS_PER_PIXEL 8

#define BLACK   0b00000000
#define WHITE   0b01110111
#define RED     0b01000100
#define GREEN   0b00100010
#define BLUE    0b00010001
#define CYAN    0b00110011
#define YELLOW  0b01100110
#define MAGENTA 0b01010101

void init_vga();
void reset_screen();
void draw_rect(int x_loc, int y_loc, int width, int height, u8 colour);
void draw_horizontal_line(int y, int end);
void draw_vertical_line(int x, int end);
void fill_square(u8 x, u8 y, int color);
void draw_path_square(u8 x, u8 y);
void draw_wall(u8 x, u8 y, u8 direction, u8 length, u8 world_size);
void draw_grid(int size);

#endif /* VGA_H_ */
