#include "xparameters.h"
#include "xil_types.h"
#include "xuartlite_l.h"
#include <stdlib.h>

#include "definitions.h"
#include "ethernet.h"
#include "vga.h"

#define SMALL_WORLD 0
#define MEDIUM_WORLD 1
#define LARGE_WORLD 2

int main (void) {
	init_ether();
	init_vga();

	char in = '\0';
	reply_world_t *r;

	int world_size, id;
	xil_printf("\r\nSize? 0/1/2 (s/m/l) ==> ");
	in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
	world_size = in - '0';
	xil_printf("%d\r\n", world_size);

	char id_a[16] = {};
	int pos = 0;
	xil_printf("ID? 0 - 100000 ==> ");
	in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
	for (; in != '\r'; in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR)) {
		if (in >= 0x30 && in <= 0x39) {
			xil_printf("%c", in);
			id_a[pos++] = in;
		}
	}

	id = atoi(id_a);
	xil_printf(" ==> %d\r\n", id);

	request_world(world_size, id);

	for (;;) {
		int status = receive_world(&r);
		if (status == 1) {
			xil_printf("World == h: %d; w: %d; \r\n", r->height, r->width);
			draw_grid(r->width);
			int i;

			// point to end of the reply_world_t struct for the waypoint array
			waypoint_t *waypoints = (void *) (r + 1);

			xil_printf("Waypoints size: %d\r\n", r->waypoints_size);

			for (i = 0; i < r->waypoints_size; i++) {
				fill_square(waypoints[i].x, waypoints[i].y, GREEN); // draw all waypoints
			}

			// point to end of waypoints for the walls size
			u8 *walls_size_ptr = (void *) (waypoints + r->waypoints_size);
			u8 walls_size = *walls_size_ptr;

			xil_printf("Walls size: %d\r\n", walls_size);

			// point to end of walls size for walls array
			wall_t *walls = (void *) (walls_size_ptr + 1);

			for (i = 0; i < walls_size; i++) {
				draw_wall(walls[i].x, walls[i].y, walls[i].direction, walls[i].length, r->width); // draw all walls
			}
			break;
		}
	}
    return 0;
}

