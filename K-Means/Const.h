#pragma once

#define MASTER 0
#define INPUT_FILE_SIZE 50
#define OUTPUT_FILE "output.txt"

typedef struct
{
	float x;
	float y;
	float z;
	float vx;
	float vy;
	float vz;
	int cluster;
} point_t;