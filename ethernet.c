#include "ethernet.h"

#define TMIT_HEADER_SIZE 14

// struct to store the message that will be sent to the server
struct message_t {
	int type; // 0 - rw, 1 - sw
	int length; // length of the message
	union { // stores either a request world or solve world
		request_world_t rqw;
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

/**
 * Initialise all ethernet related variables and configurations.
 */
void init_ether() {
    XEmacLite_Config *etherconfig = XEmacLite_LookupConfig(XPAR_EMACLITE_0_DEVICE_ID);
    XEmacLite_CfgInitialize(&ether, etherconfig, etherconfig->BaseAddress);

    XEmacLite_SetMacAddress(&ether, my_address); // set sending mac address
    XEmacLite_FlushReceive(&ether);
}

/**
 * Helper to request a world.
 * Size is 0/1/2 for small/medium/large
 * World is 0 - 100 000.
 */
void request_world(int size, int world_id) {
	xil_printf("Requesting world\r\n");

	// Construct a request_world_t with appropriate parameters
	request_world_t req;

	req.type = REQUEST_WORLD;
	req.size = size;
	req.world_id = world_id;

	// need to set the type twice because we have a switch that selects the correct element from the union
	message.type = REQUEST_WORLD;
	message.rqw = req;
	message.length = sizeof(request_world_t);

	send_frame();
}

/**
 * Helper to send a solution
 * cost: total path cost
 * world_id: an array of 4 bytes
 * size: world size 0/1/2
 */
void send_solution(int cost, u8 *world_id, int size) {
	solve_world_t req;

	req.type = SOLVE_WORLD;
	req.size = size;
	int i;
	for (i = 0; i < 4; i++)
		req.world_id[i] = world_id[i];

	req.ignore_walls = 0;
	req.cost = cost;

	message.type = SOLVE_WORLD;
	message.sw = req;
	message.length = sizeof(solve_world_t);

	send_frame();
}

/**
 * Helper to receive a frame of type REPLY_WORLD from the server.
 * Check that the type is correct (0x55AB) and the actual frame is of REPLY_WORLD.
 *
 * Returns 1 if correct frame received and sets r to contain the message
 * Returns -1 otherwise
 */
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

/**
 * Helper to receive a frame of type SOLUTION_REPLY from the server.
 * Checks that ethernet type is 0x55AB and actualy type is of SOLUTION_REPLY.
 *
 * Returns 1 if correct frame received and sets r to contain the message
 * Returns -1 otherwise
 */
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

/**
 * Helper function to actually construct the ethernet frame and sent it out.
 */
void send_frame() {
	int i = 0;
	memset(tmit_buffer, 0, sizeof(tmit_buffer));
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
