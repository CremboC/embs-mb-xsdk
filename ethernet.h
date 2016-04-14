/*
 * ethernet.h
 *
 *  Created on: 11 Apr 2016
 *      Author: pi514
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_

#define REQUEST_WORLD 1
#define REPLY_WORLD 2
#define SOLVE_WORLD 3
#define SOLUTION_REPLY 4

#include "definitions.h"
#include "xemaclite.h"

// a wrapper for the ethernet packet header
typedef struct __attribute__((packed)) {
	u8 destination[6];
	u8 source[6];
	u16 type;
	u8 reply_type;
} eth_header_t;

typedef struct {
	u8 type;
	u8 size; // 0 = small, 1 = medium, 2 = large
	u32 world_id; // 0 - 100000
} request_world_t;

typedef struct __attribute__((packed)) {
	eth_header_t eth_header;
	u32 world_id;
	u8 width;
	u8 height;
	u8 waypoints_size;
	// waypoints
	// number of wallschar *
	// walls
} reply_world_t;

typedef struct {

} solve_world_t;

typedef struct __attribute__((packed)) {

} solution_reply_t;

void init_ether();
void request_world();
int receive_world(reply_world_t **r);
void print_message();

#endif /* ETHERNET_H_ */
