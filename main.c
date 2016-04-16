#include "xparameters.h"
#include "xil_types.h"
#include "xuartlite_l.h"
#include <stdlib.h>

#include "definitions.h"
#include "ethernet.h"
#include "vga.h"
#include "fsl.h"

#define SMALL_WORLD 0
#define MEDIUM_WORLD 1
#define LARGE_WORLD 2

int main (void) {
	init_ether();
	init_vga();

	char in = '\0';
	reply_world_t *r;

	int world_size, id;
	xil_printf("\r\nEnter the size: 0/1/2 (s/m/l): ");
	in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
	world_size = in - '0';
	xil_printf(" accepted: %d\r\n", world_size);

	char id_a[16] = {};
	int pos = 0;
	xil_printf("ID? 0 - 100000 ");
	in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
	for (; in != '\r'; in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR)) {
		if (in >= 0x30 && in <= 0x39) {
//			xil_printf("%c", in);
			id_a[pos++] = in;
		} else if (in == 0x0A) {
			break;
		}
	}

	xil_printf(" converting ");
	id = atoi(id_a);
	xil_printf(" accepted: %d\r\n", id);

	request_world(world_size, id);
	u32 data;

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

			// send data to hardware to solve world
			// order is:
			// 1. world size
			xil_printf("Sending world size: %d\r\n", r->width);
			putfslx(r->width, 0, FSL_DEFAULT);
			// 2. walls number
			xil_printf("Sending walls size: %d\r\n", walls_size);
			putfslx(walls_size, 0, FSL_DEFAULT);
			// 3. walls
			xil_printf("Sending walls\r\n");
			for (i = 0; i < walls_size; i++) {
				// construct wall packet
				data = walls[i].length;
				data = (data << 8) | walls[i].direction;
				data = (data << 8) | walls[i].y;
				data = (data << 8) | walls[i].x;

				putfslx(data, 0, FSL_DEFAULT);
				xil_printf("Wall %d: %08x\r\n", i, data);
			}
			xil_printf("Sending waypoints size: %d\r\n", r->waypoints_size);
			// 4. waypoints number
			putfslx(r->waypoints_size, 0, FSL_DEFAULT);
			// 5. waypoints
			for (i = 0; i < r->waypoints_size; i++) {
				data = waypoints[i].y;
				data = (data << 8) | waypoints[i].x;

				putfslx(data, 0, FSL_DEFAULT);
				xil_printf("Wall %d: %08x\r\n", i, data);
			}
			// 6. wait for data back

			for (i = 0; i < walls_size; i++) {
				draw_wall(walls[i].x, walls[i].y, walls[i].direction, walls[i].length, r->width); // draw all walls
			}

			xil_printf("Waiting for results back\r\n");

			// reading results:
			// 1. read total distance
			u32 cost;
			getfslx(cost, 0, FSL_DEFAULT);
			xil_printf("Got cost: %d\r\n", cost);
			// 2. loop over path
			for (i = 0; i < cost + 1; i++) {
				getfslx(data, 0, FSL_DEFAULT);

				u8 x = (u8) (data & 0xFF);
				u8 y = (u8) ((data >> 8) & 0xFF);
			}

			xil_printf("Sending solution\r\n");
			send_solution(cost, world_size, id);

			xil_printf("Awaiting reply\r\n");
			solution_reply_t *sr;
			for (;;) {
				status = receive_solution_reply(&sr);
				if (status == 1) break;
			}

			xil_printf("Done: %d \r\n", sr->answer);

			break;
		}
	}
    return 0;
}

