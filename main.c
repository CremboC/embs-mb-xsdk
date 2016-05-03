// Exam number: Y0001392

#include <stdlib.h>

#include "xparameters.h"
#include "xil_types.h"
#include "xuartlite_l.h"

#include "definitions.h"
#include "ethernet.h"
#include "vga.h"
#include "fsl.h"

#define SMALL_WORLD 0
#define MEDIUM_WORLD 1
#define LARGE_WORLD 2

int main (void) {
	init_vga(); // Initialise VGA

	char in = '\0';
	int world_size, id = 0;
	for (;;) {
		init_ether(); // Initialise ethernet

		xil_printf("\r\nEnter the size (0 for small, 1 for medium, 2 for large): ");
		in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
		world_size = in - '0';
		xil_printf(" accepted: %d\r\n", world_size);

		char id_a[16] = {};
		int pos = 0;
		xil_printf("Enter world id (between 0 and 100 000): ");
		in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR);
		for (; in != '\r'; in = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR)) {
			if (in >= 0x30 && in <= 0x39) {
				xil_printf("%d", in - '0');
				id_a[pos++] = in;
			} else if (in == 0x0A) { // virtualab doesn't accept \r for some weird reason
				break;
			}
		}

		id = atoi(id_a); // convert input id into int
		xil_printf(" accepted: %d\r\n", id);

		reset_screen();
		request_world(world_size, id);

		// wait until we receive the world from the server
		reply_world_t *r;
		u32 data;
		int status;
		for (;;) {
			status = receive_world(&r);
			if (status == 1) break;
		}

		int i;
		xil_printf("Width: %d\r\n", r->width);
		draw_grid(r->width);

		// point to end of the reply_world_t struct for the waypoint array
		waypoint_t *waypoints = (void *) (r + 1);

		xil_printf("Waypoints size: %d\r\n", r->waypoints_size);

		int color = BLUE;
		for (i = 0; i < r->waypoints_size; i++) {
			color = (i == 0) ? GREEN : BLUE; // mark the first waypoint as green
			fill_square(waypoints[i].x, waypoints[i].y, color); // draw all waypoints
		}
		// point to end of waypoints for the walls size
		u8 *walls_size_ptr = (void *) (waypoints + r->waypoints_size);
		u8 walls_size = *walls_size_ptr;

		xil_printf("Walls size: %d\r\n", walls_size);

		// point to end of walls size for walls array
		wall_t *walls = (void *) (walls_size_ptr + 1);

		// draw all walls
		for (i = 0; i < walls_size; i++) {
			draw_wall(walls[i].x, walls[i].y, walls[i].direction, walls[i].length, r->width);
		}

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
		}
		xil_printf("Sending waypoints size: %d\r\n", r->waypoints_size);

		// 4. waypoints number
		putfslx(r->waypoints_size, 0, FSL_DEFAULT);

		// 5. waypoints
		for (i = 0; i < r->waypoints_size; i++) {
			data = waypoints[i].y;
			data = (data << 8) | waypoints[i].x;

			putfslx(data, 0, FSL_DEFAULT);
		}

		// 6. wait for data back
		xil_printf("Waiting for results back\r\n");

		// reading results:
		// 1. read total distance
		u32 cost;
		getfslx(cost, 0, FSL_DEFAULT);
		xil_printf("Got cost: %d\r\n", cost);

		// 2. loop over path
		for (i = 0; i < cost + 1; i++) {
			getfslx(data, 0, FSL_DEFAULT);
			draw_path_square(data & 0xFF, (data >> 8) & 0xFF); // draw path onto grid
		}

		xil_printf("Sending solution\r\n");
		init_ether();
		send_solution(cost, r->world_id, world_size);

		xil_printf("Awaiting reply\r\n");
		// wait until we receive the solution reply
		solution_reply_t *sr;
		for (;;) {
			status = receive_solution_reply(&sr);
			if (status == 1) break;
		}

		switch (sr->answer) {
		case 0:
			xil_printf("Answer is correct!\r\n");
			break;
		case 1:
			xil_printf("Answer is too long... :(\r\n");
			break;
		case 2:
			xil_printf("Answer is too short... :(\r\n");
			break;
		}

		draw_answer(sr->answer);
	}
    return 0;
}

