#include "ethernet.h"

#define TMIT_HEADER_SIZE 14

struct message_t {
	int type; // 0 - rw, 1 - sw
	int length;
	union {
		request_world_t rqw;
		reply_world_t rpw;
		solve_world_t sw;
	};
} message;

// private function declarations
void send_frame();

// mac addresses
static u8 my_address[] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x1C};
static u8 target_address[] = {0x00, 0x11, 0x22, 0x44, 0x00, 0x50};
static u8 type[] = {0x55, 0xAB};

/*
 * Buffers used for Transmission and Reception of Packets. These are declared
 * as global so that they are not a part of the stack.
 */
u8 tmit_buffer[32] = {};
u8 recv_buffer[128] = {0};

eth_header_t *eth_header = (void *) recv_buffer;

XEmacLite ether;

/**height
 * Initialise all ethernet related variables and configurations.
 */
void init_ether() {
    XEmacLite_Config *etherconfig = XEmacLite_LookupConfig(XPAR_EMACLITE_0_DEVICE_ID);
    XEmacLite_CfgInitialize(&ether, etherconfig, etherconfig->BaseAddress);

    XEmacLite_SetMacAddress(&ether, my_address); // set sending mac address
    XEmacLite_FlushReceive(&ether);
    XEmacLite_FlushReceive(&ether);
    XEmacLite_FlushReceive(&ether);
}

void request_world(int size, int world_id) {
	xil_printf("Requesting world\r\n");
	request_world_t req;

	req.type = REQUEST_WORLD;
	req.size = size;
	req.world_id = world_id;

	message.type = REQUEST_WORLD;
	message.rqw = req;
	message.length = sizeof(request_world_t);

	send_frame();
}

void send_solution(int cost, u8 world_id[4], int size) {
	solve_world_t req;

	req.type = SOLVE_WORLD;
	req.size = size;
	int i;
	for (i = 0; i < 4; i++) req.world_id[i] = world_id[i];

	req.ignore_walls = 0;
	req.cost = cost;

	message.type = SOLVE_WORLD;
	message.sw = req;
	message.length = sizeof(solve_world_t);

	send_frame();
}

int receive_world(reply_world_t **r) {
	volatile int recv_len = 0;
    recv_len = XEmacLite_Recv(&ether, recv_buffer);

    if (recv_len == 0) {
    	return -1;
    }

    if (eth_header->type != 0x55AB || eth_header->reply_type != REPLY_WORLD) {
    	return -1;
    }

	*r = (void *) recv_buffer;
	return 1;
}

int receive_solution_reply(solution_reply_t **r) {
	volatile int recv_len = 0;
    recv_len = XEmacLite_Recv(&ether, recv_buffer);

    if (recv_len == 0) {
    	return -1;
    }

    if (eth_header->type != 0x55AB || eth_header->reply_type != SOLUTION_REPLY) {
    	return -1;
    }

    *r = (void *) recv_buffer;
	return 1;
}

void send_frame() {
	int i = 0;
	memset(tmit_buffer, '!', sizeof(tmit_buffer));
	u8 *buffer = tmit_buffer;

    for(i = 0; i < 6; i++)
        *buffer++ = target_address[i];

    for(i = 0; i < 6; i++)
        *buffer++ = my_address[i];

    for(i = 0; i < 2; i++)
        *buffer++ = type[i];

    switch (message.type) {
    case REQUEST_WORLD: // request_world
    	*buffer++ = message.rqw.type;
    	*buffer++ = message.rqw.size;

    	memcpy(buffer, &message.rqw.world_id, 4);

    	break;
    case SOLVE_WORLD: // solve_world:
    	*buffer++ = message.sw.type;
    	*buffer++ = message.sw.size;

    	for (i = 0; i < 4; i++) {
    		*buffer++ = message.sw.world_id[i];
    	}

    	*buffer++ = message.sw.ignore_walls;
    	memcpy(buffer, &message.sw.cost, sizeof(u32));

    	break;
    }

    //Send the buffer
    //The size argument is the data bytes + XEL_HEADER_SIZE which is defined
    //as the size of the destination MAC plus the type/length field
    XEmacLite_FlushReceive(&ether); // clears incoming messages buffer
    XEmacLite_Send(&ether, tmit_buffer, message.length + XEL_HEADER_SIZE);
}
