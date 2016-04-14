/*
 * definitions.h
 *
 *  Created on: 12 Apr 2016
 *      Author: pi514
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include "xil_types.h"

typedef struct __attribute__((packed)) {
	u8 x;
	u8 y;
} waypoint_t;

typedef struct __attribute__((packed)) {
	u8 x;
	u8 y;
	u8 direction; // 0 horizontal; 1 vertical
	u8 length;
} wall_t;

typedef struct __attribute__((packed)) {
	u8 size;
	u8 waypoints_size;
	waypoint_t waypoints[12];
	u8 walls_size;
	wall_t walls[20];
} world_t;

#endif /* DEFINITIONS_H_ */
